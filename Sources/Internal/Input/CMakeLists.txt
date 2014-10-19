
if( APPLE AND NOT IOS )
    set( ERASE_FILES  GamepadDevice.mm GamepadManager.h GamepadManager.mm )

elseif( IOS )
    set( ERASE_FILES  GamepadDevice.cpp )

endif()


define_source_files (
GLOB_ERASE_FILES   ${ERASE_FILES}
PARENT_SCOPE)