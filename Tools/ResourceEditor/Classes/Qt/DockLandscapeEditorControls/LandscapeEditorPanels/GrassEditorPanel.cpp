#include "GrassEditorPanel.h"
#include "Scene/SceneSignals.h"
#include "Scene/SceneEditor2.h"
#include "Scene/System/LandscapeEditorDrawSystem/GrassEditorProxy.h"
#include "Tools/SliderWidget/SliderWidget.h"
#include "Constants.h"

GrassEditorPanel::GrassEditorPanel(QWidget* parent)
:	LandscapeEditorBasePanel(parent)
{
	InitUI();
	ConnectToSignals();
}

GrassEditorPanel::~GrassEditorPanel()
{
}

bool GrassEditorPanel::GetEditorEnabled()
{
	return GetActiveScene()->grassEditorSystem->IsEnabledGrassEdit();
}

void GrassEditorPanel::OnEditorEnabled()
{
}

void GrassEditorPanel::SetWidgetsState(bool enabled)
{
}

void GrassEditorPanel::BlockAllSignals(bool block)
{
}

void GrassEditorPanel::InitUI()
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    layersList = new QTableWidget(3, 2, this);
    layout->addWidget(layersList);

    // layers
    layersList->setShowGrid(false);
    layersList->horizontalHeader()->setVisible(false);
    layersList->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);
    layersList->horizontalHeader()->resizeSection(1, 26);
    layersList->verticalHeader()->setVisible(false);
    layersList->setSelectionBehavior(QAbstractItemView::SelectRows);
    layersList->setSelectionMode(QAbstractItemView::SingleSelection);
    layersList->setFocusPolicy(Qt::NoFocus);

    QString layerCBStyle = "QCheckBox::indicator:checked {image: url(:/QtIcons/layer_visible.png);} QCheckBox::indicator:unchecked {image: url(:/QtIcons/layer_invisible.png);}";

    for(int row = 0; row < LAYERS_COUNT; ++row)
    {
        QString layerName;
        layerName.sprintf("Layer %d", row);
        
        QTableWidgetItem *layerNameItem = new QTableWidgetItem(layerName);
        layerNameItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

        QHBoxLayout *wrapperLayout = new QHBoxLayout();
        wrapperLayout->setMargin(0);
        wrapperLayout->setSpacing(0);

        QWidget *layerCheckBoxWrapper = new QWidget();
        layerCheckBoxWrapper->setLayout(wrapperLayout);

        QCheckBox * layerCheckBox = new QCheckBox(layerCheckBoxWrapper);
        layerCheckBox->setStyleSheet(layerCBStyle);
        wrapperLayout->addWidget(layerCheckBox, 0, Qt::AlignCenter);

        layersList->setItem(row, 0, layerNameItem);
        layersList->setCellWidget(row, 1, layerCheckBoxWrapper);
        layersList->setRowHeight(row, 24);

        layerCheckBoxes[row] = layerCheckBox;
    }

    // sliders
    QFrame *sliderFrame = new QFrame(this);
    QGridLayout *frameLayout = new QGridLayout(sliderFrame);

    grassHeight = new QSlider(Qt::Horizontal, sliderFrame);
    grassHeight->setRange(0, 3);
    grassHeight->setSingleStep(1);
    grassHeight->setPageStep(2);
    grassHeight->setTickPosition(QSlider::TicksBothSides);
    grassHeight->setMaximumHeight(21);

    grassDensity = new QSlider(Qt::Horizontal, sliderFrame);
    grassDensity->setRange(0, 15);
    grassDensity->setSingleStep(1);
    grassDensity->setPageStep(4);
    grassDensity->setTickPosition(QSlider::TicksBothSides);
    grassDensity->setMaximumHeight(21);

    frameLayout->addWidget(new QLabel("Height:"), 0, 0, Qt::AlignRight | Qt::AlignVCenter);
    frameLayout->addWidget(grassHeight, 0, 1);

    frameLayout->addWidget(new QLabel("Density:"), 1, 0, Qt::AlignRight | Qt::AlignVCenter);
    frameLayout->addWidget(grassDensity, 1, 1);

    layout->addWidget(sliderFrame);

    // brush list
    brushList = new QTableWidget(4, 1, this);
    layout->addWidget(brushList);

    brushList->setShowGrid(false);
    brushList->horizontalHeader()->setVisible(false);
    brushList->horizontalHeader()->setResizeMode(0, QHeaderView::Stretch);
    brushList->verticalHeader()->setVisible(false);
    brushList->setSelectionBehavior(QAbstractItemView::SelectRows);
    brushList->setSelectionMode(QAbstractItemView::SingleSelection);
    brushList->setFocusPolicy(Qt::NoFocus);
}

void GrassEditorPanel::ConnectToSignals()
{ 
    QObject::connect(layersList, SIGNAL(currentCellChanged(int, int, int, int)), this, SLOT(OnLayerSelected(int, int, int, int)));
    QObject::connect(brushList, SIGNAL(currentCellChanged(int, int, int, int)), this, SLOT(OnBrushSelected(int, int, int, int)));
    QObject::connect(grassHeight, SIGNAL(valueChanged(int)), this, SLOT(OnHeightChanged(int)));
    QObject::connect(grassDensity, SIGNAL(valueChanged(int)), this, SLOT(OnDensityChanged(int)));
}

void GrassEditorPanel::StoreState()
{
    SceneEditor2* sceneEditor = GetActiveScene();
}

void GrassEditorPanel::RestoreState()
{ 
    SceneEditor2* sceneEditor = GetActiveScene();
    
    if(NULL != sceneEditor)
    {
        int curLayer = sceneEditor->grassEditorSystem->GetCurrentLayer();
        layersList->setCurrentItem(layersList->item(curLayer, 0));

        for(int i = 0; i < LAYERS_COUNT; ++i)
        {
            if(sceneEditor->grassEditorSystem->IsLayerVisible(i))
            {
                layerCheckBoxes[i]->setCheckState(Qt::Checked);
            }
            else
            {
                layerCheckBoxes[i]->setCheckState(Qt::Unchecked);
            }
        }

        int curHeight = sceneEditor->grassEditorSystem->GetBrushHeight();
        int curDensity = sceneEditor->grassEditorSystem->GetBrushDensity();
        int curBrush = sceneEditor->grassEditorSystem->GetBrushType();

        grassHeight->setValue(curHeight);
        grassDensity->setValue(curDensity);

        // fill brushes list from vegetation texture sheet
        brushList->clear();
        DAVA::VegetationRenderObject *vegetationRObj = sceneEditor->grassEditorSystem->GetCurrentVegetationObject();
        if(NULL != vegetationRObj)
        {
            const DAVA::TextureSheet &sheet = vegetationRObj->GetTextureSheet();

            // TODO:
            // ...

            brushList->setItem(0, 0, new QTableWidgetItem("Brush 1"));
            brushList->setItem(1, 0, new QTableWidgetItem("Brush 2"));
            brushList->setItem(2, 0, new QTableWidgetItem("Brush 3"));
            brushList->setItem(3, 0, new QTableWidgetItem("Brush 4"));

            brushList->setCurrentItem(brushList->item(curBrush, 0));
        }
    }
}

void GrassEditorPanel::ConnectToShortcuts()
{ }

void GrassEditorPanel::DisconnectFromShortcuts()
{ }

void GrassEditorPanel::OnLayerSelected(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    SceneEditor2* sceneEditor = GetActiveScene();
    if(NULL != sceneEditor)
    {
        sceneEditor->grassEditorSystem->SetCurrentLayer(currentRow);
    }
}

void GrassEditorPanel::OnBrushSelected(int currentRow, int currentColumn, int previousRow, int previousColumn)
{
    SceneEditor2* sceneEditor = GetActiveScene();
    if(NULL != sceneEditor)
    {
        sceneEditor->grassEditorSystem->SetBrushType(currentRow);
    }
}

void GrassEditorPanel::OnHeightChanged(int value)
{
    SceneEditor2* sceneEditor = GetActiveScene();
    if(NULL != sceneEditor)
    {
        sceneEditor->grassEditorSystem->SetBrushHeight(value);
    }
}

void GrassEditorPanel::OnDensityChanged(int value)
{
    SceneEditor2* sceneEditor = GetActiveScene();
    if(NULL != sceneEditor)
    {
        sceneEditor->grassEditorSystem->SetBrushDensity(value);
    }
}
