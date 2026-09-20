# 指定 32-bit MinGW-w64 (i686-w64-mingw32) 工具鏈，理由見 README_tech.md 第 1 節：
# 本專案模擬的 LabVIEW 是 32-bit，Call Library Function Node 只能載入位元數相同的 .dll，
# 所以編譯器一定要用 mingw32（不能用 ucrt64/x86_64 那一套）。
#
# 用固定的絕對路徑指定編譯器，而不是依賴當前終端機的 PATH 順序：
# README_tech.md 2-3 節提到，如果兩種位元的 bin/ 都被加進系統 PATH，順序在前的會蓋過另一個，
# 容易因為終端機環境不同而編到錯的位元。用這個 toolchain file 明確指定，
# 不管從哪個 shell、哪個目錄呼叫 cmake，都保證用同一顆 32-bit 編譯器。
#
# 用法：cmake -B build -G Ninja -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-mingw32.cmake

set(CMAKE_C_COMPILER "C:/msys64/mingw32/bin/gcc.exe" CACHE FILEPATH "32-bit MinGW-w64 (i686-w64-mingw32) C compiler")
