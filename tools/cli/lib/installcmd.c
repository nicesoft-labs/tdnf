/*
 * Copyright (C) 2015-2023 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU General Public License v2 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

/*
 * Module   : installcmd.c
 *
 * Abstract :
 *
 *            tdnf
 *
 *            command line tool
 *
 * Authors  : Priyesh Padmavilasom (ppadmavilasom@vmware.com)
 *
 */

#include "includes.h"

static int
tdnf_display_width(const char *psz)
{
    int width = 0;
    mbstate_t state;
    memset(&state, 0, sizeof(state));
    const char *p = psz;
    wchar_t wc;

    while (p && *p)
    {
        if (*p == '\033')
        {
            p++;
            while (*p && *p != 'm')
            {
                p++;
            }
            if (*p == 'm')
            {
                p++;
            }
            continue;
        }

        size_t n = mbrtowc(&wc, p, MB_CUR_MAX, &state);
        if (n == (size_t)-1 || n == (size_t)-2)
        {
            memset(&state, 0, sizeof(state));
            p++;
            width++;
            continue;
        }
        p += n;
        int w = wcwidth(wc);
        if (w < 0)
        {
            w = 0;
        }
        width += w;
    }

    return width;
}

static void
tdnf_print_padded(const char *psz, int nWidth)
{
    int w = tdnf_display_width(psz);
    pr_info("%s", psz);
    for (int i = w; i < nWidth; ++i)
    {
        pr_info(" ");
    }
}

static void
tdnf_print_border(const int *pnColWidths, int nCols)
{
    int i = 0, j = 0;
    pr_info("+");
    for (i = 0; i < nCols; ++i)
    {
        for (j = 0; j < pnColWidths[i] + 2; ++j)
        {
            pr_info("-");
        }
        pr_info("+");
    }
    pr_info("\n");
}

static void
tdnf_print_row(const char *const *ppszCols, const int *pnColWidths, int nCols)
{
    int i = 0;
    pr_info("|");
    for (i = 0; i < nCols; ++i)
    {
        pr_info(" ");
        tdnf_print_padded(ppszCols[i] ? ppszCols[i] : "", pnColWidths[i]);
        pr_info(" |");
    }
    pr_info("\n");
}


uint32_t
TDNFCliInstallCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs
    )
{
    uint32_t dwError = 0;

    dwError = TDNFCliAlterCommand(pContext, pCmdArgs, ALTER_INSTALL);
    BAIL_ON_CLI_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFCliEraseCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs
    )
{
    uint32_t dwError = 0;

    dwError = TDNFCliAlterCommand(pContext, pCmdArgs, ALTER_ERASE);
    BAIL_ON_CLI_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFCliUpgradeCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs
    )
{
    uint32_t dwError = 0;
    int nAlterType = ALTER_UPGRADE;

    if(pCmdArgs->nCmdCount == 1)
    {
        nAlterType = ALTER_UPGRADEALL;
    }

    dwError = TDNFCliAlterCommand(pContext, pCmdArgs, nAlterType);
    BAIL_ON_CLI_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFCliDistroSyncCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs
    )
{
    uint32_t dwError = 0;

    dwError = TDNFCliAlterCommand(pContext, pCmdArgs, ALTER_DISTRO_SYNC);
    BAIL_ON_CLI_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFCliDowngradeCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs
    )
{
    uint32_t dwError = 0;
    int nAlterType = ALTER_DOWNGRADE;

    if(pCmdArgs->nCmdCount == 1)
    {
        nAlterType = ALTER_DOWNGRADEALL;
    }

    dwError = TDNFCliAlterCommand(pContext, pCmdArgs, nAlterType);
    BAIL_ON_CLI_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFCliAutoEraseCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs
    )
{
    uint32_t dwError = 0;
    int nAlterType = ALTER_AUTOERASE;

    if(pCmdArgs->nCmdCount == 1)
    {
        nAlterType = ALTER_AUTOERASEALL;
    }

    dwError = TDNFCliAlterCommand(pContext, pCmdArgs, nAlterType);
    BAIL_ON_CLI_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFCliReinstallCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs
    )
{
    uint32_t dwError = 0;

    dwError = TDNFCliAlterCommand(pContext, pCmdArgs, ALTER_REINSTALL);
    BAIL_ON_CLI_ERROR(dwError);

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFCliAskForAction(
    PTDNF_CMD_ARGS pCmdArgs,
    PTDNF_SOLVED_PKG_INFO pSolvedPkgInfo
)
{
    uint32_t dwError = 0;
    char** ppszPackageArgs = NULL;
    int nSilent = 0;

    if(!pCmdArgs || !pSolvedPkgInfo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_CLI_ERROR(dwError);
    }

    nSilent = pCmdArgs->nNoOutput;

    if(!nSilent && pSolvedPkgInfo->ppszPkgsNotResolved)
    {
        dwError = PrintNotAvailable(pSolvedPkgInfo->ppszPkgsNotResolved);
        BAIL_ON_CLI_ERROR(dwError);
    }

    if(!pSolvedPkgInfo->nNeedAction)
    {
        dwError = ERROR_TDNF_CLI_NOTHING_TO_DO;
        //If there are unresolved, error with no match
        if(pSolvedPkgInfo->ppszPkgsNotResolved &&
           *pSolvedPkgInfo->ppszPkgsNotResolved)
        {
            dwError = ERROR_TDNF_NO_MATCH;
        }
        BAIL_ON_CLI_ERROR(dwError);
    }

    if(!nSilent)
    {
        if (pCmdArgs->nJsonOutput)
        {
            dwError = PrintSolvedInfoJson(pSolvedPkgInfo);
        } else {
            dwError = PrintSolvedInfo(pSolvedPkgInfo);
        }
        if (pCmdArgs->nDownloadOnly)
        {
            pr_info("tdnf will only download packages needed for the transaction\n");
        }
    }

    if(pSolvedPkgInfo->nNeedAction)
    {
        int nAnswer = 0;

        dwError = TDNFYesOrNo(pCmdArgs, "\xF0\x9F\x91\x89 Proceed? [y/N]", &nAnswer);
        BAIL_ON_CLI_ERROR(dwError);

        if(!nAnswer)
        {
            dwError = ERROR_TDNF_OPERATION_ABORTED;
            BAIL_ON_CLI_ERROR(dwError);
        }
    }

cleanup:
    TDNF_CLI_SAFE_FREE_STRINGARRAY(ppszPackageArgs);
    return dwError;

error:
    goto cleanup;
}

uint32_t
TDNFCliPrintActionComplete(
    PTDNF_CMD_ARGS pCmdArgs
)
{
    uint32_t dwError = 0;
    int nSilent = 0;

    if(!pCmdArgs)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_CLI_ERROR(dwError);
    }

    nSilent = pCmdArgs->nNoOutput;

    if(!nSilent)
    {
        pr_info("\n" COLOR_GREEN "Complete!" COLOR_RESET "\n");
        if (pCmdArgs->nDownloadOnly)
        {
            if (pCmdArgs->pszDownloadDir != NULL)
            {
                pr_info("Packages have been downloaded to %s.\n",
                        pCmdArgs->pszDownloadDir);
            } else {
                pr_info("Packages have been downloaded to cache.\n");
            }
        }
    }
cleanup:
    return dwError;
error:
    goto cleanup;
}

static
uint32_t
TDNFCliAskAndAlter(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs,
    PTDNF_SOLVED_PKG_INFO pSolvedPkgInfo
)
{
    uint32_t dwError = 0;
    char** ppszPackageArgs = NULL;

    if(!pContext || !pCmdArgs || !pSolvedPkgInfo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_CLI_ERROR(dwError);
    }

    dwError = TDNFCliAskForAction(pCmdArgs, pSolvedPkgInfo);
    BAIL_ON_CLI_ERROR(dwError);

    dwError = pContext->pFnAlter(
                pContext,
                pSolvedPkgInfo);
    BAIL_ON_CLI_ERROR(dwError);

    if (pCmdArgs->nJsonOutput)
    {
        dwError = TDNFCliPrintActionComplete(pCmdArgs);
        BAIL_ON_CLI_ERROR(dwError);
    }

cleanup:
    TDNF_CLI_SAFE_FREE_STRINGARRAY(ppszPackageArgs);
    return dwError;

error:
    if (pCmdArgs && pCmdArgs->nJsonOutput && dwError == ERROR_TDNF_OPERATION_ABORTED)
    {
        dwError = 0;
    }
    goto cleanup;
}

uint32_t
TDNFCliAlterCommand(
    PTDNF_CLI_CONTEXT pContext,
    PTDNF_CMD_ARGS pCmdArgs,
    TDNF_ALTERTYPE nAlterType
    )
{
    uint32_t dwError = 0;
    char** ppszPackageArgs = NULL;
    int nPackageCount = 0;
    PTDNF_SOLVED_PKG_INFO pSolvedPkgInfo = NULL;

    if(!pContext || !pContext->hTdnf || !pCmdArgs)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_CLI_ERROR(dwError);
    }

    dwError = TDNFCliParsePackageArgs(
                  pCmdArgs,
                  &ppszPackageArgs,
                  &nPackageCount);
    BAIL_ON_CLI_ERROR(dwError);

    dwError = pContext->pFnResolve(
                  pContext,
                  nAlterType,
                  &pSolvedPkgInfo);
    BAIL_ON_CLI_ERROR(dwError);

    dwError = TDNFCliAskAndAlter(pContext, pCmdArgs, pSolvedPkgInfo);
    BAIL_ON_CLI_ERROR(dwError);

cleanup:
    TDNF_CLI_SAFE_FREE_STRINGARRAY(ppszPackageArgs);
    TDNFCliFreeSolvedPackageInfo(pSolvedPkgInfo);
    return dwError;

error:
    if (dwError == ERROR_TDNF_ALREADY_INSTALLED)
    {
        dwError = ERROR_TDNF_CLI_NOTHING_TO_DO;
    }
    goto cleanup;
}

uint32_t
JDPkgList(
    PTDNF_PKG_INFO pPkgInfos,
    struct json_dump **ppJDList
)
{
    uint32_t dwError = 0;
    PTDNF_PKG_INFO pPkgInfo;
    struct json_dump *jd_list = jd_create(0);
    struct json_dump *jd_pkg = NULL;
    CHECK_JD_NULL(jd_list);

    jd_list_start(jd_list);
    for(pPkgInfo = pPkgInfos; pPkgInfo; pPkgInfo = pPkgInfo->pNext)
    {
        jd_pkg = jd_create(0);
        CHECK_JD_NULL(jd_pkg);

        CHECK_JD_RC(jd_map_start(jd_pkg));

        CHECK_JD_RC(jd_map_add_string(jd_pkg, "Name", pPkgInfo->pszName));
        CHECK_JD_RC(jd_map_add_string(jd_pkg, "Arch", pPkgInfo->pszArch));
        CHECK_JD_RC(jd_map_add_fmt(jd_pkg, "Evr", "%s-%s", pPkgInfo->pszVersion, pPkgInfo->pszRelease));
        CHECK_JD_RC(jd_map_add_int(jd_pkg, "InstallSize", pPkgInfo->dwInstallSizeBytes));
        CHECK_JD_RC(jd_map_add_string(jd_pkg, "Repo", pPkgInfo->pszRepoName));

        CHECK_JD_RC(jd_list_add_child(jd_list, jd_pkg));
        JD_SAFE_DESTROY(jd_pkg);
    }
    *ppJDList = jd_list;
cleanup:
    return dwError;
error:
    JD_SAFE_DESTROY(jd_pkg);
    JD_SAFE_DESTROY(jd_list);
    goto cleanup;
}

uint32_t
PrintSolvedInfoJson(
    PTDNF_SOLVED_PKG_INFO pSolvedPkgInfo
    )
{
    uint32_t dwError = 0;
    struct json_dump *jd = jd_create(1024);
    struct json_dump *jd_list = NULL;

    CHECK_JD_NULL(jd);

    if(!pSolvedPkgInfo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_CLI_ERROR(dwError);
    }

    CHECK_JD_RC(jd_map_start(jd));

    if(pSolvedPkgInfo->pPkgsExisting)
    {
        dwError = JDPkgList(pSolvedPkgInfo->pPkgsExisting, &jd_list);
        CHECK_JD_RC(jd_map_add_child(jd, "Exist", jd_list));
        JD_SAFE_DESTROY(jd);
    }
    if(pSolvedPkgInfo->pPkgsNotAvailable)
    {
        dwError = JDPkgList(pSolvedPkgInfo->pPkgsNotAvailable, &jd_list);
        CHECK_JD_RC(jd_map_add_child(jd, "Unavailable", jd_list));
        JD_SAFE_DESTROY(jd_list);
    }
    if(pSolvedPkgInfo->pPkgsToInstall)
    {
        dwError = JDPkgList(pSolvedPkgInfo->pPkgsToInstall, &jd_list);
        CHECK_JD_RC(jd_map_add_child(jd, "Install", jd_list));
        JD_SAFE_DESTROY(jd_list);
    }
    if(pSolvedPkgInfo->pPkgsToUpgrade)
    {
        dwError = JDPkgList(pSolvedPkgInfo->pPkgsToUpgrade, &jd_list);
        CHECK_JD_RC(jd_map_add_child(jd, "Upgrade", jd_list));
        JD_SAFE_DESTROY(jd_list);
    }
    if(pSolvedPkgInfo->pPkgsToDowngrade)
    {
        dwError = JDPkgList(pSolvedPkgInfo->pPkgsToDowngrade, &jd_list);
        CHECK_JD_RC(jd_map_add_child(jd, "Downgrade", jd_list));
        JD_SAFE_DESTROY(jd_list);
    }
    if(pSolvedPkgInfo->pPkgsToRemove)
    {
        dwError = JDPkgList(pSolvedPkgInfo->pPkgsToRemove, &jd_list);
        CHECK_JD_RC(jd_map_add_child(jd, "Remove", jd_list));
        JD_SAFE_DESTROY(jd_list);
    }
    if(pSolvedPkgInfo->pPkgsUnNeeded)
    {
        dwError = JDPkgList(pSolvedPkgInfo->pPkgsUnNeeded, &jd_list);
        CHECK_JD_RC(jd_map_add_child(jd, "UnNeeded", jd_list));
        JD_SAFE_DESTROY(jd_list);
    }
    if(pSolvedPkgInfo->pPkgsToReinstall)
    {
        dwError = JDPkgList(pSolvedPkgInfo->pPkgsToReinstall, &jd_list);
        CHECK_JD_RC(jd_map_add_child(jd, "Reinstall", jd_list));
        JD_SAFE_DESTROY(jd_list);
    }
    if(pSolvedPkgInfo->pPkgsObsoleted)
    {
        dwError = JDPkgList(pSolvedPkgInfo->pPkgsObsoleted, &jd_list);
        CHECK_JD_RC(jd_map_add_child(jd, "Obsolete", jd_list));
        JD_SAFE_DESTROY(jd_list);
    }
    pr_json(jd->buf);
    JD_SAFE_DESTROY(jd);

cleanup:
    return dwError;

error:
    JD_SAFE_DESTROY(jd_list);
    JD_SAFE_DESTROY(jd);
    goto cleanup;
}

uint32_t
PrintSolvedInfo(
    PTDNF_SOLVED_PKG_INFO pSolvedPkgInfo
    )
{
    uint32_t dwError = 0;

    if(!pSolvedPkgInfo)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_CLI_ERROR(dwError);
    }

    if(pSolvedPkgInfo->pPkgsExisting)
    {
        dwError = PrintExistingPackagesSkipped(pSolvedPkgInfo->pPkgsExisting);
        BAIL_ON_CLI_ERROR(dwError);
    }
    if(pSolvedPkgInfo->pPkgsNotAvailable)
    {
        dwError = PrintNotAvailablePackages(pSolvedPkgInfo->pPkgsNotAvailable);
        BAIL_ON_CLI_ERROR(dwError);
    }
    if(pSolvedPkgInfo->pPkgsToInstall)
    {
        dwError = PrintAction(pSolvedPkgInfo->pPkgsToInstall, ALTER_INSTALL);
        BAIL_ON_CLI_ERROR(dwError);
    }
    if(pSolvedPkgInfo->pPkgsToUpgrade)
    {
        dwError = PrintAction(pSolvedPkgInfo->pPkgsToUpgrade, ALTER_UPGRADE);
        BAIL_ON_CLI_ERROR(dwError);
    }
    if(pSolvedPkgInfo->pPkgsToDowngrade)
    {
        dwError = PrintAction(
                      pSolvedPkgInfo->pPkgsToDowngrade,
                      ALTER_DOWNGRADE);
        BAIL_ON_CLI_ERROR(dwError);
    }
    if(pSolvedPkgInfo->pPkgsToRemove)
    {
        dwError = PrintAction(pSolvedPkgInfo->pPkgsToRemove, ALTER_ERASE);
        BAIL_ON_CLI_ERROR(dwError);
    }
    if(pSolvedPkgInfo->pPkgsUnNeeded)
    {
        dwError = PrintAction(pSolvedPkgInfo->pPkgsUnNeeded, ALTER_ERASE);
        BAIL_ON_CLI_ERROR(dwError);
    }
    if(pSolvedPkgInfo->pPkgsToReinstall)
    {
        dwError = PrintAction(
                      pSolvedPkgInfo->pPkgsToReinstall,
                      ALTER_REINSTALL);
        BAIL_ON_CLI_ERROR(dwError);
    }
    if(pSolvedPkgInfo->pPkgsObsoleted)
    {
        dwError = PrintAction(pSolvedPkgInfo->pPkgsObsoleted, ALTER_OBSOLETED);
        BAIL_ON_CLI_ERROR(dwError);
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
PrintNotAvailable(
    char** ppszPkgsNotAvailable
    )
{
    uint32_t dwError = 0;
    int i = 0;

    if(!ppszPkgsNotAvailable)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_CLI_ERROR(dwError);
    }

    while(ppszPkgsNotAvailable[i])
    {
        pr_info("No package %s%s%s available\n",
                COLOR_RED,
                ppszPkgsNotAvailable[i],
                COLOR_RESET);
        ++i;
    }
cleanup:
    return dwError;
error:
    goto cleanup;
}

uint32_t
PrintExistingPackagesSkipped(
    PTDNF_PKG_INFO pPkgInfos
    )
{
    uint32_t dwError = 0;

    PTDNF_PKG_INFO pPkgInfo = NULL;

    if(!pPkgInfos)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_CLI_ERROR(dwError);
    }

    pPkgInfo = pPkgInfos;
    while(pPkgInfo)
    {
        pr_info(
            "Package %s%s-%s-%s.%s%s is already installed, skipping.\n",
            COLOR_CYAN,
            pPkgInfo->pszName,
            pPkgInfo->pszVersion,
            pPkgInfo->pszRelease,
            pPkgInfo->pszArch,
            COLOR_RESET);
        pPkgInfo = pPkgInfo->pNext;
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

uint32_t
PrintNotAvailablePackages(
    PTDNF_PKG_INFO pPkgInfos
    )
{
    uint32_t dwError = 0;

    PTDNF_PKG_INFO pPkgInfo = NULL;

    if(!pPkgInfos)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_CLI_ERROR(dwError);
    }

    pPkgInfo = pPkgInfos;
    while(pPkgInfo)
    {
        pr_info(
            "No package %s%s%s available.\n",
            COLOR_RED,
            pPkgInfo->pszName,
            COLOR_RESET);
        pPkgInfo = pPkgInfo->pNext;
    }

cleanup:
    return dwError;
error:
    goto cleanup;
}

uint32_t
PrintAction(
    PTDNF_PKG_INFO pPkgInfos,
    TDNF_ALTERTYPE nAlterType
    )
{
    uint32_t dwError = 0;
    PTDNF_PKG_INFO pPkgInfo = NULL;

    int nTotalInstallSize = 0;
    int nTotalDownloadSize = 0;
    char *pszTotalInstallSize = NULL;
    char *pszTotalDownloadSize = NULL;
    char *pszEmptyString = "";

    #define COL_COUNT 6
    //Name | Arch | [Epoch:]Version-Release | Repository | Install Size | Download Size
    int nColPercents[COL_COUNT] = {20, 15, 20, 15, 10, 10};
    int nColWidths[COL_COUNT] = {0};

    #define MAX_COL_LEN 256
    char szEpochVersionRelease[MAX_COL_LEN] = {0};
    char *ppszInfoToPrint[MAX_COL_LEN] = {0};

    if(!pPkgInfos)
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_CLI_ERROR(dwError);
    }

    switch(nAlterType)
    {
        case ALTER_INSTALL:
            pr_info("\n" COLOR_GREEN "\xF0\x9F\x93\xA5 [tdnf] \xE2\x86\x92 INSTALLING PACKAGE" COLOR_RESET "\n\n");
            break;
        case ALTER_UPGRADE:
            pr_info("\n" COLOR_CYAN "\xF0\x9F\x94\x84 [tdnf] \xE2\x86\x92 UPGRADING PACKAGE" COLOR_RESET "\n\n");
            break;
        case ALTER_ERASE:
            pr_info("\n" COLOR_RED "\xF0\x9F\xA7\xB9 [tdnf] \xE2\x86\x92 REMOVING PACKAGE" COLOR_RESET "\n\n");
            break;
        case ALTER_DOWNGRADE:
            pr_info("\n" COLOR_YELLOW "\xF0\x9F\x94\xBC [tdnf] \xE2\x86\x92 DOWNGRADING PACKAGE" COLOR_RESET "\n\n");
            break;
        case ALTER_REINSTALL:
            pr_info("\n" COLOR_MAGENTA "\xF0\x9F\x8C\x80 [tdnf] \xE2\x86\x92 REINSTALLING PACKAGE" COLOR_RESET "\n\n");
            break;
        case ALTER_OBSOLETED:
            pr_info("\n" COLOR_YELLOW "Obsoleting:" COLOR_RESET);
            break;
        default:
            pr_info("\n" COLOR_YELLOW "\xF0\x9F\x97\xBF [tdnf] \xE2\x86\x92 OBSOLETING PACKAGE" COLOR_RESET "\n\n");
            BAIL_ON_CLI_ERROR(dwError);
    }
    pr_info("\xF0\x9F\x93\xA6 Package Info:\n");

    dwError = GetColumnWidths(COL_COUNT, nColPercents, nColWidths);
    BAIL_ON_CLI_ERROR(dwError);

    {
        const char *ppszHeader[COL_COUNT] = {
            "Name",
            "Arch",
            "Version",
            "Repository",
            "Installed",
            "Download"
        };

        tdnf_print_border(nColWidths, COL_COUNT);
        tdnf_print_row(ppszHeader, nColWidths, COL_COUNT);
        tdnf_print_border(nColWidths, COL_COUNT);
    }
    
    for(pPkgInfo = pPkgInfos; pPkgInfo; pPkgInfo = pPkgInfo->pNext)
    {
        nTotalInstallSize += pPkgInfo->dwInstallSizeBytes;
        nTotalDownloadSize += pPkgInfo->dwDownloadSizeBytes;
        memset(szEpochVersionRelease, 0, MAX_COL_LEN);
        if(pPkgInfo->dwEpoch)
        {
            if(snprintf(
                szEpochVersionRelease,
                MAX_COL_LEN,
                "%u:%s-%s",
                (unsigned)pPkgInfo->dwEpoch,
                pPkgInfo->pszVersion,
                pPkgInfo->pszRelease) < 0)
            {
                dwError = errno;
                BAIL_ON_CLI_ERROR(dwError);
            }
        }
        else
        {
            if(snprintf(
                szEpochVersionRelease,
                MAX_COL_LEN,
                "%s-%s",
                pPkgInfo->pszVersion,
                pPkgInfo->pszRelease) < 0)
            {
                dwError = errno;
                BAIL_ON_CLI_ERROR(dwError);
            }
        }

        ppszInfoToPrint[0] = pPkgInfo->pszName == NULL ?
                                 pszEmptyString : pPkgInfo->pszName;
        ppszInfoToPrint[1] = pPkgInfo->pszArch == NULL ?
                                 pszEmptyString : pPkgInfo->pszArch;
        ppszInfoToPrint[2] = szEpochVersionRelease;
        ppszInfoToPrint[3] = pPkgInfo->pszRepoName == NULL ?
                                 pszEmptyString : pPkgInfo->pszRepoName;
        ppszInfoToPrint[4] = pPkgInfo->pszFormattedSize == NULL ?
                                 pszEmptyString : pPkgInfo->pszFormattedSize;
        ppszInfoToPrint[5] = pPkgInfo->pszFormattedDownloadSize == NULL ?
                                 pszEmptyString : pPkgInfo->pszFormattedDownloadSize;

        tdnf_print_row((const char *const *)ppszInfoToPrint, nColWidths, COL_COUNT);
    }

    tdnf_print_border(nColWidths, COL_COUNT);

    dwError = TDNFUtilsFormatSize(nTotalInstallSize, &pszTotalInstallSize);
    BAIL_ON_TDNF_ERROR(dwError);
    pr_info("\n\xF0\x9F\x93\x8A Totals:\n   \xE2\x80\xA2 Installed Size: \xF0\x9F\x92\xBE %s\n", pszTotalInstallSize);

    dwError = TDNFUtilsFormatSize(nTotalDownloadSize, &pszTotalDownloadSize);
    BAIL_ON_TDNF_ERROR(dwError);
    pr_info("   \xE2\x80\xA2 Download Size : \xE2\xAC\x87\xEF\xB8\x8F %s\n", pszTotalDownloadSize);

cleanup:
    TDNFFreeMemory(pszTotalInstallSize);
    TDNFFreeMemory(pszTotalDownloadSize);
    return dwError;

error:
    goto cleanup;
}
