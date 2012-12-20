#------------------------
# ApplicationLib library

#set local path
LOCAL_PATH := $(call my-dir)

MY_PROJECT_ROOT := $(LOCAL_PATH)/../..
DAVA_ROOT := $(MY_PROJECT_ROOT)/../..


# clear all variables
include $(CLEAR_VARS)

# set module name
LOCAL_MODULE := TemplateProjectNative

# set path for includes
LOCAL_C_INCLUDES := $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../Classes


# set exported includes
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_C_INCLUDES)

# set source files
LOCAL_SRC_FILES :=  \
                    main.cpp \
                    \
                    ../../Classes/FrameworkMain.cpp \
                    ../../Classes/GameCore.cpp \
                    ../../Classes/TestScreen.cpp \


LOCAL_CFLAGS := -g -O2 -Wno-deprecated -Wno-psabi -DOC_NEW_STYLE_INCLUDES -DOC_FACTOR_INTO_H_AND_CC

LOCAL_LDLIBS := -landroid
LOCAL_LDLIBS += -lz
LOCAL_LDLIBS += -lOpenSLES
LOCAL_LDLIBS += -fuse-ld=gold


# LOCAL_ARM_MODE := arm

# set included libraries
LOCAL_STATIC_LIBRARIES += libInternal

# build shared library
include $(BUILD_SHARED_LIBRARY)

# include modules
$(call import-add-path,$(DAVA_ROOT)/Sources)
$(call import-module,Internal)
