cmake_minimum_required(VERSION 3.16)

project(SecurityDebuggingAssistant VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(LW_ZHONGRILONG_PROJ)

set(DM_SDK_PATH "D:/A_ProjectVS/DM_SDK_C")
set(SAFETYALG_PATH "D:/ThirdPartyLib/safetyAlgorithm")
set(VTK_DIR "D:/ThirdPartyLib/vtk9.2/lib/cmake/vtk-9.2")

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
find_package(VTK)

include_directories(${DM_SDK_PATH}/DM_SDK_C)
include_directories(${SAFETYALG_PATH}/include)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(SecurityDebuggingAssistant
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}

        customgraphicsview.h customgraphicsview.cpp
        custompixmapitem.h custompixmapitem.cpp
        settings.h settings.cpp
        custompolygonitem.h custompolygonitem.cpp
        customellipseitem.h customellipseitem.cpp
        image.qrc appico.rc
        deviceworker.h deviceworker.cpp

        logform.h logform.cpp logform.ui
        depthdrawform.h depthdrawform.cpp depthdrawform.ui
        deviceargsetform.h deviceargsetform.cpp deviceargsetform.ui
        networksetform.h networksetform.cpp networksetform.ui
        dialogform.h dialogform.cpp dialogform.ui
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET SecurityDebuggingAssistant APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(SecurityDebuggingAssistant SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(SecurityDebuggingAssistant
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(SecurityDebuggingAssistant PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(SecurityDebuggingAssistant PRIVATE ${VTK_LIBRARIES})
target_link_libraries(SecurityDebuggingAssistant PRIVATE ${DM_SDK_PATH}/out/build/x64-Release/DM_SDK_C/dm_c_sdk.lib)
target_link_libraries(SecurityDebuggingAssistant PRIVATE ${SAFETYALG_PATH}/lib/AlarmALGVisual.lib)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.SecurityDebuggingAssistant)
endif()
set_target_properties(SecurityDebuggingAssistant PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS SecurityDebuggingAssistant
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(SecurityDebuggingAssistant)
endif()
