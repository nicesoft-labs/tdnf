/*
 * Header: defines.h
 *
 * Abstract:
 *            tdnf
 *            command line tool
 *
 * Copyright (C) 2025 ООО "НАЙС СОФТ ГРУПП" (ИНН: 5024245440)
 * Email: <niceos@ncsgp.ru>
 * Код не подлежит изменению без разрешения правообладателя.
 */

#pragma once

// Макрос для выхода при ошибке CLI
#define BAIL_ON_CLI_ERROR(unError) \
    do { \
        if (unError) { \
            goto error; \
        } \
    } while (0)

// Макрос для безопасного освобождения памяти
#define TDNF_CLI_SAFE_FREE_MEMORY(pMemory) \
    do { \
        if (pMemory) { \
            TDNFFreeMemory(pMemory); \
            pMemory = NULL; \
        } \
    } while (0)

// Макрос для безопасного освобождения массива строк
#define TDNF_CLI_SAFE_FREE_STRINGARRAY(ppArray) \
    do { \
        if (ppArray) { \
            TDNFFreeStringArray(ppArray); \
            ppArray = NULL; \
        } \
    } while (0)

// Таблица ошибок CLI 
#define TDNF_CLI_ERROR_TABLE \
{ \
    {ERROR_TDNF_CLI_BASE,                    "ERROR_TDNF_CLI_BASE",                   "❌ Общая ошибка CLI"}, \
    {ERROR_TDNF_CLI_NO_MATCH,                "ERROR_TDNF_CLI_NO_MATCH",               "🔍 Ничего не найдено по вашему запросу"}, \
    {ERROR_TDNF_CLI_INVALID_ARGUMENT,        "ERROR_TDNF_CLI_INVALID_ARGUMENT",       "🚫 Неверный аргумент"}, \
    {ERROR_TDNF_CLI_CLEAN_REQUIRES_OPTION,   "ERROR_TDNF_CLI_CLEAN_REQUIRES_OPTION",  "🧹 Команда clean требует опцию: packages, metadata, dbcache, plugins, expire-cache, all"}, \
    {ERROR_TDNF_CLI_NOT_ENOUGH_ARGS,         "ERROR_TDNF_CLI_NOT_ENOUGH_ARGS",        "⚠️ Недостаточно аргументов. Ожидается хотя бы один"}, \
    {ERROR_TDNF_CLI_NOTHING_TO_DO,           "ERROR_TDNF_CLI_NOTHING_TO_DO",          "😶 Нечего выполнять"}, \
    {ERROR_TDNF_CLI_OPTION_NAME_INVALID,     "ERROR_TDNF_CLI_OPTION_NAME_INVALID",    "🚫 Неверная опция команды"}, \
    {ERROR_TDNF_CLI_OPTION_ARG_REQUIRED,     "ERROR_TDNF_CLI_OPTION_ARG_REQUIRED",    "⚠️ Опция требует один аргумент"}, \
    {ERROR_TDNF_CLI_OPTION_ARG_UNEXPECTED,   "ERROR_TDNF_CLI_OPTION_ARG_UNEXPECTED",  "🚫 Неожиданный аргумент для опции"}, \
    {ERROR_TDNF_CLI_CHECKLOCAL_EXPECT_DIR,   "ERROR_TDNF_CLI_CHECKLOCAL_EXPECT_DIR",  "📁 Команда check-local требует путь к папке с RPM"}, \
    {ERROR_TDNF_CLI_PROVIDES_EXPECT_ARG,     "ERROR_TDNF_CLI_PROVIDES_EXPECT_ARG",    "🔍 Укажите элемент для поиска"}, \
    {ERROR_TDNF_CLI_SETOPT_NO_EQUALS,        "ERROR_TDNF_CLI_SETOPT_NO_EQUALS",       "⚠️ В setopt отсутствует знак равно. Ожидается формат key=value"}, \
    {ERROR_TDNF_CLI_NO_SUCH_CMD,             "ERROR_TDNF_CLI_NO_SUCH_CMD",            "🚫 Неверная команда. Проверьте правильность ввода"}, \
    {ERROR_TDNF_CLI_DOWNLOADDIR_REQUIRES_DOWNLOADONLY, "ERROR_TDNF_CLI_DOWNLOADDIR_REQUIRES_DOWNLOADONLY", "📥 Опция --downloaddir требует --downloadonly"}, \
    {ERROR_TDNF_CLI_ONE_DEP_ONLY,             "ERROR_TDNF_CLI_ONE_DEP_ONLY",          "⚠️ Разрешена только одна зависимость"}, \
    {ERROR_TDNF_CLI_ALLDEPS_REQUIRES_DOWNLOADONLY, "ERROR_TDNF_CLI_ALLDEPS_REQUIRES_DOWNLOADONLY", "📦 Опция --alldeps требует --downloadonly"}, \
    {ERROR_TDNF_CLI_NODEPS_REQUIRES_DOWNLOADONLY, "ERROR_TDNF_CLI_NODEPS_REQUIRES_DOWNLOADONLY", "📦 Опция --nodeps требует --downloadonly"}, \
};
