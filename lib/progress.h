#ifndef TDNF_PROGRESS_H
#define TDNF_PROGRESS_H

#include <time.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct progress_ctx
{
    const char *label;           /* text label for progress */
    unsigned long total;         /* total amount of work if known */
    unsigned long current;       /* current progress */
    struct timespec start_time;  /* when progress started */
    struct timespec last_time;   /* last update time (for speed calc) */
    struct timespec last_render; /* last render time (for rate limit) */
    unsigned long last_bytes;    /* last bytes processed */
    double ema_speed;            /* exponential moving avg speed */
    int rate_ms;                 /* min interval between renders */
    bool use_ncurses;            /* use curses bottom-line rendering */
    bool active;                 /* curses initialized */
} progress_ctx_t;

int prog_init(progress_ctx_t *ctx, const char *label,
              unsigned long total, bool use_ncurses);
int prog_update(progress_ctx_t *ctx, unsigned long current);
void prog_finish(progress_ctx_t *ctx);

#ifdef __cplusplus
}
#endif

#endif /* TDNF_PROGRESS_H */
