import os
import shutil
import build_utils
import build_config

def get_supported_targets(platform):
    if platform == 'win32':
        return ['win32', 'win10']
    elif platform == 'linux':
        return ['linux', 'android']
    elif platform == 'darwin':
        return ['macos', 'ios', 'android']
    else:
        return []

def get_dependencies_for_target(target):
    return []


def build_for_target(target, working_directory_path, root_project_path):
    root_project_path = os.path.join(root_project_path, 'Modules', 'NetworkCore')
    if target == 'win32':
        _build_win32(working_directory_path, root_project_path)
    if target == 'win10':
        _build_win10(working_directory_path, root_project_path)
    elif target == 'macos':
        _build_macos(working_directory_path, root_project_path)
    elif target == 'ios':
        _build_ios(working_directory_path, root_project_path)
    elif target == 'android':
        _build_android(working_directory_path, root_project_path)
    elif target == 'linux':
        _build_linux(working_directory_path, root_project_path)


def get_download_info():
	return 'https://github.com/weidai11/cryptopp/archive/CRYPTOPP_5_6_5.tar.gz'


def _download_and_extract(working_directory_path):
    source_folder_path = os.path.join(working_directory_path, 'cryptopp_source')

    url = get_download_info()
    build_utils.download_and_extract(
        url, working_directory_path,
        source_folder_path,
        'cryptopp-' + build_utils.get_url_file_name_no_ext(url))

    return source_folder_path

@build_utils.run_once
def _patch_sources(patch, working_directory_path):
    # Apply fixes
    build_utils.apply_patch(
        os.path.abspath(patch), working_directory_path)

def _build_win32(working_directory_path, root_project_path):
    source_folder_path = _download_and_extract(working_directory_path)
    _patch_sources('patch_win32.diff', working_directory_path)

    libs_win_root = os.path.join(root_project_path, 'Libs', 'Win32')
    _build_win(source_folder_path, libs_win_root, ('Win32', 'x64'))
    _copy_headers(source_folder_path, root_project_path)

def _build_win10(working_directory_path, root_project_path):
    source_folder_path = _download_and_extract(working_directory_path)
    _patch_sources('patch_win10.diff', working_directory_path)

    libs_win_root = os.path.join(root_project_path, 'Libs', 'win10')
    _build_win(source_folder_path, libs_win_root,
        ('Win32', 'x64', 'ARM'), rename_x86=False)
    _copy_headers(source_folder_path, root_project_path)

def _build_win(source_folder_path, libs_win_root, platforms, rename_x86=True):
    lib_name = 'cryptlib.lib'
    for configuration in ('Debug', 'Release'):
        for platform in platforms:
            build_utils.build_vs(
                os.path.join(source_folder_path, 'cryptest.sln'),
                configuration, platform=platform, target='cryptlib')
            lib_path = os.path.join(platform, 'Output', configuration, lib_name)
            if rename_x86 and platform == 'Win32':
                platform = 'x86'
            shutil.copyfile(os.path.join(source_folder_path, lib_path),
                os.path.join(libs_win_root, platform, configuration,lib_name))

def _build_macos(working_directory_path, root_project_path):
    source_folder_path = _download_and_extract(working_directory_path)

    env = build_utils.get_autotools_macos_env()
    env['CXXFLAGS'] = '{} -DNDEBUG'.format(env['CXXFLAGS']).replace('-O2','-O3')

    install_dir_macos = os.path.join(
        working_directory_path, 'gen/install_macos')
    env['PREFIX'] = install_dir_macos
    build_utils.build_with_autotools(source_folder_path, [], install_dir_macos,
        env=env, configure_exec_name=None)

    lib_path = os.path.join(install_dir_macos, 'lib/libcryptopp.a')
    shutil.copyfile(lib_path,
        os.path.join(root_project_path, 'Libs/MacOS/libcryptopp.a'))

    _copy_headers_from_install(install_dir_macos, root_project_path)


def _build_ios(working_directory_path, root_project_path):
    source_folder_path = _download_and_extract(working_directory_path)

    env = build_utils.get_autotools_ios_env()
    env['CXXFLAGS'] = '{} -DNDEBUG'.format(env['CXXFLAGS']).replace('-O2','-O3')

    install_dir_ios = os.path.join(working_directory_path, 'gen/install_ios')
    env['PREFIX'] = install_dir_ios
    build_utils.build_with_autotools(source_folder_path, [], install_dir_ios,
        env=env, configure_exec_name=None)

    lib_path = os.path.join(install_dir_ios, 'lib/libcryptopp.a')
    shutil.copyfile(lib_path,
        os.path.join(root_project_path, 'Libs/iOS/libcryptopp.a'))

    _copy_headers_from_install(install_dir_ios, root_project_path)

def _build_android(working_directory_path, root_project_path):
    source_folder_path = _download_and_extract(working_directory_path)

    ndk_path = build_utils.get_android_ndk_path(root_project_path)
    env = {'ANDROID_NDK_ROOT': ndk_path}

    install_dir_android_arm = os.path.join(working_directory_path,
        'gen/install_android_arm')
    env['PREFIX'] = install_dir_android_arm
    build_utils.run_process(['./setenv-android.sh', 'armv7a'],
        process_cwd=source_folder_path, environment=env)
    build_utils.build_with_autotools(source_folder_path, [],
        install_dir_android_arm, env=env, configure_exec_name=None,
        make_exec_name='make -f GNUmakefile-cross')

    install_dir_android_x86 = os.path.join(working_directory_path,
        'gen/install_android_x86')
    env['PREFIX'] = install_dir_android_x86
    build_utils.run_process(['./setenv-android.sh', 'x86'],
        process_cwd=source_folder_path, environment=env)
    build_utils.build_with_autotools(source_folder_path, [],
        install_dir_android_x86, env=env, configure_exec_name=None,
        make_exec_name='make -f GNUmakefile-cross')

    libs_android_root = os.path.join(root_project_path, 'Libs/Android')

    lib_path_arm = os.path.join(install_dir_android_arm, 'lib/libcryptopp.a')
    shutil.copyfile(lib_path_arm,
        os.path.join(libs_android_root, 'armeabi-v7a/libcryptopp.a'))

    lib_path_x86 = os.path.join(install_dir_android_x86, 'lib/libcryptopp.a')
    shutil.copyfile(lib_path_x86,
        os.path.join(libs_android_root, 'x86/libcryptopp.a'))

    _copy_headers_from_install(install_dir_android_arm, root_project_path)

def _build_linux(working_directory_path, root_project_path):
    source_folder_path = _download_and_extract(working_directory_path)

    env = build_utils.get_autotools_linux_env()
    env['CXXFLAGS'] = '{} -DNDEBUG'.format(env['CXXFLAGS']).replace('-O2','-O3')

    install_dir_linux = os.path.join(working_directory_path,
        'gen/install_linux')
    env['PREFIX'] = install_dir_linux
    build_utils.build_with_autotools(source_folder_path, [], install_dir_linux,
        env=env, configure_exec_name=None)

    lib_path = os.path.join(install_dir_linux, 'lib/libcryptopp.a')
    shutil.copyfile(lib_path,
        os.path.join(root_project_path, 'Libs/linux/libcryptopp.a'))

    _copy_headers_from_install(install_dir_linux, root_project_path)

def _copy_headers_from_install(install_folder_path, root_project_path):
    include_path = os.path.join(root_project_path, 'Libs/include')
    build_utils.copy_folder_recursive(
        os.path.join(install_folder_path, 'include'), include_path)

def _copy_headers(source_folder_path, root_project_path):
    include_path = os.path.join(root_project_path, 'Libs/include/cryptopp')
    build_utils.copy_files(source_folder_path, include_path, '*.h')
