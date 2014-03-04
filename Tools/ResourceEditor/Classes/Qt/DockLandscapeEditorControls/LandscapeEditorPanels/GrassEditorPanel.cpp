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
        //layerCheckBoxWrapper->setFrameStyle(QFrame::Raised);
        //layerCheckBoxWrapper->setFrameShape(QFrame::Panel);
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

    frameLayout->addWidget(new QLabel("Height:"), 0, 0, Qt::AlignRight);
    frameLayout->addWidget(grassHeight, 0, 1);

    frameLayout->addWidget(new QLabel("Density:"), 1, 0, Qt::AlignRight);
    frameLayout->addWidget(grassDensity, 1, 1);

    layout->addWidget(sliderFrame);
}

void GrassEditorPanel::ConnectToSignals()
{ }

void GrassEditorPanel::StoreState()
{ }

void GrassEditorPanel::RestoreState()
{ }

void GrassEditorPanel::ConnectToShortcuts()
{ }

void GrassEditorPanel::DisconnectFromShortcuts()
{ }
