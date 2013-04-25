#ifndef RESOURCEEDITORQT_COMMANDLIST_H
#define RESOURCEEDITORQT_COMMANDLIST_H

/*
 * MaterialViewOptionsCommands.h
 */
class CommandChangeMaterialViewOption;


/*
 * SetSwitchIndexCommands.h
 */
class CommandToggleSetSwitchIndex;


/*
 * VisibilityCheckToolCommands.h
 */
class CommandSaveTextureVisibilityTool;
class CommandPlacePointVisibilityTool;
class CommandPlaceAreaVisibilityTool;


/*
 * TextureOptionsCommands.h
 */
class ReloadTexturesAsCommand;


/*
 * CustomColorCommands.h
 */
class CommandSaveTextureCustomColors;
class CommandLoadTextureCustomColors;
class CommandDrawCustomColors;


/*
 * ParticleEditorCommands.h
 */
class CommandAddParticleEmitter;
class CommandStartStopParticleEffect;
class CommandRestartParticleEffect;
class CommandAddParticleEmitterLayer;
class CommandRemoveParticleEmitterLayer;
class CommandCloneParticleEmitterLayer;
class CommandAddParticleEmitterForce;
class CommandRemoveParticleEmitterForce;
class CommandUpdateEffect;
class CommandUpdateParticleLayer;
class CommandUpdateParticleLayerTime;
class CommandUpdateParticleLayerEnabled;
class CommandUpdateParticleForce;
class CommandLoadParticleEmitterFromYaml;
class CommandSaveParticleEmitterToYaml;


/*
 * CommandReloadTextures.h
 */
class CommandReloadTextures;


/*
 * LibraryCommands.h
 */
class CommandAddScene;
class CommandEditScene;
class CommandReloadScene;
class CommandReloadEntityFrom;
class CommandConvertScene;


/*
 * SceneGraphCommands.h
 */
class CommandRemoveRootNodes;
class CommandLockAtObject;
class CommandRemoveSceneNode;
class CommandInternalRemoveSceneNode;
class CommandDebugFlags;


/*
 * ToolsCommands.h
 */
class CommandBeast;
class CommandConvertToShadow;


/*
 * CommandCreateNode.h
 */
class CommandCreateNode;


/*
 * FileCommands.h
 */
class CommandOpenScene;
class CommandNewScene;
class CommandSaveScene;
class CommandExport;
class CommandSaveToFolderWithChilds;


/*
 * TilemapEditorCommands.h
 */
class CommandDrawTilemap;


/*
 * HeightmapEditorCommands.h
 */
class CommandDrawHeightmap;
class CommandCopyPasteHeightmap;


/*
 * EditorBodyControlCommands.h
 */
class CommandTransformObject;
class CommandCloneObject;
class CommandCloneAndTransform;
class CommandPlaceOnLandscape;
class CommandRestoreOriginalTransform;


/*
 * SceneEditorScreenMainCommands.h
 */
class CommandCreateNodeSceneEditor;

#endif
