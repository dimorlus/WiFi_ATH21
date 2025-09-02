cmake_minimum_required(VERSION 3.16)

project(HT_view LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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
