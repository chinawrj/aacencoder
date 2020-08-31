cmake_minimum_required(VERSION 2.8)

set(CMAKE_SYSTEM_NAME LINUX)

set(TOP_DIR "${CMAKE_SOURCE_DIR}")
set(VOAAC_DIR "${TOP_DIR}/thirdparty/voaac")

# include files
include_directories(${TOP_DIR})
include_directories(${VOAAC_DIR}/include)

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

# cflags
set(CMAKE_C_FLAGS   "-DHAVE_STDINT_H -DSTDC_HEADERS -Wall -Werror")
set(CMAKE_CXX_FLAGS "-Wall -Werror")

# libaudioencoder
add_library(audioencoder   SHARED ${TOP_DIR}/VoAACEncoder.cpp ${VOAAC_SRC})
add_library(audioencoder_s STATIC ${TOP_DIR}/VoAACEncoder.cpp ${VOAAC_SRC})
