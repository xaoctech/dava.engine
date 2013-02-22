#ifndef __RESOURCEEDITORQT__BASEPARTICLEEDITORCONTENTWIDGET__
#define __RESOURCEEDITORQT__BASEPARTICLEEDITORCONTENTWIDGET__

#include "DAVAEngine.h"

using namespace DAVA;

class BaseParticleEditorContentWidget
{
public:
	BaseParticleEditorContentWidget();
	
	virtual void StoreVisualState(KeyedArchive* visualStateProps) = 0;
	virtual void RestoreVisualState(KeyedArchive* visualStateProps) = 0;
	
	ParticleEmitter* GetEmitter() const {return emitter;};
	
protected:
	ParticleEmitter* emitter;
};

#endif /* defined(__RESOURCEEDITORQT__BASEPARTICLEEDITORCONTENTWIDGET__) */
