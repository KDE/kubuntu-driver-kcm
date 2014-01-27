project(kcm-drivermanager)

find_package(Qt4 4.7.0 REQUIRED)

find_package(KDE4 REQUIRED)
include(KDE4Defaults)

find_package(QApt)

add_definitions(${QT_DEFINITIONS} ${KDE4_DEFINITIONS})
add_definitions(-std=c++0x)
include_directories(${KDE4_INCLUDES} ${QT_INCLUDES} ${QAPT_INCLUDE_DIR})

add_subdirectory(src)
