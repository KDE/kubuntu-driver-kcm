project(kcm-drivermanager)

cmake_minimum_required(VERSION 2.8.12)

find_package(ECM 0.0.14 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

include(FeatureSummary)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)

find_package(Qt5 5.0.0 CONFIG REQUIRED DBus)
find_package(KF5 5.0.0 REQUIRED
    COMPONENTS
        Config
        ConfigWidgets
        I18n
        TextWidgets)

find_package(QApt REQUIRED)
find_package(DebconfKDE REQUIRED)

macro(dbus_add_activation_session_service _sources)
    foreach (_i ${_sources})
        get_filename_component(_service_file ${_i} ABSOLUTE)
        string(REGEX REPLACE "\\.service.*$" ".service" _output_file ${_i})
        set(_target ${CMAKE_CURRENT_BINARY_DIR}/${_output_file})
        configure_file(${_service_file} ${_target})
        install(FILES ${_target} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/dbus-1/services )
        #install(FILES ${_target} DESTINATION ${_install_dir})
    endforeach (_i ${ARGN})
endmacro(dbus_add_activation_session_service _sources)

add_subdirectory(src)
