set( HPP_FILES *.h
    Components/*.h
    Layouts/*.h
    Layouts/Private/*.h
    Styles/*.h
    Focus/*.h
    Private/*.h
    Private/CEF/*.h
    Input/*.h
    Scroll/*.h
    Sound/*.h
    Update/*.h
    Formula/*.h
    Formula/Private/*.h
    RichContent/*.h
    RichContent/Private/*.h
    Text/*.h
    Text/Private/*.h
    Render/*.h
    )

set( CPP_FILES *.cpp
    Components/*.cpp
    Layouts/*.cpp
    Layouts/Private/*.cpp
    Styles/*.cpp
    Focus/*.cpp
    Private/*.cpp
    Private/CEF/*.cpp
    Input/*.cpp
    Scroll/*.cpp
    Sound/*.cpp
    Update/*.cpp
    Formula/*.cpp
    Formula/Private/*.cpp
    RichContent/*.cpp
    RichContent/Private/*.cpp
    Text/*.cpp
    Text/Private/*.cpp
    Render/*.cpp
    )

set( CPP_FILES_APPLE *.mm )

set( HPP_FILES_ANDROID Private/Android/*.h )
set( CPP_FILES_ANDROID Private/Android/*.cpp   )

set( HPP_FILES_IOS Private/Ios/*.h )
set( CPP_FILES_IOS Private/Ios/*.cpp Private/Ios/*.mm  )

set( HPP_FILES_MACOS Private/Mac/*.h )
set( CPP_FILES_MACOS Private/Mac/*.cpp Private/Mac/*.mm )

set( HPP_FILES_WIN Private/Win32/*.h )
set( CPP_FILES_WIN Private/Win32/*.cpp )

set( HPP_FILES_WINUAP Private/Win10/*.h )
set( CPP_FILES_WINUAP Private/Win10/*.cpp )

set( ERASE_FILES_NOT_ANDROID     UIScreenManagerAndroid.h
                                 UIScreenManagerAndroid.cpp )

set( ERASE_FILES_NOT_IOS         UIScreenManageriPhone.h
                                 UIScreenManageriPhoneImpl.h )

set( ERASE_FILES_NOT_MACOS       UIScreenManagerDefault.h
                                 UIScreenManagerDefault.cpp  )

set( ERASE_FILES_NOT_WIN         ${ERASE_FILES_NOT_MACOS} )
set( ERASE_FILES_NOT_WINUAP      ${ERASE_FILES_NOT_MACOS} )

set( ERASE_FILES_NOT_LINUX       ${ERASE_FILES_NOT_MACOS} )

setup_main_module()
