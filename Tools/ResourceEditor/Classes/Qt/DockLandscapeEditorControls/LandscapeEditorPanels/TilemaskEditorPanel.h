#ifndef __RESOURCEEDITORQT__TILEMASKEDITORPANEL__
#define __RESOURCEEDITORQT__TILEMASKEDITORPANEL__

#include "LandscapeEditorBasePanel.h"
#include "DAVAEngine.h"

using namespace DAVA;

class QComboBox;
class SliderWidget;

class TilemaskEditorPanel: public LandscapeEditorBasePanel
{
	Q_OBJECT

public:
	static const int DEF_BRUSH_MIN_SIZE = 3;
	static const int DEF_BRUSH_MAX_SIZE = 40;
	static const int DEF_STRENGTH_MIN_VALUE = 0;
	static const int DEF_STRENGTH_MAX_VALUE = 60;

	explicit TilemaskEditorPanel(QWidget* parent = 0);
	virtual ~TilemaskEditorPanel();

private slots:
	void SetBrushSize(int brushSize);
	void SetToolImage(int imageIndex);
	void SetStrength(int strength);
	void SetDrawTexture(int textureIndex);

protected:
	virtual bool GetEditorEnabled();

	virtual void OnEditorEnabled();

	virtual void SetWidgetsState(bool enabled);
	virtual void BlockAllSignals(bool block);

	virtual void InitUI();
	virtual void ConnectToSignals();

	virtual void StoreState();
	virtual void RestoreState();

private:
	SliderWidget* sliderWidgetBrushSize;
	SliderWidget* sliderWidgetStrength;
	QComboBox* comboBrushImage;
	QComboBox* comboTileTexture;

	void InitBrushImages();
	void UpdateTileTextures();

	int32 BrushSizeUIToSystem(int32 uiValue);
	int32 BrushSizeSystemToUI(int32 systemValue);

	float32 StrengthUIToSystem(int32 uiValue);
	int32 StrengthSystemToUI(float32 systemValue);
};

#endif /* defined(__RESOURCEEDITORQT__TILEMASKEDITORPANEL__) */
