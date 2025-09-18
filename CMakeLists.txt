cmake_minimum_required(VERSION 3.5.0)

# set(Qt5_DIR "/home/arendjan/Qt/5.12.5/gcc_64/lib/cmake/Qt5")
#set(Qt5_DIR "/Users/arendjan/Qt/5.12.5/clang_64/lib/cmake/Qt5")
set(Qt5_DIR "D:/Qt/5.12.5/mingw73_64/lib/cmake/Qt5")

project(Numpyviewer)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 11)

# Find the QtWidgets library
find_package(Qt5 REQUIRED COMPONENTS Widgets Core Charts Network Svg)
find_package(Qt5Charts REQUIRED)
find_package(ZLIB)

list(APPEND RESOURCES icons.qrc)
qt5_add_resources(RCC_SOURCES ${RESOURCES})

list(APPEND NumpyviewerBin_src ${PROJECT_SOURCE_DIR}/src/main.cpp)

list(APPEND NumpyviewerLib_ui ${PROJECT_SOURCE_DIR}/mainwindow.ui)
list(APPEND NumpyviewerLib_ui ${PROJECT_SOURCE_DIR}/convertwindow.ui)
list(APPEND NumpyviewerLib_ui ${PROJECT_SOURCE_DIR}/histogram.ui)

list(APPEND NumpyviewerLib_src ${PROJECT_SOURCE_DIR}/src/cnpy.cpp)
list(APPEND NumpyviewerLib_src ${PROJECT_SOURCE_DIR}/src/convertwindow.cpp)
list(APPEND NumpyviewerLib_src ${PROJECT_SOURCE_DIR}/src/envi_parser.cpp)
list(APPEND NumpyviewerLib_src ${PROJECT_SOURCE_DIR}/src/filedownloader.cpp)
list(APPEND NumpyviewerLib_src ${PROJECT_SOURCE_DIR}/src/graphics_view_zoom.cpp)
list(APPEND NumpyviewerLib_src ${PROJECT_SOURCE_DIR}/src/histogram.cpp)
list(APPEND NumpyviewerLib_src ${PROJECT_SOURCE_DIR}/src/ini.c)
list(APPEND NumpyviewerLib_src ${PROJECT_SOURCE_DIR}/src/INIReader.cpp)
list(APPEND NumpyviewerLib_src ${PROJECT_SOURCE_DIR}/src/keyeventhandler.cpp)
list(APPEND NumpyviewerLib_src ${PROJECT_SOURCE_DIR}/src/colormap.cpp)
list(APPEND NumpyviewerLib_src ${PROJECT_SOURCE_DIR}/src/mainwindow.cpp)

list(APPEND NumpyviewerLib_hdr ${PROJECT_SOURCE_DIR}/src/convertwindow.h)
list(APPEND NumpyviewerLib_hdr ${PROJECT_SOURCE_DIR}/src/mainwindow.h)
list(APPEND NumpyviewerLib_hdr ${PROJECT_SOURCE_DIR}/src/histogram.h)
list(APPEND NumpyviewerLib_hdr ${PROJECT_SOURCE_DIR}/src/keyeventhandler.h)
list(APPEND NumpyviewerLib_hdr ${PROJECT_SOURCE_DIR}/src/graphics_view_zoom.h)
list(APPEND NumpyviewerLib_hdr ${PROJECT_SOURCE_DIR}/src/envi_parser.h)
list(APPEND NumpyviewerLib_hdr ${PROJECT_SOURCE_DIR}/src/filedownloader.h)
list(APPEND NumpyviewerLib_hdr ${PROJECT_SOURCE_DIR}/src/ini.h)
list(APPEND NumpyviewerLib_hdr ${PROJECT_SOURCE_DIR}/src/INIReader.h)
list(APPEND NumpyviewerLib_hdr ${PROJECT_SOURCE_DIR}/src/cnpy.h)
list(APPEND NumpyviewerLib_hdr ${PROJECT_SOURCE_DIR}/src/colormap.h)

qt5_wrap_cpp(NumpyviewerLib_hdr_moc ${NumpyviewerLib_hdr})
qt5_wrap_ui(NumpyviewerLib_ui_moc ${NumpyviewerLib_ui})

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_BINARY_DIR})

add_library(NumpyviewerLib SHARED
        ${NumpyviewerLib_src}
        ${NumpyviewerLib_hdr_moc}
        ${NumpyviewerLib_ui_moc}
        ${RCC_SOURCES}
        )
target_link_libraries(NumpyviewerLib Qt5::Core Qt5::Network Qt5::Widgets Qt5::Charts Qt5::Svg ${ZLIB_LIBRARIES})

set_property(TARGET NumpyviewerLib PROPERTY CXX_STANDARD 11)
set_property(TARGET NumpyviewerLib PROPERTY CXX_STANDARD_REQUIRED ON)

add_executable(Numpyviewer ${NumpyviewerBin_src})
target_link_libraries(Numpyviewer NumpyviewerLib)