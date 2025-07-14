#include "includes.h"
#include "i18n.h"

#define LOCALE_DIR "/usr/share/tdnf/locale"

typedef struct _TDNF_TRANSLATION
{
    char *pszOrig;
    char *pszTrans;
    struct _TDNF_TRANSLATION *pNext;
} TDNF_TRANSLATION;

static TDNF_TRANSLATION *gpTranslations = NULL;

static const char*
TDNFGetLocaleFile(const char *lang)
{
    static char szPath[256];
    if (!lang || !*lang)
    {
        lang = getenv("TDNF_LANG");
        if (!lang)
        {
            lang = getenv("LANG");
        }
    }
    if (!lang)
    {
        return NULL;
    }
    snprintf(szPath, sizeof(szPath), "%s/%s.lang", LOCALE_DIR, lang);
    return szPath;
}

static void
TDNFAddTranslation(const char *orig, const char *trans)
{
    TDNF_TRANSLATION *p = NULL;
    if (!orig || !trans)
    {
        return;
    }
    p = calloc(1, sizeof(*p));
    if (!p)
    {
        return;
    }
    p->pszOrig = strdup(orig);
    p->pszTrans = strdup(trans);
    p->pNext = gpTranslations;
    gpTranslations = p;
}

static const char*
TDNFTrim(const char *str)
{
    while (*str == ' ' || *str == '\t') str++;
    return str;
}

uint32_t
TDNFLoadTranslations(const char *lang)
{
    const char *path = TDNFGetLocaleFile(lang);
    FILE *f = NULL;
    char line[1024];

    if (!path)
    {
        return 0;
    }

    f = fopen(path, "r");
    if (!f)
    {
        return 0;
    }

    while (fgets(line, sizeof(line), f))
    {
        char *p = NULL;
        char *orig = NULL;
        char *trans = NULL;

        if (line[0] == '#' || line[0] == '\n')
        {
            continue;
        }

        p = strchr(line, '=');
        if (!p)
        {
            continue;
        }
        *p = '\0';
        orig = (char*)TDNFTrim(line);
        trans = TDNFTrim(p + 1);
        if (*trans)
        {
            size_t len = strlen(trans);
            if (trans[len - 1] == '\n')
            {
                trans[len - 1] = 0;
            }
        }
        TDNFAddTranslation(orig, trans);
    }

    fclose(f);
    return 0;
}

const char*
TDNFTranslate(const char *msg)
{
    TDNF_TRANSLATION *p = gpTranslations;
    while (p)
    {
        if (strcmp(p->pszOrig, msg) == 0)
        {
            return p->pszTrans;
        }
        p = p->pNext;
    }
    return msg;
}

void
TDNFFreeTranslations(void)
{
    TDNF_TRANSLATION *p = gpTranslations;
    while (p)
    {
        TDNF_TRANSLATION *next = p->pNext;
        free(p->pszOrig);
        free(p->pszTrans);
        free(p);
        p = next;
    }
    gpTranslations = NULL;
}

