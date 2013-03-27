//
//  ParticleEditorWidget.h
//  ResourceEditorQt
//
//  Created by adebt on 11/26/12.
//
//

#ifndef __ResourceEditorQt__ParticleEditorWidget__
#define __ResourceEditorQt__ParticleEditorWidget__

#include <QScrollArea>

#include <DAVAEngine.h>
#include "ParticleEmitterPropertiesWidget.h"
#include "ParticlesEditorQT/Nodes/LayerParticleEditorNode.h"

class EmitterLayerWidget;
class LayerForceWidget;

using namespace DAVA;

class ParticleEditorWidget: public QScrollArea
{
    Q_OBJECT
    
public:
    explicit ParticleEditorWidget(QWidget *parent = 0);
    ~ParticleEditorWidget();
	
protected slots:
	void OnEmitterSelected(Entity* emitterNode, BaseParticleEditorNode* editorNode);
    void OnLayerSelected(Entity* emitterNode, ParticleLayer* layer, BaseParticleEditorNode* editorNode, bool forceRefresh);
    void OnForceSelected(Entity* emitterNode, ParticleLayer* layer, int32 forceIndex, BaseParticleEditorNode* editorNode);
	void OnUpdate();
	void OnValueChanged();
	void OnNodeDeselected(BaseParticleEditorNode* particleEditorNode);
	
signals:
	void ChangeVisible(bool bVisible);
	void ValueChanged();
	
private:
	void DeleteOldWidget();
	
private:
	EmitterLayerWidget* emitterLayerWidget;
	LayerForceWidget* layerForceWidget;
	ParticleEmitterPropertiesWidget* emitterPropertiesWidget;
};

#endif /* defined(__ResourceEditorQt__ParticleEditorWidget__) */
