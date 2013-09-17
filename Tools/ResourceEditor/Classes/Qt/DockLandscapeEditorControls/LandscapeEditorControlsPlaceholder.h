#ifndef __RESOURCEEDITORQT__LANDSCAPEEDITORCONTROLSPLACEHOLDER__
#define __RESOURCEEDITORQT__LANDSCAPEEDITORCONTROLSPLACEHOLDER__

#include <QWidget>
#include "DAVAEngine.h"

#include "LandscapeEditorPanels/LandscapeEditorBasePanel.h"

class LandscapeEditorControlsPlaceholder: public QWidget
{
	Q_OBJECT
	
public:
	explicit LandscapeEditorControlsPlaceholder(QWidget* parent = 0);
	~LandscapeEditorControlsPlaceholder();
	
	void SetPanel(LandscapeEditorBasePanel* panel);
	void RemovePanel();
	
private:
	LandscapeEditorBasePanel* currentPanel;
};

#endif /* defined(__RESOURCEEDITORQT__LANDSCAPEEDITORCONTROLSPLACEHOLDER__) */
