#pragma once

#include "REPlatform/Scene/Systems/LandscapeEditorSystemV2/BaseTextureRenderLandscapeTool.h"
#include "REPlatform/Scene/Systems/LandscapeEditorSystemV2/BrushRenderHelper.h"
#include "REPlatform/Scene/Systems/LandscapeEditorSystemV2/KeyboardInputController.h"
#include "REPlatform/Scene/Systems/LandscapeEditorSystemV2/BaseBrushApplicant.h"

#include <TArc/DataProcessing/PropertiesHolder.h>

namespace DAVA
{
class BrushWidgetBuilder;
class BaseHeightEditTool : public BaseTextureRenderLandscapeTool
{
public:
    BaseHeightEditTool(LandscapeEditorSystemV2* system, const ButtonInfo& buttonInfo, const String& toolName);

    void Activate(const PropertiesItem& settings) override;
    BrushInputController* GetInputController() const override;
    BaseBrushApplicant* GetBrushApplicant() const override;
    BaseLandscapeTool::ButtonInfo GetButtonInfo() const override;
    RefPtr<Texture> GetCustomCoverTexture() const override;
    RefPtr<Texture> GetCursorTexture() const override;
    Vector2 GetBrushSize() const override;
    float32 GetBrushRotation() const override;
    void Deactivate(PropertiesItem& settings) override;

    void StoreSnapshots() override;
    std::unique_ptr<Command> CreateDiffCommand(const Rect& operationRect) const override;
    void OnCommandExecuted(const RECommandNotificationObject& notif) override;

protected:
    void CreateBrushSelector(BrushWidgetBuilder& builder) const;
    void CreateRotationSlider(BrushWidgetBuilder& builder) const;
    void CreateBrushSizeControl(BrushWidgetBuilder& builder) const;
    void CreateStrengthSlider(BrushWidgetBuilder& builder) const;

    void ResetInputController(std::unique_ptr<KeyboardInputController>&& controller);
    void InitTextures();
    void CreateBasePhases(Vector<BrushPhaseDescriptor>& phases) const;

    // Create FBO texture. Format = R32F. PixelReadback = true. Size == landscape's height texture.size
    // Also function push render pass that copy height from landscape's height texture in to created texture
    RefPtr<Texture> CreateFloatHeightTexture();
    // Create FBO texture. Format = landscape's height texture.format. PixelReadback = true. Size == landscape's height texture.size
    // Copy original height texture into created as is
    RefPtr<Texture> CreateHeightTextureCopy();
    // Create FBO texture. Format = landscape's normal texture.format. PixelReadback = true. Size == landscape's normal texture.size
    // Copy original normal texture into created as is
    RefPtr<Texture> CreateNormalTextureCopy();

    // Copy accurate height of point into float texture. Target value range [0.0, 1.0]
    void CopyHeightTextureToFloat(RefPtr<Texture> heightTexture, RefPtr<Texture> target);
    // Copy all mip levels of source texture into corresponding mip level of target as is
    void CopyTextureWithMips(RefPtr<Texture> source, RefPtr<Texture> target);

    const String& GetBrushPath() const;
    void SetBrushPath(const String& brushPath);

    float32 GetRotationAngle() const;
    void SetRotationAngle(const float32& angle);

    float32 GetBrushRadius() const;
    void SetBrushRadius(const float32& radius);

    float32 GetStrengthInMeters() const;
    void SetStrengthInMeters(const float32& v);

    String toolName;
    ButtonInfo buttonInfo;

    String cursorPath;
    RefPtr<Texture> cursorTexture;

    RefPtr<NMaterial> blitConvertMaterial;
    TextureBlitter blitter;

    RefPtr<Texture> morphTexture;
    RefPtr<Texture> normalMap;
    RefPtr<Texture> floatTexture;
    RefPtr<Texture> heightSnapshot;

    float32 brushSize = 0.25f;
    float32 strength = 0.05f;
    float32 rotation = 0.0f;

    std::unique_ptr<KeyboardInputController> inputController;
    std::unique_ptr<BaseBrushApplicant> brushApplicant;

    DAVA_VIRTUAL_REFLECTION(BaseHeightEditTool, BaseLandscapeTool);
};
} // namespace DAVA
