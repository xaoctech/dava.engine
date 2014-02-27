#ifndef __RESOURCEEDITORQT__GRASSEDITORPANEL__
#define __RESOURCEEDITORQT__GRASSEDITORPANEL__

#include "LandscapeEditorBasePanel.h"
#include "DAVAEngine.h"
#include "Qt/Scene/System/GrassEditorSystem.h"

using namespace DAVA;

class QComboBox;
class QRadioButton;
class QLineEdit;
class SliderWidget;
class QDoubleSpinBox;

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
};

#endif /* defined(__RESOURCEEDITORQT__GRASSEDITORPANEL__) */
