#ifndef MATH_FORMULAS_H     
// 如果 MATH_FORMULAS_H 這個巨集還沒被定義過，就執行這一段
// C 語言的 #include 本質上是把整個檔案內容原封不動貼過來，故要避重複定義的編譯錯誤
#define MATH_FORMULAS_H
// 把 MATH_FORMULAS_H 定義

#include <stdint.h>

// 回傳狀態碼：0 = 成功，負數 = 各種錯誤
// 1. LabVIEW 原生的 Error Cluster 慣例
//    - 負數是錯誤，正數是警告，0是執行成功(無錯誤)
//    - 直接把回傳值放到 error cluster 的 code 欄位，配合內建的 Simple/General Error Handler VI 顯示
// 2. NI 提供自訂錯誤碼的範圍
//    - 自訂錯誤碼：-8999 to -8000, 5000 to 9999, or 500,000 to 599,999
// 3. 定義錯誤常數，不用 #define 是用 typedef enum LvStatus
//    - 把相關常數綁在同一個型別
//    - 宣告變數時寫 LvStatus status = LV_OK;，意圖更清楚
//    - enum 只在你 C 語言這邊內部使用、方便閱讀與除錯，不影響對外的函式簽章
typedef enum
{
    LV_OK           = 0,
    LV_ERR_UNKNOWN  = -8000,  // 內部預設值，代表「尚未判定成功與否」
    LV_ERR_NEGATIVE = -8001,  // n 是負數
    LV_ERR_OVERFLOW = -8002,  // 計算結果超出 int64_t 能表示的範圍
    LV_ERR_BUFFER   = -8003   // 傳入的指標是 NULL，或緩衝區大小不夠
} LvStatus;

// include/*.h 近似於抽象，src/*.c 等同於實作
// 只是這個 include 在 C 語言裡的形式比較單純——單純做到「隱藏怎麼做」，沒有「一個介面多種實作可切換」那麼完整的多型能力。

// __declspec 是微軟編譯器特有語法，GCC 在 Windows（MinGW）底下也支援這個語法
// 這個函式一樣要匯出到 .dll，讓 LabVIEW 可以在 Call Library Function Node 中指定到這個函式

// 階乘：計算 n!，結果寫入 *out_result
__declspec(dllexport) int32_t __cdecl lv_factorial(int32_t n, int64_t *out_result);

// 費氏數列：計算第 n 項（n 從 0 開始，f(0)=0, f(1)=1），結果寫入 *out_result
__declspec(dllexport) int32_t __cdecl lv_fibonacci(int32_t n, int64_t *out_result);

// n x n 乘法表：依 row-major 順序（第1列n個、第2列n個...）填入 out_table
// n 同時代表表格的長與寬（正方形），上限為 10（即最大 10x10）
// out_table 由呼叫端配置，至少要有 n*n 個 int32_t 的空間
__declspec(dllexport) int32_t __cdecl lv_multiplication_table(int32_t n, int32_t *out_table);

#endif
