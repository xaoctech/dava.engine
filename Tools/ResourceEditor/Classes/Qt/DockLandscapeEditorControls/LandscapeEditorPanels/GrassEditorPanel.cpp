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

    // sliders
    QFrame *sliderFrame = new QFrame(this);
    QGridLayout *frameLayout = new QGridLayout(sliderFrame);

    grassHeight = new QSlider(Qt::Horizontal, sliderFrame);
    grassHeight->setRange(0, 15);
    grassHeight->setSingleStep(1);
    grassHeight->setPageStep(4);
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

    // layers
    layersList = new QTableWidget(GRASS_EDITOR_LAYERS_COUNT, 3, this);
    //layersList->setShowGrid(false);
    layersList->horizontalHeader()->setVisible(false);
    layersList->horizontalHeader()->resizeSection(0, 24);
    layersList->horizontalHeader()->resizeSection(1, 48);
    layersList->horizontalHeader()->setResizeMode(2, QHeaderView::Stretch);
    layersList->verticalHeader()->setVisible(false);
    layersList->setSelectionBehavior(QAbstractItemView::SelectRows);
    layersList->setSelectionMode(QAbstractItemView::SingleSelection);
    layersList->setFocusPolicy(Qt::NoFocus);
    layout->addWidget(layersList);

    QString layerCBStyle = "QCheckBox::indicator { width: 16px; height: 16px; } QCheckBox::indicator:checked {image: url(:/QtIcons/layer_visible.png); } QCheckBox::indicator:unchecked {image: url(:/QtIcons/layer_invisible.png); }";

    for(int row = 0; row < GRASS_EDITOR_LAYERS_COUNT; ++row)
    {
        QString layerName;
        layerName.sprintf("Layer %d", row);
        
        QTableWidgetItem *layerVisibilityItem = new QTableWidgetItem();
        layerVisibilityItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        layerVisibilityItem->setBackground(qApp->palette().button());

        QTableWidgetItem *layerIconItem = new QTableWidgetItem();
        layerIconItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        layerIconItem->setBackground(qApp->palette().button());

        QTableWidgetItem *layerNameItem = new QTableWidgetItem(layerName);
        layerNameItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        layerNameItem->setBackground(qApp->palette().button());

        QHBoxLayout *wrapperLayout = new QHBoxLayout();
        wrapperLayout->setMargin(2);
        wrapperLayout->setSpacing(0);

        QFrame *layerCheckBoxWrapper = new QFrame();
        layerCheckBoxWrapper->setLayout(wrapperLayout);
        layerCheckBoxWrapper->setAutoFillBackground(true);
        layerCheckBoxWrapper->setFrameStyle(QFrame::Raised);
        layerCheckBoxWrapper->setFrameShape(QFrame::Panel);
        layerCheckBoxWrapper->setStyleSheet("background-color: palette(button)");

        QCheckBox * layerCheckBox = new QCheckBox(layerCheckBoxWrapper);
        layerCheckBox->setStyleSheet(layerCBStyle);
        wrapperLayout->addWidget(layerCheckBox, 0, Qt::AlignCenter);

        layersList->setItem(row, 0, layerVisibilityItem);
        layersList->setItem(row, 1, layerIconItem);
        layersList->setItem(row, 2, layerNameItem);
        layersList->setCellWidget(row, 0, layerCheckBoxWrapper);
        layersList->setRowHeight(row, 32);
        layersList->setStyleSheet("QTableWidget::item:selected{ background-color: palette(highlight) }");

        layerCheckBoxes[row] = layerCheckBox;
    }
}

void GrassEditorPanel::ConnectToSignals()
{ 
    QObject::connect(layersList, SIGNAL(currentCellChanged(int, int, int, int)), this, SLOT(OnLayerSelected(int, int, int, int)));
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
        DAVA::VegetationRenderObject *vegetationRObj = sceneEditor->grassEditorSystem->GetCurrentVegetationObject();
        if(NULL != vegetationRObj)
        {
            QPixmap txpix;
            const DAVA::TextureSheet &sheet = vegetationRObj->GetTextureSheet();

            DAVA::FilePath texturePath = vegetationRObj->GetVegetationTexture();
            texturePath.ReplaceExtension(".png");

            if(texturePath.Exists())
            {
                 txpix = QPixmap(texturePath.GetAbsolutePathname().c_str());
            }

            for(int i = 0; i < GRASS_EDITOR_LAYERS_COUNT; ++i)
            {
                QTableWidgetItem *item = layersList->item(i, 1);

                // fill visible/invisible state for layer
                if(sceneEditor->grassEditorSystem->IsLayerVisible(i))
                {
                    layerCheckBoxes[i]->setCheckState(Qt::Checked);
                }
                else
                {
                    layerCheckBoxes[i]->setCheckState(Qt::Unchecked);
                }

                // fill texture preview for layer
                bool ok = false;
                if(!txpix.isNull())
                {
                    DAVA::Rect2i r = MapTexCoord(sheet.cells[i], txpix.width(), txpix.height());
                    QPixmap pix = txpix.copy(QRect(r.x, r.y, r.dx, r.dy));

                    if(NULL != item)
                    {
                        item->setIcon(QIcon(pix));
                        ok = true;
                    }
                }

                if(!ok)
                {
                    item->setIcon(QIcon());
                }
            }

            int curHeight = sceneEditor->grassEditorSystem->GetBrushHeight();
            int curDensity = sceneEditor->grassEditorSystem->GetBrushDensity();
            int curLayer = sceneEditor->grassEditorSystem->GetCurrentLayer();

            layersList->setCurrentItem(layersList->item(curLayer, 0));
            grassHeight->setValue(curHeight);
            grassDensity->setValue(curDensity);
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

DAVA::Rect2i GrassEditorPanel::MapTexCoord(const DAVA::TextureSheetCell &cell, DAVA::uint32 w, DAVA::uint32 h) const
{
    DAVA::AABBox2 area;

    for(int i = 0; i < 4; ++i)
    {
        DAVA::Vector2 point;
        DAVA::Vector2 cellCoords = cell.coords[i];

        point.x = cellCoords.x * w;
        point.y = cellCoords.y * h;

        area.AddPoint(point);
    }

    return GrassEditorSystem::GetAffectedImageRect(area);
}
