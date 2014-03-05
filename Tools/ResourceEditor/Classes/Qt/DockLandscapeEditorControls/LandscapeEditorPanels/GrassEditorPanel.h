#ifndef __RESOURCEEDITORQT__GRASSEDITORPANEL__
#define __RESOURCEEDITORQT__GRASSEDITORPANEL__

#include <QtGui>
#include "DAVAEngine.h"

#include "LandscapeEditorBasePanel.h"
#include "Scene/System/GrassEditorSystem.h"

#define LAYERS_COUNT 3
#define BRUSH_COUNT

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

protected slots:
    void OnLayerSelected(int currentRow, int currentColumn, int previousRow, int previousColumn);
    void OnBrushSelected(int currentRow, int currentColumn, int previousRow, int previousColumn);
    void OnHeightChanged(int value);
    void OnDensityChanged(int value);

private:
    QTableWidget *layersList;
    QTableWidget *brushList;

    QCheckBox *layerCheckBoxes[LAYERS_COUNT];
    QSlider *grassHeight;
    QSlider *grassDensity;
};

#endif /* defined(__RESOURCEEDITORQT__GRASSEDITORPANEL__) */
