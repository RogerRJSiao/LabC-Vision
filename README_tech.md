# 開發環境設定 (Windows OS + C 編譯器)

本機 (Win 11) 用來編譯 `vision_core_c/` 底下 C 原始碼所安裝的工具鏈與設定，記錄如下，供其他人或重灌時參考。

## 1. 評估軟硬體選用

### 1-1. LabVIEW 位元數與硬體驅動程式的關係

> - LabVIEW 的位元數選擇，與硬體設備、驅動程式 (drivers) 有絕對的關係。
> - Labview 官方下載 @ National Instruments：https://www.ni.com/zh-tw.html

1. **驅動程式相容性**
  - **32 位元 LabVIEW**：支援所有 NI 官方與第三方的硬體驅動程式。如果使用的是舊款硬體，通常只有 32 位元驅動程式。
  - **64 位元 LabVIEW**：必須搭配 64 位元版本的驅動程式。雖然現代 NI 驅動程式 (如 NI-DAQmx、NI-VISA) 大多同時提供 32/64 位元，但當某些非 NI 的第三方儀器驅動程式 (DLL) 檔只有 32 位元，在 64 位元 LabVIEW 中就完全無法呼叫。

2. **即時系統與硬體目標**
  - **硬體部署限制**：如果專案需要將程式下載到 NI 的嵌入式硬體執行，開發端通常必須使用 32 位元 LabVIEW，因為多數 Real-Time 模組與編譯器只與 32 位元環境整合。

> ⚠️ 本專案的 LabVIEW 為 **32 位元**，因此 `vision_core_c` 編出來給 LabVIEW (Call Library Function Node, CLFN) 呼叫的 DLL **也必須是 32 位元**，需對應使用 `mingw32`（`i686-w64-mingw32`）工具鏈，不能用 `ucrt64`（`x86_64-w64-mingw32`）編譯。

MSYS2 把不同位元的工具鏈裝在不同資料夾，彼此獨立，可同時安裝：

| 位元 | 套件名稱 | 安裝路徑 | Target |
|---|---|---|---|
| 32-bit | `mingw-w64-i686-gcc` | `C:\msys64\mingw32\bin\gcc.exe` | `i686-w64-mingw32` |
| 64-bit | `mingw-w64-ucrt-x86_64-gcc` | `C:\msys64\ucrt64\bin\gcc.exe` | `x86_64-w64-mingw32` |

## 2. 安裝 C 編譯軟體與環境

### 2-1. 安裝 MSYS2 環境、套件管理器 pacman

用 `winget` 安裝 MSYS2，預設安裝路徑：`C:\msys64`。[DOCS](https://www.msys2.org/)

```powershell
winget install --id MSYS2.MSYS2 -e --accept-source-agreements --accept-package-agreements
```

> **安裝路徑是 `C:\msys64`（官方建議）：**
> - 路徑不可包含空白字元、特殊符號。MSYS2 內部的 POSIX 工具常對含空白的路徑解析失敗。
> - 避免中文字元或其他特殊符號。路徑含中文有時會讓部分 C/C++ 工具鏈的檔案系統呼叫出錯。
> - 路徑盡量短、放在磁碟根目錄附近。Windows 傳統 Win32 API 對檔案路徑有 `MAX_PATH`（260 字元）的限制。
> - 裝在 `C:\` 下而非使用者個人資料夾，所有使用者帳號都能存取，不受限於單一使用者。

### 2-2. 透過 MSYS2 安裝 MinGW-w64 GCC 工具鏈

開啟 MSYS2 shell 執行安裝，編譯器位置在 `C:\msys64\mingw32\bin\gcc.exe`。

本專案搭配 32 位元 LabVIEW，故只需安裝 32 位元工具鏈：

```bash
# 安裝 32-bit
pacman -Sy --noconfirm --needed mingw-w64-i686-gcc
# 或安裝 64-bit
pacman -Sy --noconfirm --needed mingw-w64-ucrt-x86_64-gcc

# 安裝是否成功
gcc --version
```

> 若之後專案改搭配 64 位元 LabVIEW，才需另外安裝 `mingw-w64-ucrt-x86_64-gcc`；兩者裝在不同資料夾，可同時安裝、互不影響。

### 2-3. 設定環境變數 PATH

開啟 PS，將 `C:\msys64\mingw32\bin` 加入使用者 PATH：

```powershell
$gccPath = "C:\msys64\mingw32\bin"
$currentPath = [Environment]::GetEnvironmentVariable("Path", "User")
[Environment]::SetEnvironmentVariable("Path", "$currentPath;$gccPath", "User")
```

> ⚠️ **注意**：設定 User PATH 之後，**必須重新開啟終端機/VS Code**，新開的視窗才會套用新的 PATH。已經開著的舊終端機不會自動更新。

驗證安裝是否成功、確認是 32 位元：

```powershell
# 查看目前 PATH 指到哪一個編譯器
# i686-... 對應 32-bit，x86_64-...對應 64-bit
gcc -dumpmachine   
```

> 若要兩種位元同時保留、按需切換，**不要把兩個 `bin` 都加進系統 PATH**（順序在前的會蓋過另一個），改用完整路徑呼叫，或在單一終端機 session 暫時覆蓋 PATH。

```powershell
# 32-bit
C:\msys64\mingw32\bin\gcc.exe ttst.c -o ttst.exe
# 64-bit
C:\msys64\ucrt64\bin\gcc.exe ttst.c -o ttst.exe

# 或暫時切換當前 session 的 gcc 版本
$env:PATH = "C:\msys64\mingw32\bin;" + $env:PATH   # 之後這個 session 的 gcc 都是 32-bit
```


### 2-4. 設定 IDE 語法檢查功能

1. 指定 IntelliSense 的編譯器路徑
  - 若 `#include <stdio.h>` 等標準函式庫在 VS Code 中顯示紅色錯誤（`cannot open source file`），代表 IntelliSense 還沒找到編譯器路徑：
    1. 重新開啟 VS Code（套用新 PATH）
    2. 按 `Ctrl + Shift + P` 開啟命令選單
    3. 執行 **「C/C++: Select IntelliSense Configuration...」**
    4. 選擇剛裝好的 `C:\msys64\mingw32\bin\gcc.exe`

## 3. 編譯與執行腳本

### 3-1. 透過 gcc 手動編譯測試
```powershell
# 編譯
gcc ttst.c -o ttst.exe
# 執行
./ttst.exe
```

### 3-2. VS Code build tasks

`.vscode/tasks.json` 底下每個模組各自有一組 `build-<模組>-release`（產出 `bin/*.dll` 給 LabVIEW CLFN 用）與 `build-<模組>-test`/`-debug`（產出 `tests/`、`deployments/trial/` 底下的 `.exe`）。

新增 `common/error_messages`（錯誤碼→訊息字串對照表，供各模組共用）對應的 task：
- `build-error-messages-release`：編出 `bin/error_messages.dll`
- `build-error-messages-test`：編出 `tests/common/test_error_messages.exe`

> `.vscode/tasks.json` 一次只編一個模組，且不受版控（機器本機設定）。要一次建置全部模組 + 全部測試，改用下面的 CMake 流程。

### 3-3. CMake 建置（推薦：一個指令建置全部模組）

`vision_core_c/CMakeLists.txt` 把上面 `tasks.json` 裡逐一手動維護的 gcc 指令，統一成一份設定：一次建置 `math_formulas`、`barcode_validate`、`barcode_decode`、`error_messages` 四個模組的 `bin/*.dll`，以及 `tests/` 底下對應的單元測試，速度比一個個手動點 task 快很多，之後新增部署環境（`deployments/<新客戶>/`）也能直接重用同一份設定。

#### 3-3-1. 安裝 CMake / Ninja（32-bit 工具鏈）

跟第 2 節裝 `mingw-w64-i686-gcc` 一樣，用同一個 pacman 裝對應 32-bit 版本：

```bash
C:\msys64\usr\bin\pacman.exe -Sy --noconfirm --needed mingw-w64-i686-cmake mingw-w64-i686-ninja
```

#### 3-3-2. Configure（只需執行一次，除非改了 CMakeLists.txt）

在 `vision_core_c/` 底下執行：

```powershell
$toolchain = Join-Path (Get-Location) "cmake\toolchain-mingw32.cmake"
C:\msys64\mingw32\bin\cmake.exe -S . -B build -G Ninja "-DCMAKE_TOOLCHAIN_FILE=$toolchain" -DCMAKE_MAKE_PROGRAM=C:/msys64/mingw32/bin/ninja.exe
```

> ⚠️ 複製貼上長路徑字串容易夾帶隱藏字元，導致 PowerShell 誤判參數、報 `Could not find toolchain file`。用 `Join-Path` 讓 PowerShell 自己組路徑可以避開這個問題。

- `-B build`：中繼檔放進 `vision_core_c/build/`（既有的、git 忽略的資料夾），不會汙染 `bin/`
- `cmake/toolchain-mingw32.cmake`：**明確指定** `C:\msys64\mingw32\bin\gcc.exe`，不依賴當前終端機的 PATH 順序——避免第 2-3 節提到的「32/64 位元 PATH 互相蓋過」問題，不管從哪個 shell 呼叫都保證編出 32-bit DLL

#### 3-3-3. Build（之後每次改完程式碼都執行這個）

```powershell
C:\msys64\mingw32\bin\cmake.exe --build build
```

- 產出的 `.dll` 固定在 `vision_core_c/bin/`（跟 `tasks.json` 原本的位置完全一樣），LabVIEW 端 CLFN 記錄的相對路徑不用改
- `barcode_decode.dll` 建置完成後，CMake 會自動把 zbar 的 5 個執行期依賴 DLL（見 4-2-4 節）複製到 `bin/` 跟 `tests/barcode/` 旁邊，不用再手動複製

> ⚠️ **DLL 被鎖住的錯誤（`Permission denied` / `cannot open output file`）**：如果 LabVIEW 正開著專案（CLFN 已經載入某個 `.dll`），Windows 會鎖住該檔案不給覆寫，建置會失敗在那個模組的連結步驟。**先關閉 LabVIEW 再重新建置**即可，這是 Windows 對載入中 DLL 的限制，跟用 CMake 或原本手動 gcc 指令都一樣會遇到。

#### 3-3-4. 執行單元測試

```powershell
C:\msys64\mingw32\bin\ctest.exe --test-dir build --output-on-failure
```

會依序執行 `test_math_formulas`、`test_barcode_validate`、`test_barcode_decode`、`test_error_messages`，並自動用各測試自己的資料夾（`tests/<模組>/`）當工作目錄，讓 `tests/barcode/fixtures/ean13_sample.png` 這類相對路徑的測試素材能正確找到。

## 4. 第三方函式庫

本專案 `src/barcode/barcode_decode.c` 依賴的外部函式庫，放在 `vision_core_c/external/`：
- **stb**（`external/stb/`）：單一標頭檔原始碼，負責把影像檔案讀成灰階 pixel buffer
- **zbar**（`external/zbar/`）：預編譯二進位，負責掃描 pixel buffer、解出條碼字串

> 一般是「第三方函式庫不進版控」，改用 vcpkg / Conan / CMake FetchContent 等套件管理工具，在建置時自動抓取指定版本，讓 `git clone` 下來的 repo 保持乾淨。<br>
> 但這兩個目錄評估後決定進版控，stb 是慣例，zbar 是與 32 位元套件取得困難有關。

### 4-1. stb (影像格式解碼)

#### 4-1-1. 列入版本控制原因

1. 專案中目前尚未建置自動抓取函式庫：`stb_image.h` 本身取得管道穩定，但本專案的建置方式是 `.vscode/tasks.json` 直接呼叫 gcc，沒有 CMake，也就沒有 FetchContent/vcpkg 這層自動化，仍需要靠人工 `curl` 一次。
2. 檔案不大、原始碼可查、授權允許、換版成本低：因 vendoring 成本趨近於零，純 C 原始碼、單一標頭檔（280 KB），不是二進位，MIT / public domain 雙授權，沒有法遵疑慮；upstream 更新頻率低。
3. 暫時維持同一套流程的部署一致性：`barcode_decode.c` 同時依賴 stb 和 zbar 才能順利編譯，只 vendor 一邊、另一邊靠手動步驟完成的話，`clone` 之後仍然無法「一鍵可編譯」。

#### 4-1-2. 重建 `external/stb/`

因為搭配的 zbar 本身只接收 raw pixel buffer (灰階/Y800)，不具備讀取 `.jpg`/`.png`/`.bmp` 檔案功能，只好另找第三方套件如 [stb_image.h](https://github.com/nothings/stb) 負責「影像檔案」到 「灰階 pixel buffer」這一段：

```powershell
New-Item -ItemType Directory -Force external/stb
curl -sL -o external/stb/stb_image.h https://raw.githubusercontent.com/nothings/stb/master/stb_image.h
```

1. `stb_image.h` 是 header-only (MIT / public domain 雙授權)，不用另外編譯或連結，
2. `src/barcode/barcode_decode.c` 裡 `#define STB_IMAGE_IMPLEMENTATION` 之後直接 `#include` 就有完整實作。

### 4-2. zbar (條碼影像解碼)

#### 4-2-1. 系統位元的選擇

**64-bit：**

MSYS2 官方套件庫有現成的 64-bit zbar 套件 (`mingw64`/`ucrt64`/`clang64`/`clangarm64`)，
直接 `pacman -S mingw-w64-ucrt-x86_64-zbar`(或對應 subsystem 的套件名稱) 就能裝好，
不需要額外處理。

**32-bit：**

MSYS2 官方套件庫沒有 32-bit (`mingw32`/`i686`) 版本的 zbar。本專案模擬情境的 LabVIEW 是 32-bit，DLL 必須是 32-bit，因此必須另外處理：

改用 zbar 官方 GitHub Actions 自動建置、發佈在 linuxtv.org 的 **32-bit Windows 預編譯版**，不需要自己 `configure && make` 從原始碼編譯。

- 下載頁面：<https://linuxtv.org/downloads/zbar/binaries/>
- 下載檔案：`zbar-<版本>-win_i686-VfW.zip`(`i686` = 32-bit；`VfW` = Video for Windows，我們用不到視訊擷取功能，選這個或 `DShow` 版都可以，內含的 `libzbar-0.dll`/標頭檔是一樣的)

#### 4-2-2. 列入版本控制原因

1. 尚未查到「可自動化、有人維護」的取得管道：目前唯一可靠的來源是手動去 linuxtv.org 下載一個特定版本的 zip，但這會影響自動化部署，故直接把下載結果存入 git 。
    - MSYS2 `pacman` 官方套件庫沒有 32-bit zbar
    - vcpkg 的 i686-mingw 只是未經 CI 驗證的 community triplet
    - Conan 的 zbar port 也沒有 mingw i686 的驗證紀錄
2. 外部依賴的脆弱性：linuxtv.org 的下載頁面是個人/社群維運，版本可能被下架、換版號、換路徑。一旦連結失效，`barcode_decode.c` 就編不起來、且沒有備援。把當下驗證過能動的那份二進位存進 git，等於幫這個依賴上一份保險。
3. 檔案體積可接受：`external/zbar/` 約 3.9 MB，且更新頻率極低，對 repo 體積沒有明顯負擔。

> 若之後 vcpkg/Conan 補上了經過 CI 驗證的 i686-mingw zbar port，可以重新評估改回套件管理工具自動抓取；在那之前，vendoring 是相對務實的選擇。

#### 4-2-3. 需再安裝的套件清單 (32-bit)

在第 2 節已裝好的 `mingw-w64-i686-gcc` 基礎上，對 `barcode_decode.c` 這條 pipeline 額外需要下列套件，全部用 `C:\msys64\usr\bin\pacman.exe -Sy --noconfirm --needed <套件名稱>` 安裝：

| 套件名稱 | 用途 | 是否必要 |
|---|---|---|
| `mingw-w64-i686-libiconv` | `libzbar-0.dll` 執行期依賴，zbar 內部做字元編碼轉換用 | ✅ 必要 |
| `mingw-w64-i686-libjpeg-turbo` | 提供 `libjpeg-8.dll`，`libzbar-0.dll` 執行期依賴 | ✅ 必要 |

> `libwinpthread-1.dll`、`libgcc_s_dw2-1.dll` 這兩個 zbar 也依賴的 runtime DLL，
不用另外裝，當安裝 `mingw-w64-i686-gcc` 的時候，就會一起帶進 `C:\msys64\mingw32\bin\`。

```powershell
C:\msys64\usr\bin\pacman.exe -Sy --noconfirm --needed `
    mingw-w64-i686-gcc mingw-w64-i686-libiconv mingw-w64-i686-libjpeg-turbo
```

#### 4-2-4. 重建 `external/zbar/`

```powershell
# 1. 下載並解壓縮
curl -sL -o zbar_win32.zip https://linuxtv.org/downloads/zbar/binaries/zbar-0.23.91-win_i686-VfW.zip
Expand-Archive zbar_win32.zip -DestinationPath zbar_win32

# 2. 把需要的檔案複製進 external/zbar/(在 vision_core_c/ 底下執行)
New-Item -ItemType Directory -Force external/zbar/include, external/zbar/lib, external/zbar/bin
Copy-Item zbar_win32/mingw32/include/zbar.h, zbar_win32/mingw32/include/zbar external/zbar/include -Recurse
Copy-Item zbar_win32/mingw32/lib/libzbar.dll.a external/zbar/lib/
Copy-Item zbar_win32/mingw32/bin/libzbar-0.dll external/zbar/bin/
```

`libzbar-0.dll` 執行期還依賴幾個 mingw32 runtime DLL (`libwinpthread-1.dll`、`libgcc_s_dw2-1.dll`、
`libiconv-2.dll`、`libjpeg-8.dll`)，只要 4-2-3 的套件都裝好，這幾個 DLL 就都會出現在
`C:\msys64\mingw32\bin\`，把它們複製進 `external/zbar/bin/`：

```powershell
Copy-Item C:\msys64\mingw32\bin\libwinpthread-1.dll, `
          C:\msys64\mingw32\bin\libgcc_s_dw2-1.dll, `
          C:\msys64\mingw32\bin\libiconv-2.dll, `
          C:\msys64\mingw32\bin\libjpeg-8.dll `
          external/zbar/bin/
```

> ⚠️ **未來部署給 LabVIEW CLFN 呼叫時，要特別注意**：Windows 載入 DLL 時，相依 DLL 必須跟呼叫端的 .exe (或另一個 .dll) 放在同一個資料夾，或在系統 `PATH` 上。啟動 LabVIEW 的那台機器不一定有安裝 MSYS2，也不會自動幫你補齊這些相依檔案。<br>

- 部署步驟(1、2 為一次性設置，通常已完成；3、4 才是每次部署都要做的動作)
1. 確認 `external/zbar/`、`external/stb/` 就位後，編譯出 `bin/barcode_decode.dll`。
2. `external/zbar/bin/` 底下已經放好 `libzbar-0.dll` 本身，以及它依賴的 4 個 runtime DLL
   (`libiconv-2.dll`、`libjpeg-8.dll`、`libwinpthread-1.dll`、`libgcc_s_dw2-1.dll`)，共 5 個檔案(見 4-2-4)。
3. 打包/部署 `bin/barcode_decode.dll` 時，把 `external/zbar/bin/` 底下所有 .dll 一起複製到跟 `barcode_decode.dll` 同一層
4. 實際部署進 LabVIEW 專案時，同樣也要把這 6 個檔案放進 LabVIEW 專案中，能找到 `barcode_decode.dll` 的同一層目錄內。

- 必要的檔案明細

| 檔案 | 依賴層級 | 部署時要注意的點 |
|---|---|---|
| `barcode_decode.dll` | LabVIEW CLFN 直接指向的目標檔案 | import table 只列出 `libzbar-0.dll` 這一層，但實際上還牽動下面多層間接依賴 |
| `libzbar-0.dll` | `barcode_decode.dll` 的直接依賴 | 位置 `external/zbar/bin/`。import table 上看得到，必要性最容易被注意到 |
| `libiconv-2.dll` | `libzbar-0.dll` 的間接依賴 | zbar 內部做字元編碼轉換用 |
| `libjpeg-8.dll` | `libzbar-0.dll` 的間接依賴 | zbar 讀取 JPEG 影像時用到 |
| `libwinpthread-1.dll` | `libzbar-0.dll` 的間接依賴 (MinGW32 GCC 執行期支援函式庫) | `libzbar-0.dll` 編譯時用到 pthread API 需要，讓 Windows 上可以呼叫 POSIX threads (pthread) 的 API。<br>開發機因為裝了 `mingw-w64-i686-gcc` 而自帶，但 LabVIEW 那台部署機不會有。 |
| `libgcc_s_dw2-1.dll` | `libzbar-0.dll` 的間接依賴 (MinGW32 GCC 執行期支援函式庫) | 提供例外處理機制、底層運算的基礎，同樣是編譯器工具鏈自帶，但 LabVIEW 那台部署機不會有。 |

#### 4-2-5. `zbar_scan_image()` 回傳值語意

`zbar_scan_image()` 回傳偵測到的條碼數量，`<= 0` 視為布林 FALSE（見 [barcode_decode.c:61-65](vision_core_c/src/barcode/barcode_decode.c#L61-L65)）。常見造成 `<= 0` 的情況：

| 情況 | 說明 |
|---|---|
| 影像中沒有條碼 | 最常見。掃描完找不到符合條碼特徵的圖案，回傳 `0` |
| 校驗碼(Check Digit)不符 | 條碼存在，但光線、反光或解析度不足導致解出的校驗碼跟計算值對不上，zbar 直接判定解碼錯誤、拋棄該次結果，回傳 `0` |
| 該條碼格式(Symbology)被停用 | zbar 可設定只掃描特定格式；若影像中剛好只有被停用的格式(例如關掉了 Code 128)，會直接忽略，回傳 `0` |
| 影像或參數錯誤 | 回傳負數。例如 `zbar_image_t` 指標為 `NULL`，或影像格式不是 zbar 支援的 Y800/GREY 灰階格式(zbar 無法直接處理未轉換的 RGB/RGBA) |

- 如何查詢一個 DLL 的 import table (直接依賴哪些 DLL)

用工具鏈自帶的 `objdump -p` 即可，輸出裡找 `DLL Name:` 這個關鍵字，後面接的就是直接依賴的 DLL：

```bash
# Git Bash / MSYS2 shell
C:/msys64/mingw32/bin/objdump.exe -p bin/barcode_decode.dll | grep "DLL Name"
```
```powershell
# PowerShell
C:\msys64\mingw32\bin\objdump.exe -p bin\barcode_decode.dll | Select-String "DLL Name"
```