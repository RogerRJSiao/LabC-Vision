#include "math/math_formulas.h"

// 這個程式將與 labview 對接時的要點
// 1. 使用 int32_t 解決跨平台位元數預設問題
//    - 不同編譯器有相同的 int32_t 位元數處理
//    - 檢查 stdint.h，可得知每種編譯器實際引用的原生型別引用
//    - 避免資料讀取出現錯亂，要用 int32_t、int64_t 定義變數
// 2. 函數加上 __cdecl 滿足 32-bit 特有的呼叫慣例
//    - 理論上 gcc/MSVC 在 32-bit x86 下，函式預設是 __cdecl
//    - Labview CLFN 只有兩個選項，C 對應 __cdecl，stdcall (WINAPI) 對應 __stdcall
//    - 32-bit 特有的呼叫慣例：引數如何傳遞、堆疊由誰清理、加下底線_或後綴@

int32_t __cdecl lv_factorial(int32_t n, int64_t *out_result)
{
    // 預設不成功，明確執行才成功
    LvStatus status = LV_ERR_UNKNOWN;
    int64_t result;
    int32_t i;

    // 檢查有無指向任何有效記憶體
    if (out_result == NULL)
    {
        return LV_ERR_BUFFER;
    }
    // 輸入負數，不符合階乘的數學定義
    if (n < 0)
    {
        return LV_ERR_NEGATIVE;
    }

    result = 1;
    for (i = 2; i <= n; i++)
    {
        // 檢查 result * i 是否超出 int64_t 上限(溢位)
        if (result > INT64_MAX / i)
        {
            status = LV_ERR_OVERFLOW;
            break;
        }
        result *= i;
    }

    if (status == LV_ERR_OVERFLOW)
    {
        return status;
    }

    // 迴圈正常跑完、沒有溢位，才標記為成功
    status = LV_OK;
    *out_result = result;
    return status;
}

int32_t __cdecl lv_fibonacci(int32_t n, int64_t *out_result)
{
    LvStatus status = LV_ERR_UNKNOWN;
    int64_t a = 0, b = 1, tmp;
    int32_t i;

    // 檢查有無指向任何有效記憶體
    if (out_result == NULL)
    {
        return LV_ERR_BUFFER;
    }
    // 輸入負數，不符合費氏數列的定義
    if (n < 0)
    {
        return LV_ERR_NEGATIVE;
    }

    for (i = 0; i < n; i++)
    {
        tmp = b;
        if (i < n - 1)
        {
            // 檢查 a + b 是否超出 int64_t 上限(溢位)
            if (a > INT64_MAX - b)
            {
                status = LV_ERR_OVERFLOW;
                break;
            }
            b = a + b;
        }
        a = tmp;
    }

    if (status == LV_ERR_OVERFLOW)
    {
        return status;
    }

    // 迴圈正常跑完、沒有溢位，才標記為成功
    status = LV_OK;
    *out_result = a;
    return status;
}

int32_t __cdecl lv_multiplication_table(int32_t *out_table, int32_t buf_len)
{
    int32_t i, j;

    // 檢查有無指向任何有效記憶體
    if (out_table == NULL)
    {
        return LV_ERR_BUFFER;
    }
    if (buf_len < 81)
    {
        return LV_ERR_BUFFER;
    }

    for (i = 1; i <= 9; i++)
    {
        for (j = 1; j <= 9; j++)
        {
            out_table[(i - 1) * 9 + (j - 1)] = i * j;
        }
    }

    return LV_OK;
}
