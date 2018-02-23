import os
import build_utils

# https://chromium.googlesource.com/breakpad/breakpad/

def get_supported_targets(platform):
    if platform == 'linux':
        return ['linux']
    else:
        return []


def get_dependencies_for_target(target):
    return []


def build_for_target(target, working_directory_path, root_project_path):
    if target == 'linux':
        _build_linux(working_directory_path, root_project_path)


def _download_depot_tools(working_directory_path):
    depot_tools_path = os.path.join(working_directory_path, 'depot_tools')
    build_utils.run_process(
        ['git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git'],
        process_cwd=working_directory_path,
        shell=True)
    return depot_tools_path


def _build_linux(working_directory_path, root_project_path):
    depot_tools_path = _download_depot_tools(working_directory_path)
    source_folder_path = os.path.join(working_directory_path, 'breakpad')

    os.mkdir(source_folder_path)
    
    env = os.environ.copy()
    env['PATH'] += os.pathsep + depot_tools_path
    build_utils.run_process(
        [os.path.join(depot_tools_path, 'fetch breakpad')],
        process_cwd=source_folder_path,
        environment=env,
        shell=True)

    build_utils.apply_patch(os.path.abspath('patch.diff'), os.path.join(source_folder_path, 'src'))

    env = build_utils.get_autotools_linux_env()
    env['CXXFLAGS'] = '-std=c++14 -stdlib=libc++ -pthread'
    install_dir = os.path.join(working_directory_path, 'install_linux')

    build_utils.build_with_autotools(
        source_folder_path=os.path.join(source_folder_path, 'src'),
        configure_args=[],
        install_dir=install_dir,
        env=env)

    # copy binary files
    target_lib_path = os.path.join(root_project_path, 'Modules', 'CrashHandler', 'Libs', 'Linux')
    build_utils.copy_files_by_name(os.path.join(install_dir, 'lib'), target_lib_path, ['libbreakpad_client.a'])

    # dump_syms utility is used to convert debug symbols to breakpad format
    target_bin_path = os.path.join(root_project_path, 'Modules', 'CrashHandler', 'Bin', 'Linux')
    build_utils.copy_files_by_name(os.path.join(install_dir, 'bin'), target_bin_path, ['dump_syms'])

    _copy_headers(os.path.join(install_dir, 'include'), root_project_path)


def _copy_headers(source_path, root_project_path):
    target_include_path = os.path.join(root_project_path, 'Modules', 'CrashHandler', 'Libs', 'Include')
    build_utils.copy_folder_recursive(source_path, target_include_path)

