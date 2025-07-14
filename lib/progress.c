#include "progress.h"
#include <stdio.h>
#include <string.h>
#include <curses.h>
#include <sys/time.h>
#include <unistd.h>

#ifndef PROG_RATE_MS
#define PROG_RATE_MS 100
#endif

static double timespec_diff(const struct timespec *a, const struct timespec *b)
{
    return (a->tv_sec - b->tv_sec) +
           (a->tv_nsec - b->tv_nsec)/1e9;
}

int prog_init(progress_ctx_t *ctx, const char *label,
              unsigned long total, bool use_ncurses)
{
    if(!ctx) return -1;
    memset(ctx, 0, sizeof(*ctx));
    clock_gettime(CLOCK_MONOTONIC, &ctx->start_time);
    ctx->last_time = ctx->start_time;
    ctx->last_render = ctx->start_time;
    ctx->label = label ? label : "";
    ctx->total = total;
    ctx->rate_ms = PROG_RATE_MS;
    ctx->use_ncurses = use_ncurses && isatty(STDOUT_FILENO);
    if(ctx->use_ncurses)
    {
        initscr();
        cbreak();
        noecho();
        curs_set(0);
        ctx->active = true;
    }
    return 0;
}

int prog_update(progress_ctx_t *ctx, unsigned long current)
{
    if(!ctx || (!ctx->active && ctx->use_ncurses)) return -1;
    struct timespec now;
    clock_gettime(CLOCK_MONOTONIC, &now);

    double since_render = timespec_diff(&now, &ctx->last_render) * 1000.0;
    if(since_render < ctx->rate_ms && current < ctx->total)
        return 0;

    ctx->current = current;
    double dt = timespec_diff(&now, &ctx->last_time);
    if(dt > 0)
    {
        double cur_speed = (double)(current - ctx->last_bytes) / dt;
        if(ctx->ema_speed == 0.0)
            ctx->ema_speed = cur_speed;
        else
            ctx->ema_speed = ctx->ema_speed*0.7 + cur_speed*0.3;
    }
    ctx->last_bytes = current;
    ctx->last_time = now;
    ctx->last_render = now;

    unsigned percent = 0;
    if(ctx->total > 0)
        percent = (unsigned)((double)current * 100.0 / ctx->total);

    double eta = -1.0;
    if(ctx->total > 0 && ctx->ema_speed > 0.0)
        eta = (double)(ctx->total - current) / ctx->ema_speed;

    const int barw = 50;
    char bar[barw + 1];
    int filled = (percent * barw) / 100;
    memset(bar, '#', filled);
    memset(bar + filled, ' ', barw - filled);
    bar[barw] = '\0';

    if(ctx->use_ncurses)
    {
        int rows, cols;
        getmaxyx(stdscr, rows, cols);
        (void)cols;
        mvprintw(rows-1, 0, "%-20s [%s] %3u%% %lds", ctx->label, bar, percent, eta>0? (long)eta : 0L);
        clrtoeol();
        refresh();
    }
    else
    {
        printf("\r%-20s [%s] %3u%%", ctx->label, bar, percent);
        if(eta >= 0)
            printf(" %lds", (long)eta);
        fflush(stdout);
    }

    return 0;
}

void prog_finish(progress_ctx_t *ctx)
{
    if(!ctx) return;
    if(ctx->total > 0)
        prog_update(ctx, ctx->total);
    if(ctx->use_ncurses && ctx->active)
    {
        int rows, cols;
        getmaxyx(stdscr, rows, cols);
        (void)cols;
        move(rows-1, 0);
        clrtoeol();
        refresh();
        endwin();
        ctx->active = false;
    }
    else
    {
        printf("\n");
    }
}
