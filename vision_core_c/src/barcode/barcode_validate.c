#include <string.h>
#include "barcode/barcode_validate.h"

// 台灣身分證字號的縣市別字母 -> 兩位數代碼對照表（A-Z 依序對應）
// 這張表是官方公告、跨系統通用的固定表，同時也是「手機條碼」等其他證號檢查碼會用到的基礎表
static const int32_t TW_ID_LETTER_VALUE[26] =
{
    /* A */ 10, /* B */ 11, /* C */ 12, /* D */ 13, /* E */ 14,
    /* F */ 15, /* G */ 16, /* H */ 17, /* I */ 34, /* J */ 18,
    /* K */ 19, /* L */ 20, /* M */ 21, /* N */ 22, /* O */ 35,
    /* P */ 23, /* Q */ 24, /* R */ 25, /* S */ 26, /* T */ 27,
    /* U */ 28, /* V */ 29, /* W */ 32, /* X */ 30, /* Y */ 31,
    /* Z */ 33
};

// 手機條碼除了 '/' 開頭外，其餘 7 碼只能是這 39 個字元之一
static int32_t is_valid_carrier_char(char c)
{
    return (c >= '0' && c <= '9') ||
           (c >= 'A' && c <= 'Z') ||
           (c == '+' || c == '-' || c == '.');
}

// 一維條碼檢查 UPC-A
int32_t __cdecl lv_validate_upc(const char *code)
{
    // 預設不成功，明確執行才成功
    int32_t status = LV_BC_ERR_UNKNOWN;
    int32_t len, i;
    int32_t sum, check_digit;

    // 檢查有無指向任何有效記憶體
    if (code == NULL)
    {
        return LV_BC_ERR_BUFFER;
    }

    // UPC-A 固定 12 碼，且必須全部是數字 0-9
    // 強制轉型讓 lv 能直接對接，同時避免 conversion / possible loss of data
    // strlen 計算的是字串長度，不是陣列大小，也不是記憶體實際配置大小
    len = (int32_t)strlen(code);
    if (len != 12)
    {
        return LV_BC_ERR_FORMAT;
    }
    for (i = 0; i < len; i++)
    {
        if (code[i] < '0' || code[i] > '9')
        {
            return LV_BC_ERR_FORMAT;
        }
    }

    // 檢查碼演算法
    // - 前 11 碼：奇數位(1,3,5,7,9,11)權重 3、偶數位權重 1，產生加權總和
    // - 第 12 碼(檢查碼)：第 12 碼與前 11 碼加權總和的相加值，要能被 10 整除
    // - (sum+code[11])%10==0
    sum = 0;
    // 計算前11碼的加權總和
    for (i = 0; i < 11; i++)
    {
        int32_t digit = code[i] - '0';
        sum += (i % 2 == 0) ? digit * 3 : digit * 1;
    }
    // 比對第12碼的檢查碼
    check_digit = (10 - (sum % 10)) % 10;
    if (check_digit != (code[11] - '0'))    // 數字字元處理，如 '2' - '0' = 50 - 48 = 2
    {
        return LV_BC_ERR_CHECKSUM;
    }

    status = LV_BC_OK;
    return status;
}

// 一維條碼檢查 EAN
int32_t __cdecl lv_validate_ean(const char *code)
{
    // 預設不成功，明確執行才成功
    int32_t status = LV_BC_ERR_UNKNOWN;
    int32_t len, i;
    int32_t sum, check_digit;
    int32_t data_len;      // 扣掉檢查碼後的資料長度
    int32_t odd_weight;    // 第 1 碼(最左邊)的權重：EAN-13 為 1，EAN-8 為 3

    // 檢查有無指向任何有效記憶體
    if (code == NULL)
    {
        return LV_BC_ERR_BUFFER;
    }

    // 只接受 EAN-13 或 EAN-8，且必須全部是數字 0-9
    len = (int32_t)strlen(code);
    if (len != 13 && len != 8)
    {
        return LV_BC_ERR_FORMAT;
    }
    for (i = 0; i < len; i++)
    {
        if (code[i] < '0' || code[i] > '9')
        {
            return LV_BC_ERR_FORMAT;
        }
    }
    // 權重設定，依照 EAN-13、EAN-8 定義不同，前者 1、後者 3
    odd_weight = (len == 13) ? 1 : 3;

    // 檢查碼演算法
    // - 前 12/7 碼：由左而右，奇數位權重 odd_weight、偶數位權重 (4 - odd_weight)，
    // - 最後一碼(檢查碼)：最後一碼與前 12/7 碼加權總和的相加值，要能被 10 整除
    data_len = len - 1;
    sum = 0;
    // 計算前11碼的加權總和
    for (i = 0; i < data_len; i++)
    {
        int32_t digit = code[i] - '0';
        sum += (i % 2 == 0) ? digit * odd_weight : digit * (4 - odd_weight);
    }
    // 比對最後一碼的檢查碼
    check_digit = (10 - (sum % 10)) % 10;
    if (check_digit != (code[data_len] - '0'))
    {
        return LV_BC_ERR_CHECKSUM;
    }

    status = LV_BC_OK;
    return status;
}

// 手機條碼載具檢查
// 是否為「已註冊生效」的載具，需連線查詢電子發票整合服務平台 API 才能確認
int32_t __cdecl lv_validate_tw_carrier(const char *code)
{
    // 預設不成功，明確執行才成功
    int32_t status = LV_BC_ERR_UNKNOWN;
    int32_t len, i;

    // 檢查有無指向任何有效記憶體
    if (code == NULL)
    {
        return LV_BC_ERR_BUFFER;
    }

    // 固定 8 碼，第 1 碼固定為 '/'，其餘 7 碼只能是 [0-9A-Z.+-]
    len = (int32_t)strlen(code);
    if (len != 8 || code[0] != '/')
    {
        return LV_BC_ERR_FORMAT;
    }
    for (i = 1; i < len; i++)
    {
        if (!is_valid_carrier_char(code[i]))
        {
            return LV_BC_ERR_FORMAT;
        }
    }

    status = LV_BC_OK;
    return status;
}

// 台灣身分證號條碼檢查
int32_t __cdecl lv_validate_tw_id(const char *code)
{
    // 預設不成功，明確執行才成功
    int32_t status = LV_BC_ERR_UNKNOWN;
    int32_t len, i;
    int32_t letter_value, sum, check_digit;
    static const int32_t weight[10] = { 1, 9, 8, 7, 6, 5, 4, 3, 2, 1 }; // 加權總和權重
    int32_t values[10]; // [0]=縣市別代碼十位, [1]=縣市別代碼個位, [2..9]=身分證後 8 碼數字

    // 檢查有無指向任何有效記憶體
    if (code == NULL)
    {
        return LV_BC_ERR_BUFFER;
    }

    // 固定 10 碼：1 碼大寫英文字母 + 9 碼數字，^[A-Z][12]\d{8}$
    len = (int32_t)strlen(code);
    if (len != 10)
    {
        return LV_BC_ERR_FORMAT;
    }
    if (code[0] < 'A' || code[0] > 'Z')
    {
        return LV_BC_ERR_FORMAT;
    }
    for (i = 1; i < len; i++)
    {
        if (code[i] < '0' || code[i] > '9')
        {
            return LV_BC_ERR_FORMAT;
        }
        // 第 2 碼是生理性別碼，只能是 1(男) 或 2(女)
        if (i == 1 && code[i] != '1' && code[i] != '2')
        {
            return LV_BC_ERR_FORMAT;
        }
    }

    // checksum 的前置準備(values)
    // 第 1 碼字母：轉換成兩位數代碼，再拆成兩個值存入
    letter_value = TW_ID_LETTER_VALUE[code[0] - 'A'];   // 查詢TW_ID_LETTER_VALUE指定位置的數值
    values[0] = letter_value / 10;
    values[1] = letter_value % 10;
    // 第 2-8 碼數字：逐碼存入
    {
        // 宣告並初始化 digits
        // 把 digits 指向 code 跳過第 1 碼字母之後的位置。(型別都是指標)
        const char *digits = code + 1;
        for (i = 0; i < 8; i++)
        {
            // 從 values[2] 存到 values[9]
            values[i + 2] = digits[i] - '0';    // 指標語法糖：digits[i] 等價於 *(digits + i)
        }
    }

    // 檢查碼演算法
    // - 10 個加權值分別乘上權重 [1,9,8,7,6,5,4,3,2,1] 後加總，
    // check_digit = (10 - sum % 10) % 10，須等於身分證字號最後一碼
    sum = 0;
    for (i = 0; i < 10; i++)
    {
        sum += values[i] * weight[i];
    }
    // 比對最後一碼的檢查碼
    check_digit = (10 - (sum % 10)) % 10;
    if (check_digit != (code[9] - '0'))
    {
        return LV_BC_ERR_CHECKSUM;
    }

    status = LV_BC_OK;
    return status;
}
