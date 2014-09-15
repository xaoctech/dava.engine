#-----------------------------
# PngLib lib

# set local path for lib
LOCAL_PATH := $(call my-dir)

# clear all variables
include $(CLEAR_VARS)

# set module name
LOCAL_MODULE := libfribidi_android

# set path for includes
LOCAL_C_INCLUDES := $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../lib
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../charset

# set exported includes
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_C_INCLUDES)

# set source files
LOCAL_SRC_FILES :=  \
                  ../../fribidi.c \
                  ../../fribidi-arabic.c \
                  ../../fribidi-bidi.c \
                  ../../fribidi-bidi-types.c \
                  ../../fribidi-deprecated.c \
                  ../../fribidi-joining.c \
                  ../../fribidi-joining-types.c \
                  ../../fribidi-mem.c \
                  ../../fribidi-mirroring.c \
                  ../../fribidi-run.c \
                  ../../fribidi-shape.c \
                  ../../fribidi-char-sets.c \
                  ../../fribidi-char-sets-cap-rtl.c \
                  ../../fribidi-char-sets-cp1255.c \
                  ../../fribidi-char-sets-cp1256.c \
                  ../../fribidi-char-sets-iso8859-6.c \
                  ../../fribidi-char-sets-iso8859-8.c \
                  ../../fribidi-char-sets-utf8.c \

LOCAL_CFLAGS := -O2

# build static library
include $(BUILD_STATIC_LIBRARY)