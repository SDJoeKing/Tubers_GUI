cmake_minimum_required(VERSION 3.5)

project(Tubers_GUI VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Charts Network PrintSupport)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/source SOURCE_LIB)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Tubers_GUI
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        mtcpclient.cpp mtcpclient.h tgate.cpp tgate.h tsettings.cpp tsettings.h tsettings.ui
        tlogging.h tlogging.cpp tlogging.ui
        tchartview.h tchartview.cpp
        tchartviewform.h tchartviewform.cpp tchartviewform.ui
        styleConfig.h

        qcustomplot.cpp qcustomplot.h
        ${SOURCE_LIB}
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET Tubers_GUI APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(Tubers_GUI SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(Tubers_GUI
            ${PROJECT_SOURCES}
        )
    endif()
endif()

string(TOLOWER ${CMAKE_BUILD_TYPE} build_type)
if(build_type STREQUAL release)
    target_compile_definitions(Tubers_GUI PUBLIC QT_NO_DEBUG_OUTPUT)
endif()

target_link_libraries(Tubers_GUI PRIVATE Qt6::Core Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Charts Qt${QT_VERSION_MAJOR}::Network Qt${QT_VERSION_MAJOR}::PrintSupport )

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.Tubers_GUI)
endif()
set_target_properties(Tubers_GUI PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(Tubers_GUI PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)


include(GNUInstallDirs)
install(TARGETS Tubers_GUI
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Tubers_GUI)
endif()
