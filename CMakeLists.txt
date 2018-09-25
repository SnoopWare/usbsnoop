cmake_minimum_required(VERSION 3.0)

project(usbsnoop)

include(${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/TargetArch/cmake/TargetArch.cmake)

target_architecture(ARCH)
message(STATUS "Target architecture is \"${ARCH}\"")

enable_language(ASM_MASM)
enable_language(RC)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/FindWDK/cmake")
find_package(WDK REQUIRED)

include("${CMAKE_CURRENT_SOURCE_DIR}/3rdParty/ucm/cmake/ucm.cmake")
ucm_set_runtime(STATIC)

set(CMAKE_MFC_FLAG 1)

set(SNOOPYPRO_CERT SnoopyProCert)

set(WINSDK ${WDK_ROOT}/bin/${WDK_VERSION}/x86)
set(OPENSSL "openssl.exe")
set(MAKECERT "${WINSDK}/makecert.exe")
set(CERTUTIL "certutil.exe")
set(CERTMGR "certmgr.exe")
set(CERT2SPC "cert2spc.exe")
set(PVK2PFX "${WINSDK}/pvk2pfx.exe")
set(SIGNTOOL "${WINSDK}/signtool.exe")

add_custom_command(OUTPUT ${SNOOPYPRO_CERT}.pfx
	COMMAND "${CMAKE_COMMAND}" -E remove ${SNOOPYPRO_CERT}.pvk ${SNOOPYPRO_CERT}.cer ${SNOOPYPRO_CERT}.pfx ${SNOOPYPRO_CERT}.spc
	COMMAND "${MAKECERT}" -b 09/09/2018 -r -n \"CN=SnoopWare\" -sv ${SNOOPYPRO_CERT}.pvk ${SNOOPYPRO_CERT}.cer
	COMMAND "${CERTMGR}" -add ${SNOOPYPRO_CERT}.cer -s -r localMachine ROOT
	COMMAND "${CERTMGR}" -add ${SNOOPYPRO_CERT}.cer -s -r localMachine TRUSTEDPUBLISHER
	COMMAND "${CERT2SPC}" ${SNOOPYPRO_CERT}.cer ${SNOOPYPRO_CERT}.spc
	COMMAND "${PVK2PFX}" -pvk ${SNOOPYPRO_CERT}.pvk -spc ${SNOOPYPRO_CERT}.spc -pfx ${SNOOPYPRO_CERT}.pfx
	WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
	COMMENT "Generating SSL certificates to sign the drivers and executable ...")

add_custom_target(${SNOOPYPRO_CERT}
	DEPENDS ${SNOOPYPRO_CERT}.pfx)

set(USBSNOOP USBSnoop)

file(GLOB_RECURSE USBSNOOP_SRC
	"${CMAKE_CURRENT_SOURCE_DIR}/${USBSNOOP}/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/${USBSNOOP}/*.rc"
	"${CMAKE_CURRENT_SOURCE_DIR}/Shared/*.cpp")

wdk_add_driver(${USBSNOOP} ${USBSNOOP_SRC})
add_dependencies(${USBSNOOP} ${SNOOPYPRO_CERT})
target_compile_definitions(${USBSNOOP} PRIVATE DRIVER UNICODE _UNICODE)
target_include_directories(${USBSNOOP} PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/${USBSNOOP}")
target_include_directories(${USBSNOOP} PRIVATE
        "${CMAKE_CURRENT_SOURCE_DIR}/Shared")

add_custom_command(TARGET ${USBSNOOP}
	POST_BUILD
	COMMAND "${SIGNTOOL}" sign /v /f ${SNOOPYPRO_CERT}.pfx /t http://timestamp.verisign.com/scripts/timstamp.dll $<TARGET_FILE:${USBSNOOP}>
        COMMENT "Signing $<TARGET_FILE:${USBSNOOP} ...")

set(USBSNPYS USBSnpys)

file(GLOB_RECURSE USBSNPYS_SRC
	"${CMAKE_CURRENT_SOURCE_DIR}/${USBSNPYS}/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/${USBSNPYS}/*.rc"
	"${CMAKE_CURRENT_SOURCE_DIR}/Shared/*.cpp")

wdk_add_driver(${USBSNPYS} ${USBSNPYS_SRC})
add_dependencies(${USBSNPYS} ${SNOOPYPRO_CERT})
target_compile_definitions(${USBSNPYS} PRIVATE DRIVER UNICODE _UNICODE)
target_include_directories(${USBSNPYS} PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/${USBSNPYS}")
target_include_directories(${USBSNPYS} PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/Shared")

add_custom_command(TARGET ${USBSNPYS}
	POST_BUILD
	COMMAND "${SIGNTOOL}" sign /v /f ${SNOOPYPRO_CERT}.pfx /t http://timestamp.verisign.com/scripts/timstamp.dll $<TARGET_FILE:${USBSNPYS}>
	COMMENT "Signing $<TARGET_FILE:${USBSNPYS} ...")

set(SNOOPYPRO SnoopyPro)

file(GLOB_RECURSE SNOOPYPRO_SRC
	"${CMAKE_CURRENT_SOURCE_DIR}/${SNOOPYPRO}/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/${SNOOPYPRO}/*.rc")

add_executable(${SNOOPYPRO} ${SNOOPYPRO_SRC})
add_dependencies(${SNOOPYPRO} ${USBSNOOP} ${USBSNPYS} ${SNOOPYPRO_CERT})
target_link_libraries(${SNOOPYPRO} Setupapi)
target_compile_definitions(${SNOOPYPRO} PRIVATE UNICODE _UNICODE _AFX)
target_compile_definitions(${SNOOPYPRO} PRIVATE BIN_USBSNOOP_FILENAME="$<TARGET_FILE:${USBSNOOP}>")
target_compile_definitions(${SNOOPYPRO} PRIVATE BIN_USBSNPYS_FILENAME="$<TARGET_FILE:${USBSNPYS}>")
target_include_directories(${SNOOPYPRO} PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/${SNOOPYPRO}")
target_include_directories(${SNOOPYPRO} PRIVATE
	"${CMAKE_CURRENT_SOURCE_DIR}/Shared")
set_target_properties(${SNOOPYPRO} PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /MANIFEST:NO")
set_target_properties(${SNOOPYPRO} PROPERTIES OUTPUT_NAME ${PROJECT_NAME}_${ARCH})

if (${ARCH} STREQUAL "i386")
target_compile_definitions(${SNOOPYPRO} PUBLIC SNOOPYPRO_MANIFEST="${SNOOPYPRO}.exe.x86.manifest")
add_custom_command(TARGET ${SNOOPYPRO}
	POST_BUILD 
	COMMAND "mt.exe" -manifest \"${CMAKE_CURRENT_SOURCE_DIR}\\${SNOOPYPRO}\\${SNOOPYPRO}.exe.x86.manifest\" -outputresource:\"$<TARGET_FILE:${SNOOPYPRO}>\"\;\#1
	COMMAND "${SIGNTOOL}" sign /v /f ${SNOOPYPRO_CERT}.pfx /t http://timestamp.verisign.com/scripts/timstamp.dll $<TARGET_FILE:${SNOOPYPRO}>
	COMMENT "Adding custom manifest file and signing the executable ...")
elseif (${ARCH} STREQUAL "x86_64")
target_compile_definitions(${SNOOPYPRO} PUBLIC SNOOPYPRO_MANIFEST="${SNOOPYPRO}.exe.x64.manifest")
add_custom_command(TARGET ${SNOOPYPRO}
	POST_BUILD
	COMMAND "mt.exe" -manifest \"${CMAKE_CURRENT_SOURCE_DIR}\\${SNOOPYPRO}\\${SNOOPYPRO}.exe.x64.manifest\" -outputresource:\"$<TARGET_FILE:${SNOOPYPRO}>\"\;\#1
	COMMAND "${SIGNTOOL}" sign /v /f ${SNOOPYPRO_CERT}.pfx /t http://timestamp.digicert.com $<TARGET_FILE:${SNOOPYPRO}>
	COMMENT "Adding custom manifest file and signing the executable ...")
else()
message(FATAL_ERROR "Unsupported target architecture \"${ARCH}\"")
endif()

set_target_properties(${SNOOPYPRO}
    PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")

