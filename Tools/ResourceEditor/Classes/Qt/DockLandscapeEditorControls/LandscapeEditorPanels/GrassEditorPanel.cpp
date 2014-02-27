#include "GrassEditorPanel.h"
#include "Scene/SceneSignals.h"
#include "Scene/SceneEditor2.h"
#include "Scene/System/LandscapeEditorDrawSystem/GrassEditorProxy.h"
#include "Constants.h"

#include <QLayout>
#include <QLabel>
#include <QComboBox>
#include <QRadioButton>
#include <QPushButton>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QEvent.h>

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
    new QPushButton("Test", this);
}

void GrassEditorPanel::ConnectToSignals()
{
}

void GrassEditorPanel::StoreState()
{
}

void GrassEditorPanel::RestoreState()
{
}

void GrassEditorPanel::ConnectToShortcuts()
{ }

void GrassEditorPanel::DisconnectFromShortcuts()
{ }
