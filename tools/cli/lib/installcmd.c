/*
 * Copyright (C) 2015-2023 VMware, Inc. Все права защищены.
 *
 * Лицензировано под GNU General Public License v2 (the "License");
 * вы не можете использовать этот файл, кроме как в соответствии с Лицензией.
 * Условия Лицензии находятся в файле COPYING этой дистрибуции.
 */

/*
 * Модуль   : installcmd.c
 *
 * Аннотация :
 *
 *            tdnf
 *
 *            инструмент командной строки
 *
 * Авторы   : Прийеш Падмавиласом (ppadmavilasom@vmware.com)
 *
 */

#include "includes.h"

#define COLOR_BLUE "\033[1;34m"
#define COLOR_RED "\033[1;31m"
#define COLOR_RESET "\033[0m"
#define BOLD "\033[1m\033[30m"
#define RESET "\033[0m"

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
            pr_info(COLOR_BLUE "tdnf будет только загружать пакеты, необходимые для транзакции\n" COLOR_RESET);
        }
    }

    if(pSolvedPkgInfo->nNeedAction)
    {
        int nAnswer = 0;

        dwError = TDNFYesOrNo(pCmdArgs, COLOR_BLUE "Это нормально? [y/N]: " COLOR_RESET, &nAnswer);
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
        pr_info("\n" COLOR_BLUE "Завершено!\n" COLOR_RESET);
        if (pCmdArgs->nDownloadOnly)
        {
            if (pCmdArgs->pszDownloadDir != NULL)
            {
                pr_info(COLOR_BLUE "Пакеты были загружены в %s.\n" COLOR_RESET,
                        pCmdArgs->pszDownloadDir);
            } else {
                pr_info(COLOR_BLUE "Пакеты были загружены в кэш.\n" COLOR_RESET);
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

        CHECK_JD_RC(jd_map_add_string(jd_pkg, "Имя", pPkgInfo->pszName));
        CHECK_JD_RC(jd_map_add_string(jd_pkg, "Архитектура", pPkgInfo->pszArch));
        CHECK_JD_RC(jd_map_add_fmt(jd_pkg, "Evr", "%s-%s", pPkgInfo->pszVersion, pPkgInfo->pszRelease));
        CHECK_JD_RC(jd_map_add_int(jd_pkg, "Размер установки", pPkgInfo->dwInstallSizeBytes));
        CHECK_JD_RC(jd_map_add_string(jd_pkg, "Репозиторий", pPkgInfo->pszRepoName));

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
        CHECK_JD_RC(jd_map_add_child(jd, "Существующие", jd_list));
        JD_SAFE_DESTROY(jd);
    }
    if(pSolvedPkgInfo->pPkgsNotAvailable)
    {
        dwError = JDPkgList(pSolvedPkgInfo->pPkgsNotAvailable, &jd_list);
        CHECK_JD_RC(jd_map_add_child(jd, "Недоступные", jd_list));
        JD_SAFE_DESTROY(jd_list);
    }
    if(pSolvedPkgInfo->pPkgsToInstall)
    {
        dwError = JDPkgList(pSolvedPkgInfo->pPkgsToInstall, &jd_list);
        CHECK_JD_RC(jd_map_add_child(jd, "Установить", jd_list));
        JD_SAFE_DESTROY(jd_list);
    }
    if(pSolvedPkgInfo->pPkgsToUpgrade)
    {
        dwError = JDPkgList(pSolvedPkgInfo->pPkgsToUpgrade, &jd_list);
        CHECK_JD_RC(jd_map_add_child(jd, "Обновить", jd_list));
        JD_SAFE_DESTROY(jd_list);
    }
    if(pSolvedPkgInfo->pPkgsToDowngrade)
    {
        dwError = JDPkgList(pSolvedPkgInfo->pPkgsToDowngrade, &jd_list);
        CHECK_JD_RC(jd_map_add_child(jd, "Понизить", jd_list));
        JD_SAFE_DESTROY(jd_list);
    }
    if(pSolvedPkgInfo->pPkgsToRemove)
    {
        dwError = JDPkgList(pSolvedPkgInfo->pPkgsToRemove, &jd_list);
        CHECK_JD_RC(jd_map_add_child(jd, "Удалить", jd_list));
        JD_SAFE_DESTROY(jd_list);
    }
    if(pSolvedPkgInfo->pPkgsUnNeeded)
    {
        dwError = JDPkgList(pSolvedPkgInfo->pPkgsUnNeeded, &jd_list);
        CHECK_JD_RC(jd_map_add_child(jd, "Ненужные", jd_list));
        JD_SAFE_DESTROY(jd_list);
    }
    if(pSolvedPkgInfo->pPkgsToReinstall)
    {
        dwError = JDPkgList(pSolvedPkgInfo->pPkgsToReinstall, &jd_list);
        CHECK_JD_RC(jd_map_add_child(jd, "Переустановить", jd_list));
        JD_SAFE_DESTROY(jd_list);
    }
    if(pSolvedPkgInfo->pPkgsObsoleted)
    {
        dwError = JDPkgList(pSolvedPkgInfo->pPkgsObsoleted, &jd_list);
        CHECK_JD_RC(jd_map_add_child(jd, "Устаревшие", jd_list));
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
        pr_info(COLOR_RED "Пакет " BOLD "%s " RESET "недоступен\n" COLOR_RESET,
                ppszPkgsNotAvailable[i]);
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
            COLOR_BLUE "Пакет %s-%s-%s.%s уже установлен, пропускается.\n" COLOR_RESET,
            pPkgInfo->pszName,
            pPkgInfo->pszVersion,
            pPkgInfo->pszRelease,
            pPkgInfo->pszArch);
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
            COLOR_RED "Пакет %s недоступен.\n" COLOR_RESET,
            pPkgInfo->pszName);
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
    //Имя | Архитектура | [Эпоха:]Версия-Релиз | Репозиторий | Размер установки | Размер загрузки
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
            pr_info("\n" COLOR_BLUE "Установка:" COLOR_RESET);
            break;
        case ALTER_UPGRADE:
            pr_info("\n" COLOR_BLUE "Обновление:" COLOR_RESET);
            break;
        case ALTER_ERASE:
            pr_info("\n" COLOR_RED "Удаление:" COLOR_RESET);
            break;
        case ALTER_DOWNGRADE:
            pr_info("\n" COLOR_BLUE "Понижение версии:" COLOR_RESET);
            break;
        case ALTER_REINSTALL:
            pr_info("\n" COLOR_BLUE "Переустановка:" COLOR_RESET);
            break;
        case ALTER_OBSOLETED:
            pr_info("\n" COLOR_BLUE "Устаревание:" COLOR_RESET);
            break;
        default:
            dwError = ERROR_TDNF_INVALID_PARAMETER;
            BAIL_ON_CLI_ERROR(dwError);
    }
    pr_info("\n");

    dwError = GetColumnWidths(COL_COUNT, nColPercents, nColWidths);
    BAIL_ON_CLI_ERROR(dwError);

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
        pr_info(
            "%-*s %-*s %-*s %-*s %-*s %*s\n",
            nColWidths[0],
            ppszInfoToPrint[0],
            nColWidths[1],
            ppszInfoToPrint[1],
            nColWidths[2],
            ppszInfoToPrint[2],
            nColWidths[3],
            ppszInfoToPrint[3],
            nColWidths[4],
            ppszInfoToPrint[4],
            nColWidths[5],
            ppszInfoToPrint[5]);
    }

    dwError = TDNFUtilsFormatSize(nTotalInstallSize, &pszTotalInstallSize);
    BAIL_ON_TDNF_ERROR(dwError);
    pr_info(COLOR_BLUE "\nОбщий размер установки: %s\n" COLOR_RESET, pszTotalInstallSize);

    dwError = TDNFUtilsFormatSize(nTotalDownloadSize, &pszTotalDownloadSize);
    BAIL_ON_TDNF_ERROR(dwError);
    pr_info(COLOR_BLUE "Общий размер загрузки: %s\n" COLOR_RESET, pszTotalDownloadSize);

cleanup:
    TDNFFreeMemory(pszTotalInstallSize);
    TDNFFreeMemory(pszTotalDownloadSize);
    return dwError;

error:
    goto cleanup;
}
