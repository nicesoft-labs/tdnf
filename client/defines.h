/*
 * Copyright (C) 2015-2023 VMware, Inc. Все права защищены.
 *
 * Лицензировано под GNU Lesser General Public License v2.1 (далее "Лицензия");
 * вы не можете использовать этот файл, кроме как в соответствии с Лицензией. Условия
 * Лицензии находятся в файле COPYING данной дистрибуции.
 */

/*
 * Заголовок: defines.h
 *
 * Аннотация:
 *
 *            tdnfclientlib
 *
 *            клиентская библиотека
 *
 * Авторы: Приеш Падмавиласом (ppadmavilasom@vmware.com)
 */

#pragma once

#include "config.h"

typedef enum
{
    /* это должен быть битмаска */
    DETAIL_LIST,
    DETAIL_INFO,
    DETAIL_CHANGELOG,
    DETAIL_SOURCEPKG
} TDNF_PKG_DETAIL;

#define BAIL_ON_TDNF_RPM_ERROR(dwError) \
    do {                                                           \
        if (dwError)                                               \
        {                                                          \
            dwError = ERROR_TDNF_RPM_BASE + dwError;               \
            goto error;                                            \
        }                                                          \
    } while(0)

#define BAIL_ON_TDNF_CURL_ERROR(dwError) \
    do {                                                           \
        if (dwError)                                               \
        {                                                          \
            dwError = ERROR_TDNF_CURL_BASE + dwError;              \
            goto error;                                            \
        }                                                          \
    } while(0)

#define STR_IS_TRUE(s) ((s) && (!strcmp((s), "1") || !strcasecmp((s), "true")))

// Разное
#define TDNF_RPM_EXT                      ".rpm"
#define TDNF_NAME                         "tdnf"
#define DIR_SEPARATOR                     '/'
#define SOLV_PATCH_MARKER                 "patch:"

// Типы repomd
#define TDNF_REPOMD_TYPE_PRIMARY          "primary"
#define TDNF_REPOMD_TYPE_FILELISTS        "filelists"
#define TDNF_REPOMD_TYPE_UPDATEINFO       "updateinfo"
#define TDNF_REPOMD_TYPE_OTHER            "other"

// Определения репозитория
#define TDNF_REPO_EXT                     ".repo"
#define TDNF_CONF_FILE                    "/etc/tdnf/tdnf.conf"
#define TDNF_CONF_GROUP                   "main"
// Ключи конфигурационного файла
#define TDNF_CONF_KEY_GPGCHECK            "gpgcheck"
#define TDNF_CONF_KEY_INSTALLONLY_LIMIT   "installonly_limit"
#define TDNF_CONF_KEY_CLEAN_REQ_ON_REMOVE "clean_requirements_on_remove"
#define TDNF_CONF_KEY_REPODIR             "repodir"
#define TDNF_CONF_KEY_CACHEDIR            "cachedir"
#define TDNF_CONF_KEY_PERSISTDIR          "persistdir"
#define TDNF_CONF_KEY_PROXY               "proxy"
#define TDNF_CONF_KEY_PROXY_USER          "proxy_username"
#define TDNF_CONF_KEY_PROXY_PASS          "proxy_password"
#define TDNF_CONF_KEY_KEEP_CACHE          "keepcache"
#define TDNF_CONF_KEY_DISTROVERPKG        "distroverpkg"
#define TDNF_CONF_KEY_DISTROARCHPKG       "distroarchpkg"
#define TDNF_CONF_KEY_MAX_STRING_LEN      "maxstringlen"
#define TDNF_CONF_KEY_PLUGINS             "plugins"
#define TDNF_CONF_KEY_NO_PLUGINS          "noplugins"
#define TDNF_CONF_KEY_PLUGIN_PATH         "pluginpath"
#define TDNF_CONF_KEY_PLUGIN_CONF_PATH    "pluginconfpath"
#define TDNF_PLUGIN_CONF_KEY_ENABLED      "enabled"
#define TDNF_CONF_KEY_EXCLUDE             "excludepkgs"
#define TDNF_CONF_KEY_MINVERSIONS         "minversions"
#define TDNF_CONF_KEY_OPENMAX             "openmax"
#define TDNF_CONF_KEY_CHECK_UPDATE_COMPAT "dnf_check_update_compat"
#define TDNF_CONF_KEY_DISTROSYNC_REINSTALL_CHANGED "distrosync_reinstall_changed"
#define TDNF_CONF_KEY_PARALLEL_DOWNLOADS  "parallelDownloads"

// Ключи файла репозитория
#define TDNF_REPO_KEY_BASEURL             "baseurl"
#define TDNF_REPO_KEY_ENABLED             "enabled"
#define TDNF_REPO_KEY_METALINK            "metalink"
#define TDNF_REPO_KEY_NAME                "name"
#define TDNF_REPO_KEY_SKIP                "skip_if_unavailable"
#define TDNF_REPO_KEY_GPGCHECK            "gpgcheck"
#define TDNF_REPO_KEY_GPGKEY              "gpgkey"
#define TDNF_REPO_KEY_USERNAME            "username"
#define TDNF_REPO_KEY_PASSWORD            "password"
#define TDNF_REPO_KEY_PRIORITY            "priority"
#define TDNF_REPO_KEY_METADATA_EXPIRE     "metadata_expire"
#define TDNF_REPO_KEY_TIMEOUT             "timeout"
#define TDNF_REPO_KEY_RETRIES             "retries"
#define TDNF_REPO_KEY_MINRATE             "minrate"
#define TDNF_REPO_KEY_THROTTLE            "throttle"
#define TDNF_REPO_KEY_SSL_VERIFY          "sslverify"
#define TDNF_REPO_KEY_SSL_CA_CERT         "sslcacert"
#define TDNF_REPO_KEY_SSL_CLI_CERT        "sslclientcert"
#define TDNF_REPO_KEY_SSL_CLI_KEY         "sslclientkey"
#define TDNF_REPO_KEY_SKIP_MD_FILELISTS   "skip_md_filelists"
#define TDNF_REPO_KEY_SKIP_MD_UPDATEINFO  "skip_md_updateinfo"
#define TDNF_REPO_KEY_SKIP_MD_OTHER       "skip_md_other"

// Ключи setopt
#define TDNF_SETOPT_KEY_REPOSDIR          "reposdir"
#define TDNF_SETOPT_KEY_PARALLEL_DOWNLOADS "parallelDownloads"

// Имена файлов
#define TDNF_REPO_METADATA_MARKER         "lastrefresh"
#define TDNF_REPO_METADATA_FILE_PATH      "repodata/repomd.xml"
#define TDNF_REPO_METADATA_FILE_NAME      "repomd.xml"
#define TDNF_REPO_METALINK_FILE_NAME      "metalink"
#define TDNF_REPO_BASEURL_FILE_NAME       "baseurl"

#define TDNF_AUTOINSTALLED_FILE           "autoinstalled"
#define TDNF_HISTORY_DB_FILE              "history.db"

// Значения по умолчанию для репозитория
#define TDNF_DEFAULT_REPO_LOCATION        "/etc/yum.repos.d"
#define TDNF_DEFAULT_CACHE_LOCATION       "/var/cache/tdnf"

/* pszPersistDir - по умолчанию настраивается во время сборки,
   и настраивается с помощью "persistdir" во время выполнения */
#define TDNF_DEFAULT_DB_LOCATION          HISTORY_DB_DIR

#define TDNF_DEFAULT_DISTROVERPKG         "system-release"
#define TDNF_DEFAULT_DISTROARCHPKG        "x86_64"
#define TDNF_RPM_CACHE_DIR_NAME           "rpms"
#define TDNF_REPODATA_DIR_NAME            "repodata"
#define TDNF_SOLVCACHE_DIR_NAME           "solvcache"
#define TDNF_REPO_METADATA_EXPIRE_NEVER   "never"

#define TDNF_DEFAULT_OPENMAX              1024
#define TDNF_DEFAULT_PARALLEL_DOWNLOADS   4

// Настройки по умолчанию для репозитория
#define TDNF_REPO_DEFAULT_ENABLED            0
#define TDNF_REPO_DEFAULT_SKIP               0
#define TDNF_REPO_DEFAULT_GPGCHECK           1
#define TDNF_REPO_DEFAULT_MINRATE            0
#define TDNF_REPO_DEFAULT_THROTTLE           0
#define TDNF_REPO_DEFAULT_TIMEOUT            0
#define TDNF_REPO_DEFAULT_SSLVERIFY          1
#define TDNF_REPO_DEFAULT_RETRIES            10
#define TDNF_REPO_DEFAULT_PRIORITY           50
#define TDNF_REPO_DEFAULT_METADATA_EXPIRE    172800 // 48 часов в секундах
#define TDNF_REPO_DEFAULT_METADATA_EXPIRE_STR STRINGIFYX(TDNF_REPO_DEFAULT_METADATA_EXPIRE)
#define TDNF_REPO_DEFAULT_SKIP_MD_FILELISTS  0
#define TDNF_REPO_DEFAULT_SKIP_MD_UPDATEINFO 0
#define TDNF_REPO_DEFAULT_SKIP_MD_OTHER      0

// Имена переменных
#define TDNF_VAR_RELEASEVER               "$releasever"
#define TDNF_VAR_BASEARCH                 "$basearch"
/* Фиктивные значения setopt */
#define TDNF_SETOPT_NAME_DUMMY             "opt.dummy.name"
#define TDNF_SETOPT_VALUE_DUMMY            "opt.dummy.value"
/* Определения плагинов */
#define TDNF_DEFAULT_PLUGINS_ENABLED      0
#define TDNF_DEFAULT_PLUGIN_PATH          SYSTEM_LIBDIR"/tdnf-plugins"
#define TDNF_DEFAULT_PLUGIN_CONF_PATH     "/etc/tdnf/pluginconf.d"
#define TDNF_PLUGIN_CONF_EXT              ".conf"
#define TDNF_PLUGIN_CONF_EXT_LEN          5
#define TDNF_PLUGIN_CONF_MAIN_SECTION     "main"

#define TDNF_UNKNOWN_ERROR_STRING "Неизвестная ошибка"
#define TDNF_ERROR_TABLE \
{ \
    {ERROR_TDNF_BASE,                "ERROR_TDNF_EBASE",               "Общая базовая ошибка"}, \
    {ERROR_TDNF_PACKAGE_REQUIRED,    "ERROR_TDNF_PACKAGE_REQUIRED",    "Ожидалось имя пакета, но оно не было предоставлено"}, \
    {ERROR_TDNF_CONF_FILE_LOAD,      "ERROR_TDNF_CONF_FILE_LOAD",      "Ошибка загрузки конфигурации tdnf (/etc/tdnf/tdnf.conf)"}, \
    {ERROR_TDNF_REPO_FILE_LOAD,      "ERROR_TDNF_REPO_FILE_LOAD",      "Ошибка загрузки репозитория tdnf (обычно в /etc/yum.repos.d/)"}, \
    {ERROR_TDNF_INVALID_REPO_FILE,   "ERROR_TDNF_INVALID_REPO_FILE",   "Обнаружен недействительный файл репозитория"}, \
    {ERROR_TDNF_REPO_DIR_OPEN,       "ERROR_TDNF_REPO_DIR_OPEN",       "Ошибка открытия директории репозитория. Проверьте, существует ли repodir, указанный в tdnf.conf (обычно /etc/yum.repos.d)"}, \
    {ERROR_TDNF_NO_MATCH,            "ERROR_TDNF_NO_MATCH",            "Нет подходящих пакетов"}, \
    {ERROR_TDNF_SET_PROXY,           "ERROR_TDNF_SET_PROXY",           "Ошибка при установке прокси-сервера."}, \
    {ERROR_TDNF_SET_PROXY_USERPASS,  "ERROR_TDNF_SET_PROXY_USERPASS",  "Ошибка при установке имени пользователя и пароля прокси-сервера"}, \
    {ERROR_TDNF_NO_DISTROVERPKG,     "ERROR_TDNF_NO_DISTROVERPKG",     "Параметр distroverpkg указывает на неустановленный пакет. Проверьте /etc/tdnf/tdnf.conf"}, \
    {ERROR_TDNF_DISTROVERPKG_READ,   "ERROR_TDNF_DISTROVERPKG_READ",   "Ошибка чтения версии distroverpkg"}, \
    {ERROR_TDNF_INVALID_ALLOCSIZE,   "ERROR_TDNF_INVALID_ALLOCSIZE",   "Запрошено выделение памяти с недействительным размером"}, \
    {ERROR_TDNF_STRING_TOO_LONG,     "ERROR_TDNF_STRING_TOO_LONG",     "Запрошенный размер выделения строки слишком велик."}, \
    {ERROR_TDNF_NO_ENABLED_REPOS,    "ERROR_TDNF_NO_ENABLED_REPOS",    "Нет включенных репозиториев.\n Выполните ""tdnf repolist all"", чтобы увидеть доступные репозитории.\n Вы можете включить репозитории:\n 1. Передав --enablerepo <имя_репозитория>\n 2. Отредактировав файлы репозитория в repodir (обычно /etc/yum.repos.d)"}, \
    {ERROR_TDNF_PACKAGELIST_EMPTY,   "ERROR_TDNF_PACKAGELIST_EMPTY",   "Список пакетов пуст"}, \
    {ERROR_TDNF_GOAL_CREATE,         "ERROR_TDNF_GOAL_CREATE",         "Ошибка создания цели"}, \
    {ERROR_TDNF_INVALID_RESOLVE_ARG, "ERROR_TDNF_INVALID_RESOLVE_ARG", "Недействительный аргумент в разрешении"}, \
    {ERROR_TDNF_CLEAN_UNSUPPORTED,   "ERROR_TDNF_CLEAN_UNSUPPORTED",   "Указанный тип очистки не поддерживается в этом выпуске. Попробуйте clean all."}, \
    {ERROR_TDNF_SOLV_BASE,           "ERROR_TDNF_SOLV_BASE",           "Базовая ошибка Solv"}, \
    {ERROR_TDNF_SOLV_FAILED,         "ERROR_TDNF_SOLV_FAILED",         "Общая ошибка выполнения Solv"}, \
    {ERROR_TDNF_SOLV_OP,             "ERROR_TDNF_SOLV_OP",             "Ошибка программирования клиента Solv"}, \
    {ERROR_TDNF_SOLV_LIBSOLV,        "ERROR_TDNF_SOLV_LIBSOLV",        "Ошибка Solv, переданная из libsolv"}, \
    {ERROR_TDNF_SOLV_IO,             "ERROR_TDNF_SOLV_IO",             "Solv - ошибка ввода-вывода"}, \
    {ERROR_TDNF_SOLV_CACHE_WRITE,    "ERROR_TDNF_SOLV_CACHE_WRITE",    "Solv - ошибка записи кэша"}, \
    {ERROR_TDNF_SOLV_QUERY,          "ERROR_TDNF_SOLV_QUERY",          "Solv - некорректно сформированный запрос"}, \
    {ERROR_TDNF_SOLV_ARCH,           "ERROR_TDNF_SOLV_ARCH",           "Solv - неизвестная архитектура"}, \
    {ERROR_TDNF_SOLV_VALIDATION,     "ERROR_TDNF_SOLV_VALIDATION",     "Solv - проверка валидации не пройдена"}, \
    {ERROR_TDNF_SOLV_NO_SOLUTION,    "ERROR_TDNF_SOLV_NO_SOLUTION",    "Solv - цель не нашла решений"}, \
    {ERROR_TDNF_SOLV_NO_CAPABILITY,  "ERROR_TDNF_SOLV_NO_CAPABILITY",  "Solv - возможность недоступна"}, \
    {ERROR_TDNF_SOLV_CHKSUM,         "ERROR_TDNF_SOLV_CHKSUM",         "Solv - не удалось создать контрольную сумму"}, \
    {ERROR_TDNF_REPO_WRITE,          "ERROR_TDNF_REPO_WRITE",          "Solv - не удалось записать репозиторий"}, \
    {ERROR_TDNF_SOLV_CACHE_NOT_CREATED, "ERROR_TDNF_SOLV_CACHE_NOT_CREATED", "Solv - кэш Solv не найден"}, \
    {ERROR_TDNF_ADD_SOLV,            "ERROR_TDNF_ADD_SOLV",            "Solv - не удалось добавить solv"}, \
    {ERROR_TDNF_REPO_BASE,           "ERROR_TDNF_REPO_BASE",           "Базовая ошибка репозитория"}, \
    {ERROR_TDNF_SET_SSL_SETTINGS,    "ERROR_TDNF_SET_SSL_SETTINGS",    "Ошибка при установке настроек SSL для репозитория."}, \
    {ERROR_TDNF_REPO_PERFORM,        "ERROR_TDNF_REPO_PERFORM",        "Ошибка во время выполнения обработчика репозитория"}, \
    {ERROR_TDNF_REPO_GETINFO,        "ERROR_TDNF_REPO_GETINFO",        "Ошибка во время получения информации о результате репозитория"}, \
    {ERROR_TDNF_TRANSACTION_FAILED,  "ERROR_TDNF_TRANSACTION_FAILED",  "Транзакция rpm не удалась"}, \
    {ERROR_TDNF_NO_SEARCH_RESULTS,   "ERROR_TDNF_NO_SEARCH_RESULTS",   "Совпадений не найдено"}, \
    {ERROR_TDNF_RPMRC_NOTFOUND,      "ERROR_TDNF_RPMRC_NOTFOUND",      "Общая ошибка rpm - не найдено (возможно, поврежден файл rpm)"}, \
    {ERROR_TDNF_RPMRC_FAIL,          "ERROR_TDNF_RPMRC_FAIL",          "Общая ошибка rpm"}, \
    {ERROR_TDNF_RPMRC_NOTTRUSTED,    "ERROR_TDNF_RPMRC_NOTTRUSTED",    "Подпись rpm в порядке, но ключ не является доверенным"}, \
    {ERROR_TDNF_RPMRC_NOKEY,         "ERROR_TDNF_RPMRC_NOKEY",         "Открытый ключ недоступен. Установите открытый ключ с помощью rpm --import или используйте --nogpgcheck для игнорирования."}, \
    {ERROR_TDNF_INVALID_PUBKEY_FILE, "ERROR_TDNF_INVALID_PUBKEY_FILE", "Файл открытого ключа недействителен или поврежден"}, \
    {ERROR_TDNF_KEYURL_UNSUPPORTED,  "ERROR_TDNF_KEYURL_UNSUPPORTED",  "URL-схемы GpgKey, кроме file, не поддерживаются"}, \
    {ERROR_TDNF_KEYURL_INVALID,      "ERROR_TDNF_KEYURL_INVALID",      "URL GpgKey недействителен"}, \
    {ERROR_TDNF_RPM_NOT_SIGNED,      "ERROR_TDNF_RPM_NOT_SIGNED",      "RPM не подписан. Используйте --nogpgcheck для игнорирования."}, \
    {ERROR_TDNF_RPMTD_CREATE_FAILED, "ERROR_TDNF_RPMTD_CREATE_FAILED", "Не удалось создать контейнер данных RPM. Используйте --nogpgcheck для игнорирования."}, \
    {ERROR_TDNF_RPM_GET_RSAHEADER_FAILED, "ERROR_TDNF_RPM_GET_RSAHEADER_FAILED", "RPM не подписан. Используйте --skipsignature или --nogpgcheck для игнорирования."}, \
    {ERROR_TDNF_RPM_GPG_PARSE_FAILED, "ERROR_TDNF_RPM_GPG_PARSE_FAILED", "Не удалось разобрать gpg-ключ RPM. Используйте --nogpgcheck для игнорирования."}, \
    {ERROR_TDNF_RPM_GPG_NO_MATCH,    "ERROR_TDNF_RPM_GPG_NO_MATCH",     "RPM подписан, но не соответствует известным ключам. Используйте --nogpgcheck для игнорирования."}, \
    {ERROR_TDNF_AUTOERASE_UNSUPPORTED, "ERROR_TDNF_AUTOERASE_UNSUPPORTED", "autoerase / autoremove не поддерживается."}, \
    {ERROR_TDNF_RPM_CHECK,           "ERROR_TDNF_RPM_CHECK",           "Проверка rpm сообщила об ошибках"}, \
    {ERROR_TDNF_RPMTS_BAD_ROOT_DIR,  "ERROR_TDNF_RPMTS_BAD_ROOT_DIR",  "Недействительная корневая директория"}, \
    {ERROR_TDNF_METADATA_EXPIRE_PARSE, "ERROR_TDNF_METADATA_EXPIRE_PARSE", "Не удалось разобрать значение metadata_expire. Проверьте файлы репозитория."}, \
    {ERROR_TDNF_PROTECTED,           "ERROR_TDNF_PROTECTED",           "Операция приведет к удалению защищенного пакета."}, \
    {ERROR_TDNF_DOWNGRADE_NOT_ALLOWED, "ERROR_TDNF_DOWNGRADE_NOT_ALLOWED", "Понижение версии ниже минимальной не допускается. Проверьте 'minversions' в конфигурации."}, \
    {ERROR_TDNF_PERM,                "ERROR_TDNF_PERM",                "Операция не разрешена. Требуются права root."}, \
    {ERROR_TDNF_OPT_NOT_FOUND,       "ERROR_TDNF_OPT_NOT_FOUND",       "Требуемый параметр не найден"}, \
    {ERROR_TDNF_OPERATION_ABORTED,   "ERROR_TDNF_OPERATION_ABORTED",   "Операция прервана."}, \
    {ERROR_TDNF_INVALID_INPUT,       "ERROR_TDNF_INVALID_INPUT",       "Недействительный ввод."}, \
    {ERROR_TDNF_CACHE_DISABLED,      "ERROR_TDNF_CACHE_DISABLED",      "Установлен режим только кэша, но данные репозитория не найдены"}, \
    {ERROR_TDNF_CACHE_DIR_OUT_OF_DISK_SPACE, "ERROR_TDNF_CACHE_DIR_OUT_OF_DISK_SPACE", "Недостаточно места на диске в директории кэша /var/cache/tdnf (если не указано иное в конфигурации). Попробуйте освободить место."}, \
    {ERROR_TDNF_DUPLICATE_REPO_ID,   "ERROR_TDNF_DUPLICATE_REPO_ID",   "Дублирующийся идентификатор репозитория"}, \
    {ERROR_TDNF_EVENT_CTXT_ITEM_NOT_FOUND, "ERROR_TDNF_EVENT_CTXT_ITEM_NOT_FOUND", "Элемент контекста события не найден. Обычно это связано с событиями плагинов. Попробуйте --noplugins для деактивации всех плагинов или --disableplugin=<плагин> для деактивации конкретного. Вы можете навсегда деактивировать проблемный плагин, установив enable=0 в файле конфигурации плагина."}, \
    {ERROR_TDNF_EVENT_CTXT_ITEM_INVALID_TYPE, "ERROR_TDNF_EVENT_CTXT_ITEM_INVALID_TYPE", "Несоответствие типа элемента события. Обычно это связано с событиями плагинов. Попробуйте --noplugins для деактивации всех плагинов или --disableplugin=<плагин> для деактивации конкретного. Вы можете навсегда деактивировать проблемный плагин, установив enable=0 в файле конфигурации плагина."}, \
    {ERROR_TDNF_NO_GPGKEY_CONF_ENTRY, "ERROR_TDNF_NO_GPGKEY_CONF_ENTRY", "Отсутствует запись gpgkey для этого репозитория. Добавьте gpgkey в файл репозитория или используйте --nogpgcheck для игнорирования."}, \
    {ERROR_TDNF_URL_INVALID,         "ERROR_TDNF_URL_INVALID",         "URL недействителен."}, \
    {ERROR_TDNF_SIZE_MISMATCH,       "ERROR_TDNF_SIZE_MISMATCH",       "Размер файла не совпадает."}, \
    {ERROR_TDNF_CHECKSUM_MISMATCH,   "ERROR_TDNF_CHECKSUM_MISMATCH",   "Контрольная сумма файла не совпадает."}, \
    {ERROR_TDNF_BASEURL_DOES_NOT_EXISTS, "ERROR_TDNF_BASEURL_DOES_NOT_EXISTS", "Base URL и Metalink URL не найдены в файле репозитория"}, \
    {ERROR_TDNF_CHECKSUM_VALIDATION_FAILED, "ERROR_TDNF_CHECKSUM_VALIDATION_FAILED", "Не удалось проверить контрольную сумму для repomd.xml, загруженного по URL из metalink"}, \
    {ERROR_TDNF_METALINK_RESOURCE_VALIDATION_FAILED, "ERROR_TDNF_METALINK_RESOURCE_VALIDATION_FAILED", "В файле metalink отсутствует ресурс для загрузки файла"}, \
    {ERROR_TDNF_FIPS_MODE_FORBIDDEN, "ERROR_TDNF_FIPS_MODE_FORBIDDEN", "Вызов API дайджеста запрещен в режиме FIPS!"}, \
    {ERROR_TDNF_CURLE_UNSUPPORTED_PROTOCOL, "ERROR_TDNF_CURLE_UNSUPPORTED_PROTOCOL", "Curl не поддерживает этот протокол"}, \
    {ERROR_TDNF_CURLE_FAILED_INIT,   "ERROR_TDNF_CURLE_FAILED_INIT",   "Ошибка инициализации Curl"}, \
    {ERROR_TDNF_CURLE_URL_MALFORMAT, "ERROR_TDNF_CURLE_URL_MALFORMAT", "URL кажется поврежденным. Выполните clean all и makecache"}, \
    {ERROR_TDNF_SYSTEM_BASE,         "ERROR_TDNF_SYSTEM_BASE",         "Неизвестная системная ошибка"}, \
    {ERROR_TDNF_HISTORY_NODB,        "ERROR_TDNF_HISTORY_ERROR",       "Ошибка базы данных истории"}, \
    {ERROR_TDNF_HISTORY_NODB,        "ERROR_TDNF_HISTORY_NODB",        "База данных истории не существует"}, \
};

// remoterepo.c
#define sizeOfStruct(ARRAY) (sizeof(ARRAY)/sizeof(*ARRAY))

// metalink.c
typedef void (*TDNF_ML_FREE_FUNC) (void* data);

#define TAG_NAME_FILE "file"
#define TAG_NAME_SIZE "size"
#define TAG_NAME_HASH "hash"
#define TAG_NAME_URL  "url"
