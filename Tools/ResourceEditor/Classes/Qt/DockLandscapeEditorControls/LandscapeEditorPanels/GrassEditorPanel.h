#ifndef __RESOURCEEDITORQT__GRASSEDITORPANEL__
#define __RESOURCEEDITORQT__GRASSEDITORPANEL__

#include <QtGui>
#include "DAVAEngine.h"

#include "LandscapeEditorBasePanel.h"
#include "Scene/System/GrassEditorSystem.h"

#define LAYERS_COUNT 3

class GrassEditorPanel: public LandscapeEditorBasePanel
{
	Q_OBJECT

public:
	explicit GrassEditorPanel(QWidget* parent = 0);
	~GrassEditorPanel();

protected:
	virtual bool GetEditorEnabled();
	virtual void OnEditorEnabled();

	virtual void SetWidgetsState(bool enabled);
	virtual void BlockAllSignals(bool block);
	
	virtual void InitUI();
	virtual void ConnectToSignals();
	
	virtual void StoreState();
	virtual void RestoreState();

	virtual void ConnectToShortcuts();
	virtual void DisconnectFromShortcuts();

private:
    QTableWidget *layersList;

    QCheckBox *layerCheckBoxes[LAYERS_COUNT];
    QSlider *grassHeight;
    QSlider *grassDensity;
};

#endif /* defined(__RESOURCEEDITORQT__GRASSEDITORPANEL__) */
