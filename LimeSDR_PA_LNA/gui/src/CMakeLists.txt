########################################################################
## Create lime-suite project
########################################################################
cmake_minimum_required(VERSION 3.1.3)
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo" CACHE TYPE INTERNAL FORCE)
project("PA_CONTROL")

#include modules for finding CyAPI
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")

cmake_policy(SET CMP0015 OLD)
if(${CMAKE_MAJOR_VERSION} GREATER 2)
    cmake_policy(SET CMP0043 NEW)
endif()

########################################################################
## compiler flags
########################################################################

#gcc c++11 enable flags
if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
endif()

#visual studio compiler flags
if (MSVC)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    set( MSVC_MULTITHREAD_COMPILE_FLAGS "/MP")
    set( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${MSVC_MULTITHREAD_COMPILE_FLAGS}" )
endif(MSVC)

#enable c++11 extensions for OSX
if (APPLE)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -Wc++11-extensions")
   add_definitions(-D__unix__=1) #we use this for unix detection, but clang does not define it
endif(APPLE)

########################################################################
## public headers
########################################################################
set(PA_CONTROL_PUBLIC_HEADERS
    ErrorReporting.h
    PAControlConfig.h
    ConnectionRegistry/IConnection.h
    ConnectionRegistry/ConnectionHandle.h
    ConnectionRegistry/ConnectionRegistry.h
    protocols/ADCUnits.h
    protocols/LMS64CCommands.h
    protocols/LMS64CProtocol.h
    protocols/LMSBoards.h
)

include(FeatureSummary)
include(CMakeDependentOption)

########################################################################
## lime suite library
########################################################################

set(LIME_SUITE_SOURCES
    ErrorReporting.cpp
    ConnectionRegistry/IConnection.cpp
    ConnectionRegistry/ConnectionHandle.cpp
    ConnectionRegistry/ConnectionRegistry.cpp
    protocols/LMS64CProtocol.cpp
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(ConnectionRegistry)
include_directories(sonyPA)
include_directories(protocols)
include_directories(lms7suiteEvents)

include(FeatureSummary)
include(CMakeDependentOption)
cmake_dependent_option(ENABLE_LIBRARY "Enable build library" ON "TRUE" OFF)
add_feature_info(LimeSuiteLibrary ENABLE_LIBRARY "The lime suite library")
if (ENABLE_LIBRARY)
    add_library(LimeSuite ${LIME_SUITE_SOURCES})
    set_property(TARGET LimeSuite PROPERTY POSITION_INDEPENDENT_CODE TRUE)
    set_target_properties(LimeSuite PROPERTIES DEFINE_SYMBOL "LIME_DLL")

    set(LIME_SUITE_LIBS LimeSuite)
endif()

########################################################################
## Supported connections
########################################################################
include(ConnectionEVB7COM/CMakeLists.txt)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/ConnectionRegistry/BuiltinConnections.in.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/BuiltinConnections.cpp
)

target_sources(LimeSuite PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/BuiltinConnections.cpp)

########################################################################
## wx widgets dependency
########################################################################
set(wxWidgets_USE_DEBUG OFF)
set(wxWidgets_USE_UNICODE ON)
set(wxWidgets_USE_UNIVERSAL OFF)
set(wxWidgets_USE_STATIC ON)
find_package(wxWidgets COMPONENTS richtext xml html xrc adv aui core base gl)
message(STATUS "wxWidgets_DEFINITIONS: ${wxWidgets_DEFINITIONS}")
message(STATUS "wxWidgets_INCLUDE_DIRS: ${wxWidgets_INCLUDE_DIRS}")
message(STATUS "wxWidgets_LIBRARIES: ${wxWidgets_LIBRARIES}")

########################################################################
## lms7suite gui app
########################################################################
set(LMS7SUITE_GUI_SOURCES
    utilities_gui/dlgConnectionSettings.cpp
    utilities_gui/pnlMiniLog.cpp
    utilities_gui/SPI_wxgui.cpp
    utilities_gui/dlgFullMessageLog.cpp
    utilities_gui/dlgDeviceInfo.cpp
    utilities_gui/utilities_gui.cpp
    lms7suiteAppFrame.cpp
    lms7suiteApp_gui.cpp
    lms7suiteApp.cpp
    sonyPA/sonyPA_wxgui.cpp
    lms7suiteEvents/lms7suiteEvents.cpp
)

include(FeatureSummary)
include(CMakeDependentOption)
cmake_dependent_option(ENABLE_LMS7_GUI "Enable build lms7suite" ON "ENABLE_LIBRARY;wxWidgets_FOUND" OFF)
add_feature_info(PAControlGUILMS7 ENABLE_LMS7_GUI "GUI Application for LMS7")
if (ENABLE_LMS7_GUI)
    include_directories(utilities_gui)
    include(${wxWidgets_USE_FILE})
   
    set(BINARY_OUTPUT_DIR "${CMAKE_BINARY_DIR}/bin")

    add_executable(paControl ${LMS7SUITE_GUI_SOURCES} resource.rc)
    set_target_properties(paControl PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${BINARY_OUTPUT_DIR})
    target_link_libraries(paControl ${LIME_SUITE_LIBS} ${wxWidgets_LIBRARIES})

    if (MSVC)
        set_property(TARGET paControl APPEND PROPERTY LINK_FLAGS /SUBSYSTEM:WINDOWS)
    endif(MSVC)

endif()

#########################################################################
# summary
#########################################################################
include(FeatureSummary)
message(STATUS "")
message(STATUS "######################################################")
message(STATUS "## ${PROJECT_NAME} enabled features")
message(STATUS "######################################################")
feature_summary(WHAT ENABLED_FEATURES)
message(STATUS "######################################################")
message(STATUS "## ${PROJECT_NAME} disabled features")
message(STATUS "######################################################")
feature_summary(WHAT DISABLED_FEATURES)
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Lime Suite Version: ${LIME_SUITE_VERSION}")
message(STATUS "Lime Suite ABI: ${LIME_SUITE_ABI}")

