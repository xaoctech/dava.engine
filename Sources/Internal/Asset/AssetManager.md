Asset System Requirements
------------------------------

Asset 
---------
+ Dependencies
? Quality? (texture quality / model quality / remove of quality that is not required on load)
+ Easy memory management for end user (shared_ptr)
- Formats (.text / .bin)
- Export interface
- Streaming support
- Asset reload, and ability to perform custom resource reconfiguration after load. Probably ability to disable reload for some assets

Asset System
----------------
- Common cache for all types of resources
- Common interface for resource creation / deletion
- Automatic memory management
- Resource reload after change on disk
- Streaming 

Streaming System (Scene System?)
------------------
- Ability to add specific model to scene
- Ability to stream specified map 

List of engine assets
-------------------------

Texture - .tex
---------------
Link to file and some information about texture properties.
Stream texture on levels

Geometry / PolygonGroup - .geo
----------
Pure geometric data

Prefab .pfb
----------- 
- Storage of single entity with all dependencies. 
- Can refer other prefabs or can be build using over prefabs
- Can override some othe prefabs properties (See documentation or videos about lumberyard slices) 
# Implementation notes
-- It could be PrefabComponent with path. 
- Ability to stream models by LOD? 

Map .map
----------
- Ability to store large world
- Streaming of everything according to world movement
- Ability to stream terrain / vegetation / forrests
- Ability to edit map simultaneously

# Implementation notes
-- It could be MapComponent with corresponding MapSystem that manage map loading?  


Material(FX) .fx
----------- 

Visual Script .vsc
--------------

Effect .eff
------------

Sound .snd
-----------

Font .fnt (?)
----------

Sprite .sprite
---------------

GameConfig .gcon
----------------
Setup of various ingame properties, and ability to influence the game from config
It's yaml file. 


Зависимости между объектами
- Scene->Prefab->Shader(Material)->Texture 

SyncLoad -> ничего не должно сломаться :) 


Вопросы
-------------- 
1. Являются ли Asset-ы тем же что и загружено потом? (или Asset это ресурс на диске, а объект который из него загружен это уже рантайм представление и оно не является Asset-ом)



Мысли по поводу финальной архитектуры
--------------------------------------
LightmapComponent - компонент, который говорит что у данной модели есть Lightmap. Позволяет декомпозировать лайтмапы, и сами модели. 


