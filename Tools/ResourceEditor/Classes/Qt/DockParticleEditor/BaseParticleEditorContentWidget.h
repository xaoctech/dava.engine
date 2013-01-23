#ifndef __RESOURCEEDITORQT__BASEPARTICLEEDITORCONTENTWIDGET__
#define __RESOURCEEDITORQT__BASEPARTICLEEDITORCONTENTWIDGET__

#include "DAVAEngine.h"

using namespace DAVA;

class BaseParticleEditorContentWidget
{
public:
	virtual void StoreVisualState(KeyedArchive* visualStateProps) = 0;
	virtual void RestoreVisualState(KeyedArchive* visualStateProps) = 0;
};

#endif /* defined(__RESOURCEEDITORQT__BASEPARTICLEEDITORCONTENTWIDGET__) */
