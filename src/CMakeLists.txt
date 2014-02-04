configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/Version.h)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(kcm_SRCS
    main.cpp
    Module.cpp
    DriverWidget.cpp
)

set_source_files_properties(${kcm_SRCS} PROPERTIES INCLUDE drivermanagerdbustypes.h)
set(drivermanagerobject_xml org.kubuntu.DriverManager.xml)
set_source_files_properties(${drivermanagerobject_xml} PROPERTIES INCLUDE drivermanagerdbustypes.h)
qt4_add_dbus_interface(kcm_SRCS ${drivermanagerobject_xml} drivermanager_interface)

dbus_add_activation_session_service(org.kubuntu.DriverManager.service.in)

kde4_add_ui_files(kcm_SRCS Module.ui DriverWidget.ui)
kde4_add_plugin(kcm_driver_manager ${kcm_SRCS})

target_link_libraries(kcm_driver_manager
    ${KDE4_KDEUI_LIBRARY}
    ${KDE4_SOLID_LIBRARY}
    ${QAPT_LIBRARY}
)

install(TARGETS kcm_driver_manager DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kcm_driver_manager.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/DriverManager_DBus DESTINATION ${KDE4_LIBEXEC_INSTALL_DIR})