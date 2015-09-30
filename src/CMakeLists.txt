configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Version.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/Version.h)

set(kcm_SRCS
    main.cpp
    Module.cpp
    DriverManager.cpp
    DriverWidget.cpp
    Device.cpp
)

set_source_files_properties(${kcm_SRCS} PROPERTIES INCLUDE drivermanagerdbustypes.h)
set(drivermanagerobject_xml org.kubuntu.DriverManager.xml)
set_source_files_properties(${drivermanagerobject_xml} PROPERTIES INCLUDE drivermanagerdbustypes.h)
qt5_add_dbus_interface(kcm_SRCS ${drivermanagerobject_xml} DriverMangerInterface)

dbus_add_activation_session_service(org.kubuntu.DriverManager.service.in)

qt5_wrap_ui(kcm_SRCS Module.ui DriverWidget.ui)

add_library(kcm_driver_manager MODULE ${kcm_SRCS})

target_link_libraries(kcm_driver_manager
    Qt5::DBus
    KF5::ConfigWidgets
    KF5::I18n
    QApt::Main
    DebconfKDE::Main)

install(TARGETS kcm_driver_manager DESTINATION ${PLUGIN_INSTALL_DIR})
install(FILES kcm_driver_manager.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/DriverManager_DBus DESTINATION ${LIBEXEC_INSTALL_DIR})

