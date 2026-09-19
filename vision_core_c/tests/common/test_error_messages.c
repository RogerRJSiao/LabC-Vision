#include <stdio.h>
#include <string.h>
#include <windows.h>
#include "common/error_messages.h"
#include "math/math_formulas.h"
#include "barcode/barcode_validate.h"
#include "barcode/barcode_decode.h"

static int pass_count = 0;
static int fail_count = 0;

#define TEST_ASSERT(cond, msg) \
    do { \
        if (cond) { \
            printf("[PASS] %s\n", msg); \
            pass_count++; \
        } else { \
            printf("[FAIL] %s\n", msg); \
            fail_count++; \
        } \
    } while (0)

int main(void)
{
    char buf[128];
    char tiny[4];
    int32_t status;

    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    // ---- 涵蓋各模組的已知錯誤碼 ----
    status = lv_get_error_message(LV_OK, buf, sizeof(buf));
    TEST_ASSERT(status == LV_EM_OK && strlen(buf) > 0, "LV_OK 應查到訊息");

    status = lv_get_error_message(LV_ERR_NEGATIVE, buf, sizeof(buf));
    TEST_ASSERT(status == LV_EM_OK && strlen(buf) > 0, "LV_ERR_NEGATIVE 應查到訊息");

    status = lv_get_error_message(LV_BC_ERR_CHECKSUM, buf, sizeof(buf));
    TEST_ASSERT(status == LV_EM_OK && strlen(buf) > 0, "LV_BC_ERR_CHECKSUM 應查到訊息");

    status = lv_get_error_message(LV_BD_ERR_FILE_OPEN, buf, sizeof(buf));
    TEST_ASSERT(status == LV_EM_OK && strlen(buf) > 0, "LV_BD_ERR_FILE_OPEN 應查到訊息");

    // ---- 未知碼應有 fallback ----
    status = lv_get_error_message(-1, buf, sizeof(buf));
    TEST_ASSERT(status == LV_EM_OK && strstr(buf, "UNKNOWN_ERROR_CODE") != NULL, "未知碼應回傳通用 fallback 訊息");

    // ---- 參數檢查 ----
    status = lv_get_error_message(LV_OK, NULL, sizeof(buf));
    TEST_ASSERT(status == LV_EM_ERR_BUFFER, "out_message 為 NULL 應回傳 LV_EM_ERR_BUFFER");

    status = lv_get_error_message(LV_OK, buf, 0);
    TEST_ASSERT(status == LV_EM_ERR_BUFFER, "out_message_size <= 0 應回傳 LV_EM_ERR_BUFFER");

    // ---- 緩衝區太小應截斷但不溢位 ----
    status = lv_get_error_message(LV_BD_ERR_FILE_OPEN, tiny, sizeof(tiny));
    TEST_ASSERT(status == LV_EM_ERR_TRUNCATED, "緩衝區太小應回傳 LV_EM_ERR_TRUNCATED");
    TEST_ASSERT(strlen(tiny) == sizeof(tiny) - 1, "截斷後仍應以 '\\0' 結尾且不超出緩衝區");

    printf("\n%d passed, %d failed\n", pass_count, fail_count);

    return fail_count == 0 ? 0 : 1;
}
