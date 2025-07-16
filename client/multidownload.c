#include "includes.h"

typedef struct _MD_PROGRESS {
    int *rows;
    long nRows;
    bool is_tty;
} MD_PROGRESS;

static MD_PROGRESS g_md_prog = {0};

curl_off_t g_md_bytes_total = 0;
curl_off_t g_md_bytes_done = 0;
int g_md_pkgs_total = 0;
int g_md_pkgs_done = 0;
int g_md_pkgs_cached = 0;

static int
md_alloc_row(void)
{
    for(int i = 0; i < g_md_prog.nRows; ++i)
    {
        if(!g_md_prog.rows[i])
        {
            g_md_prog.rows[i] = 1;
            return i;
        }
    }
    return -1;
}

static void
md_release_row(int row)
{
    if(row >= 0 && row < g_md_prog.nRows)
    {
        g_md_prog.rows[row] = 0;
    }
}


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
    char *pszSpeed = NULL;
    const int barw = 50;
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
        if(pData->pBytesDone)
        {
            *pData->pBytesDone += (dlNow - pData->last_bytes);
        }
        pData->last_bytes = dlNow;
    }
    else
    {
        pData->prev_time = 0;
        dPercent = 100;
        if(pData->pBytesDone)
        {
            *pData->pBytesDone += (dlTotal - pData->last_bytes);
        }
    }

    if (!g_md_prog.is_tty)
    {
        if (TDNFUtilsFormatSpeed(speed, &pszSpeed) != 0)
        {
            pszSpeed = NULL;
        }
        pr_info("%s %u%% %ld %s %ld\n",
                pData->pszData,
                dPercent,
                dlNow,
                pszSpeed ? pszSpeed : "0 b/s",
                (long)eta);
        TDNF_SAFE_FREE_MEMORY(pszSpeed);
    }
    else
    {
        if(pData->row < 0)
            pData->row = md_alloc_row();
        char bar[barw + 1];
        int filled = (dPercent * barw) / 100;
        memset(bar, '#', filled);
        memset(bar + filled, ' ', barw - filled);
        bar[barw] = '\0';
        int up = g_md_prog.nRows - 1 - pData->row;
        if(up > 0) 
            printf("\033[%dA", up);
        
        printf("\r");

        if (TDNFUtilsFormatSpeed(speed, &pszSpeed) != 0)
        {
            pszSpeed = NULL;
        }
        
        if (GlobalGetColor())
        {
            pr_info("%-20s " TDNF_COLOR_GREEN "[%s]" TDNF_COLOR_RESET " %3u%% %s %ld",
                    pData->pszData,
                    bar,
                    dPercent,
                    pszSpeed ? pszSpeed : "0 b/s",
                    (long)eta);
        }
        else
        {
            pr_info("%-20s [%s] %3u%% %s %ld",
                    pData->pszData,
                    bar,
                    dPercent,
                    pszSpeed ? pszSpeed : "0 b/s",
                    (long)eta);
        }
        TDNF_SAFE_FREE_MEMORY(pszSpeed);
        printf("\033[K");
        if(up > 0)
            printf("\033[%dB", up);
    }


        /* overall progress bar at row 0 */
        if(g_md_pkgs_total > 0)
        {
            int top = g_md_prog.nRows - 1;
            printf("\033[%dA\r", top);
            uint32_t pct = (g_md_bytes_total > 0) ?
                (uint32_t)(((double)g_md_bytes_done / (double)g_md_bytes_total) * 100.0) : 0;
            char gbar[barw + 1];
            int gfilled = (pct * barw) / 100;
            memset(gbar, '#', gfilled);
            memset(gbar + gfilled, ' ', barw - gfilled);
            gbar[barw] = '\0';
            if (GlobalGetColor())
            {
                if(g_md_pkgs_cached > 0)
                {
                    pr_info("%-20s " TDNF_COLOR_GREEN "[%s]" TDNF_COLOR_RESET " %3u%% (%d/%d packages) (%d cached)",
                            "",
                            gbar,
                            pct,
                            g_md_pkgs_done,
                            g_md_pkgs_total,
                            g_md_pkgs_cached);
                }
                else
                {
                    pr_info("%-20s " TDNF_COLOR_GREEN "[%s]" TDNF_COLOR_RESET " %3u%% (%d/%d packages)",
                            "",
                            gbar,
                            pct,
                            g_md_pkgs_done,
                            g_md_pkgs_total);
                }
            }
            else
            {
                if(g_md_pkgs_cached > 0)
                {
                    pr_info("%-20s [%s] %3u%% (%d/%d packages) (%d cached)",
                            "",
                            gbar,
                            pct,
                            g_md_pkgs_done,
                            g_md_pkgs_total,
                            g_md_pkgs_cached);
                }
                else
                {
                    pr_info("%-20s [%s] %3u%% (%d/%d packages)",
                            "",
                            gbar,
                            pct,
                            g_md_pkgs_done,
                            g_md_pkgs_total);
                }
            }
            printf("\033[K");
            printf("\033[%dB", top);
        }
    
    fflush(stdout);

    return 0;
}

static uint32_t
md_set_progress_cb(
    CURL *pCurl,
    pcb_data *pData,
    const char *pszData
    )
{
    uint32_t dwError = 0;

    if(!pCurl || !pData || IsNullOrEmptyString(pszData))
    {
        dwError = ERROR_TDNF_INVALID_PARAMETER;
        BAIL_ON_TDNF_ERROR(dwError);
    }

    /* retain pBytesTotal/pBytesDone as they may be pre-set by the caller */
    curl_off_t *pBytesTotal = pData->pBytesTotal;
    curl_off_t *pBytesDone = pData->pBytesDone;
    /* reset all other fields without clobbering the counters */
    pData->cur_time = 0;
    pData->prev_time = 0;
    pData->last_bytes = 0;
    pData->last_time = 0;
    pData->ema_speed = 0.0;
    memset(pData->pszData, 0, sizeof(pData->pszData));
    pData->pBytesTotal = pBytesTotal;
    pData->pBytesDone = pBytesDone;
    pData->row = -1;
    strncpy(pData->pszData, pszData, sizeof(pData->pszData) - 1);

    dwError = curl_easy_setopt(pCurl, CURLOPT_XFERINFOFUNCTION, progress_cb);
    BAIL_ON_TDNF_CURL_ERROR(dwError);

    dwError = curl_easy_setopt(pCurl, CURLOPT_XFERINFODATA, pData);
    BAIL_ON_TDNF_CURL_ERROR(dwError);

    dwError = curl_easy_setopt(pCurl, CURLOPT_NOPROGRESS, 0L);
    BAIL_ON_TDNF_CURL_ERROR(dwError);

cleanup:
    return dwError;

error:
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
    g_md_prog.is_tty = isatty(STDOUT_FILENO);
    g_md_prog.nRows = g_nMax + 3; /* overall progress + spacer + pkg rows */
    if(g_md_prog.is_tty)
    {
        g_md_prog.rows = calloc(g_md_prog.nRows, sizeof(int));
        /* reserve first row for overall progress and spacer line */
        if(g_md_prog.rows) {
            g_md_prog.rows[0] = 1;
            if(g_md_prog.nRows > 1)
                g_md_prog.rows[1] = 1;
        }
        for(int i = 0; i < g_md_prog.nRows; ++i)
            printf("\n");
        fflush(stdout);
    }
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
    if(!h)
        return;

    if(h->pCurl)
    {
        /*
         * Explicitly disable callbacks before cleaning up the handle. This
         * prevents libcurl from invoking progress callbacks with memory that
         * has already been released.
         */
        curl_easy_setopt(h->pCurl, CURLOPT_XFERINFOFUNCTION, NULL);
        curl_easy_setopt(h->pCurl, CURLOPT_XFERINFODATA, NULL);
        curl_easy_setopt(h->pCurl, CURLOPT_NOPROGRESS, 1L);
        curl_easy_cleanup(h->pCurl);
    }

    if(h->fp)
        fclose(h->fp);

    if(h->pszTmp)
        free(h->pszTmp);

    if(h->pszDest)
        free(h->pszDest);

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

    h->cb.row = -1;
    h->cb.pBytesTotal = &g_md_bytes_total;
    h->cb.pBytesDone = &g_md_bytes_done;
    if(pszProgress)
        strncpy(h->cb.pszData, pszProgress, sizeof(h->cb.pszData)-1);

    curl_easy_setopt(pCurl, CURLOPT_WRITEDATA, fp);
    md_set_progress_cb(pCurl, &h->cb, pszProgress ? pszProgress : "");
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
                    if(msg->data.result == CURLE_OK) {
                        rename(h->pszTmp, h->pszDest);
                        g_md_pkgs_cached++;
                    }
                    if(g_md_prog.is_tty && h->cb.row >= 0)
                    {
                        int up = g_md_prog.nRows - 1 - h->cb.row;
                        if(up > 0)
                            printf("\033[%dA", up);
                        printf("\033[2K");
                        if(up > 0)
                            printf("\033[%dB", up);
                        fflush(stdout);
                        md_release_row(h->cb.row);
                    }
                    g_md_pkgs_done++;
                    /*
                     * Make sure libcurl stops invoking callbacks before we
                     * remove and free the handle. Without this, libcurl may
                     * attempt to call the progress callback after the
                     * associated memory has been released when the package
                     * comes from cache.
                     */
                    curl_easy_setopt(msg->easy_handle, CURLOPT_XFERINFOFUNCTION, NULL);
                    curl_easy_setopt(msg->easy_handle, CURLOPT_XFERINFODATA, NULL);
                    curl_easy_setopt(msg->easy_handle, CURLOPT_NOPROGRESS, 1L);
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
                if(msg->data.result == CURLE_OK) {
                    rename(h->pszTmp, h->pszDest);
                    g_md_pkgs_cached++;
                }
                if(g_md_prog.is_tty && h->cb.row >= 0)
                {
                    int up = g_md_prog.nRows - 1 - h->cb.row;
                    if(up > 0)
                        printf("\033[%dA", up);
                    printf("\033[2K");
                    if(up > 0)
                        printf("\033[%dB", up);
                    fflush(stdout);
                    md_release_row(h->cb.row);
                }
                g_md_pkgs_done++;
                /*
                 * Disable callbacks before removing the handle to avoid
                 * libcurl accessing freed memory if it tries to report
                 * progress after completion.
                 */
                curl_easy_setopt(msg->easy_handle, CURLOPT_XFERINFOFUNCTION, NULL);
                curl_easy_setopt(msg->easy_handle, CURLOPT_XFERINFODATA, NULL);
                curl_easy_setopt(msg->easy_handle, CURLOPT_NOPROGRESS, 1L);
                curl_multi_remove_handle(g_pMulti, msg->easy_handle);
                free_handle(h);
            }
        }
    }
    curl_multi_cleanup(g_pMulti);
    g_pMulti = NULL;
    if(g_md_prog.rows)
    {
        free(g_md_prog.rows);
        g_md_prog.rows = NULL;
    }
    g_md_prog.nRows = 0;
    g_md_prog.is_tty = false;
    g_md_bytes_total = 0;
    g_md_bytes_done = 0;
    g_md_pkgs_total = 0;
    g_md_pkgs_done = 0;
    g_md_pkgs_cached = 0;
    return 0;
}
