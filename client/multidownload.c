#include "includes.h"

static int
progress_cb(
    void *pUserData,
    curl_off_t dlTotal,
    curl_off_t dlNow,
    curl_off_t ulTotal,
    curl_off_t ulNow
    )
{
    uint32_t dPercent;
    double speed = 0.0;
    double cur_speed = 0.0;
    double eta = 0.0;
    pcb_data *pData = (pcb_data *)pUserData;

    UNUSED(ulNow);
    UNUSED(ulTotal);

    if (dlTotal <= 0)
    {
        return 0;
    }

    if (dlNow < dlTotal)
    {
        time(&pData->cur_time);
        if (pData->prev_time &&
            difftime(pData->cur_time, pData->prev_time) < 1.0)
        {
            return 0;
        }
        pData->prev_time = pData->cur_time;
        dPercent = (uint32_t)(((double)dlNow / (double)dlTotal) * 100.0);

        if (pData->last_time &&
            difftime(pData->cur_time, pData->last_time) > 0.0)
        {
            cur_speed = (double)(dlNow - pData->last_bytes) /
                        difftime(pData->cur_time, pData->last_time);
            if (pData->ema_speed == 0.0)
            {
                pData->ema_speed = cur_speed;
            }
            else
            {
                pData->ema_speed = 0.7 * pData->ema_speed + 0.3 * cur_speed;
            }
            speed = pData->ema_speed;
            if (speed > 0.0)
            {
                eta = (double)(dlTotal - dlNow) / speed;
            }
        }

        pData->last_time = pData->cur_time;
        pData->last_bytes = dlNow;
    }
    else
    {
        pData->prev_time = 0;
        dPercent = 100;
    }

    if (!isatty(STDOUT_FILENO))
    {
        pr_info("%s %u%% %ld %ld %ld\n",
                pData->pszData,
                dPercent,
                dlNow,
                (long)speed,
                (long)eta);
    }
    else
    {
        const int barw = 50;
        char bar[barw + 1];
        int filled = (dPercent * barw) / 100;
        memset(bar, '#', filled);
        memset(bar + filled, ' ', barw - filled);
        bar[barw] = '\0';

        if (GlobalGetColor())
        {
            pr_info("%-20s " TDNF_COLOR_GREEN "[%s]" TDNF_COLOR_RESET " %3u%% %ld %ld\r",
                    pData->pszData,
                    bar,
                    dPercent,
                    (long)speed,
                    (long)eta);
        }
        else
        {
            pr_info("%-20s [%s] %3u%% %ld %ld\r",
                    pData->pszData,
                    bar,
                    dPercent,
                    (long)speed,
                    (long)eta);
        }
    }

    fflush(stdout);

    return 0;
}

static uint32_t
md_set_progress_cb(
    CURL *pCurl,
    const char *pszData
    )
{
    uint32_t dwError = 0;
    pcb_data *pData = NULL;

    if(!pCurl || IsNullOrEmptyString(pszData))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    pData = calloc(1, sizeof(pcb_data));
    if(!pData)
    {
        dwError = ENOMEM;
        BAIL_ON_TDNF_SYSTEM_ERROR_UNCOND(dwError);
    }

    dwError = curl_easy_setopt(pCurl, CURLOPT_XFERINFOFUNCTION, progress_cb);
    BAIL_ON_TDNF_CURL_ERROR(dwError);

    strncpy(pData->pszData, pszData, sizeof(pData->pszData) - 1);
    dwError = curl_easy_setopt(pCurl, CURLOPT_XFERINFODATA, pData);
    BAIL_ON_TDNF_CURL_ERROR(dwError);

    dwError = curl_easy_setopt(pCurl, CURLOPT_NOPROGRESS, 0L);
    BAIL_ON_TDNF_CURL_ERROR(dwError);

cleanup:
    return dwError;

error:
    if(pData) free(pData);
    goto cleanup;
}

typedef struct _DL_HANDLE {
    CURL *pCurl;
    FILE *fp;
    char *pszTmp;
    char *pszDest;
    char **ppszFilePath;
    pcb_data cb;
    struct _DL_HANDLE *pNext;
} DL_HANDLE;

static CURLM *g_pMulti = NULL;
static DL_HANDLE *g_pHandles = NULL;
static long g_nMax = 1;

uint32_t
TDNFMultiBegin(long nMax)
{
    g_pMulti = curl_multi_init();
    if(!g_pMulti)
        return ERROR_TDNF_CURL_INIT;
    g_nMax = nMax > 0 ? nMax : 1;
    curl_multi_setopt(g_pMulti, CURLMOPT_MAX_TOTAL_CONNECTIONS, g_nMax);
#ifdef CURLMOPT_MAX_HOST_CONNECTIONS
    curl_multi_setopt(g_pMulti, CURLMOPT_MAX_HOST_CONNECTIONS, g_nMax);
#endif
#ifdef CURLPIPE_MULTIPLEX
    curl_multi_setopt(g_pMulti, CURLMOPT_PIPELINING, CURLPIPE_MULTIPLEX);
#endif
    return 0;
}

static void
free_handle(DL_HANDLE *h)
{
    if(!h) return;
    if(h->fp) fclose(h->fp);
    if(h->pCurl) curl_easy_cleanup(h->pCurl);
    if(h->pszTmp) free(h->pszTmp);
    if(h->pszDest) free(h->pszDest);
    free(h);
}

uint32_t
TDNFMultiAdd(CURL *pCurl, FILE *fp, const char *pszTmp, const char *pszDest,
             char **ppszFilePath, const char *pszProgress)
{
    uint32_t dwError = 0;
    DL_HANDLE *h = calloc(1, sizeof(*h));
    if(!h) return ENOMEM;
    h->pCurl = pCurl;
    h->fp = fp;
    h->pszTmp = strdup(pszTmp);
    h->pszDest = strdup(pszDest);
    h->ppszFilePath = ppszFilePath;
    if(ppszFilePath) *ppszFilePath = strdup(pszDest);

    memset(&h->cb,0,sizeof(h->cb));
    if(pszProgress)
        strncpy(h->cb.pszData, pszProgress, sizeof(h->cb.pszData)-1);

    curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, fp);
    md_set_progress_cb(pCurl, pszProgress ? pszProgress : "");
    curl_easy_setopt(pCurl, CURLOPT_XFERINFODATA, &h->cb);
    curl_easy_setopt(pCurl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(pCurl, CURLOPT_PRIVATE, h);

    dwError = curl_multi_add_handle(g_pMulti, pCurl);
    if(dwError)
    {
        free_handle(h);
        return dwError;
    }

    h->pNext = g_pHandles;
    g_pHandles = h;
    return 0;
}

uint32_t
TDNFMultiPerform(void)
{
    int running = 0;
    CURLMsg *msg = NULL;
    int msgs = 0;
    curl_multi_perform(g_pMulti, &running);
    while(running)
    {
        curl_multi_wait(g_pMulti, NULL, 0, 1000, NULL);
        curl_multi_perform(g_pMulti, &running);
        while((msg = curl_multi_info_read(g_pMulti, &msgs)))
        {
            if(msg->msg == CURLMSG_DONE)
            {
                DL_HANDLE *h = NULL;
                curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &h);
                if(h)
                {
                    if(msg->data.result == CURLE_OK)
                        rename(h->pszTmp, h->pszDest);
                    curl_multi_remove_handle(g_pMulti, msg->easy_handle);
                    free_handle(h);
                }
            }
        }
    }
    /* clean remaining */
    while((msg = curl_multi_info_read(g_pMulti, &msgs)))
    {
        if(msg->msg == CURLMSG_DONE)
        {
            DL_HANDLE *h = NULL;
            curl_easy_getinfo(msg->easy_handle, CURLINFO_PRIVATE, &h);
            if(h)
            {
                if(msg->data.result == CURLE_OK)
                    rename(h->pszTmp, h->pszDest);
                curl_multi_remove_handle(g_pMulti, msg->easy_handle);
                free_handle(h);
            }
        }
    }
    curl_multi_cleanup(g_pMulti);
    g_pMulti = NULL;
    return 0;
}
