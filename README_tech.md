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