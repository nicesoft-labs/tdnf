#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

static void extract_pkgname(const char *problem, char *out, size_t outsz)
{
    const char *beg = strstr(problem, " requires ");
    const char *end = NULL;
    if (beg) {
        beg += strlen(" requires ");
        end = strchr(beg, ',');
    }
    size_t i = 0;
    if (!beg || !end) {
        out[0] = '\0';
        return;
    }
    for (; beg < end && i < outsz - 1; beg++) {
        if (*beg != ' ')
            out[i++] = *beg;
    }
    out[i] = '\0';
}

int main(void)
{
    char longtok[400];
    memset(longtok, 'a', sizeof(longtok));
    longtok[sizeof(longtok) - 1] = '\0';

    char problem[512];
    snprintf(problem, sizeof(problem), "pkgA requires %s, something", longtok);

    char out[256];
    extract_pkgname(problem, out, sizeof(out));

    printf("%zu\n", strlen(out));
    assert(strlen(out) == sizeof(out) - 1);
    return 0;
}
