cmake_minimum_required(VERSION 3.20)
project(vision_core_c C)

# 路徑一律以 CMAKE_SOURCE_DIR 為錨點，.dll 固定輸出到 vision_core_c/bin/，
# 跟 vision_hmi_lv/**/*.vi 裡 CLFN 記錄的相對路徑對得上，不用重新設定。
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

set(ZBAR_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/external/zbar/include)
set(ZBAR_LIB_DIR     ${CMAKE_SOURCE_DIR}/external/zbar/lib)
set(STB_INCLUDE_DIR  ${CMAKE_SOURCE_DIR}/external/stb)

# zbar 執行期依賴，要跟 barcode_decode.dll 放同一層（見 README_tech.md 4-2-4 節）
set(ZBAR_RUNTIME_DLLS
    ${CMAKE_SOURCE_DIR}/external/zbar/bin/libzbar-0.dll
    ${CMAKE_SOURCE_DIR}/external/zbar/bin/libiconv-2.dll
    ${CMAKE_SOURCE_DIR}/external/zbar/bin/libjpeg-8.dll
    ${CMAKE_SOURCE_DIR}/external/zbar/bin/libwinpthread-1.dll
    ${CMAKE_SOURCE_DIR}/external/zbar/bin/libgcc_s_dw2-1.dll
)

# LabVIEW CLFN 呼叫用的模組，產出 bin/<name>.dll（沿用 tasks.json 的命名慣例：
# 不加 "lib" 前綴，import library 用 "lib<name>.a"）
function(add_lv_module name)
    cmake_parse_arguments(ARG "" "" "SOURCES;INCLUDES;LINK_DIRS;LINK_LIBS" ${ARGN})

    add_library(${name} SHARED ${ARG_SOURCES})
    target_include_directories(${name} PRIVATE ${CMAKE_SOURCE_DIR}/include ${ARG_INCLUDES})
    target_compile_options(${name} PRIVATE -Wall)
    target_link_options(${name} PRIVATE -static-libgcc)
    set_target_properties(${name} PROPERTIES
        PREFIX ""
        IMPORT_PREFIX "lib"
        IMPORT_SUFFIX ".a"
    )

    if(ARG_LINK_DIRS)
        target_link_directories(${name} PRIVATE ${ARG_LINK_DIRS})
    endif()
    if(ARG_LINK_LIBS)
        target_link_libraries(${name} PRIVATE ${ARG_LINK_LIBS})
    endif()
endfunction()

add_lv_module(math_formulas
    SOURCES ${CMAKE_SOURCE_DIR}/src/math/math_formulas.c
)

add_lv_module(barcode_validate
    SOURCES ${CMAKE_SOURCE_DIR}/src/barcode/barcode_validate.c
)

add_lv_module(barcode_decode
    SOURCES ${CMAKE_SOURCE_DIR}/src/barcode/barcode_decode.c
    INCLUDES ${STB_INCLUDE_DIR} ${ZBAR_INCLUDE_DIR}
    LINK_DIRS ${ZBAR_LIB_DIR}
    LINK_LIBS zbar
)

add_lv_module(error_messages
    SOURCES ${CMAKE_SOURCE_DIR}/src/common/error_messages.c
)

# 複製 zbar 執行期依賴到 barcode_decode.dll 旁邊，LabVIEW 才載得動
add_custom_command(TARGET barcode_decode POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${ZBAR_RUNTIME_DLLS} $<TARGET_FILE_DIR:barcode_decode>
    COMMENT "複製 zbar 執行期依賴到 barcode_decode.dll 旁邊"
)

# 單元測試：直接編譯 src/*.c，不連結上面的 .dll；輸出維持在 tests/<module>/，
# 讓 tests/barcode/fixtures/ 這類相對路徑的測試素材找得到。
enable_testing()

function(add_lv_test name)
    cmake_parse_arguments(ARG "" "OUTPUT_DIR;WORKING_DIR" "SOURCES;INCLUDES;LINK_DIRS;LINK_LIBS;COPY_DLLS" ${ARGN})

    add_executable(${name} ${ARG_SOURCES})
    target_include_directories(${name} PRIVATE ${CMAKE_SOURCE_DIR}/include ${ARG_INCLUDES})
    target_compile_options(${name} PRIVATE -Wall -Wextra -g)
    target_link_options(${name} PRIVATE -static-libgcc)
    if(ARG_LINK_DIRS)
        target_link_directories(${name} PRIVATE ${ARG_LINK_DIRS})
    endif()
    if(ARG_LINK_LIBS)
        target_link_libraries(${name} PRIVATE ${ARG_LINK_LIBS})
    endif()
    if(ARG_OUTPUT_DIR)
        set_target_properties(${name} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${ARG_OUTPUT_DIR})
        # MinGW 的執行檔連結規則會強制產生 import library，關不掉，導去這裡避免弄髒 bin/
        set_target_properties(${name} PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${ARG_OUTPUT_DIR})
    endif()
    if(ARG_COPY_DLLS)
        add_custom_command(TARGET ${name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${ARG_COPY_DLLS} $<TARGET_FILE_DIR:${name}>
        )
    endif()

    add_test(NAME ${name} COMMAND ${name} WORKING_DIRECTORY ${ARG_WORKING_DIR})
endfunction()

add_lv_test(test_math_formulas
    SOURCES ${CMAKE_SOURCE_DIR}/tests/math/test_math_formulas.c
            ${CMAKE_SOURCE_DIR}/src/math/math_formulas.c
    OUTPUT_DIR ${CMAKE_SOURCE_DIR}/tests/math
    WORKING_DIR ${CMAKE_SOURCE_DIR}/tests/math
)

add_lv_test(test_barcode_validate
    SOURCES ${CMAKE_SOURCE_DIR}/tests/barcode/test_barcode_validate.c
            ${CMAKE_SOURCE_DIR}/src/barcode/barcode_validate.c
    OUTPUT_DIR ${CMAKE_SOURCE_DIR}/tests/barcode
    WORKING_DIR ${CMAKE_SOURCE_DIR}/tests/barcode
)

add_lv_test(test_barcode_decode
    SOURCES ${CMAKE_SOURCE_DIR}/tests/barcode/test_barcode_decode.c
            ${CMAKE_SOURCE_DIR}/src/barcode/barcode_decode.c
            ${CMAKE_SOURCE_DIR}/src/barcode/barcode_validate.c
    INCLUDES ${STB_INCLUDE_DIR} ${ZBAR_INCLUDE_DIR}
    LINK_DIRS ${ZBAR_LIB_DIR}
    LINK_LIBS zbar
    OUTPUT_DIR ${CMAKE_SOURCE_DIR}/tests/barcode
    WORKING_DIR ${CMAKE_SOURCE_DIR}/tests/barcode
    COPY_DLLS ${ZBAR_RUNTIME_DLLS}
)

add_lv_test(test_error_messages
    SOURCES ${CMAKE_SOURCE_DIR}/tests/common/test_error_messages.c
            ${CMAKE_SOURCE_DIR}/src/common/error_messages.c
    OUTPUT_DIR ${CMAKE_SOURCE_DIR}/tests/common
    WORKING_DIR ${CMAKE_SOURCE_DIR}/tests/common
)
