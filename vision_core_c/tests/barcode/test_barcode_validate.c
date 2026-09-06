// tests/*：真正的 unit test，每一筆案例都用 TEST_ASSERT 自動比對「預期值」與「實際值」
// 結束時回報 pass/fail 統計，並用 exit code 讓 CI 判斷（0 = 全過，非 0 = 有失敗）
#include <stdio.h>
#include <windows.h>
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

    // 強制輸出UTF-8
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    // ---- lv_validate_upc ----
    status = lv_validate_upc("036000291452"); // 真實存在的 UPC-A (Kleenex)，檢查碼正確
    TEST_ASSERT(status == LV_BC_OK, "UPC-A 036000291452 應通過驗證 (LV_BC_OK)");

    status = lv_validate_upc("036000291459"); // 故意改錯檢查碼
    TEST_ASSERT(status == LV_BC_ERR_CHECKSUM, "UPC-A 檢查碼錯誤應回傳 LV_BC_ERR_CHECKSUM");

    status = lv_validate_upc("12345"); // 長度不對
    TEST_ASSERT(status == LV_BC_ERR_FORMAT, "UPC-A 長度不足應回傳 LV_BC_ERR_FORMAT");

    status = lv_validate_upc("03600029145A"); // 含非數字字元
    TEST_ASSERT(status == LV_BC_ERR_FORMAT, "UPC-A 含非數字字元應回傳 LV_BC_ERR_FORMAT");

    status = lv_validate_upc(NULL);
    TEST_ASSERT(status == LV_BC_ERR_BUFFER, "code 為 NULL 應回傳 LV_BC_ERR_BUFFER");

    // ---- lv_validate_ean ----
    status = lv_validate_ean("4006381333931"); // 真實存在的 EAN-13
    TEST_ASSERT(status == LV_BC_OK, "EAN-13 4006381333931 應通過驗證 (LV_BC_OK)");

    status = lv_validate_ean("4006381333930"); // 故意改錯檢查碼
    TEST_ASSERT(status == LV_BC_ERR_CHECKSUM, "EAN-13 檢查碼錯誤應回傳 LV_BC_ERR_CHECKSUM");

    status = lv_validate_ean("96385074"); // 真實存在的 EAN-8
    TEST_ASSERT(status == LV_BC_OK, "EAN-8 96385074 應通過驗證 (LV_BC_OK)");

    status = lv_validate_ean("123456"); // 長度不對（不是 8 也不是 13）
    TEST_ASSERT(status == LV_BC_ERR_FORMAT, "EAN 長度不合法應回傳 LV_BC_ERR_FORMAT");

    status = lv_validate_ean(NULL);
    TEST_ASSERT(status == LV_BC_ERR_BUFFER, "code 為 NULL 應回傳 LV_BC_ERR_BUFFER");

    // ---- lv_validate_tw_carrier ----
    status = lv_validate_tw_carrier("/ABC123."); // 格式正確
    TEST_ASSERT(status == LV_BC_OK, "手機條碼格式正確應回傳 LV_BC_OK");

    status = lv_validate_tw_carrier("ABC1234."); // 開頭不是 '/'
    TEST_ASSERT(status == LV_BC_ERR_FORMAT, "手機條碼開頭非 '/' 應回傳 LV_BC_ERR_FORMAT");

    status = lv_validate_tw_carrier("/ABC12"); // 長度不足
    TEST_ASSERT(status == LV_BC_ERR_FORMAT, "手機條碼長度不足應回傳 LV_BC_ERR_FORMAT");

    status = lv_validate_tw_carrier("/abc123."); // 小寫字母不在允許字元集內
    TEST_ASSERT(status == LV_BC_ERR_FORMAT, "手機條碼含小寫字母應回傳 LV_BC_ERR_FORMAT");

    status = lv_validate_tw_carrier(NULL);
    TEST_ASSERT(status == LV_BC_ERR_BUFFER, "code 為 NULL 應回傳 LV_BC_ERR_BUFFER");

    // ---- lv_validate_tw_id ----
    status = lv_validate_tw_id("A123456789"); // 真實存在、常見的合法範例身分證字號
    TEST_ASSERT(status == LV_BC_OK, "身分證字號 A123456789 應通過驗證 (LV_BC_OK)");

    status = lv_validate_tw_id("A123456780"); // 故意改錯檢查碼
    TEST_ASSERT(status == LV_BC_ERR_CHECKSUM, "身分證字號檢查碼錯誤應回傳 LV_BC_ERR_CHECKSUM");

    status = lv_validate_tw_id("112345678A"); // 開頭不是英文字母
    TEST_ASSERT(status == LV_BC_ERR_FORMAT, "身分證字號開頭非英文字母應回傳 LV_BC_ERR_FORMAT");

    status = lv_validate_tw_id("A12345678"); // 長度不足
    TEST_ASSERT(status == LV_BC_ERR_FORMAT, "身分證字號長度不足應回傳 LV_BC_ERR_FORMAT");

    status = lv_validate_tw_id("A323456789"); // 第 2 碼(性別碼)不是 1 或 2
    TEST_ASSERT(status == LV_BC_ERR_FORMAT, "身分證字號性別碼不是 1/2 應回傳 LV_BC_ERR_FORMAT");

    status = lv_validate_tw_id(NULL);
    TEST_ASSERT(status == LV_BC_ERR_BUFFER, "code 為 NULL 應回傳 LV_BC_ERR_BUFFER");

    // ---- 統計結果 ----
    printf("\n%d passed, %d failed\n", pass_count, fail_count);

    return fail_count == 0 ? 0 : 1;
}
