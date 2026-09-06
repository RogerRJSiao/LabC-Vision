#ifndef BARCODE_VALIDATE_H
#define BARCODE_VALIDATE_H

#include <stdint.h>

// 沿用 math/math_formulas.h 的設計慣例
// 1. int32_t / __cdecl：跨平台位元數一致、符合 LabVIEW CLFN 的呼叫慣例
// 2. 用 typedef enum 定義狀態碼，不用 #define，把相關常數綁在同一個型別
// 3. 0 = 成功，負數 = 錯誤，落在 NI 保留給自訂錯誤碼的範圍 (-8999 ~ -8000)

typedef enum
{
    LV_BC_OK           = 0,
    LV_BC_ERR_UNKNOWN  = -8100,  // 內部預設值，代表「尚未判定成功與否」
    LV_BC_ERR_BUFFER   = -8101,  // 傳入的指標是 NULL
    LV_BC_ERR_FORMAT   = -8102,  // 格式不符：長度不對、含有不合法字元、開頭字元錯誤等
    LV_BC_ERR_CHECKSUM = -8103   // 格式正確，但檢查碼(驗證碼)比對不過
} BarcodeStatus;

// UPC-A：12 碼純數字，第 12 碼為檢查碼
__declspec(dllexport) int32_t __cdecl lv_validate_upc(const char *code);

// EAN 條碼：8 碼(EAN-8) 或 13 碼(EAN-13) 純數字，最後一碼為檢查碼
__declspec(dllexport) int32_t __cdecl lv_validate_ean(const char *code);

// 台灣電子發票手機條碼載具：格式：8 碼字元，第 1 碼固定為 '/'，其餘 7 碼取自 [0-9 A-Z + - .] 共 39 個字元
__declspec(dllexport) int32_t __cdecl lv_validate_tw_carrier(const char *code);

// 台灣身分證字號：1 碼大寫英文字母(縣市別) + 9 碼數字(性別碼 1/2 + 7 碼流水號 + 檢查碼)
__declspec(dllexport) int32_t __cdecl lv_validate_tw_id(const char *code);

#endif
