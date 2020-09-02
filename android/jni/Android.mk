LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
TOP_DIR := ${LOCAL_PATH}/../..
VOAAC_DIR := ${TOP_DIR}/thirdparty/vo-aac
LOCAL_SRC_FILES := \
    ${VOAAC_DIR}/common/cmnMemory.c \
    ${VOAAC_DIR}/aacenc/basic_op/basicop2.c \
    ${VOAAC_DIR}/aacenc/basic_op/oper_32b.c \
    ${VOAAC_DIR}/aacenc/src/aac_rom.c \
    ${VOAAC_DIR}/aacenc/src/aacenc.c \
    ${VOAAC_DIR}/aacenc/src/aacenc_core.c \
    ${VOAAC_DIR}/aacenc/src/adj_thr.c \
    ${VOAAC_DIR}/aacenc/src/band_nrg.c \
    ${VOAAC_DIR}/aacenc/src/bit_cnt.c \
    ${VOAAC_DIR}/aacenc/src/bitbuffer.c \
    ${VOAAC_DIR}/aacenc/src/bitenc.c \
    ${VOAAC_DIR}/aacenc/src/block_switch.c \
    ${VOAAC_DIR}/aacenc/src/channel_map.c \
    ${VOAAC_DIR}/aacenc/src/dyn_bits.c \
    ${VOAAC_DIR}/aacenc/src/grp_data.c \
    ${VOAAC_DIR}/aacenc/src/interface.c \
    ${VOAAC_DIR}/aacenc/src/line_pe.c \
    ${VOAAC_DIR}/aacenc/src/ms_stereo.c \
    ${VOAAC_DIR}/aacenc/src/pre_echo_control.c \
    ${VOAAC_DIR}/aacenc/src/psy_configuration.c \
    ${VOAAC_DIR}/aacenc/src/psy_main.c \
    ${VOAAC_DIR}/aacenc/src/qc_main.c \
    ${VOAAC_DIR}/aacenc/src/quantize.c \
    ${VOAAC_DIR}/aacenc/src/sf_estim.c \
    ${VOAAC_DIR}/aacenc/src/spreading.c \
    ${VOAAC_DIR}/aacenc/src/stat_bits.c \
    ${VOAAC_DIR}/aacenc/src/tns.c \
    ${VOAAC_DIR}/aacenc/src/transform.c \
    ${VOAAC_DIR}/aacenc/src/memalign.c \
    ${TOP_DIR}/VoAACEncoder.cpp
LOCAL_C_INCLUDES += ${TOP_DIR} ${VOAAC_DIR}/include
LOCAL_CFLAGS += -Wall -Werror
LOCAL_LDLIBS := -llog
LOCAL_MODULE := libvoaacencoder
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
TOP_DIR := ${LOCAL_PATH}/../..
FDKAAC_DIR := ${TOP_DIR}/thirdparty/fdk-aac
FDKAAC_SRC_LIST := $(wildcard ${FDKAAC_DIR}/libAACenc/src/*.cpp)
FDKAAC_SRC_LIST += $(wildcard ${FDKAAC_DIR}/libArithCoding/src/*.cpp)
FDKAAC_SRC_LIST += $(wildcard ${FDKAAC_DIR}/libFDK/src/*.cpp)
FDKAAC_SRC_LIST += $(wildcard ${FDKAAC_DIR}/libMpegTPEnc/src/*.cpp)
FDKAAC_SRC_LIST += $(wildcard ${FDKAAC_DIR}/libPCMutils/src/*.cpp)
FDKAAC_SRC_LIST += $(wildcard ${FDKAAC_DIR}/libSACenc/src/*.cpp)
FDKAAC_SRC_LIST += $(wildcard ${FDKAAC_DIR}/libSBRenc/src/*.cpp)
FDKAAC_SRC_LIST += $(wildcard ${FDKAAC_DIR}/libSYS/src/*.cpp)
LOCAL_SRC_FILES := $(FDKAAC_SRC_LIST) ${TOP_DIR}/FdkAACEncoder.cpp
LOCAL_C_INCLUDES += \
    ${TOP_DIR} \
    ${FDKAAC_DIR}/libAACenc/include \
    ${FDKAAC_DIR}/libArithCoding/include \
    ${FDKAAC_DIR}/libFDK/include \
    ${FDKAAC_DIR}/libMpegTPEnc/include \
    ${FDKAAC_DIR}/libPCMutils/include \
    ${FDKAAC_DIR}/libSACenc/include \
    ${FDKAAC_DIR}/libSBRenc/include \
    ${FDKAAC_DIR}/libSYS/include
LOCAL_CFLAGS += -Wall -Werror
LOCAL_LDLIBS := -llog
LOCAL_MODULE := libfdkaacencoder
include $(BUILD_SHARED_LIBRARY)
