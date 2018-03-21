#pragma once

#include <REPlatform/Scene/Systems/LandscapeEditorSystemV2/BaseLandscapeTool.h>
#include <REPlatform/Scene/Systems/LandscapeEditorSystemV2/KeyboardInputController.h>

#include <TArc/Utils/ReflectedPairsVector.h>

#include <Base/RefPtr.h>
#include <Render/Texture.h>
#include <Reflection/Reflection.h>
#include <Reflection/ReflectedMeta.h>

class ObjectPlacementTool : public DAVA::BaseLandscapeTool
{
public:
    enum eMode
    {
        MODE_SPAWN,
        MODE_REMOVE
    };

    ObjectPlacementTool(DAVA::LandscapeEditorSystemV2* system);
    ~ObjectPlacementTool() override;

    ButtonInfo GetButtonInfo() const override;
    void Activate(const DAVA::PropertiesItem& settings) override;
    DAVA::BrushInputController* GetInputController() const override;
    DAVA::BaseBrushApplicant* GetBrushApplicant() const override;
    QWidget* CreateEditorWidget(const WidgetParams& params) override;
    DAVA::Asset<DAVA::Texture> GetCursorTexture() const override;
    DAVA::Color GetCursorColor() const override;
    DAVA::Vector2 GetBrushSize() const override;
    DAVA::float32 GetBrushRotation() const override;
    void Deactivate(DAVA::PropertiesItem& settings) override;

private:
    class ObjectPlacementApplicant;
    bool GetLandscapeProjection(const DAVA::Vector2& landscapePosition, DAVA::Vector3& result) const;
    DAVA::Rect MapLandscapeRect(const DAVA::Rect& r) const;
    bool IsValidToAdd(const DAVA::Vector3& position, DAVA::float32 objSphereRadius, DAVA::float32 objDistance) const;
    DAVA::Vector<DAVA::Entity*> LookupObjects(const DAVA::Rect& r) const;

    DAVA::float32 GetBrushRadius() const;
    void SetBrushRadius(const DAVA::float32& brushRadius);

    eMode GetMode() const;
    void SetMode(eMode newMode);

    DAVA::float32 GetMinRotation() const;
    void SetMinRotation(const DAVA::float32& v);
    DAVA::float32 GetMaxRotation() const;
    void SetMaxRotation(const DAVA::float32& v);

    DAVA::float32 GetMinScale() const;
    void SetMinScale(const DAVA::float32& v);
    DAVA::float32 GetMaxScale() const;
    void SetMaxScale(const DAVA::float32& v);

    DAVA::float32 GetMinLandscapeInjection() const;
    void SetMinLandscapeInjection(const DAVA::float32& v);
    DAVA::float32 GetMaxLandscapeInjection() const;
    void SetMaxLandscapeInjection(const DAVA::float32& v);

    DAVA::float32 GetMinDistanceBetweenObjects() const;
    void SetMinDistanceBetweenObjects(const DAVA::float32& v);

    void OnRefreshModels();
    void OnAddLayer();
    void OnLoadBrush();
    void OnLoadBrushImpl(const DAVA::PropertiesItem& settings);
    void OnSaveBrush();
    void OnSaveBrushImpl(DAVA::PropertiesItem& settings);

    const DAVA::ReflectedPairsVector<DAVA::Entity*, DAVA::String>& GetLayers() const;
    DAVA::Entity* GetCurrentLayer() const;
    void SetCurrentLayer(DAVA::Entity* layer);
    void TrySetCurrentLayer(const DAVA::String& layerName);

    void ResetFilterData();
    void ApplyFilterData();

    struct Model
    {
        ObjectPlacementTool* tool = nullptr;
        DAVA::String name;
        DAVA::String path;
        DAVA::RefPtr<DAVA::Entity> model;
        DAVA::AABBox3 bbox;

        const DAVA::String& GetName() const;
        void SetPath(const DAVA::String& path);

        DAVA_REFLECTION(Model);
    };

    struct CreationParams
    {
        DAVA::float32 minRotationRandom = 0.0f;
        DAVA::float32 maxRotationRandom = 0.0f;
        DAVA::float32 minScaleRandom = 1.0f;
        DAVA::float32 maxScaleRandom = 1.0f;
        DAVA::float32 minLandscapeInjection = 0.0f;
        DAVA::float32 maxLandscapeInjection = 0.0f;
        DAVA::float32 minDistanceBetweenObjects = 1.0f;
    };

    DAVA::Vector<Model*> models;
    DAVA::float32 brushSize = 0.25;
    eMode mode = MODE_SPAWN;
    CreationParams params;

    DAVA::Asset<DAVA::Texture> cursorTexture;
    std::unique_ptr<DAVA::KeyboardInputController> inputController;
    std::unique_ptr<DAVA::BaseBrushApplicant> applicant;

    QPointer<QWidget> weakRootWidget;

    DAVA_VIRTUAL_REFLECTION(ObjectPlacementTool, DAVA::BaseLandscapeTool);
};
