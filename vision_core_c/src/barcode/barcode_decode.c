#include <string.h>
#include "barcode/barcode_decode.h"

// 編入 stb_image.h 
//   - 定義 STB_IMAGE_IMPLEMENTATION，只能在整個專案裡定義一次，也在這裡展開
//   - 不用連結，直接放入編譯檔。不產生單獨的 .dll (objdump 依賴清單裡完全沒有 stb 相關的東西)
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
// 宣告 zbar.h：用於一般函式庫(標頭.h + 二進位 .dll)
//   - 真正執行的程式碼在外部的 external/zbar/bin/libzbar-0.dll
//   - 透過 -lzbar 連結、執行期動態載入，故部署時需要多帶 libzbar-0.dll 那幾個檔案
#include <zbar.h>

// 從影像檔案解碼出第一個偵測到的條碼字串
int32_t __cdecl lv_decode_barcode_from_file(const char *image_path, char *out_code, int32_t out_code_size)
{
    // 預設不成功，明確執行才成功
    int32_t status = LV_BD_ERR_UNKNOWN;
    int width, height, orig_channels;
    unsigned char *gray_buffer;
    zbar_image_scanner_t *scanner;
    zbar_image_t *image;
    int symbol_count;
    const zbar_symbol_t *symbol;
    const char *data;
    size_t data_len;

    // 檢查有無指向任何有效記憶體
    if (image_path == NULL || out_code == NULL)
    {
        return LV_BD_ERR_BUFFER;
    }

    // RGB/RGBA 轉成灰階(單通道)/Y800格式
    //  - 條碼掃描演算法：只關心每一個像素亮或暗，完全用不到色彩資訊
    //  - Y800 是一種影像像素格式的代號，只存亮度(Y)，不存色彩(UV)，每個像素用 8 bit 表示
    //  - 用 stb_image 把影像檔案讀成單通道灰階 buffer，實作是 stbi_load() 第四個引數 desired_channels=1
    //  - stbi__fopen 在 Windows 下可能轉成 _wfopen 處理路徑，邏輯上等價於 fopen(filename, "rb")
    gray_buffer = stbi_load(image_path, &width, &height, &orig_channels, 1);
    if (gray_buffer == NULL)
    {
        // 檔案不存在、無讀取權限，或不是 stb_image 支援的格式
        return LV_BD_ERR_FILE_OPEN;
    }

    // 建立並設定 zbar 掃描器
    // 掃描所有支援的條碼格式(EAN/UPC/Code128/QR...)
    scanner = zbar_image_scanner_create();
    zbar_image_scanner_set_config(scanner, 0, ZBAR_CFG_ENABLE, 1);

    // 存取灰階資料 
    // 建立 zbar_image_t 結構(image)，並設定中繼資料(Y800 格式存入、像素長寬)
    image = zbar_image_create();
    zbar_image_set_format(image, zbar_fourcc('Y', '8', '0', '0')); //中繼資料設定
    zbar_image_set_size(image, width, height);
    // 把 stbi_load 產生的 gray_buffer 指標交給 image
    // 同時，決定第2引數的記憶體由原建立的 stbi 來釋放，故在第4引數設為NULL
    zbar_image_set_data(image, gray_buffer, (unsigned long)(width * height), NULL);

    // 執行掃描演算法
    symbol_count = zbar_scan_image(scanner, image);
    if (symbol_count <= 0)
    {
        status = LV_BD_ERR_NOT_FOUND;
    }
    else
    {
        // 取得條碼中的字元
        // 簡化處理：只取畫面中第一個偵測到的條碼
        symbol = zbar_image_first_symbol(image);
        data = zbar_symbol_get_data(symbol);
        data_len = strlen(data);
        // 檢查out_code長度是否夠長
        if ((int32_t)data_len >= out_code_size)
        {
            // 讀出的字串太長
            status = LV_BD_ERR_OUT_TOO_SMALL;
        }
        else
        {
            // 把讀出的字串寫入out_code
            memcpy(out_code, data, data_len + 1); // 重要：data_len +1 是把 '\0' 一起複製過去
            status = LV_BD_OK;
        }
    }

    // 釋放資源
    // 順序先釋放 zbar 物件，再釋放 stb_image 配置的原始像素 buffer
    zbar_image_destroy(image);  // 銷毀 image 描述物件本身，不會處理 gray_buffer 的銷毀
    zbar_image_scanner_destroy(scanner);
    // gray_buffer 是由 stbi_load 配置出來的，理論上要用 stbi_image_free 釋放
    stbi_image_free(gray_buffer);

    return status;
}
