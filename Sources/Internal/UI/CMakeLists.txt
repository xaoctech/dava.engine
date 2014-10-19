

if( WIN32 )
    set( ERASE_FILES UITextFieldAndroid.cpp UITextFieldAndroid.h UIScreenManagerAndroid.h UIScreenManagerAndroid.cpp )

elseif(ANDROID)
    set( ERASE_FILES UIScreenManagerMacOS.cpp UIScreenManagerMacOS.h )

elseif(APPLE AND NOT IOS )
    set( ERASE_FILES UITextFieldAndroid.cpp UITextFieldAndroid.h )

elseif( IOS )
    set( ERASE_FILES UITextFieldAndroid.cpp UITextFieldAndroid.h UIScreenManagerMacOS.cpp UIScreenManagerMacOS.h UIScreenManagerAndroid.h UIScreenManagerAndroid.cpp )

endif()

define_source_files (
GLOB_ERASE_FILES  ${ERASE_FILES}
PARENT_SCOPE )


