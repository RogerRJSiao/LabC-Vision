# LabC-Vision
Construction and indpection of LabVIEW (UI/HMI), C language and Computer Vision.

## 專案結構

```
LabC-Vision/
├── vision_core_c/
│   ├── include/          對外公開介面 (.h)，LabVIEW CLFN 綁定用
│   │   └── common/       跨模組共用邏輯 (例如 error_messages：錯誤碼→訊息字串)
│   ├── src/              實際邏輯實作 (.c)，內部呼叫第三方函式庫
│   ├── external/         第三方函式庫本身 (zbar、tesseract)
│   ├── deployments/      各客戶/產線客製層 (trial、prod_a…)，各自產出獨立 .dll
│   ├── tests/            單元測試
│   ├── build/            CMake 編譯中繼檔 (git 忽略)
│   └── bin/              編譯完成的 .dll 輸出位置
│
└── vision_hmi_lv/        LabVIEW 專案，負責畫面與操作流程
    ├── LabC-Vision.lvproj  LabVIEW 專案檔
    ├── Main.vi             主程式，畫面與流程控制
    ├── SubVIs/             子程式 (.vi)，例如 DecodeBarcode、ValidateBarcode、HandleError
    ├── Libraries/          各客戶/產線客製層 (prod_a…)
    ├── Resources/          圖示、設定檔等資源
    └── builds/             編譯輸出位置 (git 忽略)
```

- 新增套件/功能時，是否要開新資料夾？ => 「這是 LabVIEW 會獨立呼叫的功能嗎？」
    - 是 → 在 `include/`、`src/` 各開一個新資料夾（比照 barcode、ocr）
    - 否 → 不用開新資料夾，寫進既有功能的 wrapper，或放進 `common/` 共用


## 資料流程：從 LabVIEW 輸入到 C 語言判斷

```
1. LabVIEW 前面板取得影像 (相機/檔案)
        ↓
2. Main.vi 裡的 Call Library Function Node (CLFN)
   把影像資料 (byte array) 與參數傳給指定的 .dll
        ↓
3. deployments/<部署>/ 產出的 .dll (例如 libvision_trial.dll)
   DLL 的匯出函式，其簽名對應 include/ 裡宣告的介面
        ↓
4. include/*.h 宣告的函式，實作在 src/*.c
   在這裡呼叫 external/ 的第三方函式庫 (zbar/tesseract) 做真正的辨識
        ↓
5. 結果 (字串/數值) 透過參數傳回 CLFN
        ↓
6. LabVIEW 顯示或做後續判斷
```

簡單說明：**LabVIEW 只負責畫面與呼叫，實際判斷邏輯都在 C 語言這邊完成，兩者透過 .dll 的固定介面溝通。**

## 錯誤處理

每個模組各自定義獨立的錯誤碼 enum（以 -100 為單位分段，避免碰撞），但錯誤碼→常數名稱字串（例如 `LV_BC_ERR_CHECKSUM` 轉成 `"LV_BC_ERR_CHECKSUM"`）的對照表統一放在 `include/common/error_messages.h` / `src/common/error_messages.c`（編成獨立的 `bin/error_messages.dll`），是唯一的訊息來源。

```
1. 任一 SubVI 呼叫的 CLFN 回傳狀態碼 (int32)
        ↓
2. LabVIEW SubVIs/HandleError.vi
   呼叫 error_messages.dll 的 lv_get_error_message 把狀態碼轉成訊息字串
        ↓
3. 合併成 LabVIEW 原生 Error Cluster，寫入 SubVIs/ErrorLogFGV.vi 的歷史紀錄 (todo)
        ↓
4. Main.vi 前面板：
   - 常駐、非阻斷的錯誤顯示區，即時顯示所有輸出的錯誤/警告訊息
   - 嚴重錯誤 (Critical) 額外跳出 LabVIEW 內建的 General Error Handler 對話框 (todo)
```

新增模組或新增錯誤碼時，記得同步在 `error_messages.c` 的對照表加一筆。

## 開發環境設定

C 編譯器工具鏈（MSYS2 / MinGW-w64 GCC）的安裝步驟、環境變數設定與已知問題排解，請見 [README_tech.md](README_tech.md)。
