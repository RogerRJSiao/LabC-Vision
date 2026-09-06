#ifndef BARCODE_DECODE_H
#define BARCODE_DECODE_H

#include <stdint.h>

// 沿用 barcode_validate.h 的設計慣例
// 1. int32_t / __cdecl：跨平台位元數一致、符合 LabVIEW CLFN 的呼叫慣例
// 2. 用 typedef enum 定義狀態碼，不用 #define，把相關常數綁在同一個型別
// 3. 0 = 成功，負數 = 錯誤，落在 NI 保留給自訂錯誤碼的範圍 (-8999 ~ -8000)
//    這裡從 -8200 起跳，跟 barcode_validate.h 的 -8100 系列錯開，避免兩個模組的錯誤碼混在一起看不出來源

// 底層實作：stb_image（vendor 在 external/stb/）負責把影像檔案(.bmp/.jpg/.png...)讀成灰階像素，
// 再交給 zbar（vendor 在 external/zbar/，32-bit 預編譯版）掃描解碼，取第一個偵測到的條碼字串

typedef enum
{
    LV_BD_OK                = 0,
    LV_BD_ERR_UNKNOWN       = -8200,  // 內部預設值，代表「尚未判定成功與否」
    LV_BD_ERR_BUFFER        = -8201,  // image_path 或 out_code 是 NULL
    LV_BD_ERR_FILE_OPEN     = -8202,  // 檔案不存在，或不是 stb_image 支援的影像格式
    LV_BD_ERR_NOT_FOUND     = -8203,  // 影像有效，但掃描不到任何條碼
    LV_BD_ERR_OUT_TOO_SMALL = -8204   // 解碼出的字串長度超過 out_code_size，資料被截斷、視為失敗
} BarcodeDecodeStatus;

// 從影像檔案解碼出第一個偵測到的條碼字串（不驗證內容是否合法，只負責「讀出字串」）
// image_path:     影像檔案路徑（呼叫端配置，唯讀），支援 stb_image 能讀的格式（.bmp/.jpg/.png/.gif...）
// out_code:       呼叫端配置的緩衝區，用來接住解碼出的字串（LabVIEW 端配一個夠大的 String 傳入）
// out_code_size:  out_code 緩衝區大小（bytes，須含結尾 '\0' 的空間）
// 解出來的字串可以直接丟給 barcode_validate.h 的 lv_validate_xxx 系列函式做格式/檢查碼驗證
__declspec(dllexport) int32_t __cdecl lv_decode_barcode_from_file(
    const char *image_path, char *out_code, int32_t out_code_size);

#endif
