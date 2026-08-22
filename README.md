# LabC-Vision
Construction and indpection of LabVIEW (UI/HMI), C language and Computer Vision.

## 專案結構

```
LabC-Vision/
├── vision_core_c/
│   ├── include/          對外公開介面 (.h)，LabVIEW CLFN 綁定用
│   ├── src/              實際邏輯實作 (.c)，內部呼叫第三方函式庫
│   ├── external/         第三方函式庫本身 (zbar、tesseract)
│   ├── deployments/      各客戶/產線客製層 (trial、prod_a…)，各自產出獨立 .dll
│   ├── tests/            單元測試
│   ├── build/            CMake 編譯中繼檔 (git 忽略)
│   └── bin/              編譯完成的 .dll 輸出位置
│
└── vision_hmi_lv/        LabVIEW 專案 (.lvproj、.vi)，負責畫面與操作流程
```

- 新增套件/功能時，是否要開新資料夾？ => 「這是 LabVIEW 會獨立呼叫的功能嗎？」
    - 是 → 在 `include/`、`src/` 各開一個新資料夾（比照 barcode、ocr）
    - 否 → 不用開新資料夾，寫進既有功能的 wrapper，或放進 `common/` 共用


## 資料流程：從 LabVIEW 輸入到 C 語言判斷

```
1. LabVIEW 前面板取得影像 (相機/檔案)
        ↓
2. Main_UI.vi 裡的 Call Library Function Node (CLFN)
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
