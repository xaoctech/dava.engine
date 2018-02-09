import os
import shutil
import build_utils
import build_config


def get_supported_targets(platform):
    if platform == 'win32':
        return ['win32', 'win10']
    elif platform == 'darwin':
        return ['macos', 'ios', 'android']
    elif platform == 'linux':
        return ['android', 'linux']
    else:
        return []


def get_dependencies_for_target(target):
    return []


def build_for_target(target, working_directory_path, root_project_path):
    root_project_path = os.path.join(root_project_path, 'Modules', 'NetworkCore')
    if target == 'win32':
        _build_win32(working_directory_path, root_project_path)
    elif target == 'win10':
        _build_win10(working_directory_path, root_project_path)
    elif target == 'macos':
        _build_macos(working_directory_path, root_project_path)
    elif target == 'ios':
        _build_ios(working_directory_path, root_project_path)
    elif target == 'linux':
        _build_linux(working_directory_path, root_project_path)
    elif target == 'android':
        _build_android(working_directory_path, root_project_path)


def get_download_info():
    return 'http://enet.bespin.org/download/enet-1.3.13.tar.gz'


def _download_and_extract(working_directory_path):
    source_folder_path = os.path.join(working_directory_path, 'enet_source')

    url = get_download_info()
    build_utils.download_and_extract(
        url, working_directory_path,
        source_folder_path,
        build_utils.get_url_file_name_no_ext(url))

    return source_folder_path


def _build_win32(working_directory_path, root_project_path):
    source_folder_path = _download_and_extract(working_directory_path)
    gen_folder_path = os.path.join(working_directory_path, 'gen')
    solution_name = 'enet.sln'
    target_name = 'enet'

    build_x86_folder = os.path.join(gen_folder_path, 'build_win32_x86')
    build_x64_folder = os.path.join(gen_folder_path, 'build_win32_x64')

    build_utils.cmake_generate_build_vs(build_x86_folder, source_folder_path,
                            build_config.get_cmake_generator_win32_x86(), solution_name,
                            target_name, 'Win32')
    build_utils.cmake_generate_build_vs(build_x64_folder, source_folder_path,
                            build_config.get_cmake_generator_win32_x64(), solution_name,
                            target_name, 'x64')

    lib_name = 'enet.lib'
    release_lib_name = os.path.join('Release', lib_name)
    debug_lib_name = os.path.join('Debug', lib_name)
    lib_path_x86_release = os.path.join(build_x86_folder, release_lib_name)
    lib_path_x86_debug = os.path.join(build_x86_folder, debug_lib_name)
    lib_path_x64_release = os.path.join(build_x64_folder, release_lib_name)
    lib_path_x64_debug = os.path.join(build_x64_folder, debug_lib_name)

    root_lib_path = os.path.join(root_project_path, 'Libs', 'Win32')
    _copy_lib(lib_path_x86_release, os.path.join(root_lib_path, 'x86', 'Release'), lib_name)
    _copy_lib(lib_path_x86_debug, os.path.join(root_lib_path, 'x86', 'Debug'), lib_name)
    _copy_lib(lib_path_x64_release, os.path.join(root_lib_path, 'x64', 'Release'), lib_name)
    _copy_lib(lib_path_x64_debug, os.path.join(root_lib_path, 'x64', 'Debug'), lib_name)

    _copy_headers(source_folder_path, root_project_path)


@build_utils.run_once
def _patch_sources(source_folder_path, working_directory_path, patch_postifx):
    build_utils.apply_patch(
        os.path.abspath('patch' + patch_postifx + '.diff'),
        working_directory_path)


def _build_win10(working_directory_path, root_project_path):
    source_folder_path = _download_and_extract(working_directory_path)
    _patch_sources(source_folder_path, working_directory_path, '_win10')

    gen_folder_path = os.path.join(working_directory_path, 'gen')
    solution_name = 'enet.sln'
    target_name = 'enet'

    build_win10_x86_folder = os.path.join(gen_folder_path, 'build_win10_x86')
    build_win10_x64_folder = os.path.join(gen_folder_path, 'build_win10_x64')
    build_win10_arm_folder = os.path.join(gen_folder_path, 'build_win10_arm')

    cmake_additional_args = ['-DCMAKE_SYSTEM_NAME=WindowsStore', '-DCMAKE_SYSTEM_VERSION=10.0', '-DWIN10=1']
    build_utils.cmake_generate_build_vs(build_win10_x86_folder, source_folder_path, build_config.get_cmake_generator_win10_x86(), solution_name, target_name, 'Win32', cmake_additional_args)
    build_utils.cmake_generate_build_vs(build_win10_x64_folder, source_folder_path, build_config.get_cmake_generator_win10_x64(), solution_name, target_name, 'x64', cmake_additional_args)
    build_utils.cmake_generate_build_vs(build_win10_arm_folder, source_folder_path, build_config.get_cmake_generator_win10_arm(), solution_name, target_name, 'ARM', cmake_additional_args)

    lib_name = 'enet.lib'
    release_lib_name = os.path.join('Release', lib_name)
    debug_lib_name = os.path.join('Debug', lib_name)

    lib_path_win10_x86_release = os.path.join(build_win10_x86_folder, release_lib_name)
    lib_path_win10_x86_debug = os.path.join(build_win10_x86_folder, debug_lib_name)
    lib_path_win10_x64_release = os.path.join(build_win10_x64_folder, release_lib_name)
    lib_path_win10_x64_debug = os.path.join(build_win10_x64_folder, debug_lib_name)
    lib_path_win10_arm_release = os.path.join(build_win10_arm_folder, release_lib_name)
    lib_path_win10_arm_debug = os.path.join(build_win10_arm_folder, debug_lib_name)


    root_lib_path = os.path.join(root_project_path, 'Libs', 'win10')
    _copy_lib(lib_path_win10_x86_release, os.path.join(root_lib_path, 'Win32', 'Release'), lib_name)
    _copy_lib(lib_path_win10_x86_debug, os.path.join(root_lib_path, 'Win32', 'Debug'), lib_name)
    _copy_lib(lib_path_win10_x64_release, os.path.join(root_lib_path, 'x64', 'Release'), lib_name)
    _copy_lib(lib_path_win10_x64_debug, os.path.join(root_lib_path, 'x64', 'Debug'), lib_name)
    _copy_lib(lib_path_win10_arm_release, os.path.join(root_lib_path, 'arm', 'Release'), lib_name)
    _copy_lib(lib_path_win10_arm_debug, os.path.join(root_lib_path, 'arm', 'Debug'), lib_name)

    _copy_headers(source_folder_path, root_project_path)


def _build_macos(working_directory_path, root_project_path):
    source_folder_path = _download_and_extract(working_directory_path)
    install_dir = os.path.join(
        working_directory_path, 'gen/install_macos')
    env = build_utils.get_autotools_macos_env()
    env['CFLAGS'] += ' -DNDEBUG -O3'
    build_utils.build_with_autotools(
        source_folder_path, [], install_dir,
        env=env)

    lib_path = os.path.join(install_dir, 'lib/libenet.a')
    _copy_lib(lib_path, os.path.join(root_project_path, 'Libs', 'MacOS'), 'libenet.a')
    _copy_headers_from_install(install_dir, root_project_path)


def _build_ios(working_directory_path, root_project_path):
    source_folder_path = _download_and_extract(working_directory_path)
    install_dir = os.path.join(
        working_directory_path, 'gen/install_ios')
    build_utils.build_with_autotools(
        source_folder_path,
        ['--host=armv7-apple-darwin', '--disable-shared', '--enable-static'],
        install_dir, env=build_utils.get_autotools_ios_env())

    lib_path = os.path.join(install_dir, 'lib/libenet.a')
    _copy_lib(lib_path, os.path.join(root_project_path, 'Libs', 'iOS'), 'libenet.a')
    _copy_headers_from_install(install_dir, root_project_path)


def _build_android(working_directory_path, root_project_path):
    source_folder_path = _download_and_extract(working_directory_path)

    real_root = os.path.join(os.path.dirname(os.path.realpath(__file__)), '../../../..')
    env_arm = build_utils.get_autotools_android_arm_env(real_root)
    install_dir_android_arm = os.path.join(working_directory_path, 'gen/install_android_arm')
    build_utils.build_with_autotools(
        source_folder_path, ['--host=arm-linux-androideabi', '--disable-shared', '--enable-static'],
        install_dir_android_arm, env=env_arm)

    lib_path_arm = os.path.join(install_dir_android_arm, 'lib/libenet.a')
    _copy_lib(lib_path_arm, os.path.join(root_project_path, 'Libs', 'Android', 'armeabi-v7a'), 'libenet.a')

    env_x86 = build_utils.get_autotools_android_x86_env(real_root)
    install_dir_android_x86 = os.path.join(working_directory_path, 'gen/install_android_x86')
    build_utils.build_with_autotools(
        source_folder_path, ['--host=i686-linux-android', '--disable-shared', '--enable-static'],
        install_dir_android_x86, env=env_x86)

    lib_path_x86 = os.path.join(install_dir_android_x86, 'lib/libenet.a')
    _copy_lib(lib_path_x86, os.path.join(root_project_path, 'Libs', 'Android', 'x86'), 'libenet.a')

    _copy_headers_from_install(install_dir_android_arm, root_project_path)


def _build_linux(working_directory_path, root_project_path):
    source_folder_path = _download_and_extract(working_directory_path)
    install_dir = os.path.join(working_directory_path, 'gen/install_linux')
    env = build_utils.get_autotools_linux_env()
    env['CFLAGS'] = '-DNDEBUG -O3'
    build_utils.build_with_autotools(
        source_folder_path,
        ['--disable-shared', '--enable-static'],
        install_dir,
        env=env)

    lib_path = os.path.join(install_dir, 'lib/libenet.a')
    _copy_lib(lib_path, os.path.join(root_project_path, 'Libs', 'linux'), 'libenet.a')

    _copy_headers_from_install(install_dir, root_project_path)


def _copy_headers_from_install(install_folder_path, root_project_path):
    include_path = os.path.join(root_project_path, 'Libs/include')
    build_utils.copy_folder_recursive(
        os.path.join(install_folder_path, 'include'), include_path)


def _copy_headers(source_folder_path, root_project_path):
    include_path = os.path.join(root_project_path, 'Libs/include')
    build_utils.copy_files(
        os.path.join(source_folder_path, 'include'), include_path, '*.h')


def _copy_lib(src, dst, lib=''):
    if not os.path.isdir(dst):
        os.makedirs(dst)
    shutil.copyfile(src, os.path.join(dst, lib))
