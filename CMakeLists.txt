cmake_minimum_required(VERSION 3.16.0)

project(effect_particleeffects)

option(KWIN_BUILD_WAYLAND "Build with Wayland support" ON)

set(QT_MAJOR_VERSION "6")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(ECM ${QT_MAJOR_VERSION} REQUIRED NO_MODULE)

list(APPEND CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(FeatureSummary)

find_package(Qt${QT_MAJOR_VERSION} REQUIRED
    COMPONENTS
        Core Gui DBus
)

find_package(KF6 REQUIRED
    COMPONENTS
        Config ConfigWidgets KCMUtils GuiAddons I18n CoreAddons
)

message(STATUS "Include dirs: ${KF6Config_INCLUDE_DIRS}")
include_directories(${KF6I18n_INCLUDE_DIRS})


find_package(KWin REQUIRED COMPONENTS
    kwineffects
)
find_package(KWinDBusInterface CONFIG REQUIRED)

feature_summary(WHAT ALL)

set(kwin4_effect_particleeffect_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/particleeffect.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/particleeffect.qrc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/plugin.cpp
)
kconfig_add_kcfg_files(kwin4_effect_particleeffect_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/kcm/particleconfig.kcfgc
)

add_library(kwin_effect_particleeffect SHARED ${kwin4_effect_particleeffect_SOURCES})

target_link_libraries(kwin_effect_particleeffect
    PRIVATE
        Qt${QT_MAJOR_VERSION}::Core
        KF6::ConfigGui
        KF6::I18n
        KF6::KCMUtils
        KWin::kwin
)

set(kwin_particleeffect_config_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/src/kcm/particle_config.cpp)
ki18n_wrap_ui(kwin_particleeffect_config_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/src/kcm/particle_config.ui)
kconfig_add_kcfg_files(kwin_particleeffect_config_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/src/kcm/particleconfig.kcfgc)
qt_add_dbus_interface(kwin_particleeffect_config_SRCS ${KWIN_EFFECTS_INTERFACE} kwineffects_interface)

add_library(kwin_particleeffect_config MODULE ${kwin_particleeffect_config_SRCS})
target_link_libraries(kwin_particleeffect_config
    KF6::I18n
    KF6::KCMUtils
    KF6::CoreAddons
    Qt6::DBus
    KF6::WindowSystem
)

install(
    TARGETS
        kwin_effect_particleeffect
    DESTINATION
        ${KDE_INSTALL_PLUGINDIR}/kwin/effects/plugins
)

install(TARGETS kwin_particleeffect_config DESTINATION ${KDE_INSTALL_PLUGINDIR}/kwin/effects/configs)
