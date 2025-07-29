/*
 * Copyright (C) 2015-2023 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU Lesser General Public License v2.1 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

#include "includes.h"
#include <pthread.h>
#include <signal.h>


typedef struct _PROGRESS_STATE
{
    time_t cur_time;
    time_t prev_time;
    curl_off_t dlTotal;
    curl_off_t dlNow;
    char pszData[64];
    int active;
} PROGRESS_STATE, *PPROGRESS_STATE;

static PPROGRESS_STATE g_pProgressStates = NULL;
static int g_nProgressStates = 0;
static pthread_mutex_t g_progress_mutex = PTHREAD_MUTEX_INITIALIZER;
static __thread int g_tls_progress_index = -1;
static struct sigaction g_orig_sigint;

static void
progress_sigint_handler(int sig)
{
    UNUSED(sig);
    pthread_mutex_lock(&g_progress_mutex);
    if (g_pProgressStates)
    {
        for (int i = 0; i < g_nProgressStates + 1; i++)
            printf("\n");
        fflush(stdout);
    }
    pthread_mutex_unlock(&g_progress_mutex);
    sigaction(SIGINT, &g_orig_sigint, NULL);
    raise(SIGINT);
}

static void
_redraw_progress_locked()
{
    if(!g_pProgressStates)
        return;

    int active_count = 0;

    /* Move cursor to the beginning of the progress block so the cursor
     * stays below the list of packages being downloaded, similar to dnf
     */
    printf("\033[%dA", g_nProgressStates);

    for(int i = 0; i < g_nProgressStates; i++)
    {
        PROGRESS_STATE *st = &g_pProgressStates[i];
        printf("\033[2K");
        if(st->active)
        {
            active_count++;
            int percent = 0;
            if(st->dlTotal > 0)
            {
                percent = (int)(((double)st->dlNow / (double)st->dlTotal) * 100.0);
            }
            int bar = 20;
            int filled = (st->dlTotal > 0) ? (int)((double)st->dlNow * bar / st->dlTotal) : 0;
            printf("%-20s [" COLOR_GREEN, st->pszData);
            for(int j = 0; j < filled; j++)
                printf("#");
            for(int j = filled; j < bar; j++)
                printf(" ");
            printf(COLOR_RESET "] %3d%%\n", percent);
        }
        else
        {
            printf("\n");
        }
    }

    if(active_count > 0)
    {
        printf("\033[%dA", g_nProgressStates);
    }
    fflush(stdout);
}

static int
progress_cb(
    void *pUserData,
    curl_off_t dlTotal,
    curl_off_t dlNow,
    curl_off_t ulTotal,
    curl_off_t ulNow
    )
{
    PROGRESS_STATE *pState = (PROGRESS_STATE *)pUserData;

    UNUSED(ulNow);
    UNUSED(ulTotal);

    if (dlTotal <= 0)
    {
        return 0;
    }

    if (dlNow < dlTotal)
    {
        time(&pState->cur_time);
        if (pState->prev_time &&
            difftime(pState->cur_time, pState->prev_time) < 1.0)
        {
            return 0;
        }
        pState->prev_time = pState->cur_time;
    }
    else
    {
        pState->prev_time = 0;
    }

    pState->dlTotal = dlTotal;
    pState->dlNow = dlNow;

    pthread_mutex_lock(&g_progress_mutex);
    _redraw_progress_locked();
    pthread_mutex_unlock(&g_progress_mutex);

    return 0;
}

uint32_t
set_progress_cb(
    CURL *pCurl,
    const char *pszData
    )
{
    uint32_t dwError = 0;
    PROGRESS_STATE *pState = NULL;

    if(!pCurl || IsNullOrEmptyString(pszData))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = curl_easy_setopt(pCurl, CURLOPT_XFERINFOFUNCTION, progress_cb);
    BAIL_ON_TDNF_CURL_ERROR(dwError);

    if(g_tls_progress_index < 0 || g_tls_progress_index >= g_nProgressStates)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pState = &g_pProgressStates[g_tls_progress_index];
    memset(pState, 0, sizeof(PROGRESS_STATE));
    strncpy(pState->pszData, pszData, sizeof(pState->pszData) - 1);
    pState->active = 1;
    /* coverity[bad_sizeof] */
    dwError = curl_easy_setopt(pCurl, CURLOPT_XFERINFODATA, pState);
    BAIL_ON_TDNF_CURL_ERROR(dwError);

    dwError = curl_easy_setopt(pCurl, CURLOPT_NOPROGRESS, 0L);
    BAIL_ON_TDNF_CURL_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}


uint32_t
TDNFDownloadFileFromRepo(
    PTDNF pTdnf,
    PTDNF_REPO_DATA pRepo,
    const char *pszLocation,
    const char *pszFile,
    const char *pszProgressData
)
{
    uint32_t dwError = 0;
    char *pszUrl = NULL;

    if(!pTdnf ||
       !pTdnf->pArgs || !pRepo ||
       IsNullOrEmptyString(pszLocation) ||
       IsNullOrEmptyString(pszFile))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if (pRepo->ppszBaseUrls && pRepo->ppszBaseUrls[0]) {
        /* Try one base URL after the other until we succeed */
        /* Note: this can be improved:
         * 1) we could start with the last good URL next time instead of
         *    starting with 0 each time
         * 2) we could store a list of known bad/good URLs
         */
        for (int i = 0; pRepo->ppszBaseUrls[i]; i++) {
            dwError = TDNFJoinPath(&pszUrl, pRepo->ppszBaseUrls[i], pszLocation, NULL);
            BAIL_ON_TDNF_ERROR(dwError);

            dwError = TDNFDownloadFile(pTdnf, pRepo, pszUrl, pszFile, pszProgressData);
            if (dwError == 0) {
                break;
            }
            TDNF_SAFE_FREE_MEMORY(pszUrl);
        }
    } else {
        /* If there is no base url, pszLocation should contain the whole URL.
           This is the case for packages from the command line. */
        dwError = TDNFDownloadFile(pTdnf, pRepo, pszLocation, pszFile, pszProgressData);
    }
    BAIL_ON_TDNF_ERROR(dwError);

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszUrl);
    return dwError;
error:
    goto cleanup;
}

uint32_t
TDNFDownloadFile(
    PTDNF pTdnf,
    PTDNF_REPO_DATA pRepo,
    const char *pszFileUrl,
    const char *pszFile,
    const char *pszProgressData
    )
{
    uint32_t dwError = 0;
    CURL *pCurl = NULL;
    FILE *fp = NULL;
    char *pszUserPass = NULL;
    char *pszFileTmp = NULL;
    /* lStatus reads CURLINFO_RESPONSE_CODE. Must be long */
    long lStatus = 0;
    int i;
    int nNoOutput = 1;

    /* TDNFFetchRemoteGPGKey sends pszProgressData as NULL */
    if(!pTdnf ||
       !pRepo ||
       IsNullOrEmptyString(pszFileUrl) ||
       IsNullOrEmptyString(pszFile))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pCurl = curl_easy_init();
    if(!pCurl)
    {
        dwError = ERROR_TDNF_CURL_INIT;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFRepoGetUserPass(pTdnf, pRepo, &pszUserPass);
    BAIL_ON_TDNF_ERROR(dwError);

    if(!IsNullOrEmptyString(pszUserPass))
    {
        dwError = curl_easy_setopt(
                      pCurl,
                      CURLOPT_USERPWD,
                      pszUserPass);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFRepoApplyProxySettings(pTdnf->pConf, pCurl);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFRepoApplyDownloadSettings(pRepo, pCurl);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFRepoApplySSLSettings(pRepo, pCurl);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = curl_easy_setopt(pCurl, CURLOPT_URL, pszFileUrl);
    BAIL_ON_TDNF_CURL_ERROR(dwError);

    dwError = curl_easy_setopt(pCurl, CURLOPT_FOLLOWLOCATION, 1L);
    BAIL_ON_TDNF_CURL_ERROR(dwError);

    if (!pTdnf->pArgs->nQuiet && pszProgressData != NULL)
    {
        //print progress only if tty or verbose is specified.
        if (isatty(STDOUT_FILENO) || pTdnf->pArgs->nVerbose)
        {
            dwError = set_progress_cb(pCurl, pszProgressData);
            BAIL_ON_TDNF_ERROR(dwError);
            nNoOutput = 0;
        }
    }

    dwError = TDNFAllocateStringPrintf(&pszFileTmp,
                                       "%s.tmp",
                                       pszFile);
    BAIL_ON_TDNF_ERROR(dwError);

    for(i = 0; i <= pRepo->nRetries; i++)
    {
        fp = fopen(pszFileTmp, "wb");
        if(!fp)
        {
            dwError = errno;
            BAIL_ON_TDNF_SYSTEM_ERROR_UNCOND(dwError);
        }

        dwError = curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, fp);
        BAIL_ON_TDNF_CURL_ERROR(dwError);

        if (i > 0)
        {
            pr_info("retrying %d/%d\n", i, pRepo->nRetries);
        }
        dwError = curl_easy_perform(pCurl);
        if (dwError == CURLE_OK)
        {
            fclose(fp);
            fp = NULL;
            break;
        }
        if (i == pRepo->nRetries || TDNFCurlErrorIsFatal(dwError))
        {
            BAIL_ON_TDNF_CURL_ERROR(dwError);
        }
        fclose(fp);
        fp = NULL;
    }

    if (!nNoOutput) {
        pthread_mutex_lock(&g_progress_mutex);
        if(g_tls_progress_index >=0 && g_tls_progress_index < g_nProgressStates)
        {
            g_pProgressStates[g_tls_progress_index].active = 0;
            _redraw_progress_locked();
            pr_info("%s completed\n", pszProgressData);
        }
        pthread_mutex_unlock(&g_progress_mutex);
    }

    dwError = curl_easy_getinfo(pCurl,
                                CURLINFO_RESPONSE_CODE,
                                &lStatus);
    BAIL_ON_TDNF_CURL_ERROR(dwError);

    if(lStatus >= 400)
    {
        pr_err(
                "Error: %ld when downloading %s\n. Please check repo url "
                "or refresh metadata with 'tdnf makecache'.\n",
                lStatus,
                pszFileUrl);
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }
    else
    {
        if (rename(pszFileTmp, pszFile) == -1)
        {
            dwError = errno;
            BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
        }
        if (chmod(pszFile, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH) == -1)
        {
            dwError = errno;
            BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
        }
    }

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszUserPass);
    TDNF_SAFE_FREE_MEMORY(pszFileTmp);
    if(fp)
    {
        /* coverity[dead_error_line] */
        fclose(fp);
    }
    if(pCurl)
    {
        curl_easy_cleanup(pCurl);
    }
    return dwError;

error:
    if(fp)
    {
        fclose(fp);
        fp = NULL;
    }
    if(!IsNullOrEmptyString(pszFileTmp))
    {
        unlink(pszFileTmp);
    }

    goto cleanup;
}

uint32_t
TDNFCreatePackageUrl(
    PTDNF pTdnf,
    const char* pszRepoId,
    const char* pszPackageLocation,
    char **ppszPackageUrl
    )
{
    uint32_t dwError = 0;
    char *pszPackageUrl = NULL;
    PTDNF_REPO_DATA pRepo = NULL;

    if(!pTdnf ||
       !pTdnf->pArgs ||
       IsNullOrEmptyString(pszPackageLocation) ||
       IsNullOrEmptyString(pszRepoId) ||
       !ppszPackageUrl)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFFindRepoById(pTdnf, pszRepoId, &pRepo);
    BAIL_ON_TDNF_ERROR(dwError);

    if (pRepo->ppszBaseUrls && pRepo->ppszBaseUrls[0]) {
        dwError = TDNFJoinPath(&pszPackageUrl, pRepo->ppszBaseUrls[0], pszPackageLocation, NULL);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    else
    {
        dwError = TDNFAllocateString(pszPackageLocation, &pszPackageUrl);
        BAIL_ON_TDNF_ERROR(dwError);
    }
    *ppszPackageUrl = pszPackageUrl;

cleanup:
    return dwError;
error:
    TDNF_SAFE_FREE_MEMORY(pszPackageUrl);
    goto cleanup;
}

uint32_t
TDNFDownloadPackage(
    PTDNF pTdnf,
    const char* pszPackageLocation,
    const char* pszPkgName,
    PTDNF_REPO_DATA pRepo,
    const char* pszRpmCacheDir
    )
{
    uint32_t dwError = 0;
    char *pszPackageFile = NULL;
    char *pszCopyOfPackageLocation = NULL;
    int nSize;

    if(!pTdnf ||
       !pTdnf->pArgs ||
       IsNullOrEmptyString(pszPackageLocation) ||
       IsNullOrEmptyString(pszPkgName) ||
       !pRepo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFAllocateString(pszPackageLocation,
                                 &pszCopyOfPackageLocation);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFJoinPath(&pszPackageFile,
                           pszRpmCacheDir,
                           basename(pszCopyOfPackageLocation),
                           NULL);
    BAIL_ON_TDNF_ERROR(dwError);

    /* don't download if file is already there. Older versions may have left
       size 0 files, so check for those too */
    dwError = TDNFGetFileSize(pszPackageFile, &nSize);
    if ((dwError == ERROR_TDNF_FILE_NOT_FOUND) || (nSize == 0))
    {
        dwError = TDNFDownloadFileFromRepo(pTdnf,
                                   pRepo,
                                   pszPackageLocation,
                                   pszPackageFile,
                                   pszPkgName);
    }
    else if(dwError == 0)
    {
        pr_info("%s package already downloaded\n", pszPkgName);
    }
    BAIL_ON_TDNF_ERROR(dwError);

cleanup:
    TDNF_SAFE_FREE_MEMORY(pszCopyOfPackageLocation);
    TDNF_SAFE_FREE_MEMORY(pszPackageFile);
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFDownloadPackageToCache(
    PTDNF pTdnf,
    const char* pszPackageLocation,
    const char* pszPkgName,
    PTDNF_REPO_DATA pRepo,
    char** ppszFilePath
    )
{
    uint32_t dwError = 0;
    char* pszRpmCacheDir = NULL;
    char* pszNormalRpmCacheDir = NULL;

    if(!pTdnf ||
       IsNullOrEmptyString(pszPackageLocation) ||
       IsNullOrEmptyString(pszPkgName) ||
       !pRepo ||
       !ppszFilePath)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFJoinPath(&pszRpmCacheDir,
                           pTdnf->pConf->pszCacheDir,
                           pRepo->pszId,
                           "rpms",
                           NULL);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFNormalizePath(pszRpmCacheDir,
                                &pszNormalRpmCacheDir);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFDownloadPackageToTree(pTdnf,
                                        pszPackageLocation,
                                        pszPkgName,
                                        pRepo,
                                        pszNormalRpmCacheDir,
                                        ppszFilePath);
    BAIL_ON_TDNF_ERROR(dwError);
cleanup:
    TDNF_SAFE_FREE_MEMORY(pszNormalRpmCacheDir);
    TDNF_SAFE_FREE_MEMORY(pszRpmCacheDir);
    return dwError;
error:
    goto cleanup;
}

/*
 * TDNFDownloadPackageToTree()
 *
 * Download a package while preserving the directory path. For example,
 * if pszPackageLocation is "RPMS/x86_64/foo-1.2-3.rpm", the destination will
 * be downloaded under the destination directory in RPMS/x86_64/foo-1.2-3.rpm
 * (so 'RPMS/x86_64/' will be preserved).
*/

uint32_t
TDNFDownloadPackageToTree(
    PTDNF pTdnf,
    const char* pszPackageLocation,
    const char* pszPkgName,
    PTDNF_REPO_DATA pRepo,
    char* pszNormalRpmCacheDir,
    char** ppszFilePath
    )
{
    uint32_t dwError = 0;
    char* pszFilePath = NULL;
    char* pszNormalPath = NULL;
    char* pszDownloadCacheDir = NULL;
    char* pszRemotePath = NULL;

    if(!pTdnf ||
       IsNullOrEmptyString(pszPackageLocation) ||
       IsNullOrEmptyString(pszPkgName) ||
       !pRepo ||
       IsNullOrEmptyString(pszNormalRpmCacheDir) ||
       !ppszFilePath)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFPathFromUri(pszPackageLocation, &pszRemotePath);
    if (dwError == ERROR_TDNF_URL_INVALID)
    {
        dwError = TDNFAllocateString(pszPackageLocation, &pszRemotePath);
    }
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFJoinPath(&pszFilePath, pszNormalRpmCacheDir, pszRemotePath, NULL);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFNormalizePath(
                  pszFilePath,
                  &pszNormalPath);
    BAIL_ON_TDNF_ERROR(dwError);

    if (strncmp(pszNormalRpmCacheDir, pszNormalPath,
                strlen(pszNormalRpmCacheDir)))
    {
        dwError = ERROR_TDNF_URL_INVALID;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFDirName(pszNormalPath, &pszDownloadCacheDir);
    BAIL_ON_TDNF_ERROR(dwError);

    if(access(pszDownloadCacheDir, F_OK))
    {
        if(errno != ENOENT)
        {
            dwError = errno;
        }
        BAIL_ON_TDNF_SYSTEM_ERROR(dwError);

        dwError = TDNFUtilsMakeDirs(pszDownloadCacheDir);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    if(access(pszNormalPath, F_OK))
    {
        if(errno != ENOENT)
        {
            dwError = errno;
            BAIL_ON_TDNF_SYSTEM_ERROR(dwError);
        }
        dwError = TDNFDownloadPackage(pTdnf, pszPackageLocation, pszPkgName,
            pRepo, pszDownloadCacheDir);
        BAIL_ON_TDNF_ERROR(dwError);
    }

    *ppszFilePath = pszNormalPath;
cleanup:
    TDNF_SAFE_FREE_MEMORY(pszFilePath);
    TDNF_SAFE_FREE_MEMORY(pszDownloadCacheDir);
    TDNF_SAFE_FREE_MEMORY(pszRemotePath);
    return dwError;

error:
    TDNF_SAFE_FREE_MEMORY(pszNormalPath);
    goto cleanup;

}

/*
 * TDNFDownloadPackageToDirectory()
 *
 * Download a package withou preserving the directory path. For example,
 * if pszPackageLocation is "RPMS/x86_64/foo-1.2-3.rpm", the destination will
 * be downloaded under the destination directory (pszDirectory) as foo-1.2-3.rpm
 * (so RPMS/x86_64/ will be stripped).
*/

uint32_t
TDNFDownloadPackageToDirectory(
    PTDNF pTdnf,
    const char* pszPackageLocation,
    const char* pszPkgName,
    PTDNF_REPO_DATA pRepo,
    const char* pszDirectory,
    char** ppszFilePath
    )
{
    uint32_t dwError = 0;
    char* pszFilePath = NULL;
    char* pszRemotePath = NULL;
    char* pszFileName = NULL;

    if(!pTdnf ||
       IsNullOrEmptyString(pszPackageLocation) ||
       IsNullOrEmptyString(pszPkgName) ||
       !pRepo ||
       IsNullOrEmptyString(pszDirectory) ||
       !ppszFilePath)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    dwError = TDNFPathFromUri(pszPackageLocation, &pszRemotePath);
    if (dwError == ERROR_TDNF_URL_INVALID)
    {
        dwError = TDNFAllocateString(pszPackageLocation, &pszRemotePath);
    }
    BAIL_ON_TDNF_ERROR(dwError);

    pszFileName = basename(pszRemotePath);

    dwError = TDNFJoinPath(&pszFilePath, pszDirectory, pszFileName, NULL);
    BAIL_ON_TDNF_ERROR(dwError);

    dwError = TDNFDownloadPackage(pTdnf, pszPackageLocation, pszPkgName,
                                  pRepo, pszDirectory);
    BAIL_ON_TDNF_ERROR(dwError);

    *ppszFilePath = pszFilePath;
cleanup:
    TDNF_SAFE_FREE_MEMORY(pszRemotePath);
    return dwError;

error:
    TDNF_SAFE_FREE_MEMORY(pszFilePath);
    goto cleanup;
}


typedef struct _TDNF_DOWNLOAD_TASK
{
    PTDNF pTdnf;
    PTDNF_PKG_INFO pInfo;
    PTDNF_REPO_DATA pRepo;
    int progress_index;
    uint32_t dwError;
} TDNF_DOWNLOAD_TASK, *PTDNF_DOWNLOAD_TASK;

static void*
_download_task_fn(void *data)
{
    PTDNF_DOWNLOAD_TASK task = (PTDNF_DOWNLOAD_TASK)data;
    g_tls_progress_index = task->progress_index;
    char *pszPath = NULL;
    if (!task->pTdnf->pArgs->nDownloadOnly ||
        task->pTdnf->pArgs->pszDownloadDir == NULL)
    {
        task->dwError = TDNFDownloadPackageToCache(task->pTdnf,
                                                   task->pInfo->pszLocation,
                                                   task->pInfo->pszName,
                                                   task->pRepo,
                                                   &pszPath);
    }
    else
    {
        task->dwError = TDNFDownloadPackageToDirectory(task->pTdnf,
                                                       task->pInfo->pszLocation,
                                                       task->pInfo->pszName,
                                                       task->pRepo,
                                                       task->pTdnf->pArgs->pszDownloadDir,
                                                       &pszPath);
    }
    TDNF_SAFE_FREE_MEMORY(pszPath);
    g_tls_progress_index = -1;
    return NULL;
}

uint32_t
TDNFPreDownloadPackages(
    PTDNF pTdnf,
    PTDNF_PKG_INFO pInfos
    )
{
    uint32_t dwError = 0;
    int count = 0, idx = 0, running = 0;
    PTDNF_PKG_INFO p = NULL;
    pthread_t *threads = NULL;
    PTDNF_DOWNLOAD_TASK tasks = NULL;

    g_nProgressStates = 0;
    g_pProgressStates = NULL;

    for (p = pInfos; p; p = p->pNext)
        count++;

    if (count == 0)
        return 0;

    threads = calloc(count, sizeof(pthread_t));
    tasks = calloc(count, sizeof(TDNF_DOWNLOAD_TASK));
    g_pProgressStates = calloc(count, sizeof(PROGRESS_STATE));
    g_nProgressStates = count;
    if (!threads || !tasks || !g_pProgressStates)
    {
        dwError = ERROR_TDNF_INVALID_ALLOCSIZE;
        goto cleanup;
    }

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = progress_sigint_handler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, &g_orig_sigint);

    
    if (isatty(STDOUT_FILENO) && !pTdnf->pArgs->nQuiet)
    {
        for(int i = 0; i < count; i++)
            printf("\n");
    }

    int nParallel = pTdnf->pConf->nParallelDownloads;
    if (nParallel < 1)
        nParallel = 1;

    for (p = pInfos, idx = 0; p; p = p->pNext, idx++)
    {
        tasks[idx].pTdnf = pTdnf;
        tasks[idx].pInfo = p;
        dwError = TDNFFindRepoById(pTdnf, p->pszRepoName, &tasks[idx].pRepo);
        BAIL_ON_TDNF_ERROR(dwError);
        tasks[idx].progress_index = idx;


        pthread_create(&threads[idx], NULL, _download_task_fn, &tasks[idx]);
        running++;
        if (running >= nParallel)
        {
            pthread_join(threads[idx - nParallel + 1], NULL);
            running--;
        }
    }

    for (int j = idx - running; j < idx; j++)
    {
        pthread_join(threads[j], NULL);
    }

    for (int i = 0; i < idx; i++)
    {
        if (tasks[i].dwError)
        {
            dwError = tasks[i].dwError;
            break;
        }
    }

cleanup:
    free(threads);
    free(tasks);
    free(g_pProgressStates);
    g_pProgressStates = NULL;
    g_nProgressStates = 0;
    sigaction(SIGINT, &g_orig_sigint, NULL);
    return dwError;
error:
    goto cleanup;
}
