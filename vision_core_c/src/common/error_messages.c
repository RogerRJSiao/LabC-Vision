#include <string.h>
#include <stdio.h>
#include "common/error_messages.h"
#include "math/math_formulas.h"
#include "barcode/barcode_validate.h"
#include "barcode/barcode_decode.h"

// 用各模組實際的 enum 常數而非魔數，錯誤碼一改這裡就編譯失敗，強迫同步更新
typedef struct
{
    int32_t code;
    const char *message;
} ErrorMessageEntry;

// # 把識別字轉成字串，message 自動跟 code 同名，不用手動打兩次
#define ERROR_ENTRY(x) { (x), #x }

static const ErrorMessageEntry ERROR_MESSAGE_TABLE[] = {
    // math/math_formulas.h
    ERROR_ENTRY(LV_OK),
    ERROR_ENTRY(LV_ERR_UNKNOWN),
    ERROR_ENTRY(LV_ERR_NEGATIVE),
    ERROR_ENTRY(LV_ERR_OVERFLOW),
    ERROR_ENTRY(LV_ERR_BUFFER),

    // barcode/barcode_validate.h
    ERROR_ENTRY(LV_BC_ERR_UNKNOWN),
    ERROR_ENTRY(LV_BC_ERR_BUFFER),
    ERROR_ENTRY(LV_BC_ERR_FORMAT),
    ERROR_ENTRY(LV_BC_ERR_CHECKSUM),

    // barcode/barcode_decode.h
    ERROR_ENTRY(LV_BD_ERR_UNKNOWN),
    ERROR_ENTRY(LV_BD_ERR_BUFFER),
    ERROR_ENTRY(LV_BD_ERR_FILE_OPEN),
    ERROR_ENTRY(LV_BD_ERR_NOT_FOUND),
    ERROR_ENTRY(LV_BD_ERR_OUT_TOO_SMALL),

    // common/error_messages.h（本模組自身的狀態碼）
    ERROR_ENTRY(LV_EM_ERR_UNKNOWN),
    ERROR_ENTRY(LV_EM_ERR_BUFFER),
    ERROR_ENTRY(LV_EM_ERR_TRUNCATED)
};

#undef ERROR_ENTRY

static const int32_t ERROR_MESSAGE_TABLE_COUNT =
    (int32_t)(sizeof(ERROR_MESSAGE_TABLE) / sizeof(ERROR_MESSAGE_TABLE[0]));

// 把訊息字串安全寫入呼叫端緩衝區，並依是否截斷回傳對應狀態
static int32_t copy_message(const char *message, char *out_message, int32_t out_message_size)
{
    size_t message_len = strlen(message);

    if ((int32_t)message_len >= out_message_size)
    {
        // 空間不夠，截斷寫入但保留結尾 '\0'
        memcpy(out_message, message, (size_t)out_message_size - 1);
        out_message[out_message_size - 1] = '\0';
        return LV_EM_ERR_TRUNCATED;
    }

    memcpy(out_message, message, message_len + 1); // +1 一併複製 '\0'
    return LV_EM_OK;
}

int32_t __cdecl lv_get_error_message(int32_t code, char *out_message, int32_t out_message_size)
{
    int32_t i;
    char fallback[64];

    if (out_message == NULL || out_message_size <= 0)
    {
        return LV_EM_ERR_BUFFER;
    }

    for (i = 0; i < ERROR_MESSAGE_TABLE_COUNT; i++)
    {
        if (ERROR_MESSAGE_TABLE[i].code == code)
        {
            return copy_message(ERROR_MESSAGE_TABLE[i].message, out_message, out_message_size);
        }
    }

    // 查無對應碼：回傳通用訊息，呼應各模組既有的 *_ERR_UNKNOWN 慣例
    snprintf(fallback, sizeof(fallback), "UNKNOWN_ERROR_CODE(%d)", code);
    return copy_message(fallback, out_message, out_message_size);
}
