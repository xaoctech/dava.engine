/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#include "LODEditor.h"
#include "ui_LODEditor.h"

#include "EditorLODData.h"
#include "DistanceSlider.h"

#include "Classes/Qt/Main/QtMainWindowHandler.h"
#include "Classes/Qt/Scene/SceneSignals.h"

#include <QLabel>


struct DistanceWidget
{
    QLabel *name;
    QDoubleSpinBox *distance;
    
    void SetVisible(bool visible)
    {
        name->setVisible(visible);
        distance->setVisible(visible);
    }
};

static DistanceWidget distanceWidgets[DAVA::LodComponent::MAX_LOD_LAYERS];


LODEditor::LODEditor(QWidget* parent)
:	QWidget(parent),
	ui(new Ui::LODEditor)
{
	ui->setupUi(this);
    posSaver.Attach(this);
    
    editedLODData = new EditorLODData();
    connect(editedLODData, SIGNAL(DataChanged()), SLOT(LODDataChanged()));
    
    SetupInternalSignals();
    SetupSceneSignals();
    
    ForceDistanceStateChanged(Qt::Unchecked);
    LODDataChanged();
}

LODEditor::~LODEditor()
{
    SafeDelete(editedLODData);
    
	delete ui;
}

void LODEditor::SetupInternalSignals()
{
    InitCorrectionSpinBox(ui->lod0Correction, 0);
    InitCorrectionSpinBox(ui->lod1Correction, 1);
    InitCorrectionSpinBox(ui->lod2Correction, 2);
    InitCorrectionSpinBox(ui->lod3Correction, 3);
    
    connect(ui->enableForceDistance, SIGNAL(stateChanged(int)), SLOT(ForceDistanceStateChanged(int)));
    connect(ui->forceSlider, SIGNAL(valueChanged(int)), SLOT(ForceDistanceChanged(int)));
    ui->forceSlider->setRange(0, DAVA::LodComponent::MAX_LOD_DISTANCE);
    ui->forceSlider->setValue(0);
    
    connect(ui->distanceSlider, SIGNAL(DistanceChanged(int, double)), SLOT(LODDistanceChangedBySlider(int, double)));
    
    InitDistanceSpinBox(ui->lod0Name, ui->lod0Distance, 0);
    InitDistanceSpinBox(ui->lod1Name, ui->lod1Distance, 1);
    InitDistanceSpinBox(ui->lod2Name, ui->lod2Distance, 2);
    InitDistanceSpinBox(ui->lod3Name, ui->lod3Distance, 3);
    
    SetForceLayerValues(DAVA::LodComponent::MAX_LOD_LAYERS);
    connect(ui->forceLayer, SIGNAL(activated(const QString &)), SLOT(ForceLayerActivated(const QString &)));
}

void LODEditor::SetupSceneSignals()
{
    connect(SceneSignals::Instance(), SIGNAL(Activated(SceneEditor2 *)), this, SLOT(SceneActivated(SceneEditor2 *)));
    connect(SceneSignals::Instance(), SIGNAL(Deactivated(SceneEditor2 *)), this, SLOT(SceneDeactivated(SceneEditor2 *)));
    connect(SceneSignals::Instance(), SIGNAL(StructureChanged(SceneEditor2 *, DAVA::Entity *)), this, SLOT(SceneStructureChanged(SceneEditor2 *, DAVA::Entity *)));
}


void LODEditor::LODCorrectionChanged(double value)
{
    QDoubleSpinBox *spinBox = dynamic_cast<QDoubleSpinBox *>(sender());
    if(spinBox)
    {
        //TODO set new value to scene
        int lodLevel = spinBox->property("tag").toInt();
        
        UpdateSpinboxColor(spinBox);
    }
}


void LODEditor::ForceDistanceStateChanged(int checked)
{
    bool enable = (checked == Qt::Checked);
    editedLODData->EnableForceDistance(enable);
    
    ui->forceSlider->setEnabled(enable);
    ui->forceLayer->setEnabled(!enable);
}

void LODEditor::ForceDistanceChanged(int distance)
{
    editedLODData->SetForceDistance(distance);
}


void LODEditor::InitCorrectionSpinBox(QDoubleSpinBox *spinbox, int index)
{
    spinbox->setRange(-100.f, +100.f);  //persents
    spinbox->setProperty(ResourceEditor::TAG.c_str(), index);
    spinbox->setValue(0.f);
    
    connect(spinbox, SIGNAL(valueChanged(double)), SLOT(LODCorrectionChanged(double)));
}

void LODEditor::InitDistanceSpinBox(QLabel *name, QDoubleSpinBox *spinbox, int index)
{
    spinbox->setRange(0.f, DAVA::LodComponent::MAX_LOD_DISTANCE);  //distance 
    spinbox->setProperty(ResourceEditor::TAG.c_str(), index);
    spinbox->setValue(0.f);
    spinbox->setKeyboardTracking(false);
    
    connect(spinbox, SIGNAL(valueChanged(double)), SLOT(LODDistanceChangedBySpinbox(double)));
    
    distanceWidgets[index].name = name;
    distanceWidgets[index].distance = spinbox;
    
    distanceWidgets->SetVisible(false);
}


void LODEditor::UpdateSpinboxColor(QDoubleSpinBox *spinbox)
{
    QColor color = ((spinbox->value() > 0)) ? Qt::red : Qt::green;
    
    QPalette* palette = new QPalette();
    palette->setColor(QPalette::Text, color);
    
    spinbox->setPalette(*palette);
}


void LODEditor::SceneActivated(SceneEditor2 *scene)
{
    //TODO: set gloabal scene settings

    editedLODData->GetDataFromSelection(scene);
}


void LODEditor::SceneDeactivated(SceneEditor2 *scene)
{
    //TODO: clear/save gloabal scene settings

    ForceDistanceStateChanged(Qt::Unchecked);
    editedLODData->Clear();
}


void LODEditor::SceneStructureChanged(SceneEditor2 *scene, DAVA::Entity *parent)
{
    editedLODData->GetDataFromSelection(scene);
}


void LODEditor::LODDataChanged()
{
    DAVA::int32 lodLayersCount = editedLODData->GetLayersCount();
    
    ui->distanceSlider->SetLayersCount(lodLayersCount);
    SetForceLayerValues(lodLayersCount);
    
    for (DAVA::int32 i = 0; i < lodLayersCount; ++i)
    {
        distanceWidgets[i].SetVisible(true);
        
        DAVA::float32 distance = editedLODData->GetLayerDistance(i);
        
        SetSpinboxValue(distanceWidgets[i].distance, distance);
        ui->distanceSlider->SetDistance(i, distance);
    }
    for (DAVA::int32 i = lodLayersCount; i < DAVA::LodComponent::MAX_LOD_LAYERS; ++i)
    {
        distanceWidgets[i].SetVisible(false);
    }
}

void LODEditor::LODDistanceChangedBySlider(int layerNum, double value)
{
    editedLODData->SetLayerDistance(layerNum, value);
    SetSpinboxValue(distanceWidgets[layerNum].distance, value);
}

void LODEditor::LODDistanceChangedBySpinbox(double value)
{
    QDoubleSpinBox *spinBox = dynamic_cast<QDoubleSpinBox *>(sender());
    if(spinBox)
    {
        //TODO set new value to scene
        int lodLevel = spinBox->property(ResourceEditor::TAG.c_str()).toInt();
        
        editedLODData->SetLayerDistance(lodLevel, value);
        ui->distanceSlider->SetDistance(lodLevel, value);
    }
}

void LODEditor::SetSpinboxValue(QDoubleSpinBox *spinbox, double value)
{
    bool wasBlocked = spinbox->blockSignals(true);
    spinbox->setValue(value);
    spinbox->blockSignals(wasBlocked);
}

void LODEditor::ForceLayerActivated(const QString & text)
{
    int layer = text.toInt();
    editedLODData->SetForceLayer(layer);
}

void LODEditor::SetForceLayerValues(int layersCount)
{
    ui->forceLayer->clear();
    
    ui->forceLayer->addItem(Format("%d", DAVA::LodComponent::INVALID_LOD_LAYER));
    for(DAVA::int32 i = 0; i < layersCount; ++i)
    {
        ui->forceLayer->addItem(Format("%d", i));
    }
}

