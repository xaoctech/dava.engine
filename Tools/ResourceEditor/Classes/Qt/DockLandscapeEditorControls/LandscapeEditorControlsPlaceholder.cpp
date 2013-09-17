#include "LandscapeEditorControlsPlaceholder.h"

#include <QVBoxLayout>

LandscapeEditorControlsPlaceholder::LandscapeEditorControlsPlaceholder(QWidget* parent)
:	QWidget(parent)
,	currentPanel(NULL)
{
	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	
	setLayout(layout);
}

LandscapeEditorControlsPlaceholder::~LandscapeEditorControlsPlaceholder()
{
}

void LandscapeEditorControlsPlaceholder::SetPanel(LandscapeEditorBasePanel* panel)
{
	if (!panel || panel == currentPanel)
	{
		return;
	}
	
	RemovePanel();
	
	currentPanel = panel;
	layout()->addWidget(panel);
	
	panel->InitPanel();
}

void LandscapeEditorControlsPlaceholder::RemovePanel()
{
	if (!currentPanel)
	{
		return;
	}
	
	currentPanel->DeinitPanel();
	
	currentPanel->setParent(NULL);
	layout()->removeWidget(currentPanel);
	currentPanel = NULL;
}
