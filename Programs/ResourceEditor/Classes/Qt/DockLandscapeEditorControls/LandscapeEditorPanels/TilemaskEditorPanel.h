#pragma once

#include "LandscapeEditorBasePanel.h"
#include "DAVAEngine.h"

#include "Render/UniqueStateSet.h"

namespace DAVA
{
class RECommandNotificationObject;
} // namespace DAVA

class QComboBox;
class QRadioButton;
class SliderWidget;
class TileTexturePreviewWidget;
class QFrame;

class TilemaskEditorPanel : public LandscapeEditorBasePanel
{
    Q_OBJECT

public:
    static const int DEF_BRUSH_MIN_SIZE = 1;
    static const int DEF_BRUSH_MAX_SIZE = 40;
    static const int DEF_STRENGTH_MIN_VALUE = 0;
    static const int DEF_STRENGTH_MAX_VALUE = 60;
    static const int STRENGTH_MAX_BOUNDARY = 999;

    explicit TilemaskEditorPanel(QWidget* parent = 0);
    virtual ~TilemaskEditorPanel();

private slots:
    void SetBrushSize(int brushSize);
    void SetToolImage(int imageIndex);
    void SetStrength(int strength);
    void SetDrawTexture(int textureIndex);
    void SetNormalDrawing();
    void SetCopyPaste();

    void IncreaseBrushSize();
    void DecreaseBrushSize();
    void IncreaseBrushSizeLarge();
    void DecreaseBrushSizeLarge();

    void IncreaseStrength();
    void DecreaseStrength();
    void IncreaseStrengthLarge();
    void DecreaseStrengthLarge();

    void PrevTexture();
    void NextTexture();

    void PrevTool();
    void NextTool();

    void OnCommandExecuted(DAVA::SceneEditor2* scene, const DAVA::RECommandNotificationObject& commandNotification);

protected:
    bool GetEditorEnabled() override;

    void OnEditorEnabled() override;

    void SetWidgetsState(bool enabled) override;
    void BlockAllSignals(bool block) override;

    void InitUI() override;
    void ConnectToSignals() override;

    void StoreState() override;
    void RestoreState() override;

    void ConnectToShortcuts() override;
    void DisconnectFromShortcuts() override;

private:
    SliderWidget* sliderWidgetBrushSize;
    SliderWidget* sliderWidgetStrength;
    QComboBox* comboBrushImage;
    TileTexturePreviewWidget* tileTexturePreviewWidget;
    QRadioButton* radioDraw;
    QRadioButton* radioCopyPaste;
    QFrame* frameStrength;
    QFrame* frameTileTexturesPreview;

    void InitBrushImages();
    void UpdateTileTextures();

    void UpdateControls();

    DAVA::int32 BrushSizeUIToSystem(DAVA::int32 uiValue);
    DAVA::int32 BrushSizeSystemToUI(DAVA::int32 systemValue);

    DAVA::float32 StrengthUIToSystem(DAVA::int32 uiValue);
    DAVA::int32 StrengthSystemToUI(DAVA::float32 systemValue);

    void SplitImageToChannels(DAVA::Image* image, DAVA::Image*& r, DAVA::Image*& g, DAVA::Image*& b, DAVA::Image*& a);
};
