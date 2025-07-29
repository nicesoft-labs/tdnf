/*
 * Copyright (C) 2015-2023 VMware, Inc. All Rights Reserved.
 *
 * Licensed under the GNU General Public License v2 (the "License");
 * you may not use this file except in compliance with the License. The terms
 * of the License are located in the COPYING file of this distribution.
 */

/*
 * Header : defines.h
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

#pragma once

#define BAIL_ON_CLI_ERROR(unError) \
    do {                                                           \
        if (unError)                                               \
        {                                                          \
            goto error;                                            \
        }                                                          \
    } while(0)

#define TDNF_CLI_SAFE_FREE_MEMORY(pMemory) \
    do {                                                           \
        if (pMemory) {                                             \
            TDNFFreeMemory(pMemory);                               \
            pMemory = NULL;                                        \
        }                                                          \
    } while(0)

#define TDNF_CLI_SAFE_FREE_STRINGARRAY(ppArray) \
    do {                                                           \
        if (ppArray) {                                             \
            TDNFFreeStringArray(ppArray);                          \
            ppArray = NULL;                                        \
        }                                                          \
    } while(0)

#define TDNF_CLI_ERROR_TABLE \
{ \
    {ERROR_TDNF_CLI_BASE,                    "ERROR_TDNF_CLI_BASE",                   "Общая базовая ошибка."}, \
    {ERROR_TDNF_CLI_NO_MATCH,                "ERROR_TDNF_CLI_NO_MATCH",               "Поиск не дал результатов."}, \
    {ERROR_TDNF_CLI_INVALID_ARGUMENT,        "ERROR_TDNF_CLI_INVALID_ARGUMENT",       "Недопустимый аргумент."}, \
    {ERROR_TDNF_CLI_CLEAN_REQUIRES_OPTION,   "ERROR_TDNF_CLI_CLEAN_REQUIRES_OPTION",  "Команда clean требует указания опции: packages, metadata, dbcache, plugins, expire-cache, all"}, \
    {ERROR_TDNF_CLI_NOT_ENOUGH_ARGS,         "ERROR_TDNF_CLI_NOT_ENOUGH_ARGS",        "Парсер командной строки не может продолжить. Ожидался как минимум один аргумент."}, \
    {ERROR_TDNF_CLI_NOTHING_TO_DO,           "ERROR_TDNF_CLI_NOTHING_TO_DO",          "Нечего выполнять."}, \
    {ERROR_TDNF_CLI_OPTION_NAME_INVALID,     "ERROR_TDNF_CLI_OPTION_NAME_INVALID",    "Ошибка командной строки: недопустимая опция."}, \
    {ERROR_TDNF_CLI_OPTION_ARG_REQUIRED,     "ERROR_TDNF_CLI_OPTION_ARG_REQUIRED",    "Ошибка командной строки: ожидался один аргумент."}, \
    {ERROR_TDNF_CLI_OPTION_ARG_UNEXPECTED,   "ERROR_TDNF_CLI_OPTION_ARG_UNEXPECTED",  "Ошибка командной строки: аргумент был неожиданным."}, \
    {ERROR_TDNF_CLI_CHECKLOCAL_EXPECT_DIR,   "ERROR_TDNF_CLI_CHECKLOCAL_EXPECT_DIR",  "Команда check-local требует указания пути к каталогу RPM в качестве параметра."}, \
    {ERROR_TDNF_CLI_PROVIDES_EXPECT_ARG,     "ERROR_TDNF_CLI_PROVIDES_EXPECT_ARG",    "Требуется элемент для соответствия."}, \
    {ERROR_TDNF_CLI_SETOPT_NO_EQUALS,        "ERROR_TDNF_CLI_SETOPT_NO_EQUALS",       "Отсутствует знак равно в аргументе setopt. Опция setopt требует аргумент в формате ключ=значение."}, \
    {ERROR_TDNF_CLI_NO_SUCH_CMD,             "ERROR_TDNF_CLI_NO_SUCH_CMD",            "Проверьте правильность команды."}, \
    {ERROR_TDNF_CLI_DOWNLOADDIR_REQUIRES_DOWNLOADONLY, "ERROR_TDNF_CLI_DOWNLOADDIR_REQUIRES_DOWNLOADONLY", "Опция --downloaddir требует использования --downloadonly."}, \
    {ERROR_TDNF_CLI_ONE_DEP_ONLY,             "ERROR_TDNF_CLI_ONE_DEP_ONLY",          "Допускается только одна зависимость."}, \
    {ERROR_TDNF_CLI_ALLDEPS_REQUIRES_DOWNLOADONLY, "ERROR_TDNF_CLI_ALLDEPS_REQUIRES_DOWNLOADONLY", "Опция --alldeps требует использования --downloadonly."}, \
    {ERROR_TDNF_CLI_NODEPS_REQUIRES_DOWNLOADONLY, "ERROR_TDNF_CLI_NODEPS_REQUIRES_DOWNLOADONLY", "Опция --nodeps требует использования --downloadonly."}, \
};
