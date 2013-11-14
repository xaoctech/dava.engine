/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/



#include "ParticleEffectPropertiesWidget.h"
#include "Commands2/ParticleEditorCommands.h"
#include "../Scene/SceneDataManager.h"

#include <QLineEdit>
#include <QEvent>
#include <QMenu>
#include <QPushButton>

static const int TreeItemTypeEmitter  = QTreeWidgetItem::UserType+1;
static const int TreeItemTypeLayer    = QTreeWidgetItem::UserType+2;
static const int TreeItemTypeForce    = QTreeWidgetItem::UserType+3;
static const int TreeItemTypeExternal = QTreeWidgetItem::UserType+4;

ParticleEffectPropertiesWidget::ParticleEffectPropertiesWidget(QWidget* parent) :
	QWidget(parent),
	BaseParticleEditorContentWidget()	
{	
	mainLayout = new QVBoxLayout();
	mainLayout->setAlignment(Qt::AlignTop);
	this->setLayout(mainLayout);
	
	effectPlaybackSpeedLabel = new QLabel("effect playback speed");
	mainLayout->addWidget(effectPlaybackSpeedLabel);
	
	effectPlaybackSpeed = new QSlider(Qt::Horizontal, this);
	effectPlaybackSpeed->setTracking(true);
	effectPlaybackSpeed->setRange(0, 4); // 25%, 50%, 100%, 200%, 400% - 5 values total.
	effectPlaybackSpeed->setTickPosition(QSlider::TicksBelow);
	effectPlaybackSpeed->setTickInterval(1);
	effectPlaybackSpeed->setSingleStep(1);
	mainLayout->addWidget(effectPlaybackSpeed);

	checkboxStopOnLoad = new QCheckBox("Stop on load");
	mainLayout->addWidget(checkboxStopOnLoad);

	connect(effectPlaybackSpeed, SIGNAL(valueChanged(int)), this, SLOT(OnValueChanged()));
	connect(checkboxStopOnLoad, SIGNAL(stateChanged(int)), this, SLOT(OnValueChanged()));

	effectTree = new QTreeWidget();
	mainLayout->addWidget(effectTree);
	effectTree->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(effectTree, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(ShowContextMenuForEffectTree(const QPoint&)));
	connect(effectTree, SIGNAL(itemDoubleClicked (QTreeWidgetItem*, int)), this, SLOT(OnTreeItemDoubleClck(QTreeWidgetItem*, int)));
	iconEmitter = QIcon(":/QtIcons/emitter_particle.png");
	iconLayer = QIcon(":/QtIcons/layer_particle.png");
	iconForce = QIcon(":/QtIcons/force.png");
	iconExternal = QIcon(":/QtIcons/external.png");

	mainLayout->addStretch();

	particleEffect = NULL;
	blockSignals = false;
}

ParticleEffectPropertiesWidget::~ParticleEffectPropertiesWidget()
{
}

void ParticleEffectPropertiesWidget::InitWidget(QWidget *widget, bool connectWidget)
{
	mainLayout->addWidget(widget);
	if(connectWidget)
		connect(widget, SIGNAL(ValueChanged()), this, SLOT(OnValueChanged()));
}

void ParticleEffectPropertiesWidget::ShowContextMenuForEffectTree(const QPoint &pos)
{
	QMenu contextMenu;
	currSelectedTreeItem = effectTree->itemAt(pos);	
	EffectTreeData treeData = currSelectedTreeItem->data(0, Qt::UserRole).value<EffectTreeData>();	
	int i;
	switch(currSelectedTreeItem->type())
	{
	case TreeItemTypeEmitter:		
		for (i=0; i<EE_TOTAL; ++i)
		{
			if (!GetEmitterLine(treeData.emmiter, EmitterExternals(i)))
			{
				contextMenu.addAction(QString("Add External ")+QString(EXTERNAL_NAMES[i].c_str()))->setData(QVariant(i));
			}
		}		
		break;
	case TreeItemTypeLayer:
		for (i=EE_TOTAL; i<EL_TOTAL; ++i)
		{
			if (!GetLayerLine(treeData.layer, LayerExternals(i)))
			{
				contextMenu.addAction(QString("Add External ")+QString(EXTERNAL_NAMES[i].c_str()))->setData(QVariant(i));
			}
		}
		break;
	case TreeItemTypeForce:
		for (i=EL_TOTAL; i<EF_TOTAL; ++i)
		{
			if (!GetForceLine(treeData.force, ForceExternals(i)))
			{
				contextMenu.addAction(QString("Add External ")+QString(EXTERNAL_NAMES[i].c_str()))->setData(QVariant(i));
			}
		}
		break;
	case TreeItemTypeExternal:
		contextMenu.addAction(QString("Remove External ")+QString(EXTERNAL_NAMES[treeData.externalParamId].c_str()))->setData(QVariant(-1));
	}
	connect(&contextMenu, SIGNAL(triggered(QAction*)), this, SLOT(OnContextMenuCommand(QAction*)));
	contextMenu.exec(effectTree->viewport()->mapToGlobal(pos));
}

void ParticleEffectPropertiesWidget::OnTreeItemDoubleClck(QTreeWidgetItem *treeItem, int column)
{
	EffectTreeData data = treeItem->data(0, Qt::UserRole).value<EffectTreeData>();
	if (treeItem->type() == TreeItemTypeExternal) //edit external on double click
	{
		int externalId = data.externalParamId;
		if (externalId<EE_TOTAL)
		{			
			EditEmitterModifiable(data.emmiter, EmitterExternals(externalId));			
		}
		else if (externalId<EL_TOTAL)
		{			
			EditLayerModifiable(data.layer, LayerExternals(externalId));
		}
		else
		{				
			EditForceModifiable(data.force, ForceExternals(externalId));		
		}
	}
}

void ParticleEffectPropertiesWidget::OnContextMenuCommand(QAction *action)
{
	int commandId = action->data().toInt();
	if (commandId==-1)
	{
		EffectTreeData data = currSelectedTreeItem->data(0, Qt::UserRole).value<EffectTreeData>();
		int externalId = data.externalParamId;
		if (externalId<EE_TOTAL)
		{			
			particleEffect->UnRegisterModifiable(GetEmitterLine(data.emmiter, EmitterExternals(commandId)));
			RemoveEmitterLineModifiable(data.emmiter, EmitterExternals(externalId));			
		}
		else if (externalId<EL_TOTAL)
		{			
			particleEffect->UnRegisterModifiable(GetLayerLine(data.layer, LayerExternals(commandId)));
			RemoveLayerLineModifiable(data.layer, LayerExternals(externalId));
		}
		else
		{		
			particleEffect->UnRegisterModifiable(GetForceLine(data.force, ForceExternals(commandId)));
			RemoveForceLineModifiable(data.force, ForceExternals(externalId));			
		}
		delete currSelectedTreeItem;
	}
	else if (commandId<EE_TOTAL)
	{
		EffectTreeData data = currSelectedTreeItem->data(0, Qt::UserRole).value<EffectTreeData>();
		SetEmitterLineModifiable(data.emmiter, EmitterExternals(commandId));
		particleEffect->RegisterModifiable(GetEmitterLine(data.emmiter, EmitterExternals(commandId)));
		data.externalParamId = commandId;
		QTreeWidgetItem *externalItem = new QTreeWidgetItem(currSelectedTreeItem, TreeItemTypeExternal);
		externalItem->setText(0, QString("External ")+QString(EXTERNAL_NAMES[commandId].c_str()));
		externalItem->setIcon(0, iconExternal);
		externalItem->setData(0, Qt::UserRole, QVariant::fromValue(data));		
		EditEmitterModifiable(data.emmiter, EmitterExternals(commandId), true);		
	}
	else if (commandId<EL_TOTAL)
	{
		EffectTreeData data = currSelectedTreeItem->data(0, Qt::UserRole).value<EffectTreeData>();
		SetLayerLineModifiable(data.layer, LayerExternals(commandId));
		particleEffect->RegisterModifiable(GetLayerLine(data.layer, LayerExternals(commandId)));
		data.externalParamId = commandId;
		QTreeWidgetItem *externalItem = new QTreeWidgetItem(currSelectedTreeItem, TreeItemTypeExternal);
		externalItem->setText(0, QString("External ")+QString(EXTERNAL_NAMES[commandId].c_str()));
		externalItem->setIcon(0, iconExternal);
		externalItem->setData(0, Qt::UserRole, QVariant::fromValue(data));
		EditLayerModifiable(data.layer, LayerExternals(commandId), true);
		
	}
	else
	{	
		EffectTreeData data = currSelectedTreeItem->data(0, Qt::UserRole).value<EffectTreeData>();
		SetForceLineModifiable(data.force, ForceExternals(commandId));
		particleEffect->UnRegisterModifiable(GetForceLine(data.force, ForceExternals(commandId)));
		data.externalParamId = commandId;
		QTreeWidgetItem *externalItem = new QTreeWidgetItem(currSelectedTreeItem, TreeItemTypeExternal);
		externalItem->setText(0, QString("External ")+QString(EXTERNAL_NAMES[commandId].c_str()));
		externalItem->setIcon(0, iconExternal);
		externalItem->setData(0, Qt::UserRole, QVariant::fromValue(data));
		EditForceModifiable(data.force, ForceExternals(commandId), true);
	}
}

void ParticleEffectPropertiesWidget::OnValueChanged()
{
	if(blockSignals)
		return;
	
	DVASSERT(particleEffect != 0);
	float playbackSpeed = ConvertFromSliderValueToPlaybackSpeed(effectPlaybackSpeed->value());
	bool stopOnLoad = checkboxStopOnLoad->isChecked();

	CommandUpdateEffect* commandUpdateEffect = new CommandUpdateEffect(particleEffect);
	commandUpdateEffect->Init(playbackSpeed, stopOnLoad);

	DVASSERT(activeScene != 0);
	activeScene->Exec(commandUpdateEffect);

	Init(activeScene, particleEffect);
}

void ParticleEffectPropertiesWidget::Init(SceneEditor2* scene, DAVA::ParticleEffectComponent *effect)
{
	DVASSERT(effect != 0);
	this->particleEffect = effect;
	this->emitter = NULL;
	SetActiveScene(scene);

	blockSignals = true;

	// Normalize Playback Speed to the UISlider range.
	float32 playbackSpeed = particleEffect->GetPlaybackSpeed();
	effectPlaybackSpeed->setValue(ConvertFromPlaybackSpeedToSliderValue(playbackSpeed));
	UpdatePlaybackSpeedLabel();
	BuildEffectTree();

	
	checkboxStopOnLoad->setChecked(particleEffect->IsStopOnLoad());
	blockSignals = false;
}


ModifiablePropertyLineI* ParticleEffectPropertiesWidget::GetEmitterLine(ParticleEmitter *emitter, EmitterExternals lineId)
{
	switch(lineId)
	{
	case EE_EMISSION_VECTOR:
		return dynamic_cast<ModifiablePropertyLineI*>(emitter->emissionVector.Get());
	case EE_EMISSION_RANGE:
		return dynamic_cast<ModifiablePropertyLineI*>(emitter->emissionRange.Get());
	case EE_RADUS:
		return dynamic_cast<ModifiablePropertyLineI*>(emitter->radius.Get());
	case EE_SIZE:
		return dynamic_cast<ModifiablePropertyLineI*>(emitter->size.Get());
	case EE_COLOR_OVER_LIFE:
		return dynamic_cast<ModifiablePropertyLineI*>(emitter->colorOverLife.Get());
	}
	return NULL;
}
ModifiablePropertyLineI* ParticleEffectPropertiesWidget::GetLayerLine(ParticleLayer *layer, LayerExternals lineId)
{
	switch (lineId)
	{
	case EL_LIFE:
		return dynamic_cast<ModifiablePropertyLineI*>(layer->life.Get());
	case EL_LIFE_VARIATION:
		return dynamic_cast<ModifiablePropertyLineI*>(layer->lifeVariation.Get());	
	case EL_NUMBER:
		return dynamic_cast<ModifiablePropertyLineI*>(layer->number.Get());
	case EL_NUMBER_VARIATION:
		return dynamic_cast<ModifiablePropertyLineI*>(layer->numberVariation.Get());
	case EL_SIZE:
		return dynamic_cast<ModifiablePropertyLineI*>(layer->size.Get());
	case EL_SIZE_VARIATION:
		return dynamic_cast<ModifiablePropertyLineI*>(layer->sizeVariation.Get());
	case EL_SIZE_OVERLIFE:
		return dynamic_cast<ModifiablePropertyLineI*>(layer->sizeOverLifeXY.Get());
	case EL_VELOCITY:
		return dynamic_cast<ModifiablePropertyLineI*>(layer->velocity.Get());
	case EL_VELOCITY_VARIATON:
		return dynamic_cast<ModifiablePropertyLineI*>(layer->velocityVariation.Get());
	case EL_VELOCITY_OVERLIFE:
		return dynamic_cast<ModifiablePropertyLineI*>(layer->velocityOverLife.Get());
	case EL_SPIN:
		return dynamic_cast<ModifiablePropertyLineI*>(layer->spin.Get());
	case EL_SPIN_VARIATION:
		return dynamic_cast<ModifiablePropertyLineI*>(layer->spinVariation.Get());
	case EL_SPIN_OVERLIFE:
		return dynamic_cast<ModifiablePropertyLineI*>(layer->spinOverLife.Get());
	case EL_COLOR:
		return dynamic_cast<ModifiablePropertyLineI*>(layer->colorRandom.Get());
	case EL_ALPHA_OVERLIFE:
		return dynamic_cast<ModifiablePropertyLineI*>(layer->alphaOverLife.Get());
	case EL_COLOR_OVERLIFE:
		return dynamic_cast<ModifiablePropertyLineI*>(layer->colorOverLife.Get());
	case EL_ANGLE:
		return dynamic_cast<ModifiablePropertyLineI*>(layer->angle.Get());
	case EL_ANGLE_VARIATION:
		return dynamic_cast<ModifiablePropertyLineI*>(layer->angleVariation.Get());
	case EL_ANIM_SPEED_OVERLIFE:
		return dynamic_cast<ModifiablePropertyLineI*>(layer->animSpeedOverLife.Get());
	}
	return NULL;
}
ModifiablePropertyLineI* ParticleEffectPropertiesWidget::GetForceLine(ParticleForce *force, ForceExternals lineId)
{
	switch(lineId)
	{
	case EF_FORCE:
		return dynamic_cast<ModifiablePropertyLineI*>(force->GetForce().Get());
	case EF_FORCE_VARIATION:
		return dynamic_cast<ModifiablePropertyLineI*>(force->GetForceVariation().Get());
	case EF_FORCE_OVERLIFE:
		return dynamic_cast<ModifiablePropertyLineI*>(force->GetForceOverlife().Get());
	}
	return NULL;
}

void ParticleEffectPropertiesWidget::SetEmitterLineModifiable(ParticleEmitter *emitter, EmitterExternals lineId)
{
	switch(lineId)
	{
	case EE_EMISSION_VECTOR:
		PropertyLineHelper::MakeModifiable(emitter->emissionVector); break;
	case EE_EMISSION_RANGE:
		PropertyLineHelper::MakeModifiable(emitter->emissionRange); break;
	case EE_RADUS:
		PropertyLineHelper::MakeModifiable(emitter->radius); break;
	case EE_SIZE:
		PropertyLineHelper::MakeModifiable(emitter->size); break;
	case EE_COLOR_OVER_LIFE:
		PropertyLineHelper::MakeModifiable(emitter->colorOverLife); break;
	}
}
void ParticleEffectPropertiesWidget::SetLayerLineModifiable(ParticleLayer *layer, LayerExternals lineId)
{
	switch (lineId)
	{
	case EL_LIFE:
		PropertyLineHelper::MakeModifiable(layer->life); break;
	case EL_LIFE_VARIATION:
		PropertyLineHelper::MakeModifiable(layer->lifeVariation); break;
	case EL_NUMBER:
		PropertyLineHelper::MakeModifiable(layer->number); break;
	case EL_NUMBER_VARIATION:
		PropertyLineHelper::MakeModifiable(layer->numberVariation); break;
	case EL_SIZE:
		PropertyLineHelper::MakeModifiable(layer->size); break;
	case EL_SIZE_VARIATION:
		PropertyLineHelper::MakeModifiable(layer->sizeVariation); break;
	case EL_SIZE_OVERLIFE:
		PropertyLineHelper::MakeModifiable(layer->sizeOverLifeXY); break;
	case EL_VELOCITY:
		PropertyLineHelper::MakeModifiable(layer->velocity); break;
	case EL_VELOCITY_VARIATON:
		PropertyLineHelper::MakeModifiable(layer->velocityVariation); break;
	case EL_VELOCITY_OVERLIFE:
		PropertyLineHelper::MakeModifiable(layer->velocityOverLife); break;
	case EL_SPIN:
		PropertyLineHelper::MakeModifiable(layer->spin); break;
	case EL_SPIN_VARIATION:
		PropertyLineHelper::MakeModifiable(layer->spinVariation); break;
	case EL_SPIN_OVERLIFE:
		PropertyLineHelper::MakeModifiable(layer->spinOverLife); break;
	case EL_COLOR:
		PropertyLineHelper::MakeModifiable(layer->colorRandom); break;
	case EL_ALPHA_OVERLIFE:
		PropertyLineHelper::MakeModifiable(layer->alphaOverLife); break;
	case EL_COLOR_OVERLIFE:
		PropertyLineHelper::MakeModifiable(layer->colorOverLife); break;
	case EL_ANGLE:
		PropertyLineHelper::MakeModifiable(layer->angle); break;
	case EL_ANGLE_VARIATION:
		PropertyLineHelper::MakeModifiable(layer->angleVariation); break;
	case EL_ANIM_SPEED_OVERLIFE:
		PropertyLineHelper::MakeModifiable(layer->animSpeedOverLife); break;
	}
}
void ParticleEffectPropertiesWidget::SetForceLineModifiable(ParticleForce *force, ForceExternals lineId)
{
	switch(lineId)
	{
	case EF_FORCE:
		force->SetForce(PropertyLineHelper::MakeModifiable(force->GetForce())); break;
	case EF_FORCE_VARIATION:
		force->SetForceVariation(PropertyLineHelper::MakeModifiable(force->GetForceVariation())); break;
	case EF_FORCE_OVERLIFE:
		force->SetForceOverLife(PropertyLineHelper::MakeModifiable(force->GetForceOverlife())); break;
	}
}

void ParticleEffectPropertiesWidget::RemoveEmitterLineModifiable(ParticleEmitter *emitter, EmitterExternals lineId)
{
	switch(lineId)
	{
	case EE_EMISSION_VECTOR:
		PropertyLineHelper::RemoveModifiable(emitter->emissionVector); break;
	case EE_EMISSION_RANGE:
		PropertyLineHelper::RemoveModifiable(emitter->emissionRange); break;
	case EE_RADUS:
		PropertyLineHelper::RemoveModifiable(emitter->radius); break;
	case EE_SIZE:
		PropertyLineHelper::RemoveModifiable(emitter->size); break;
	case EE_COLOR_OVER_LIFE:
		PropertyLineHelper::RemoveModifiable(emitter->colorOverLife); break;
	}
}
void ParticleEffectPropertiesWidget::RemoveLayerLineModifiable(ParticleLayer *layer, LayerExternals lineId)
{
	switch (lineId)
	{
	case EL_LIFE:
		PropertyLineHelper::RemoveModifiable(layer->life); break;
	case EL_LIFE_VARIATION:
		PropertyLineHelper::RemoveModifiable(layer->lifeVariation); break;
	case EL_NUMBER:
		PropertyLineHelper::RemoveModifiable(layer->number); break;
	case EL_NUMBER_VARIATION:
		PropertyLineHelper::RemoveModifiable(layer->numberVariation); break;
	case EL_SIZE:
		PropertyLineHelper::RemoveModifiable(layer->size); break;
	case EL_SIZE_VARIATION:
		PropertyLineHelper::RemoveModifiable(layer->sizeVariation); break;
	case EL_SIZE_OVERLIFE:
		PropertyLineHelper::RemoveModifiable(layer->sizeOverLifeXY); break;
	case EL_VELOCITY:
		PropertyLineHelper::RemoveModifiable(layer->velocity); break;
	case EL_VELOCITY_VARIATON:
		PropertyLineHelper::RemoveModifiable(layer->velocityVariation); break;
	case EL_VELOCITY_OVERLIFE:
		PropertyLineHelper::RemoveModifiable(layer->velocityOverLife); break;
	case EL_SPIN:
		PropertyLineHelper::RemoveModifiable(layer->spin); break;
	case EL_SPIN_VARIATION:
		PropertyLineHelper::RemoveModifiable(layer->spinVariation); break;
	case EL_SPIN_OVERLIFE:
		PropertyLineHelper::RemoveModifiable(layer->spinOverLife); break;
	case EL_COLOR:
		PropertyLineHelper::RemoveModifiable(layer->colorRandom); break;
	case EL_ALPHA_OVERLIFE:
		PropertyLineHelper::RemoveModifiable(layer->alphaOverLife); break;
	case EL_COLOR_OVERLIFE:
		PropertyLineHelper::RemoveModifiable(layer->colorOverLife); break;
	case EL_ANGLE:
		PropertyLineHelper::RemoveModifiable(layer->angle); break;
	case EL_ANGLE_VARIATION:
		PropertyLineHelper::RemoveModifiable(layer->angleVariation); break;
	case EL_ANIM_SPEED_OVERLIFE:
		PropertyLineHelper::RemoveModifiable(layer->animSpeedOverLife); break;
	}
}
void ParticleEffectPropertiesWidget::RemoveForceLineModifiable(ParticleForce *force, ForceExternals lineId)
{
	switch(lineId)
	{
	case EF_FORCE:
		force->SetForce(PropertyLineHelper::RemoveModifiable(force->GetForce())); break;
	case EF_FORCE_VARIATION:
		force->SetForceVariation(PropertyLineHelper::RemoveModifiable(force->GetForceVariation())); break;
	case EF_FORCE_OVERLIFE:
		force->SetForceOverLife(PropertyLineHelper::RemoveModifiable(force->GetForceOverlife())); break;
	}
}


void ParticleEffectPropertiesWidget::EditEmitterModifiable(ParticleEmitter *emitter, EmitterExternals lineId, bool onAdd)
{
	switch(lineId)
	{
	case EE_EMISSION_VECTOR:
		EditModificationLine(emitter->emissionVector, onAdd); break;
	case EE_EMISSION_RANGE:
		EditModificationLine(emitter->emissionRange, onAdd); break;
	case EE_RADUS:
		EditModificationLine(emitter->radius, onAdd); break;
	case EE_SIZE:
		EditModificationLine(emitter->size, onAdd); break;
	case EE_COLOR_OVER_LIFE:
		EditModificationLine(emitter->colorOverLife, onAdd); break;
	}
}
void ParticleEffectPropertiesWidget::EditLayerModifiable(ParticleLayer *layer, LayerExternals lineId, bool onAdd)
{
	switch (lineId)
	{
	case EL_LIFE:
		EditModificationLine(layer->life, onAdd); break;
	case EL_LIFE_VARIATION:
		EditModificationLine(layer->lifeVariation, onAdd); break;
	case EL_NUMBER:
		EditModificationLine(layer->number, onAdd); break;
	case EL_NUMBER_VARIATION:
		EditModificationLine(layer->numberVariation, onAdd); break;
	case EL_SIZE:
		EditModificationLine(layer->size, onAdd); break;
	case EL_SIZE_VARIATION:
		EditModificationLine(layer->sizeVariation, onAdd); break;
	case EL_SIZE_OVERLIFE:
		EditModificationLine(layer->sizeOverLifeXY, onAdd); break;
	case EL_VELOCITY:
		EditModificationLine(layer->velocity, onAdd); break;
	case EL_VELOCITY_VARIATON:
		EditModificationLine(layer->velocityVariation, onAdd); break;
	case EL_VELOCITY_OVERLIFE:
		EditModificationLine(layer->velocityOverLife, onAdd); break;
	case EL_SPIN:
		EditModificationLine(layer->spin, onAdd); break;
	case EL_SPIN_VARIATION:
		EditModificationLine(layer->spinVariation, onAdd); break;
	case EL_SPIN_OVERLIFE:
		EditModificationLine(layer->spinOverLife, onAdd); break;
	case EL_COLOR:
		EditModificationLine(layer->colorRandom, onAdd); break;
	case EL_ALPHA_OVERLIFE:
		EditModificationLine(layer->alphaOverLife, onAdd); break;
	case EL_COLOR_OVERLIFE:
		EditModificationLine(layer->colorOverLife, onAdd); break;
	case EL_ANGLE:
		EditModificationLine(layer->angle, onAdd); break;
	case EL_ANGLE_VARIATION:
		EditModificationLine(layer->angleVariation, onAdd); break;
	case EL_ANIM_SPEED_OVERLIFE:
		EditModificationLine(layer->animSpeedOverLife, onAdd); break;
	}
}
void ParticleEffectPropertiesWidget::EditForceModifiable(ParticleForce *force, ForceExternals lineId, bool onAdd)
{
	switch(lineId)
	{
	case EF_FORCE:
		EditModificationLine(force->GetForce(), onAdd); break;
	case EF_FORCE_VARIATION:
		EditModificationLine(force->GetForceVariation(), onAdd); break;
	case EF_FORCE_OVERLIFE:
		EditModificationLine(force->GetForceOverlife(), onAdd); break;
	}
}


void ParticleEffectPropertiesWidget::BuildEffectTree()
{
	currSelectedTreeItem = NULL;
	effectTree->clear();	
	effectTree->setHeaderLabel(QString(particleEffect->GetEntity()->GetName().c_str()));
	QTreeWidgetItem *root = effectTree->invisibleRootItem();
	EffectTreeData data;
	int32 childrenCount = particleEffect->GetEntity()->GetChildrenCount();
	for (int32 emitterId = 0; emitterId < childrenCount; emitterId++)
	{
		Entity *currEntity = particleEffect->GetEntity()->GetChild(emitterId);
		RenderObject * ro = GetRenderObject(currEntity);
		if(ro && ro->GetType() == RenderObject::TYPE_PARTICLE_EMTITTER)
		{			
			ParticleEmitter * emitter = static_cast<ParticleEmitter*>(ro);
			data.emmiter = emitter;
			QTreeWidgetItem *emitterItem = new QTreeWidgetItem(root, TreeItemTypeEmitter);
			emitterItem->setText(0, QString(currEntity->GetName().c_str()));
			emitterItem->setIcon(0, iconEmitter);						
			emitterItem->setData(0, Qt::UserRole, QVariant::fromValue(data));
			//externals
			for (int32 externalId = 0; externalId<EE_TOTAL; ++externalId)
			{
				if (GetEmitterLine(emitter, EmitterExternals(externalId)))
				{
					data.externalParamId = externalId;
					QTreeWidgetItem *externalItem = new QTreeWidgetItem(emitterItem, TreeItemTypeExternal);
					externalItem->setText(0, QString("External ")+QString(EXTERNAL_NAMES[externalId].c_str()));
					externalItem->setIcon(0, iconExternal);
					externalItem->setData(0, Qt::UserRole, QVariant::fromValue(data));
				}
			}
			data.externalParamId=0;
			//layers
			int32 numLayers = emitter->GetLayers().size();
			for (int32 layerId = 0; layerId<numLayers; ++layerId)
			{
				ParticleLayer* layer = emitter->GetLayers()[layerId];
				QTreeWidgetItem *layerItem = new QTreeWidgetItem(emitterItem, TreeItemTypeLayer);
				data.layer = layer;
				layerItem->setText(0, QString(layer->layerName.c_str()));
				layerItem->setIcon(0, iconLayer);
				layerItem->setData(0, Qt::UserRole, QVariant::fromValue(data));
				//externals
				for (int32 externalId = EE_TOTAL; externalId<EL_TOTAL; ++externalId)
				{
					if (GetLayerLine(layer, LayerExternals(externalId)))
					{
						data.externalParamId = externalId;
						QTreeWidgetItem *externalItem = new QTreeWidgetItem(layerItem, TreeItemTypeExternal);
						externalItem->setText(0, QString("External ")+QString(EXTERNAL_NAMES[externalId].c_str()));
						externalItem->setIcon(0, iconExternal);
						externalItem->setData(0, Qt::UserRole, QVariant::fromValue(data));
					}
				}
				data.externalParamId=0;
				//forces
				int32 numForces = layer->forces.size();
				for (int32 forceId=0; forceId<numForces; ++forceId)
				{
					ParticleForce* force = layer->forces[forceId];
					data.force = force;
					QTreeWidgetItem *forceItem = new QTreeWidgetItem(layerItem, TreeItemTypeForce);
					forceItem->setText(0, QString("force"));
					forceItem->setIcon(0, iconForce);
					forceItem->setData(0, Qt::UserRole, QVariant::fromValue(data));
					//externals
					for (int32 externalId = EL_TOTAL; externalId<EF_TOTAL; ++externalId)
					{
						if (GetForceLine(force, ForceExternals(externalId)))
						{
							data.externalParamId = externalId;
							QTreeWidgetItem *externalItem = new QTreeWidgetItem(forceItem, TreeItemTypeExternal);
							externalItem->setText(0, QString("External ")+QString(EXTERNAL_NAMES[externalId].c_str()));
							externalItem->setIcon(0, iconExternal);
							externalItem->setData(0, Qt::UserRole, QVariant::fromValue(data));
						}
					}
					data.externalParamId=0;
				}				
				data.force = NULL;
			}			
			data.layer = NULL;
		}
	}
	particleEffect->RebuildEffectModifiables();
}

void ParticleEffectPropertiesWidget::UpdatePlaybackSpeedLabel()
{
	if (!particleEffect)
	{
		return;
	}
	
	float32 playbackSpeedValue = particleEffect->GetPlaybackSpeed();
	effectPlaybackSpeedLabel->setText(QString("playback speed: %1x").arg(playbackSpeedValue));
}

void ParticleEffectPropertiesWidget::StoreVisualState(KeyedArchive* /* visualStateProps */)
{
	// Nothing to store for now.
}

void ParticleEffectPropertiesWidget::RestoreVisualState(KeyedArchive* /* visualStateProps */)
{
	// Nothing to restore for now.
}

void EditModificationLineDialog::InitName(const String& name, bool onAdd)
{
	QHBoxLayout *nameLayot = new QHBoxLayout();
	QLabel *varNameLabel = new QLabel("External Variable Name");
	nameLayot->addWidget(varNameLabel);
	variableName = new QLineEdit();
	variableName->setText(QString(name.c_str()));
	nameLayot->addWidget(variableName);
	dialogLayout->addLayout(nameLayot);
	if (onAdd)
	{
		variableName->setFocus();
		variableName->selectAll();
	}
}

String EditModificationLineDialog::GetVariableName()
{
	return variableName->text().toStdString();
}

void EditModificationLineDialog::InitButtons()
{
	dialogLayout->addStretch();
	QHBoxLayout* btnBox = new QHBoxLayout();
	QPushButton* btnCancel = new QPushButton("Cancel", this);
	QPushButton* btnOk = new QPushButton("Ok", this);
	btnOk->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	btnCancel->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	btnBox->addWidget(btnCancel);
	btnBox->addStretch();
	btnBox->addWidget(btnOk);
	dialogLayout->addLayout(btnBox);	
	connect(btnOk, SIGNAL(clicked(bool)), this, SLOT(accept()));
	connect(btnCancel, SIGNAL(clicked(bool)), this, SLOT(reject()));
	btnOk->setDefault(true);
}

template <> void EditModificationLineDialog::Init<float32>(ModifiablePropertyLine<float32>* line, bool onAdd)
{
	dialogLayout = new QVBoxLayout();
	setLayout(dialogLayout);
	InitName(line->GetValueName(), onAdd);
	timeLine = new TimeLineWidget(this);	
	timeLine->Init(0.0f, 1.0f, false);
	timeLine->AddLine(0, PropLineWrapper<float32>(line->GetModificationLine()).GetProps(), Qt::blue, "");	
	dialogLayout->addWidget(timeLine);
	InitButtons();
	
}

template <> void EditModificationLineDialog::Init<Vector2>(ModifiablePropertyLine<Vector2>* line, bool onAdd)
{
	dialogLayout = new QVBoxLayout();
	setLayout(dialogLayout);
	InitName(line->GetValueName(), onAdd);
	timeLine = new TimeLineWidget(this);	
	timeLine->Init(0.0f, 1.0f, false);
	Vector<QColor> vectorColors;
	vectorColors.push_back(Qt::red); vectorColors.push_back(Qt::darkGreen);
	Vector<QString> vectorLegends;
	vectorLegends.push_back("x"); vectorLegends.push_back("y");
	timeLine->AddLines(PropLineWrapper<Vector2>(line->GetModificationLine()).GetProps(), vectorColors, vectorLegends);	
	dialogLayout->addWidget(timeLine);
	InitButtons();	
}

template <> void EditModificationLineDialog::Init<Vector3>(ModifiablePropertyLine<Vector3>* line, bool onAdd)
{
	dialogLayout = new QVBoxLayout();
	setLayout(dialogLayout);
	InitName(line->GetValueName(), onAdd);
	timeLine = new TimeLineWidget(this);	
	timeLine->Init(0.0f, 1.0f, false);
	Vector<QColor> vectorColors;
	vectorColors.push_back(Qt::red); vectorColors.push_back(Qt::darkGreen); vectorColors.push_back(Qt::blue);
	Vector<QString> vectorLegends;
	vectorLegends.push_back("x"); vectorLegends.push_back("y"); vectorLegends.push_back("z");
	timeLine->AddLines(PropLineWrapper<Vector3>(line->GetModificationLine()).GetProps(), vectorColors, vectorLegends);	
	dialogLayout->addWidget(timeLine);
	InitButtons();	
}

template <> void EditModificationLineDialog::Init<Color>(ModifiablePropertyLine<Color>* line, bool onAdd)
{
	dialogLayout = new QVBoxLayout();
	setLayout(dialogLayout);
	InitName(line->GetValueName(), onAdd);
	gradientLine = new GradientPickerWidget(this);
	gradientLine->Init(0,1);
	gradientLine->SetValues(PropLineWrapper<Color>(line->GetModificationLine()).GetProps());
	dialogLayout->addWidget(gradientLine);
	InitButtons();
	
}


template <> void EditModificationLineDialog::UpdateLine<float32> (ModifiablePropertyLine<float32>* line)
{
	PropLineWrapper<float32> lineWrap;
	if(!timeLine->GetValue(0, lineWrap.GetPropsPtr()))
		return;
	line->SetModificationLine(lineWrap.GetPropLine());
}

template <> void EditModificationLineDialog::UpdateLine<Vector2> (ModifiablePropertyLine<Vector2>* line)
{
	PropLineWrapper<Vector2> lineWrap;
	if(!timeLine->GetValues(lineWrap.GetPropsPtr()))
		return;
	line->SetModificationLine(lineWrap.GetPropLine());
}

template <> void EditModificationLineDialog::UpdateLine<Vector3> (ModifiablePropertyLine<Vector3>* line)
{
	PropLineWrapper<Vector3> lineWrap;
	if(!timeLine->GetValues(lineWrap.GetPropsPtr()))
		return;
	line->SetModificationLine(lineWrap.GetPropLine());
}

template <> void EditModificationLineDialog::UpdateLine<Color> (ModifiablePropertyLine<Color>* line)
{
	PropLineWrapper<Color> lineWrap;
	if(!gradientLine->GetValues(lineWrap.GetPropsPtr()))
		return;
	line->SetModificationLine(lineWrap.GetPropLine());
}