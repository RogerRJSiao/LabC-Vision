// tests/*：真正的 unit test，不像 deployments/trial/test_math_formulas.c 只靠印出結果讓人眼判讀
// 這裡每一筆案例都用 TEST_ASSERT 自動比對「預期值」與「實際值」
// 結束時回報 pass/fail 統計，並用 exit code 讓 CI 判斷（0 = 全過，非 0 = 有失敗）
#include <stdio.h>
#include <windows.h>
#include "math/math_formulas.h"

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
    int64_t result;
    int32_t table[100]; // 最大 10x10
    int32_t status, i, j, n;

    // 強制輸出UTF-8
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    // ---- lv_factorial ----
    status = lv_factorial(5, &result);
    TEST_ASSERT(status == LV_OK && result == 120, "5! = 120 (LV_OK)");

    status = lv_factorial(0, &result);
    TEST_ASSERT(status == LV_OK && result == 1, "0! = 1 (LV_OK)");

    status = lv_factorial(-1, &result);
    TEST_ASSERT(status == LV_ERR_NEGATIVE, "(-1)! 應回傳 LV_ERR_NEGATIVE");

    status = lv_factorial(21, &result);
    TEST_ASSERT(status == LV_ERR_OVERFLOW, "21! 應超出 int64_t 上限，回傳 LV_ERR_OVERFLOW");

    status = lv_factorial(5, NULL);
    TEST_ASSERT(status == LV_ERR_BUFFER, "out_result 為 NULL 應回傳 LV_ERR_BUFFER");

    // ---- lv_fibonacci ----
    status = lv_fibonacci(10, &result);
    TEST_ASSERT(status == LV_OK && result == 55, "fib(10) = 55 (LV_OK)");

    status = lv_fibonacci(0, &result);
    TEST_ASSERT(status == LV_OK && result == 0, "fib(0) = 0 (LV_OK)");

    status = lv_fibonacci(-1, &result);
    TEST_ASSERT(status == LV_ERR_NEGATIVE, "fib(-1) 應回傳 LV_ERR_NEGATIVE");

    status = lv_fibonacci(10, NULL);
    TEST_ASSERT(status == LV_ERR_BUFFER, "out_result 為 NULL 應回傳 LV_ERR_BUFFER");

    // ---- lv_multiplication_table ----
    n = 9;
    status = lv_multiplication_table(n, table);
    TEST_ASSERT(status == LV_OK, "multiplication_table(n=9) 應回傳 LV_OK");
    {
        int ok = 1;
        for (i = 1; i <= n && ok; i++)
        {
            for (j = 1; j <= n; j++)
            {
                if (table[(i - 1) * n + (j - 1)] != i * j)
                {
                    ok = 0;
                    break;
                }
            }
        }
        TEST_ASSERT(ok, "multiplication_table(n=9) 內容應符合 i*j");
    }

    status = lv_multiplication_table(10, table);
    TEST_ASSERT(status == LV_OK, "multiplication_table(n=10) 應回傳 LV_OK（上限邊界）");

    status = lv_multiplication_table(11, table);
    TEST_ASSERT(status == LV_ERR_BUFFER, "multiplication_table(n=11) 超過上限，應回傳 LV_ERR_BUFFER");

    status = lv_multiplication_table(0, table);
    TEST_ASSERT(status == LV_ERR_BUFFER, "multiplication_table(n=0) 應回傳 LV_ERR_BUFFER");

    status = lv_multiplication_table(-1, table);
    TEST_ASSERT(status == LV_ERR_NEGATIVE, "multiplication_table(n=-1) 應回傳 LV_ERR_NEGATIVE");

    status = lv_multiplication_table(9, NULL);
    TEST_ASSERT(status == LV_ERR_BUFFER, "out_table 為 NULL 應回傳 LV_ERR_BUFFER");

    // ---- 統計結果 ----
    printf("\n%d passed, %d failed\n", pass_count, fail_count);

    return fail_count == 0 ? 0 : 1;
}
