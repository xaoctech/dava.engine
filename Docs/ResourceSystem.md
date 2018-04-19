
# Info

* [Intro](https://confluence.wargaming.net/pages/viewpage.action?pageId=456518146)
* [Resource System в движке](https://confluence.wargaming.net/pages/viewpage.action?pageId=388511778)
* [Details](https://confluence.wargaming.net/display/CTBLITZ/%5BOPWOTB%5D+Resource+System)
* [Tags](https://confluence.wargaming.net/display/WOTB/Resource+System.+Tags)


# How to
* DAVA Engine requires several pythons for correct work. [Download Python](https://www.python.org/downloads/)
    * Python 2.7.10 - for most of scripts
    * Python 3.6.4 - for Resource System
    * [How to setup Python2 and Python3 on mac](https://gist.github.com/Bouke/11261620)
* Download and setup virtualenv for python3
    * run command "pip3 install virtualenv"
* Clone [Resource System](https://stash-dava.wargaming.net/projects/DF/repos/resourcesystem/browse) repo to use it for resources pipeline
* Setup [Resource System](https://stash-dava.wargaming.net/projects/DF/repos/resourcesystem/browse)
    * create a virtual environment for a project: 
        * $ cd res-cmd-line 
        * $ virtualenv --no-site-packages --python=${PATH-TO-PYTHON-3} venv
    * activate venv before installing packages:
        * MacOS: $ source venv/bin/activate
        * Windows: $ source venv/Scripts/activate   
    * install required packages at res-cmd-line folder: 
        * $ pip install -r .meta/packages
* Setup config for project
* Use res-cmd-line/src/main.py to work with client side of resource system


# Additional
[Additional Guide for Resource System](https://stash-dava.wargaming.net/projects/DF/repos/resourcesystem/browse/res-cmd-line/README.md)