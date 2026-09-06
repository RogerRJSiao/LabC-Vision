// tests/*：真正的 unit test，每一筆案例都用 TEST_ASSERT 自動比對「預期值」與「實際值」
// 結束時回報 pass/fail 統計，並用 exit code 讓 CI 判斷（0 = 全過，非 0 = 有失敗）
//
// 這支測試串起「解碼 -> 驗證」的完整流程：
// lv_decode_barcode_from_file 從圖檔解出條碼字串後，直接丟給 barcode_validate.h 的函式驗證格式/檢查碼
#include <stdio.h>
#include <windows.h>
#include "barcode/barcode_decode.h"
#include "barcode/barcode_validate.h"

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
    int32_t status;
    char code[64];

    // 強制輸出UTF-8
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    // ---- 正常路徑：解碼 + 驗證串接 ----
    status = lv_decode_barcode_from_file("fixtures/ean13_sample.png", code, sizeof(code));
    TEST_ASSERT(status == LV_BD_OK, "ean13_sample.png 應成功解出條碼 (LV_BD_OK)");
    printf("  解出字串: %s\n", code);

    status = lv_validate_ean(code);
    TEST_ASSERT(status == LV_BC_OK, "解碼出的字串丟給 lv_validate_ean 應通過驗證 (LV_BC_OK)");

    // ---- 緩衝區太小：故意給一個裝不下的 out_code_size ----
    status = lv_decode_barcode_from_file("fixtures/ean13_sample.png", code, 3);
    TEST_ASSERT(status == LV_BD_ERR_OUT_TOO_SMALL, "out_code_size 太小應回傳 LV_BD_ERR_OUT_TOO_SMALL");

    // ---- 檔案不存在 ----
    status = lv_decode_barcode_from_file("fixtures/not_exist.png", code, sizeof(code));
    TEST_ASSERT(status == LV_BD_ERR_FILE_OPEN, "檔案不存在應回傳 LV_BD_ERR_FILE_OPEN");

    // ---- 指標檢查 ----
    status = lv_decode_barcode_from_file(NULL, code, sizeof(code));
    TEST_ASSERT(status == LV_BD_ERR_BUFFER, "image_path 為 NULL 應回傳 LV_BD_ERR_BUFFER");

    status = lv_decode_barcode_from_file("fixtures/ean13_sample.png", NULL, sizeof(code));
    TEST_ASSERT(status == LV_BD_ERR_BUFFER, "out_code 為 NULL 應回傳 LV_BD_ERR_BUFFER");

    // ---- 統計結果 ----
    printf("\n%d passed, %d failed\n", pass_count, fail_count);

    return fail_count == 0 ? 0 : 1;
}
