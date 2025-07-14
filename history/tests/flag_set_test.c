#include <stdio.h>
#include <sqlite3.h>
#include "../history.h"

int main(void)
{
    struct history_ctx *ctx;
    int rc;
    sqlite3_stmt *stmt = NULL;
    int count = -1;
    int value = -1;

    ctx = create_history_ctx(":memory:");
    if (!ctx)
        return 1;

    rc = history_record_state(ctx); /* ensure tables exist */
    if (rc)
        goto error;

    rc = history_add_transaction(ctx, "test");
    if (rc)
        goto error;

    rc = history_set_auto_flag(ctx, "flag", 1);
    if (rc)
        goto error;
    rc = history_set_auto_flag(ctx, "flag", 0);
    if (rc)
        goto error;
    rc = history_set_auto_flag(ctx, "flag", 1);
    if (rc)
        goto error;

    rc = sqlite3_prepare_v2(ctx->db,
            "SELECT COUNT(*) FROM flag_set;",
            -1, &stmt, NULL);
    if (rc != SQLITE_OK)
        goto error;
    if (sqlite3_step(stmt) == SQLITE_ROW)
        count = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt); stmt = NULL;

    rc = sqlite3_prepare_v2(ctx->db,
            "SELECT value FROM flag_set LIMIT 1;",
            -1, &stmt, NULL);
    if (rc != SQLITE_OK)
        goto error;
    if (sqlite3_step(stmt) == SQLITE_ROW)
        value = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt); stmt = NULL;

    destroy_history_ctx(ctx);

    if (count == 1 && value == 1)
        return 0;

error:
    if (stmt)
        sqlite3_finalize(stmt);
    destroy_history_ctx(ctx);
    fprintf(stderr, "Test failed\n");
    return 1;
}
