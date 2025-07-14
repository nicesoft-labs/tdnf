#include "includes.h"

// Сообщение с описанием использования команды tdnf
static const char *help_msg =
    "📘 Использование: tdnf [опции] КОМАНДА\n\n"
    "⚙️ Общие опции:\n"
    "   [--assumeno]                      ❎ Предполагать 'нет' для всех запросов\n"
    "   [-y, --assumeyes]                 ✔️ Подтверждать 'да' автоматически\n"
    "   [-C, --cacheonly]                 🗂️ Работать только с кэшем\n"
    "   [-c [файл_конфигурации]]          🧾 Указать файл конфигурации\n"
    "   [--debugsolver]                   🐛 Отладка решателя зависимостей\n"
    "   [--disableexcludes]               🚷 Отключить исключения\n"
    "   [--disableplugin=<имя>]           🧱 Отключить плагин\n"
    "   [--disablerepo=<ID>]              ❌ Отключить репозиторий\n"
    "   [--downloaddir=<путь>]            📁 Папка для загрузки\n"
    "   [--downloadonly]                  📥 Только загрузить, без установки\n"
    "   [--enablerepo=<ID>]               🟢 Включить репозиторий\n"
    "   [--enableplugin=<имя>]            🧩 Включить плагин\n"
    "   [--exclude [файл1,файл2,...]]     🛑 Исключить файлы\n"
    "   [--installroot [путь]]            🏠 Корневая директория установки\n"
    "   [--noautoremove]                  🚫 Без автоудаления зависимостей\n"
    "   [--nogpgcheck]                    🔓 Отключить проверку GPG\n"
    "   [--noplugins]                     📴 Отключить все плагины\n"
    "   [--nocolor]                       🌈 Без цветного прогресса\n"
    "   [-q, --quiet]                     🤐 Тихий режим\n"
    "   [--reboot-required]               🔁 Показывать пакеты, требующие перезагрузки\n"
    "   [--refresh]                       ♻️ Обновить метаданные\n"
    "   [--releasever ВЕРСИЯ]             🧷 Указать версию релиза\n"
    "   [--repo=<ID>]                     📦 Задать репозиторий\n"
    "   [--repofrompath=<ID>,<путь>]      🛤️ Репозиторий из указанного пути\n"
    "   [--repoid=<ID>]                   🆔 Указать ID репозитория\n"
    "   [--rpmverbosity [уровень]]        📣 Уровень детализации RPM\n"
    "   [--security]                      🛡️ Только обновления безопасности\n"
    "   [--sec-severity <уровень>]        🚨 Уровень критичности CVSS\n"
    "   [--setopt ОПЦИИ]                  🛠️ Установить параметры\n"
    "   [--skip-broken]                   🧩 Пропустить повреждённые пакеты\n"
    "   [--skipconflicts]                 ⚔️ Пропустить конфликты\n"
    "   [--skipdigest]                    🧪 Пропустить проверку хэша\n"
    "   [--skipsignature]                 ✍️ Пропустить проверку подписи\n"
    "   [--skipobsoletes]                 🗑️ Пропустить устаревшие пакеты\n"
    "   [--testonly]                      🧫 Только тестировать\n"
    "   [--version]                       🆓 Показать версию tdnf\n\n"

    "🔎 Опции для repoquery (выборка):\n"
    "   [--available]                     📚 Доступные пакеты\n"
    "   [--duplicates]                    📛 Дубликаты пакетов\n"
    "   [--extras]                        🌟 Дополнительные пакеты\n"
    "   [--file <файл>]                   📄 Поиск по файлу\n"
    "   [--installed]                     🖥️ Установленные пакеты\n"
    "   [--whatdepends <имя>]             🧷 Что зависит от\n"
    "   [--whatrequires <имя>]            🧲 Что требует\n"
    "   [--whatenhances <имя>]            📈 Что улучшает\n"
    "   [--whatobsoletes <имя>]           🧹 Что устаревает\n"
    "   [--whatprovides <имя>]            📦 Что предоставляет\n"
    "   [--whatrecommends <имя>]          💬 Что рекомендуется\n"
    "   [--whatsuggests <имя>]            🧠 Что предлагается\n"
    "   [--whatsupplements <имя>]         ➕ Что дополняет\n\n"

    "🧠 Опции для repoquery (анализ):\n"
    "   [--depends]                       🔗 Показать зависимости\n"
    "   [--enhances]                      🚀 Показать улучшения\n"
    "   [--list]                          📋 Показать список\n"
    "   [--obsoletes]                     ❌ Показать устаревшие пакеты\n"
    "   [--provides]                      📬 Показать возможности\n"
    "   [--recommends]                    🗣️ Показать рекомендации\n"
    "   [--requires]                      📌 Показать требования\n"
    "   [--requires-pre]                  ⏱️ Предварительные требования\n"
    "   [--suggests]                      💭 Показать предложения\n"
    "   [--source]                        🧾 Исходные пакеты\n"
    "   [--supplements]                   🔧 Показать дополнения\n\n"

    "📦 Опции для reposync:\n"
    "   [--arch=<арх>]                    🧮 Указать архитектуру\n"
    "   [--delete]                        ❎ Удалить старые пакеты\n"
    "   [--download-path=<путь>]          📤 Путь загрузки\n"
    "   [--download-metadata]             🗃️ Загрузить метаданные\n"
    "   [--gpgcheck]                      🔐 Включить проверку GPG\n"
    "   [--metadata-path=<путь>]          📇 Папка для метаданных\n"
    "   [--newest-only]                   🔝 Только последние версии\n"
    "   [--norepopath]                    🚷 Игнорировать путь репо\n"
    "   [--source]                        🧾 Скачивать исходники\n"
    "   [--urls]                          🌐 Показать URL пакетов\n\n"

    "🧰 Основные команды:\n"
    "autoerase          🧼 То же, что 'autoremove'\n"
    "autoremove         🧼 Удалить зависимости\n"
    "check              🕵️ Проверить репозитории\n"
    "check-local        🧭 Проверить локальные RPM\n"
    "check-update       🔄 Проверить обновления\n"
    "clean              🧹 Очистка кэша\n"
    "distro-sync        🧬 Синхронизировать версии\n"
    "downgrade          📉 Понизить версию\n"
    "erase              ❌ Удалить пакеты\n"
    "help               🆘 Показать справку\n"
    "history            🗓️ История операций\n"
    "info               🛈 Информация о пакете\n"
    "install            📲 Установить пакеты\n"
    "list               📃 Список пакетов\n"
    "makecache          💾 Сгенерировать кэш\n"
    "mark               🔖 Отметить пакеты\n"
    "provides           🧾 То же, что 'whatprovides'\n"
    "whatprovides       🔍 Найти поставщика\n"
    "reinstall          🔁 Переустановить\n"
    "remove             🗑️ Удалить пакеты\n"
    "repolist           🗂️ Список репозиториев\n"
    "repoquery          🧮 Запрос к репо\n"
    "reposync           ⬇️ Скачать репозиторий\n"
    "search             🔎 Поиск пакетов\n"
    "update             ⬆️ Обновить пакеты\n"
    "update-to          📈 Обновить до версии\n"
    "updateinfo         🧾 Уведомления об обновлениях\n"
    "upgrade            ⬆️ Обновить пакеты\n"
    "upgrade-to         ⬆️ Обновление до конкретной версии\n"
    "\n"
    "📖 Документация: https://niceos.ru/\n";

// Показать сообщение об ошибке при отсутствии команды
void
TDNFCliShowUsage(void)
{
    pr_crit("❌ Укажите команду для выполнения!\n");
    TDNFCliShowHelp();
}

// Показать справку
void
TDNFCliShowHelp(void)
{
    pr_crit("%s\n", help_msg);
}

// Показать сообщение о неверной команде
void
TDNFCliShowNoSuchCommand(const char *pszCmd)
{
    pr_crit("🚫 Команда '%s' не найдена. Используйте /usr/bin/tdnf --help для справки.\n",
            pszCmd ? pszCmd : "");
}

// Показать сообщение о неверной опции
void
TDNFCliShowNoSuchOption(const char *pszOption)
{
    pr_crit("🚫 Опция '%s' не найдена. Используйте /usr/bin/tdnf --help для справки.\n",
            pszOption ? pszOption : "");
}

// Обработчик команды help
uint32_t
TDNFCliHelpCommand(PTDNF_CLI_CONTEXT pContext, PTDNF_CMD_ARGS pCmdArgs)
{
    if (!pCmdArgs || !pContext)
    {
        pr_crit("❌ Ошибка: Неверные параметры функции!\n");
        return ERROR_TDNF_INVALID_PARAMETER;
    }

    TDNFCliShowHelp();
    return 0;
}
