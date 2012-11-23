#------------------------
# ApplicationLib library

#set local path
LOCAL_PATH := $(call my-dir)

# clear all variables
include $(CLEAR_VARS)

# set module name
LOCAL_MODULE := UnitTestsLib

# set path for includes
LOCAL_C_INCLUDES := $(LOCAL_PATH)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../Classes


# set exported includes
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_C_INCLUDES)

# set source files
LOCAL_SRC_FILES :=  \
                    AndroidLayer.cpp \
                    AndroidDelegate.cpp \
                    \
                    ../../Classes/BaseScreen.cpp \
                    ../../Classes/EntityTest.cpp \
                    ../../Classes/FrameworkMain.cpp \
                    ../../Classes/GameCore.cpp \
                    ../../Classes/HashMapTest.cpp \
                    ../../Classes/KeyedArchiveTest.cpp \
                    ../../Classes/KeyedArchiveYamlTest.cpp \
                    ../../Classes/MemoryAllocatorsTest.cpp \
                    ../../Classes/PVRTest.cpp \
                    ../../Classes/SampleTest.cpp \
                    ../../Classes/SoundTest.cpp \
                    ../../Classes/SplitTest.cpp \
                    ../../Classes/TestTemplate.cpp \
                    ../../Classes/TextureUtils.cpp \

LOCAL_CFLAGS := -g -O2

LOCAL_LDLIBS := -lz -lOpenSLES -landroid -fuse-ld=gold -fno-exceptions

# set included libraries
#LOCAL_STATIC_LIBRARIES := libInternal libbox2d
LOCAL_STATIC_LIBRARIES := libInternal

# build shared library
include $(BUILD_SHARED_LIBRARY)

# include modules
$(call import-module, Internal)