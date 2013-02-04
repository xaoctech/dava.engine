APP_STL := gnustl_static
APP_GNUSTL_FORCE_CPP_FEATURES := rtti

APP_CFLAGS = -marm -g

APP_CFLAGS += -DNDK_DEBUG=1 -O0 #debug
APP_OPTIM := debug

APP_ABI := armeabi-v7a
