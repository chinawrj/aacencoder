cmake_minimum_required(VERSION 3.4.1)

project(AACEncoder)

set(CMAKE_SYSTEM_NAME LINUX)

set(TOP_DIR "${CMAKE_SOURCE_DIR}")
set(VOAAC_DIR "${TOP_DIR}/thirdparty/vo-aac")
set(FDKAAC_DIR "${TOP_DIR}/thirdparty/fdk-aac")

# include files
include_directories(${TOP_DIR})
include_directories(${VOAAC_DIR}/include)
include_directories(${FDKAAC_DIR}/libAACenc/include)
include_directories(${FDKAAC_DIR}/libArithCoding/include)
include_directories(${FDKAAC_DIR}/libFDK/include)
include_directories(${FDKAAC_DIR}/libMpegTPEnc/include)
include_directories(${FDKAAC_DIR}/libPCMutils/include)
include_directories(${FDKAAC_DIR}/libSACenc/include)
include_directories(${FDKAAC_DIR}/libSBRenc/include)
include_directories(${FDKAAC_DIR}/libSYS/include)

# VOAAC_SRC source files
set (VOAAC_SRC
    ${VOAAC_DIR}/common/cmnMemory.c
    ${VOAAC_DIR}/aacenc/basic_op/basicop2.c
    ${VOAAC_DIR}/aacenc/basic_op/oper_32b.c
    ${VOAAC_DIR}/aacenc/src/aac_rom.c
    ${VOAAC_DIR}/aacenc/src/aacenc.c
    ${VOAAC_DIR}/aacenc/src/aacenc_core.c
    ${VOAAC_DIR}/aacenc/src/adj_thr.c
    ${VOAAC_DIR}/aacenc/src/band_nrg.c
    ${VOAAC_DIR}/aacenc/src/bit_cnt.c
    ${VOAAC_DIR}/aacenc/src/bitbuffer.c
    ${VOAAC_DIR}/aacenc/src/bitenc.c
    ${VOAAC_DIR}/aacenc/src/block_switch.c
    ${VOAAC_DIR}/aacenc/src/channel_map.c
    ${VOAAC_DIR}/aacenc/src/dyn_bits.c
    ${VOAAC_DIR}/aacenc/src/grp_data.c
    ${VOAAC_DIR}/aacenc/src/interface.c
    ${VOAAC_DIR}/aacenc/src/line_pe.c
    ${VOAAC_DIR}/aacenc/src/ms_stereo.c
    ${VOAAC_DIR}/aacenc/src/pre_echo_control.c
    ${VOAAC_DIR}/aacenc/src/psy_configuration.c
    ${VOAAC_DIR}/aacenc/src/psy_main.c
    ${VOAAC_DIR}/aacenc/src/qc_main.c
    ${VOAAC_DIR}/aacenc/src/quantize.c
    ${VOAAC_DIR}/aacenc/src/sf_estim.c
    ${VOAAC_DIR}/aacenc/src/spreading.c
    ${VOAAC_DIR}/aacenc/src/stat_bits.c
    ${VOAAC_DIR}/aacenc/src/tns.c
    ${VOAAC_DIR}/aacenc/src/transform.c
    ${VOAAC_DIR}/aacenc/src/memalign.c
)

# FDKAAC_SRC source files
file(GLOB FDK_SRC src
    ${FDKAAC_DIR}/libAACenc/src/*.cpp
    ${FDKAAC_DIR}/libArithCoding/src/*.cpp
    ${FDKAAC_DIR}/libFDK/src/*.cpp
    ${FDKAAC_DIR}/libMpegTPEnc/src/*.cpp
    ${FDKAAC_DIR}/libPCMutils/src/*.cpp
    ${FDKAAC_DIR}/libSACenc/src/*.cpp
    ${FDKAAC_DIR}/libSBRenc/src/*.cpp
    ${FDKAAC_DIR}/libSYS/src/*.cpp
)

# cflags
set(CMAKE_C_FLAGS   "-Wall -Werror")
set(CMAKE_CXX_FLAGS "-Wall -Werror")

# libvoaacencoder
add_library(voaacencoder   SHARED ${TOP_DIR}/VoAACEncoder.cpp ${VOAAC_SRC})
add_library(voaacencoder_s STATIC ${TOP_DIR}/VoAACEncoder.cpp ${VOAAC_SRC})

# libfdkaacencoder
add_library(fdkaacencoder   SHARED ${TOP_DIR}/FdkAACEncoder.cpp ${FDK_SRC})
add_library(fdkaacencoder_s STATIC ${TOP_DIR}/FdkAACEncoder.cpp ${FDK_SRC})
