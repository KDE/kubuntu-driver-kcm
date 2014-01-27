project(kcm-drivermanager)

find_package(Qt4 4.7.0 REQUIRED)

find_package(KDE4 REQUIRED)
include(KDE4Defaults)

find_package(QApt)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
add_definitions(-std=c++0x)
include_directories(${KDE4_INCLUDES} ${QT_INCLUDES} ${QAPT_INCLUDE_DIR})

# This macro is defined in FindPolkitQt.cmake
macro(dbus_add_activation_system_service _sources)
    foreach (_i ${_sources})
        get_filename_component(_service_file ${_i} ABSOLUTE)
        string(REGEX REPLACE "\\.service.*$" ".service" _output_file ${_i})
        set(_target ${CMAKE_CURRENT_BINARY_DIR}/${_output_file})
        configure_file(${_service_file} ${_target})
        install(FILES ${_target} DESTINATION ${CMAKE_INSTALL_PREFIX}/share/dbus-1/system-services )
        #install(FILES ${_target} DESTINATION ${_install_dir})
    endforeach (_i ${ARGN})
endmacro(dbus_add_activation_system_service _sources)

add_subdirectory(src)
