/*
 * Copyright (C) 2015-2023 VMware, Inc. Все права защищены.
 *
 * Лицензировано под GNU General Public License v2 (the "License");
 * вы не можете использовать этот файл без соблюдения условий лицензии. Условия
 * лицензии находятся в файле COPYING в данной дистрибутиве.
 */

/*
 * Модуль   : installcmd.c
 *
 * Описание :
 *
 *            tdnf
 *
 *            Инструмент командной строки
 *
 * Авторы   : Приеш Падмавиласом (ppadmavilasom@vmware.com)
 */

#include "includes.h"
#define MIN_TABLE_WIDTH 80
#define COLOR_RESET "\033[0m"
#define COLOR_GREEN "\033[32m"
#define COLOR_CYAN "\033[36m"
#define COLOR_RED "\033[31m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_MAGENTA "\033[35m"

// Вычисление ширины строки с учетом Unicode-символов
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
            while (*p && *p != 'm') p++;
            if (*p == 'm') p++;
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
        if (w < 0) w = 0;
        width += w;
    }

    return width;
}

// Вывод строки с дополнением пробелами до нужной ширины
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

// Вывод границы таблицы
static void
tdnf_print_border(const int *pnColWidths, int nCols)
{
    pr_info("┌");
    for (int i = 0; i < nCols; ++i)
    {
        for (int j = 0; j < pnColWidths[i] + 2; ++j)
        {
            pr_info("─");
        }
        pr_info("┬");
    }
    pr_info("\b┐\n");
}

// Вывод строки таблицы
static void
tdnf_print_row(const char *const *ppszCols, const int *pnColWidths, int nCols)
{
    pr_info("│");
    for (int i = 0; i < nCols; ++i)
    {
        pr_info(" ");
        tdnf_print_padded(ppszCols[i] ? ppszCols[i] : "", pnColWidths[i]);
        pr_info(" │");
    }
    pr_info("\n");
}

// Команда установки пакета
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

// Команда удаления пакета
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

// Команда обновления пакета
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

// Команда синхронизации дистрибутива
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

// Команда понижения версии пакета
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

// Команда автоматического удаления пакета
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

// Команда переустановки пакета
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

// Запрос подтверждения действия
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
        if(pSolvedPkgInfo->ppszPkgsNotResolved && *pSolvedPkgInfo->ppszPkgsNotResolved)
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
            pr_info("tdnf будет только загружать пакеты, необходимые для транзакции\n");
        }
    }

    if(pSolvedPkgInfo->nNeedAction)
    {
        int nAnswer = 0;

        dwError = TDNFYesOrNo(pCmdArgs, "👉 Продолжить? [д/Н]", &nAnswer);
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

// Вывод сообщения о завершении действия
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
        pr_info("\n" COLOR_GREEN "✓ Завершено!" COLOR_RESET "\n");
        if (pCmdArgs->nDownloadOnly)
        {
            if (pCmdArgs->pszDownloadDir != NULL)
            {
                pr_info("Пакеты загружены в %s.\n", pCmdArgs->pszDownloadDir);
            } else {
                pr_info("Пакеты загружены в кэш.\n");
            }
        }
    }
cleanup:
    return dwError;
error:
    goto cleanup;
}

// Выполнение запроса и изменения
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

    dwError = pContext->pFnAlter(pContext, pSolvedPkgInfo);
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

// Основная команда изменения
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

    dwError = TDNFCliParsePackageArgs(pCmdArgs, &ppszPackageArgs, &nPackageCount);
    BAIL_ON_CLI_ERROR(dwError);

    dwError = pContext->pFnResolve(pContext, nAlterType, &pSolvedPkgInfo);
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

// Формирование JSON-списка пакетов
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

    jd_lis
t_start(jd_list);
    for(pPkgInfo = pPkgInfos; pPkgInfo; pPkgInfo = pPkgInfo->pNext)
    {
        jd_pkg = jd_create(0);
        CHECK_JD_NULL(jd_pkg);

        CHECK_JD_RC(jd_map_start(jd_pkg));

        CHECK_JD_RC(jd_map_add_string(jd_pkg, "Имя", pPkgInfo->pszName));
        CHECK_JD_RC(jd_map_add_string(jd_pkg, "Архитектура", pPkgInfo->pszArch));
        CHECK_JD_RC(jd_map_add_fmt(jd_pkg, "Версия", "%s-%s", pPkgInfo->pszVersion, pPkgInfo->pszRelease));
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

// Вывод информации о решенных пакетах в формате JSON
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
        JD_SAFE_DESTROY(jd_list);
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
        CHECK_JD_RC(jd_map_add_child(jd, "Установка", jd_list));
        JD_SAFE_DESTROY(jd_list);
    }
    if(pSolvedPkgInfo->pPkgsToUpgrade)
    {
        dwError = JDPkgList(pSolvedPkgInfo->pPkgsToUpgrade, &jd_list);
        CHECK_JD_RC(jd_map_add_child(jd, "Обновление", jd_list));
        JD_SAFE_DESTROY(jd_list);
    }
    if(pSolvedPkgInfo->pPkgsToDowngrade)
    {
        dwError = JDPkgList(pSolvedPkgInfo->pPkgsToDowngrade, &jd_list);
        CHECK_JD_RC(jd_map_add_child(jd, "Понижение версии", jd_list));
        JD_SAFE_DESTROY(jd_list);
    }
    if(pSolvedPkgInfo->pPkgsToRemove)
    {
        dwError = JDPkgList(pSolvedPkgInfo->pPkgsToRemove, &jd_list);
        CHECK_JD_RC(jd_map_add_child(jd, "Удаление", jd_list));
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
        CHECK_JD_RC(jd_map_add_child(jd, "Переустановка", jd_list));
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

// Вывод информации о решенных пакетах
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
        dwError = Print

NotAvailablePackages(pSolvedPkgInfo->pPkgsNotAvailable);
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
        dwError = PrintAction(pSolvedPkgInfo->pPkgsToDowngrade, ALTER_DOWNGRADE);
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
        dwError = PrintAction(pSolvedPkgInfo->pPkgsToReinstall, ALTER_REINSTALL);
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

// Вывод информации о недоступных пакетах
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
        pr_info("🚫 Пакет %s%s%s недоступен\n",
                COLOR_RED, ppszPkgsNotAvailable[i], COLOR_RESET);
        ++i;
    }
cleanup:
    return dwError;
error:
    goto cleanup;
}

// Вывод информации о пропущенных существующих пакетах
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
            "ℹ️ Пакет %s%s-%s-%s.%s%s уже установлен, пропускается.\n",
            COLOR_CYAN, pPkgInfo->pszName, pPkgInfo->pszVersion,
            pPkgInfo->pszRelease, pPkgInfo->pszArch, COLOR_RESET);
        pPkgInfo = pPkgInfo->pNext;
    }

cleanup:
    return dwError;

error:
    goto cleanup;
}

// Вывод информации о недоступных пакетах
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
        pr_info("🚫 Пакет %s%s%s недоступен.\n",
                COLOR_RED, pPkgInfo->pszName, COLOR_RESET);
        pPkgInfo = pPkgInfo->pNext;
    }

cleanup:
    return dwError;
error:
    goto cleanup;
}

// Вывод информации о действиях с пакетами
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
    // Имя | Архитектура | Версия | Репозиторий | Размер установки | Размер загрузки
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
            pr_info("\n" COLOR_GREEN "📦 [tdnf] → УСТАНОВКА ПАКЕТА" COLOR_RESET "\n\n");
            break;
        case ALTER_UPGRADE:
            pr_info("\n" COLOR_CYAN "🔄 [tdnf] → ОБНОВЛЕНИЕ ПАКЕТА" COLOR_RESET "\n\n");
            break;
        case ALTER_ERASE:
            pr_info("\n" COLOR_RED "🗑️ [tdnf] → УДАЛЕНИЕ ПАКЕТА" COLOR_RESET "\n\n");
            break;
        case ALTER_DOWNGRADE:
            pr_info("\n" COLOR_YELLOW "🔽 [tdnf] → ПОНИЖЕНИЕ ВЕРСИИ ПАКЕТА" COLOR_RESET "\n\n");
            break;
        case ALTER_REINSTALL:
            pr_info("\n" COLOR_MAGENTA "🔁 [tdnf] → ПЕРЕУСТАНОВКА ПАКЕТА" COLOR_RESET "\n\n");
            break;
        case ALTER_OBSOLETED:
            pr_info("\n" COLOR_YELLOW "🗑️ Устаревшие пакеты:" COLOR_RESET "\n\n");
            break;
        default:
            pr_info("\n" COLOR_YELLOW "🗑️ [tdnf] → УСТАРЕВШИЕ ПАКЕТЫ" COLOR_RESET "\n\n");
            BAIL_ON_CLI_ERROR(dwError);
    }
    pr_info("📦 Информация о пакетах:\n");

    int nConsoleWidth = 0;
    bool bCompact = false;

    dwError = GetConsoleWidth(&nConsoleWidth);
    BAIL_ON_CLI_ERROR(dwError);
    if(nConsoleWidth < MIN_TABLE_WIDTH)
    {
        int nSumPercent = 0;
        int nAvailWidth = 0;
        int nBorderWidth = 1 + (3 * COL_COUNT);
        int nUsedWidth = 0;
        pr_info("⚠️ Терминал слишком узкий, используется компактный вид\n");
        bCompact = true;

        nAvailWidth = MIN_TABLE_WIDTH - nBorderWidth;
        for(int i = 0; i < COL_COUNT; ++i)
        {
            nSumPercent += nColPercents[i];
        }
        for(int i = 0; i < COL_COUNT; ++i)
        {
            nColWidths[i] = (nAvailWidth * nColPercents[i]) / nSumPercent;
            nUsedWidth += nColWidths[i];
        }
        if(nUsedWidth < nAvailWidth)
        {
            nColWidths[COL_COUNT - 1] += (nAvailWidth - nUsedWidth);
        }
    }
    else
    {
        dwError = GetColumnWidths(COL_COUNT, nColPercents, nColWidths);
        BAIL_ON_CLI_ERROR(dwError);
    }

    {
        const char *ppszHeader[COL_COUNT] = {
            "Имя",
            "Архитектура",
            "Версия",
            "Репозиторий",
            "Установлено",
            "Загрузка"
        };

        if(bCompact)
        {
            pr_info("%-*s %-*s %-*s %-*s %-*s %-*s\n",
                    nColWidths[0], ppszHeader[0],
                    nColWidths[1], ppszHeader[1],
                    nColWidths[2], ppszHeader[2],
                    nColWidths[3], ppszHeader[3],
                    nColWidths[4], ppszHeader[4],
                    nColWidths[5], ppszHeader[5]);
        }
        else
        {
            tdnf_print_border(nColWidths, COL_COUNT);
            tdnf_print_row(ppszHeader, nColWidths, COL_COUNT);
            tdnf_print_border(nColWidths, COL_COUNT);
        }
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

        ppszInfoToPrint[0] = pPkgInfo->pszName == NULL ? pszEmptyString : pPkgInfo->pszName;
        ppszInfoToPrint[1] = pPkgInfo->pszArch == NULL ? pszEmptyString : pPkgInfo->pszArch;
        ppszInfoToPrint[2] = szEpochVersionRelease;
        ppszInfoToPrint[3] = pPkgInfo->pszRepoName == NULL ? pszEmptyString : pPkgInfo->pszRepoName;
        ppszInfoToPrint[4] = pPkgInfo->pszFormattedSize == NULL ? pszEmptyString : pPkgInfo->pszFormattedSize;
        ppszInfoToPrint[5] = pPkgInfo->pszFormattedDownloadSize == NULL ? pszEmptyString : pPkgInfo->pszFormattedDownloadSize;

        if(bCompact)
        {
            pr_info("%-*s %-*s %-*s %-*s %-*s %-*s\n",
                    nColWidths[0], ppszInfoToPrint[0],
                    nColWidths[1], ppszInfoToPrint[1],
                    nColWidths[2], ppszInfoToPrint[2],
                    nColWidths[3], ppszInfoToPrint[3],
                    nColWidths[4], ppszInfoToPrint[4],
                    nColWidths[5], ppszInfoToPrint[5]);
        }
        else
        {
            tdnf_print_row((const char *const *)ppszInfoToPrint, nColWidths, COL_COUNT);
        }
    }

    if(!bCompact)
    {
        tdnf_print_border(nColWidths, COL_COUNT);
    }

    dwError = TDNFUtilsFormatSize(nTotalInstallSize, &pszTotalInstallSize);
    BAIL_ON_TDNF_ERROR(dwError);
    pr_info("\n📊 Итоги:\n   • Размер установки: 💾 %s\n", pszTotalInstallSize);

    dwError = TDNFUtilsFormatSize(nTotalDownloadSize, &pszTotalDownloadSize);
    BAIL_ON_TDNF_ERROR(dwError);
    pr_info("   • Размер загрузки: ⬇️ %s\n", pszTotalDownloadSize);

cleanup:
    TDNFFreeMemory(pszTotalInstallSize);
    TDNFFreeMemory(pszTotalDownloadSize);
    return dwError;

error:
    goto cleanup;
}
