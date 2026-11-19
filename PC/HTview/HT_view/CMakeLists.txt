cmake_minimum_required(VERSION 3.16)

project(HT_view LANGUAGES CXX)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Исправление совместимости Qt 6.6.0 и VS2026 / MSVC v144+
if(MSVC)
    # 1. Генерируем реальный файл-заглушку прямо в папке сборки
    set(HACK_FILE_PATH "${CMAKE_CURRENT_BINARY_DIR}/msvc_qt_compat_hack.h")
    file(WRITE "${HACK_FILE_PATH}" "
#pragma once
#include <iterator>
namespace std {
    template<typename T>
    inline T* make_checked_array_iterator(T* p, size_t, size_t = 0) {
        return p;
    }
}
")

    # 2. Подменяем макрос stdext на std
    add_compile_definitions(stdext=std)

    # 3. Принудительно подключаем созданный файл ко всем исходникам проекта
    add_compile_options("/FI${HACK_FILE_PATH}")
endif()


# ВКЛЮЧИТЬ MOC для Qt
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)


# ОБЫЧНАЯ динамическая линковка Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Widgets PrintSupport)

# Исходные файлы
set(SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    qcustomplot.cpp 
    qcustomplot.h
    csvparser.h
    csvparser.cpp
    csvworker.h
    csvworker.cpp
    app.rc
)

add_executable(HT_view ${SOURCES})

# Динамическая линковка
target_link_libraries(HT_view 
    Qt6::Core 
    Qt6::Widgets 
    Qt6::PrintSupport
)

# Windows-specific
if(WIN32)
    set_target_properties(HT_view PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()
