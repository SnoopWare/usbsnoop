cmake_minimum_required(VERSION 3.0)

project(usbsnoop)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/FindWDK/cmake")
find_package(WDK REQUIRED)

include("${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/ucm/cmake/ucm.cmake")
ucm_set_runtime(STATIC)

set(CMAKE_MFC_FLAG 1)

set(USBSNOOP USBSnoop)

file(GLOB_RECURSE USBSNOOP_SRC
	"${CMAKE_CURRENT_SOURCE_DIR}/${USBSNOOP}/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Shared/*.cpp")

wdk_add_driver(${USBSNOOP} ${USBSNOOP_SRC})
target_compile_definitions(${USBSNOOP} PRIVATE DRIVER UNICODE _UNICODE)
target_include_directories(${USBSNOOP} PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/${USBSNOOP}")
target_include_directories(${USBSNOOP} PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/Shared")

set(USBSNPYS USBSnpys)

file(GLOB_RECURSE USBSNPYS_SRC
	"${CMAKE_CURRENT_SOURCE_DIR}/${USBSNPYS}/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/Shared/*.cpp")

wdk_add_driver(${USBSNPYS} ${USBSNPYS_SRC})
target_compile_definitions(${USBSNPYS} PRIVATE DRIVER UNICODE _UNICODE)
target_include_directories(${USBSNPYS} PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/${USBSNPYS}")
target_include_directories(${USBSNPYS} PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/Shared")

set(SNOOPYPRO SnoopyPro)

file(GLOB_RECURSE SNOOPYPRO_SRC
	"${CMAKE_CURRENT_SOURCE_DIR}/${SNOOPYPRO}/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/${SNOOPYPRO}/*.rc")

add_executable(${SNOOPYPRO} ${SNOOPYPRO_SRC})
add_dependencies(${SNOOPYPRO} ${USBSNOOP} ${USBSNPYS})
target_link_libraries(${SNOOPYPRO} Setupapi)
target_compile_definitions(${SNOOPYPRO} PRIVATE UNICODE _UNICODE _AFX)
target_compile_definitions(${SNOOPYPRO} PRIVATE BIN_USBSNOOP_FILENAME="$<TARGET_FILE:${USBSNOOP}>")
target_compile_definitions(${SNOOPYPRO} PRIVATE BIN_USBSNPYS_FILENAME="$<TARGET_FILE:${USBSNPYS}>")
target_include_directories(${SNOOPYPRO} PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/${SNOOPYPRO}")
target_include_directories(${SNOOPYPRO} PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/Shared")
set_target_properties(${SNOOPYPRO} PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /MANIFEST:NO")

set_target_properties(${SNOOPYPRO}
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

