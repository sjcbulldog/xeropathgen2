cmake_minimum_required(VERSION 3.5)

project(xeropath2 VERSION 2.0 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Widgets Charts)
find_package(Qt6 REQUIRED COMPONENTS Widgets Charts)

include(sources.cmake)

qt_add_executable(xeropath2
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
)

target_link_libraries(xeropath2 PRIVATE Qt6::Widgets)
target_link_libraries(xeropath2 PRIVATE Qt6::Charts)

set_target_properties(xeropath2 PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER xeropath2
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE FALSE
)

install(TARGETS xeropath2
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

qt_finalize_executable(xeropath2)
