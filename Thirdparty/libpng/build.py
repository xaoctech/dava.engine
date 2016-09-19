import sys
import os
import shutil
import subprocess
import build_utils

def get_internal_dependencies_for_platform(platform):
	if platform == 'win32':
		return ['zlib']
	else:
		return []

def get_supported_build_platforms():
	return ['win32', 'darwin']

def build(output_folder_path, root_project_path):
	# Files to download and according names are different on Windows and macOS

	if sys.platform == 'win32':
		libpng_sources_filename = 'libpng_source.zip'
		download_link = 'ftp://ftp.simplesystems.org/pub/libpng/png/src/libpng16/lpng1625.zip'
		inner_folder_name = 'lpng1625'
	else:
		libpng_sources_filename = 'libpng_source.tar.gz'
		download_link = 'ftp://ftp.simplesystems.org/pub/libpng/png/src/libpng16/libpng-1.6.25.tar.gz'
		inner_folder_name = 'libpng-1.6.25'

	# Path to downloaded archive
	libpng_source_archive_filepath = os.path.join(output_folder_path, libpng_sources_filename)

	# Path to extracted source folder
	libpng_source_folder_path = os.path.join(output_folder_path, 'libpng_source')

	# Download & unarchive
	build_utils.download_if_doesnt_exist(download_link, libpng_source_archive_filepath)
	build_utils.unzip_inplace(libpng_source_archive_filepath)

	# Rename version-dependent folder name to simpler one
	# In case other builder will need to use this folder
	shutil.move(os.path.join(output_folder_path, inner_folder_name), libpng_source_folder_path)

	# Apply fixes
	build_utils.apply_patch(os.path.abspath('patch.diff'), output_folder_path)

	# Add configuration file to source folder
	# It is used to generate additional header & source files
	shutil.copyfile('pngusr.dfa', os.path.join(libpng_source_folder_path, 'pngusr.dfa'))

	if sys.platform == 'win32':
		# Folders for library to be built into

		build_x86_folder = os.path.join(output_folder_path, 'gen/build_win32_x86')
		build_x64_folder = os.path.join(output_folder_path, 'gen/build_win32_x64')
		build_win10_x86_folder = os.path.join(output_folder_path, 'gen/build_win10_x86')
		build_win10_x64_folder = os.path.join(output_folder_path, 'gen/build_win10_x64')
		build_win10_arm_folder = os.path.join(output_folder_path, 'gen/build_win10_arm')
		build_android_folder = os.path.join(output_folder_path, 'gen/build_android')

		# VS build variables

		solution_name = 'libpng.sln'
		target_name = 'png_static'
		cmake_flags =  [ '-DZLIB_LIBRARY=' + os.path.join(output_folder_path, '../zlib/gen/build_x86/Release/zlib.lib'), '-DZLIB_INCLUDE_DIR=' + os.path.join(output_folder_path, '../zlib/zlib_source/') ]
		cmake_win10_flags = cmake_flags + ['-DCMAKE_SYSTEM_NAME=WindowsStore', '-DCMAKE_SYSTEM_VERSION=10.0' ]

		# Build all VS projects

		build_utils.cmake_generate_build_vs(build_x86_folder, libpng_source_folder_path, 'Visual Studio 12', solution_name, target_name, 'Win32', cmake_flags)
		build_utils.cmake_generate_build_vs(build_x64_folder, libpng_source_folder_path, 'Visual Studio 12 Win64', solution_name, target_name, 'Win64', cmake_flags)
		build_utils.cmake_generate_build_vs(build_win10_x86_folder, libpng_source_folder_path, 'Visual Studio 14 2015', solution_name, target_name, 'Win32', cmake_win10_flags)
		build_utils.cmake_generate_build_vs(build_win10_x64_folder, libpng_source_folder_path, 'Visual Studio 14 2015 Win64', solution_name, target_name, 'Win64', cmake_win10_flags)
		build_utils.cmake_generate_build_vs(build_win10_arm_folder, libpng_source_folder_path, 'Visual Studio 14 2015 ARM', solution_name, target_name, 'ARM', cmake_win10_flags)

		# Copy created configuration header to root folder (required to build library for Android)
		shutil.copyfile(os.path.join(build_x86_folder, 'pnglibconf.h'), os.path.join(libpng_source_folder_path, 'pnglibconf.h'))

		build_utils.build_android_ndk('android_ndk_project', build_android_folder, debug=False, ndk_additional_args = [ 'SRC_PATH=' + os.path.join('../../', libpng_source_folder_path) ])

		# Rename libraries to libpng.lib

		lib_path_x86_debug = os.path.join(build_x86_folder, 'Debug/libpng.lib')
		lib_path_x86_release = os.path.join(build_x86_folder, 'Release/libpng.lib')
		lib_path_x64_debug = os.path.join(build_x64_folder, 'Debug/libpng.lib')
		lib_path_x64_release = os.path.join(build_x64_folder, 'Release/libpng.lib')
		lib_path_win10_x86_debug = os.path.join(build_win10_x86_folder, 'Debug/libpng.lib')
		lib_path_win10_x86_release = os.path.join(build_win10_x86_folder, 'Release/libpng.lib')
		lib_path_win10_x64_debug = os.path.join(build_win10_x64_folder, 'Debug/libpng.lib')
		lib_path_win10_x64_release = os.path.join(build_win10_x64_folder, 'Release/libpng.lib')
		lib_path_win10_arm_debug = os.path.join(build_win10_arm_folder, 'Debug/libpng.lib')
		lib_path_win10_arm_release = os.path.join(build_win10_arm_folder, 'Release/libpng.lib')
		lib_path_android_armeabiv7a = os.path.join(build_android_folder, 'local/armeabi-v7a/libpng.a')
		lib_path_android_x86 = os.path.join(build_android_folder, 'local/x86/libpng.a')

		shutil.move(os.path.join(build_x86_folder, 'Debug/libpng16_staticd.lib'), lib_path_x86_debug)
		shutil.move(os.path.join(build_x86_folder, 'Release/libpng16_static.lib'), lib_path_x86_release)
		shutil.move(os.path.join(build_x64_folder, 'Debug/libpng16_staticd.lib'), lib_path_x64_debug)
		shutil.move(os.path.join(build_x64_folder, 'Release/libpng16_static.lib'), lib_path_x64_release)
		shutil.move(os.path.join(build_win10_x86_folder, 'Debug/libpng16_staticd.lib'), lib_path_win10_x86_debug)
		shutil.move(os.path.join(build_win10_x86_folder, 'Release/libpng16_static.lib'), lib_path_win10_x86_release)
		shutil.move(os.path.join(build_win10_x64_folder, 'Debug/libpng16_staticd.lib'), lib_path_win10_x64_debug)
		shutil.move(os.path.join(build_win10_x64_folder, 'Release/libpng16_static.lib'), lib_path_win10_x64_release)
		shutil.move(os.path.join(build_win10_arm_folder, 'Debug/libpng16_staticd.lib'), lib_path_win10_arm_debug)
		shutil.move(os.path.join(build_win10_arm_folder, 'Release/libpng16_static.lib'), lib_path_win10_arm_release)

		# Move built files into Libs/lib_CMake
		# TODO: update pathes after switching to new folders structure

		shutil.copyfile(lib_path_x86_debug, os.path.join(root_project_path, 'Libs/lib_CMake/win/x86/Debug/pnglib_wind.lib'))
		shutil.copyfile(lib_path_x86_release, os.path.join(root_project_path, 'Libs/lib_CMake/win/x86/Release/pnglib_win.lib'))
		shutil.copyfile(lib_path_x64_debug, os.path.join(root_project_path, 'Libs/lib_CMake/win/x64/Debug/pnglib_wind.lib'))
		shutil.copyfile(lib_path_x64_release, os.path.join(root_project_path, 'Libs/lib_CMake/win/x64/Release/pnglib_win.lib'))
		shutil.copyfile(lib_path_win10_x86_debug, os.path.join(root_project_path, 'Libs/lib_CMake/win10/Win32/Debug/pnglib_wind.lib'))
		shutil.copyfile(lib_path_win10_x86_release, os.path.join(root_project_path, 'Libs/lib_CMake/win10/Win32/Release/pnglib_win.lib'))
		shutil.copyfile(lib_path_win10_x64_debug, os.path.join(root_project_path, 'Libs/lib_CMake/win10/x64/Debug/pnglib_wind.lib'))
		shutil.copyfile(lib_path_win10_x64_release, os.path.join(root_project_path, 'Libs/lib_CMake/win10/x64/Release/pnglib_win.lib'))
		shutil.copyfile(lib_path_win10_arm_debug, os.path.join(root_project_path, 'Libs/lib_CMake/win10/arm/Debug/pnglib_wind.lib'))
		shutil.copyfile(lib_path_win10_arm_release, os.path.join(root_project_path, 'Libs/lib_CMake/win10/arm/Release/pnglib_win.lib'))
		shutil.copyfile(lib_path_android_armeabiv7a, os.path.join(root_project_path, 'Libs/lib_CMake/android/armeabi-v7a/libpng.a'))
		shutil.copyfile(lib_path_android_x86, os.path.join(root_project_path, 'Libs/lib_CMake/android/x86/libpng.a'))
	else:
		# Folders for library to be built into

		build_folder_macos = os.path.join(output_folder_path, 'gen/build_macos')
		build_folder_ios = os.path.join(output_folder_path, 'gen/build_ios')
		build_android_folder = os.path.join(output_folder_path, 'gen/build_android')

		# Generate & build XCode projects for macOS and iOS

		build_utils.cmake_generate_build_xcode(build_folder_macos, libpng_source_folder_path, 'Xcode', 'libpng.xcodeproj', 'png_static')
		build_utils.cmake_generate_build_xcode(build_folder_ios, libpng_source_folder_path, 'Xcode', 'libpng.xcodeproj', 'png_static', [ '-DCMAKE_TOOLCHAIN_FILE=' + os.path.join(root_project_path, 'Sources/CMake/Toolchains/ios.toolchain.cmake') ])

		# Copy created configuration header to root folder (required to build library for Android)
		shutil.copyfile(os.path.join(build_folder_macos, 'pnglibconf.h'), os.path.join(libpng_source_folder_path, 'pnglibconf.h'))

		build_utils.build_android_ndk('./android_ndk_project', build_android_folder, debug=False, ndk_additional_args = [ 'SRC_PATH=' + os.path.join('../../', libpng_source_folder_path) ])

		# Move built files into Libs/lib_CMake
		# TODO: update pathes after switching to new folders structure
		
		lib_path_macos_release = os.path.join(build_folder_macos, 'Release/libpng16.a')
		lib_path_ios_release = os.path.join(build_folder_ios, 'Release-iphoneos/libpng16.a')
		lib_path_android_armeabiv7a = os.path.join(build_android_folder, 'local/armeabi-v7a/libpng.a')
		lib_path_android_x86 = os.path.join(build_android_folder, 'local/x86/libpng.a')

		shutil.copyfile(lib_path_macos_release, os.path.join(root_project_path, 'Libs/lib_CMake/mac/libpng_macos.a'))
		shutil.copyfile(lib_path_ios_release, os.path.join(root_project_path, 'Libs/lib_CMake/ios/libpng_ios.a'))
		shutil.copyfile(lib_path_android_armeabiv7a, os.path.join(root_project_path, 'Libs/lib_CMake/android/armeabi-v7a/libpng.a'))
		shutil.copyfile(lib_path_android_x86, os.path.join(root_project_path, 'Libs/lib_CMake/android/x86/libpng.a'))

	return True