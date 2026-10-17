
cmake_minimum_required(VERSION 3.20)

project(ExedraSim LANGUAGES CXX)

# =========================================================
# ビルド設定
# =========================================================

# Makefiles などの単一構成ジェネレーターでは
# ビルドタイプが未指定なら Debug を使用する。
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(
        CMAKE_BUILD_TYPE Debug
        CACHE STRING "Build type" FORCE
    )
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)


# =========================================================
# ソースファイル
# =========================================================

set(IMGUI_BACKEND_SOURCES
    imgui/imgui_impl_glfw.cpp
    imgui/imgui_impl_opengl3.cpp
)

if(CMAKE_BUILD_TYPE STREQUAL "Release")

    # Release: Unity Build
    set(APP_SOURCES
        ${IMGUI_BACKEND_SOURCES}
        unity.cpp
    )

else()

    # Debug: 各 cpp を個別にコンパイル
    set(APP_SOURCES
        imgui/imgui.cpp
        imgui/imgui_draw.cpp
        imgui/imgui_tables.cpp
        imgui/imgui_widgets.cpp

        ${IMGUI_BACKEND_SOURCES}

        image_loader.cpp
        formation/formationView.cpp
        simulation/simulationView.cpp
        simulation/simulationLogic.cpp
        simulation/EffectList.cpp
        simulation/ProcessList.cpp
        main.cpp
    )

endif()


# =========================================================
# 実行ファイル
# =========================================================

add_executable(ExedraSim ${APP_SOURCES})

# =========================================================
# 同梱日本語フォント
# =========================================================

set(EXEDRA_FONT_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/resources/meiryo.ttc"
    "${CMAKE_CURRENT_SOURCE_DIR}/resources/meiryob.ttc"
)

foreach(EXEDRA_FONT_FILE IN LISTS EXEDRA_FONT_FILES)
    if(EXISTS "${EXEDRA_FONT_FILE}")
        get_filename_component(EXEDRA_FONT_NAME "${EXEDRA_FONT_FILE}" NAME)
        if(APPLE)
            # .app/Contents/Resources/ にフォントを格納する。
            set_source_files_properties("${EXEDRA_FONT_FILE}" PROPERTIES
                MACOSX_PACKAGE_LOCATION Resources
            )
            target_sources(ExedraSim PRIVATE "${EXEDRA_FONT_FILE}")
        elseif(NOT WIN32)
            # Linux は実行ファイルの隣の resources フォルダーに格納する。
            add_custom_command(TARGET ExedraSim POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                    "$<TARGET_FILE_DIR:ExedraSim>/resources"
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${EXEDRA_FONT_FILE}"
                    "$<TARGET_FILE_DIR:ExedraSim>/resources/${EXEDRA_FONT_NAME}"
                COMMENT "Copying bundled Japanese font: ${EXEDRA_FONT_NAME}"
            )
        endif()
    else()
        message(WARNING "${EXEDRA_FONT_FILE} is missing; the application may fall back to ImGui's default font.")
    endif()
endforeach()

# macOS では .app バンドルとして生成する
if(APPLE)
    set_target_properties(ExedraSim PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_BUNDLE_NAME "Exedra Sim"
    )
endif()


# =========================================================
# インクルードディレクトリ
# =========================================================

target_include_directories(ExedraSim PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/imgui"
    "${CMAKE_CURRENT_SOURCE_DIR}/glfw/include"
)


# =========================================================
# Windows 固有設定
# =========================================================

if(WIN32)

    enable_language(RC)

    target_sources(ExedraSim PRIVATE
        resources/app_icon.rc
    )

    target_link_directories(ExedraSim PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/lib"
        "${CMAKE_CURRENT_SOURCE_DIR}/glfw/lib"
    )

    target_link_libraries(ExedraSim PRIVATE
        webp
        glfw3
        opengl32
        gdi32
    )

    # 既存 Makefile の Release 設定を維持する。
    target_link_options(ExedraSim PRIVATE
        "$<$<CONFIG:Release>:-static>"
        "$<$<CONFIG:Release>:-static-libgcc>"
        "$<$<CONFIG:Release>:-static-libstdc++>"
        "$<$<CONFIG:Release>:-mwindows>"
    )

endif()

# =========================================================
# macOS 固有設定
# =========================================================

if(APPLE)

    find_package(PkgConfig REQUIRED)
    pkg_check_modules(WEBP REQUIRED IMPORTED_TARGET libwebp)

    find_package(glfw3 CONFIG REQUIRED)
    find_package(OpenGL REQUIRED)

    target_link_libraries(ExedraSim PRIVATE
        PkgConfig::WEBP
        glfw
        OpenGL::GL
    )

endif()



# =========================================================
# Linux 固有設定
# =========================================================

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")

    find_package(PkgConfig REQUIRED)
    pkg_check_modules(WEBP REQUIRED IMPORTED_TARGET libwebp)

    find_package(glfw3 CONFIG REQUIRED)
    find_package(OpenGL REQUIRED)

    target_link_libraries(ExedraSim PRIVATE
        PkgConfig::WEBP
        glfw
        OpenGL::GL
        ${CMAKE_DL_LIBS}
        pthread
    )

endif()

# =========================================================
# コンパイルオプション
# =========================================================

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")

    target_compile_options(ExedraSim PRIVATE
        -Wall
        "$<$<CONFIG:Debug>:-O0>"
        "$<$<CONFIG:Debug>:-g>"
        "$<$<CONFIG:Release>:-O2>"
    )

endif()
