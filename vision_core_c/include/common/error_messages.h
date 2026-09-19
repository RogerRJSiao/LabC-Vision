#ifndef ERROR_MESSAGES_H
#define ERROR_MESSAGES_H

#include <stdint.h>

// 沿用其他模組慣例（見 math/math_formulas.h）：int32_t/__cdecl 對應 LabVIEW CLFN、
// typedef enum 定義狀態碼、0=成功/負數=錯誤(NI保留範圍 -8999~-8000)
// 這裡從 -8900 起跳，跟 math(-8000)/barcode_validate(-8100)/barcode_decode(-8200) 錯開

// 查詢「訊息本身」失敗時用的狀態碼（跟被查的 code 是否為錯誤無關）
typedef enum
{
    LV_EM_OK             = 0,
    LV_EM_ERR_UNKNOWN    = -8900,  // 內部預設值，代表「尚未判定成功與否」
    LV_EM_ERR_BUFFER     = -8901,  // out_message 是 NULL，或 out_message_size <= 0
    LV_EM_ERR_TRUNCATED  = -8902   // 訊息長度超過 out_message_size，已截斷寫入
} ErrorMessageStatus;

// 單一事實來源：把任一模組的狀態碼轉成常數名稱字串（例如 LV_BC_ERR_CHECKSUM -> "LV_BC_ERR_CHECKSUM"）
// 新模組加錯誤碼時，要同步更新 error_messages.c 的對照表
//
// code:             任一模組的狀態碼；查無對應時回傳 "UNKNOWN_ERROR_CODE(數值)"
// out_message:      輸出緩衝區
// out_message_size: 緩衝區大小（bytes，含結尾 '\0'）
// 回傳值：LV_EM_OK 成功；LV_EM_ERR_BUFFER / LV_EM_ERR_TRUNCATED 為本函式自身錯誤
__declspec(dllexport) int32_t __cdecl lv_get_error_message(
    int32_t code, char *out_message, int32_t out_message_size);

#endif
