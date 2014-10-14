
if( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
    set( ERASE_FILES  MailComposeDelegate.h MailComposeDelegate.mm )
    set( ADDED_CPP_PATTERNS  *.m *.mm  )
endif()

define_source_files (
GLOB_CPP_PATTERNS *.cpp ${ADDED_CPP_PATTERNS}
GLOB_H_PATTERNS   *.h   ${ADDED_H_PATTERNS}
GLOB_ERASE_FILES   ${ERASE_FILES}
PARENT_SCOPE)