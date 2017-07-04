set( HPP_FILES *.h   )
set( CPP_FILES *.cpp )
set( CPP_FILES_APPLE *.mm  )

set( CPP_FILES_IOS TemplateiOS/*.cpp TemplateiOS/*.mm  )
set( HPP_FILES_IOS TemplateiOS/*.h  )

set( CPP_FILES_MACOS TemplateMacOS/*.cpp TemplateMacOS/*.mm  )
set( HPP_FILES_MACOS TemplateMacOS/*.h  )

set( CPP_FILES_WIN   TemplateWin32/*.cpp )
set( HPP_FILES_WIN   TemplateWin32/*.h  )

set( CPP_FILES_WINUAP TemplateWin32/*.cpp )
set( HPP_FILES_WINUAP TemplateWin32/*.h  )

set( CPP_FILES_LINUX Linux/*.cpp )
set( HPP_FILES_LINUX Linux/*.h  )

set( ERASE_FILES_ANDROID  )

set( ERASE_FILES_IOS  )

set( ERASE_FILES_MACOS  )

set( ERASE_FILES_WIN )
set( ERASE_FILES_WINUAP ${ERASE_FILES_WIN32} )

setup_main_module()
