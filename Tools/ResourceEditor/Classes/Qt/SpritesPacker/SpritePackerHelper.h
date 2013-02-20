#ifndef __ResourceEditorQt__ParticlesEditorSpritePackerHelper__
#define __ResourceEditorQt__ParticlesEditorSpritePackerHelper__

#include "DAVAEngine.h"
#include <QFutureWatcher>

class SceneData;
namespace DAVA {

// Sprite Packer Helper for Particles Editor.
class SpritePackerHelper : public QObject, public DAVA::StaticSingleton<SpritePackerHelper>
{
	Q_OBJECT

public:
	
	SpritePackerHelper();

	void UpdateParticleSprites();
	
	void UpdateParticleSpritesAsync();
	

signals:

	void readyAll();

protected: 
	
	void ReloadParticleSprites(SceneData* sceneData);

	QFuture<void> *future;
	QFutureWatcher<void> watcher;

private slots:
	
    void threadRepackAllFinished();
	
};
};
#endif /* __ResourceEditorQt__ParticlesEditorSpritePackerHelper__ */