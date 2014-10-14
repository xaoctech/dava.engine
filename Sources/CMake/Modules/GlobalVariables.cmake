
if ( GLOBAL_VAR_FOUND )
    return ()
endif ()
set ( GLOBAL_VAR_FOUND 1 )


set ( DAVA_LIBRARY                     "DavaFramework" )
set ( DAVA_ROOT_DIR                    "${CMAKE_CURRENT_LIST_DIR}/../../.." )
set ( DAVA_ENGINE_DIR                  "${CMAKE_CURRENT_LIST_DIR}/../../Internal" )
set ( DAVA_THIRD_PARTY_ROOT_PATH       "${CMAKE_CURRENT_LIST_DIR}/../../../Libs" )
set ( DAVA_CONFIGURE_FILES_PATH        "${CMAKE_CURRENT_LIST_DIR}/../ConfigureFiles" )


set ( DAVA_THIRD_PARTY_INCLUDES_PATH   "${DAVA_THIRD_PARTY_ROOT_PATH}/include" 
                                       "${DAVA_ENGINE_DIR}/../External" 
                                       "${DAVA_ENGINE_DIR}/../Tools" 
                                       "${DAVA_THIRD_PARTY_ROOT_PATH}/glew/include" 
                                       "${DAVA_THIRD_PARTY_ROOT_PATH}/fmod/include" 
                                       "${DAVA_ENGINE_DIR}/../../Tools/ColladaConverter/Collada15/FCollada" 
                                       "${DAVA_ENGINE_DIR}/../../Tools/ColladaConverter/Collada15/External/Cg/include" 
                                      ) 


if     ( ANDROID )
    set ( DAVA_THIRD_PARTY_LIBRARIES_PATH  "${DAVA_THIRD_PARTY_ROOT_PATH}/lib_CMake/android/${ANDROID_NDK_ABI_NAME}" ) 
    
elseif ( IOS     ) 
    set ( DAVA_THIRD_PARTY_LIBRARIES_PATH  "${DAVA_THIRD_PARTY_ROOT_PATH}/lib_CMake/ios" ) 
    
elseif ( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
    set ( DAVA_THIRD_PARTY_LIBRARIES_PATH  "${DAVA_THIRD_PARTY_ROOT_PATH}/lib_CMake/mac" ) 

else   ()
    set ( DAVA_THIRD_PARTY_LIBRARIES_PATH  "${DAVA_THIRD_PARTY_ROOT_PATH}/lib_CMake/win" ) 
    
endif  ()

set ( DAVA_INCLUDE_DIR ${DAVA_ENGINE_DIR} ${DAVA_THIRD_PARTY_INCLUDES_PATH} )

#ImageMagick
if( APPLE AND NOT IOS ) 

	set ( DAVA_IMAGE_MAGICK_INCLUDES_PATH  
	     "${CMAKE_CURRENT_LIST_DIR}/../../../Libs/ImageMagick-6.7.4/include/ImageMagick"
	     "${CMAKE_CURRENT_LIST_DIR}/../../../Libs/ImageMagick-6.7.4/delegates/include" 
	     "${CMAKE_CURRENT_LIST_DIR}/../../../Libs/ImageMagick-6.7.4/" 
	     )

	set ( DAVA_IMAGE_MAGICK_LIBRARIES_PATH  
	     "${CMAKE_CURRENT_LIST_DIR}/../../../Libs/ImageMagick-6.7.4" )  

	file ( GLOB DAVA_IMAGE_MAGICK_LIBRARIES_RELEASE  ${DAVA_IMAGE_MAGICK_LIBRARIES_PATH}/lib/*.a ${DAVA_IMAGE_MAGICK_LIBRARIES_PATH}/delegates/lib/*.a )

	file ( GLOB DAVA_IMAGE_MAGICK_LIBRARIES_DEBUG    "${DAVA_IMAGE_MAGICK_LIBRARIES_RELEASE}" )

elseif( APPLE AND IOS ) 

else()

	set ( DAVA_IMAGE_MAGICK_INCLUDES_PATH  
	     "${CMAKE_CURRENT_LIST_DIR}/../../../Libs/ImageMagick-6.7.4-Windows"
	     "${CMAKE_CURRENT_LIST_DIR}/../../../Libs/ImageMagick-6.7.4-Windows/Magick++/lib" )

	set ( DAVA_IMAGE_MAGICK_LIBRARIES_PATH  
	     "${CMAKE_CURRENT_LIST_DIR}/../../../Libs/ImageMagick-6.7.4-Windows/VisualMagick/lib" )  

	file ( GLOB DAVA_IMAGE_MAGICK_LIBRARIES_RELEASE  "${DAVA_IMAGE_MAGICK_LIBRARIES_PATH}/CORE_RL_*.lib" )
	file ( GLOB DAVA_IMAGE_MAGICK_LIBRARIES_DEBUG    "${DAVA_IMAGE_MAGICK_LIBRARIES_PATH}/CORE_DB_*.lib" )

endif()





