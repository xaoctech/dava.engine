#system
from os.path import split, basename, dirname, join, normpath
from zipfile import ZipFile
from os import stat, remove
from StringIO import StringIO

#pip
from lxml import etree

def file_name_from_path(path):
    head, tail = split(path)
    return tail or basename(head)


appx_manifest_file = "AppxManifest.xml"
appx_bundle_manifest_file = "AppxMetadata/AppxBundleManifest.xml"
#x = {"uap": "http://schemas.microsoft.com/appx/manifest/foundation/windows10"}
#y =  {"uap": "http://schemas.microsoft.com/appx/2013/bundle"}

def get_display_name_from_appx(zip_file, manifest_path):
    with zip_file.open(manifest_path) as f:
        x = etree.parse(f)
        root = x.getroot()
        apps = root.find("{*}Applications")
        if apps is None:
            return
        app = apps.find("{*}Application")
        visual = app.find("{*}VisualElements")
        return visual.get("DisplayName")


def get_identity_name_from_appx(zip_file, manifest_path):
    with zip_file.open(manifest_path) as f:
        x = etree.parse(f)
        root = x.getroot()
        identity = root.find("{*}Identity")
        name = identity.get("Name")
        return name


def get_executable_name_from_appx(zip_file, manifest_path):
    with zip_file.open(manifest_path) as f:
        x = etree.parse(f)
        root = x.getroot()
        apps = root.find("{*}Applications")
        if apps is None:
            return
        app = apps.find("{*}Application")
        x = app.get("Executable")
        return x


def get_version_from_appx(zip_file, manifest_path):
    with zip_file.open(manifest_path) as f:
        x = etree.parse(f)
        root = x.getroot()
        identity = root.find("{*}Identity")
        version = identity.get("Version")
        if version is not None:
            return [int(i) for i in version.split(".")]


class Package:
    def __init__(self, package_path):
        self.path = normpath(package_path)
        self.file_name = file_name_from_path(self.path)
        self.size_in_bytes = stat(self.path).st_size
        self.__display_name = None
        with ZipFile(package_path, "r") as zf:
            manifest_path = appx_manifest_file
            if self.file_name.lower().endswith(".appxbundle"):
                manifest_path = appx_bundle_manifest_file
            self.identity_name = get_identity_name_from_appx(zf, manifest_path)
            self.version = get_version_from_appx(zf, manifest_path)


    def get_display_name(self):
        if self.__display_name is not None: 
            return self.__display_name

        path = self.path
        if self.file_name.lower().endswith(".appxbundle"): 
            with ZipFile(self.path, "r") as zf:
                def find_first_application_file():
                    with zf.open(appx_bundle_manifest_file) as f:
                        x = etree.parse(f)
                        root = x.getroot()
                        packages = root.find("{*}Packages")
                        for t in packages.findall("{*}Package"):
                            if t.get("Type").lower() == "application":
                                return t.get("FileName")
            
                app_to_extract = find_first_application_file()
                if app_to_extract is not None:
                    dir = dirname(self.path)
                    zf.extract(app_to_extract, dir)
                    path = join(dir, app_to_extract)
        
        try:                
            with ZipFile(path, "r") as zf:
                self.__display_name = get_display_name_from_appx(zf, appx_manifest_file)
                return self.__display_name
        finally:
            if path != self.path:
                remove(path)