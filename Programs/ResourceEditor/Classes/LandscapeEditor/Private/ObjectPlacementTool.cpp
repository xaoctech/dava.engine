#include "Classes/LandscapeEditor/Private/ObjectPlacementTool.h"

#include "Classes/LandscapeEditor/Private/MassObjectCreationComponents.h"
#include "Classes/LandscapeEditor/Private/MassObjectCreationSystem.h"
#include "Classes/PropertyPanel/FilePathExtensions.h"

#include <REPlatform/Commands/EntityAddCommand.h>
#include <REPlatform/Commands/EntityRemoveCommand.h>
#include <REPlatform/Commands/RECommandBatch.h>
#include <REPlatform/DataNodes/ProjectManagerData.h>
#include <REPlatform/Scene/SceneEditor2.h>
#include <REPlatform/Scene/Systems/CollisionSystem.h>
#include <REPlatform/Scene/Systems/LandscapeEditorSystemV2/BaseBrushApplicant.h>
#include <REPlatform/Scene/Systems/LandscapeEditorSystemV2/BrushWidgetBuilder.h>
#include <REPlatform/Scene/Systems/LandscapeEditorSystemV2/LandscapeEditorSystemV2.h>
#include <REPlatform/Scene/Systems/StructureSystem.h>
#include <REPlatform/Scene/Utils/Utils.h>

#include <TArc/Controls/ControlDescriptor.h>
#include <TArc/Controls/DoubleSpinBox.h>
#include <TArc/Controls/FilePathEdit.h>
#include <TArc/Controls/ListView.h>
#include <TArc/Controls/PopupLineEdit.h>
#include <TArc/Controls/PropertyPanel/BaseComponentValue.h>
#include <TArc/Controls/QtBoxLayouts.h>
#include <TArc/Controls/ReflectedButton.h>
#include <TArc/Controls/Slider.h>
#include <TArc/Controls/SpinSlider.h>
#include <TArc/Core/Deprecated.h>
#include <TArc/DataProcessing/PropertiesHolder.h>
#include <TArc/Utils/Utils.h>
#include <TArc/WindowSubSystem/UI.h>

#include <Base/GlobalEnum.h>
#include <Command/Command.h>
#include <Engine/Engine.h>
#include <FileSystem/FilePath.h>
#include <Math/Color.h>
#include <Math/Rect.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Components/RenderComponent.h>
#include <Scene3D/Components/TransformComponent.h>
#include <Scene3D/Scene.h>
#include <Utils/Random.h>

#include <QInputDialog>
#include <QModelIndex>
#include <QPainter>
#include <QStyleOptionViewItem>
#include <QStyledItemDelegate>

ENUM_DECLARE(ObjectPlacementTool::eMode)
{
    ENUM_ADD_DESCR(ObjectPlacementTool::MODE_SPAWN, "Spawn objects");
    ENUM_ADD_DESCR(ObjectPlacementTool::MODE_REMOVE, "Remove objects");
}

namespace ObjectPlacementToolDetail
{
class SpawnEntityCommand : public DAVA::EntityAddCommand
{
public:
    SpawnEntityCommand(DAVA::Entity* e, DAVA::Entity* parent)
        : DAVA::EntityAddCommand(e, parent, nullptr)
    {
    }

    void Redo() override
    {
        if (firstRedo == true)
        {
            firstRedo = false;
        }
        else
        {
            DAVA::EntityAddCommand::Redo();
        }
    }

private:
    bool firstRedo = true;
    DAVA_VIRTUAL_REFLECTION_IN_PLACE(SpawnEntityCommand, DAVA::EntityAddCommand)
    {
        DAVA::ReflectionRegistrator<SpawnEntityCommand>::Begin()
        .End();
    }
};

class MassRemoveEntityCommand : public DAVA::EntityRemoveCommand
{
public:
    MassRemoveEntityCommand(DAVA::Entity* e)
        : DAVA::EntityRemoveCommand(e)
    {
    }

    void Redo() override
    {
        if (firstRedo == true)
        {
            firstRedo = false;
        }
        else
        {
            DAVA::EntityRemoveCommand::Redo();
        }
    }

private:
    bool firstRedo = true;
    DAVA_VIRTUAL_REFLECTION_IN_PLACE(MassRemoveEntityCommand, DAVA::EntityRemoveCommand)
    {
        DAVA::ReflectionRegistrator<MassRemoveEntityCommand>::Begin()
        .End();
    }
};

class ListViewItemDelegate : public QStyledItemDelegate
{
    using TBase = QStyledItemDelegate;

public:
    ListViewItemDelegate(DAVA::ContextAccessor* accessor_, DAVA::UI* ui_, const DAVA::WindowKey& wndKey_, DAVA::DataWrappersProcessor* processor_, QObject* parent)
        : QStyledItemDelegate(parent)
        , accessor(accessor_)
        , ui(ui_)
        , wndKey(wndKey_)
        , processor(processor_)
    {
    }

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override
    {
        DVASSERT(createdControl == nullptr);
        DVASSERT(cachedValue.IsEmpty());

        cachedValue = index.data(Qt::EditRole).value<QString>().toStdString();

        DAVA::FilePathEdit::Params p(accessor, ui, wndKey);
        p.fields[DAVA::FilePathEdit::Fields::Value] = "value";

        ListViewItemDelegate* nonConstThis = const_cast<ListViewItemDelegate*>(this);
        DAVA::FilePathEdit* edit = new DAVA::FilePathEdit(p, processor, DAVA::Reflection::Create(DAVA::ReflectedObject(nonConstThis)), parent);
        createdControl = edit;
        createdControl->ForceUpdate();
        return createdControl->ToWidgetCast();
    }

    void destroyEditor(QWidget* editor, const QModelIndex& index) const override
    {
        if (editor == nullptr)
        {
            return;
        }

        DVASSERT(createdControl != nullptr);
        createdControl->TearDown();
        createdControl = nullptr;
        editor->deleteLater();

        cachedValue = DAVA::FilePath();
    }

    void setEditorData(QWidget* editor, const QModelIndex& index) const override
    {
    }

    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override
    {
        model->setData(index, QString::fromStdString(cachedValue.GetAbsolutePathname()));
    }

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override
    {
        TBase::paint(painter, option, index);

        painter->save();
        painter->setPen(QPen(option.palette.window(), 2.0f));
        painter->drawRect(option.rect.adjusted(-1, -1, -1, -1));
        painter->restore();
    }

    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override
    {
        QSize result = TBase::sizeHint(option, index);
        result.rwidth() += 2;

        return result;
    }

private:
    DAVA::FilePath GetValue() const
    {
        return cachedValue;
    }

    void SetValue(const DAVA::FilePath& path)
    {
        cachedValue = path;
        emit commitData(createdControl->ToWidgetCast());
    }

private:
    DAVA::ContextAccessor* accessor = nullptr;
    DAVA::UI* ui = nullptr;
    DAVA::WindowKey wndKey;
    DAVA::DataWrappersProcessor* processor = nullptr;
    mutable DAVA::ControlProxy* createdControl = nullptr;
    mutable DAVA::FilePath cachedValue;

    DAVA_REFLECTION(ListViewItemDelegate)
    {
        DAVA::ReflectionRegistrator<ListViewItemDelegate>::Begin()
        .Field("value", &ListViewItemDelegate::GetValue, &ListViewItemDelegate::SetValue)[CreateSceneFileMeta()]
        .End();
    }
};

class ModelsListView : public DAVA::ListView
{
public:
    ModelsListView(const ListView::Params& params, DAVA::DataWrappersProcessor* wrappersProcessor, DAVA::Reflection model, QWidget* parent = nullptr)
        : ListView(params, wrappersProcessor, model, parent)
        , processor(wrappersProcessor)
    {
    }

    QAbstractItemDelegate* CreateDelegate() override
    {
        return new ListViewItemDelegate(controlParams.accessor, controlParams.ui, controlParams.wndKey, processor, this);
    }

private:
    DAVA::DataWrappersProcessor* processor = nullptr;
};
} // namespace ObjectPlacementToolDetail

class ObjectPlacementTool::ObjectPlacementApplicant : public DAVA::BaseBrushApplicant
{
public:
    ObjectPlacementApplicant(ObjectPlacementTool* tool_)
        : tool(tool_)
    {
    }

    void StoreSnapshots() override
    {
        batch.reset(new DAVA::RECommandBatch("Place objects", 1));
    }

    void ApplyBrush(DAVA::Scene* scene, const DAVA::Rect& applyRect) override
    {
        if (tool->GetMode() == MODE_SPAWN)
        {
            ApplySpawnBrush(scene, applyRect);
        }
        else
        {
            ApplyRemovingBrush(scene, applyRect);
        }
    }

    void ApplySpawnBrush(DAVA::Scene* scene, const DAVA::Rect& applyRect)
    {
        DAVA::Random* random = DAVA::GetEngineContext()->random;

        if (tool->models.size() < 2)
        {
            return;
        }

        DAVA::Rect landscapeRect = tool->MapLandscapeRect(applyRect);

        DAVA::float32 y = landscapeRect.y;
        DAVA::float32 x = landscapeRect.x;

        DAVA::Vector2 rectCenter = landscapeRect.GetCenter();
        DAVA::float32 brushRadius = 0.5f * landscapeRect.dx;

        DAVA::float32 minDistance = std::max(tool->params.minDistanceBetweenObjects, 0.00001f);
        DAVA::float32 halfMinDistance = 0.5f * minDistance;

        DAVA::Entity* parentEntity = tool->GetCurrentLayer();
        DVASSERT(parentEntity != nullptr);

        ObjectPlacementTool::Model* model = GetCurrentModel();

        auto offsetRand = [random]() {
            DAVA::float32 rand = random->RandFloat32InBounds(-0.5f, 0.5f);
            if (rand < 0.0)
            {
                return rand - 0.5f;
            }

            return rand + 0.5f;
        };

        bool spawned = false;
        while (y < landscapeRect.y + landscapeRect.dy)
        {
            DAVA::float32 maxY = 0.0;
            while (x < landscapeRect.x + landscapeRect.dx)
            {
                DAVA::float32 xOffset = offsetRand() * halfMinDistance;
                DAVA::float32 yOffset = offsetRand() * halfMinDistance;
                DAVA::Vector2 position(x + xOffset, y + yOffset);
                if ((rectCenter - position).Length() < brushRadius)
                {
                    spawned = true;
                    DAVA::float32 moveStep = SpawnObject(model, position, minDistance, parentEntity, random);
                    maxY = std::max(maxY, moveStep);
                    x += moveStep;
                    model = GetCurrentModel();
                }
                x += 2.0f * minDistance;
            }

            x = landscapeRect.x;
            y += maxY;
            y += 2.0f * minDistance;
        }

        if (spawned == false)
        {
            SpawnObject(model, rectCenter, minDistance, parentEntity, random);
        }
    }

    struct EntitySphereNode
    {
        DAVA::Entity* e = nullptr;
        DAVA::Vector3 pos;
        DAVA::float32 radius = 0;
    };

    void ApplyRemovingBrush(DAVA::Scene* scene, const DAVA::Rect& applyRect)
    {
        using namespace DAVA;

        Vector<Entity*> objects = tool->LookupObjects(tool->MapLandscapeRect(applyRect));
        if (objects.empty() == true)
        {
            return;
        }

        Vector<EntitySphereNode> spheres;
        spheres.reserve(objects.size());
        for (DAVA::Entity* e : objects)
        {
            AABBox3 retBBox;

            RenderComponent* renderComponent = e->GetComponent<RenderComponent>();
            TransformComponent* transformComponent = e->GetComponent<TransformComponent>();
            if (renderComponent && transformComponent)
            {
                AABBox3 wtBox;
                renderComponent->GetRenderObject()->GetBoundingBox().GetTransformedBox(transformComponent->GetWorldTransform(), wtBox);
                retBBox.AddAABBox(wtBox);

                Vector3 sizes = retBBox.GetSize();
                float32 maxSize = 0.5f * std::max(sizes.x, sizes.y);

                spheres.push_back(EntitySphereNode{ e, retBBox.GetCenter(), maxSize });
            }
        }

        bool removeAll = tool->params.minDistanceBetweenObjects < 0.0001f;
        if (removeAll == true)
        {
            for (DAVA::Entity* e : objects)
            {
                DAVA::Entity* parent = e->GetParent();
                DVASSERT(parent != nullptr);

                batch->Add(std::make_unique<ObjectPlacementToolDetail::MassRemoveEntityCommand>(e));
                parent->RemoveNode(e);
            }
        }
        else
        {
            for (size_t leftIndex = 0; leftIndex < objects.size() - 1; ++leftIndex)
            {
                for (size_t rightIndex = leftIndex + 1; rightIndex < objects.size(); ++rightIndex)
                {
                    EntitySphereNode& left = spheres[leftIndex];
                    EntitySphereNode& right = spheres[rightIndex];

                    if (left.e == nullptr || right.e == nullptr)
                    {
                        continue;
                    }

                    float32 distance = (left.pos - right.pos).Length();
                    if (distance - left.radius - right.radius < tool->params.minDistanceBetweenObjects)
                    {
                        DAVA::Entity* parent = right.e->GetParent();
                        DVASSERT(parent != nullptr);

                        batch->Add(std::make_unique<ObjectPlacementToolDetail::MassRemoveEntityCommand>(right.e));
                        parent->RemoveNode(right.e);
                        right.e = nullptr;
                    }
                }
            }
        }
    }

    std::unique_ptr<DAVA::Command> CreateDiffCommand(const DAVA::Rect& operationRect) override
    {
        return std::move(batch);
    }

private:
    DAVA::float32 SpawnObject(ObjectPlacementTool::Model* model, const DAVA::Vector2& position,
                              DAVA::float32 minDistance, DAVA::Entity* parent, DAVA::Random* random)
    {
        DAVA::AABBox3 objectBox = model->bbox;
        DAVA::Vector3 size = objectBox.GetSize();
        DAVA::float32 moveStep = std::sqrt(size.x * size.x + size.y * size.y);
        DAVA::float32 step = 0.5f * moveStep;

        DAVA::Vector3 landscapePos;
        if (tool->GetLandscapeProjection(position, landscapePos) == true)
        {
            DAVA::float32 rotation = random->RandFloat32InBounds(tool->params.minRotationRandom, tool->params.maxRotationRandom);
            DAVA::float32 scale = random->RandFloat32InBounds(tool->params.minScaleRandom, tool->params.maxScaleRandom);
            DAVA::float32 belowLand = random->RandFloat32InBounds(tool->params.minLandscapeInjection, tool->params.maxLandscapeInjection);

            landscapePos.z += belowLand;

            DAVA::Matrix4 translateMatrix = DAVA::Matrix4::MakeTranslation(landscapePos);
            DAVA::Matrix4 rotationMatrix = DAVA::Matrix4::MakeRotation(DAVA::Vector3(0.0f, 0.0f, 1.0f), DAVA::PI_2 * rotation);
            DAVA::Matrix4 scaleMatrix = DAVA::Matrix4::MakeScale(DAVA::Vector3(scale, scale, scale));

            DAVA::Matrix4 transform = scaleMatrix * rotationMatrix * translateMatrix;

            if (tool->IsValidToAdd(landscapePos, step * scale, minDistance))
            {
                DAVA::RefPtr<DAVA::Entity> clone(model->model->Clone());
                clone->SetLocalTransform(transform);

                MassCreatedObjectComponent* component = new MassCreatedObjectComponent();
                component->SetSourceModelPath(model->path);
                clone->AddComponent(component);

                DAVA::QueryObjectDataComponent* queryDataComponent = new DAVA::QueryObjectDataComponent();
                queryDataComponent->SetData(1);
                clone->AddComponent(queryDataComponent);

                batch->Add(std::make_unique<ObjectPlacementToolDetail::SpawnEntityCommand>(clone.Get(), parent));
                parent->AddNode(clone.Get());

                moveStep *= scale;
                NextModel();
            }
        }

        return moveStep;
    }

    void NextModel()
    {
        size_t modelsCount = tool->models.size();
        DVASSERT(modelsCount > 0);
        --modelsCount;

        currentModelIndex++;
        if (currentModelIndex >= modelsCount)
        {
            currentModelIndex = 0;
        }
    }

    ObjectPlacementTool::Model* GetCurrentModel()
    {
        return tool->models[currentModelIndex];
    }

private:
    std::unique_ptr<DAVA::RECommandBatch> batch;
    ObjectPlacementTool* tool = nullptr;
    size_t currentModelIndex = 0;
};

ObjectPlacementTool::ObjectPlacementTool(DAVA::LandscapeEditorSystemV2* system)
    : BaseLandscapeTool(system)
{
    cursorTexture = DAVA::CreateSingleMipTexture(DAVA::FilePath("~res:/ResourceEditor/LandscapeEditorV2/PlacementCursor/cursor.png"));
    inputController.reset(new DAVA::KeyboardInputController());
    applicant.reset(new ObjectPlacementApplicant(this));

    inputController->RegisterVarCallback(DAVA::eInputElements::KB_H, [this](const DAVA::Vector2& delta) {
        brushSize = DAVA::Saturate(brushSize + 0.1 * delta.dy);
    });
}

ObjectPlacementTool::~ObjectPlacementTool()
{
    DVASSERT(models.empty() == true);
}

DAVA::BaseLandscapeTool::ButtonInfo ObjectPlacementTool::GetButtonInfo() const
{
    ButtonInfo info;
    info.icon = DAVA::SharedIcon(":/QtIcons/user_object.png");
    info.tooltip = "Place objects";

    return info;
}

void ObjectPlacementTool::Activate(const DAVA::PropertiesItem& settings)
{
    OnLoadBrushImpl(settings);
}

DAVA::BrushInputController* ObjectPlacementTool::GetInputController() const
{
    return inputController.get();
}

DAVA::BaseBrushApplicant* ObjectPlacementTool::GetBrushApplicant() const
{
    return applicant.get();
}

QWidget* ObjectPlacementTool::CreateEditorWidget(const WidgetParams& params)
{
    using namespace DAVA;

    Reflection model = Reflection::Create(ReflectedObject(this));
    BrushWidgetBuilder builder(params, model);

    {
        Widget* wLoadSave = new Widget(builder.GetWidget());
        QtHBoxLayout* layout = new QtHBoxLayout();
        layout->setSpacing(2);
        layout->setMargin(1);
        wLoadSave->SetLayout(layout);

        ReflectedPushButton::Params loadBrush(params.accessor, params.ui, params.wndKey);
        loadBrush.fields[ReflectedPushButton::Fields::Clicked] = "onLoadBrush";
        loadBrush.fields[ReflectedPushButton::Fields::Text].BindConstValue("Load");
        loadBrush.fields[ReflectedPushButton::Fields::Icon].BindConstValue(QIcon(":/TArc/Resources/import.png"));
        loadBrush.fields[ReflectedPushButton::Fields::Tooltip].BindConstValue("Load brush");
        wLoadSave->AddControl(new ReflectedPushButton(loadBrush, params.processor, model, wLoadSave->ToWidgetCast()));

        ReflectedPushButton::Params saveBrush(params.accessor, params.ui, params.wndKey);
        saveBrush.fields[ReflectedPushButton::Fields::Clicked] = "onSaveBrush";
        saveBrush.fields[ReflectedPushButton::Fields::Text].BindConstValue("Save");
        saveBrush.fields[ReflectedPushButton::Fields::Icon].BindConstValue(QIcon(":/TArc/Resources/export.png"));
        saveBrush.fields[ReflectedPushButton::Fields::Tooltip].BindConstValue("Save brush");
        wLoadSave->AddControl(new ReflectedPushButton(saveBrush, params.processor, model, wLoadSave->ToWidgetCast()));

        wLoadSave->ForceUpdate();
        builder.AddRow("", wLoadSave);
    }

    {
        Widget* wLayer = new Widget(builder.GetWidget());
        QtHBoxLayout* layout = new QtHBoxLayout();
        layout->setSpacing(2);
        layout->setMargin(1);
        wLayer->SetLayout(layout);

        ComboBox::Params layerComboParams(params.accessor, params.ui, params.wndKey);
        layerComboParams.fields[ComboBox::Fields::Enumerator] = "layers";
        layerComboParams.fields[ComboBox::Fields::Value] = "currentLayer";
        layerComboParams.fields[ComboBox::Fields::MultipleValueText].BindConstValue("Empty layer");
        wLayer->AddControl(new ComboBox(layerComboParams, params.processor, model, wLayer->ToWidgetCast()));

        ReflectedButton::Params addLayer(params.accessor, params.ui, params.wndKey);
        addLayer.fields[ReflectedButton::Fields::Clicked] = "onAddLayer";
        addLayer.fields[ReflectedButton::Fields::AutoRaise].BindConstValue(false);
        addLayer.fields[ReflectedButton::Fields::Icon].BindConstValue(QIcon(":/TArc/Resources/cplus.png"));
        addLayer.fields[ReflectedButton::Fields::IconSize].BindConstValue(BaseComponentValue::toolButtonIconSize);
        addLayer.fields[ReflectedButton::Fields::Tooltip].BindConstValue("Add layer");
        wLayer->AddControl(new ReflectedButton(addLayer, params.processor, model, wLayer->ToWidgetCast()));

        wLayer->ForceUpdate();
        builder.AddRow("Layer", wLayer);
    }

    {
        ControlDescriptorBuilder<ComboBox::Fields> p;
        p[ComboBox::Fields::Value] = "mode";
        builder.RegisterParam<ComboBox>("Mode (Ctrl)", p);
    }

    {
        Widget* modelsList = new Widget(builder.GetWidget());
        QtVBoxLayout* layout = new QtVBoxLayout();
        layout->setSpacing(2);
        layout->setMargin(1);
        modelsList->SetLayout(layout);

        layout->addWidget(new QLabel("Models", modelsList->ToWidgetCast()), 0, Qt::AlignTop);

        ListView::Params p(params.accessor, params.ui, params.wndKey);
        p.fields[ListView::Fields::ValueList] = "models";
        p.fields[ListView::Fields::ItemEditingEnabled].BindConstValue(true);
        p.fields[ListView::Fields::ValueFieldName].BindConstValue(FastName("path"));
        p.fields[ListView::Fields::Enabled].BindConstValue(true);

        modelsList->AddControl(new ObjectPlacementToolDetail::ModelsListView(p, params.processor, model, modelsList->ToWidgetCast()));
        modelsList->ForceUpdate();
        builder.AddRow("", modelsList);
    }
    {
        ControlDescriptorBuilder<SpinSlider::Fields> p;
        p[SpinSlider::Fields::SliderValue] = "brushSize";
        p[SpinSlider::Fields::SpinValue] = "brushRadius";
        builder.RegisterParam<SpinSlider>("Brush size (H)", p);
    }
    {
        ControlDescriptorBuilder<SpinSlider::Fields> p;
        p[SpinSlider::Fields::SliderValue] = "minDistanceBetweenObjects";
        p[SpinSlider::Fields::SpinValue] = "minDistanceBetweenObjects";
        builder.RegisterParam<SpinSlider>("Min distance between objects", p);
    }
    // rotation random controls
    {
        Widget* rotationWidget = new Widget(builder.GetWidget());
        QtHBoxLayout* layout = new QtHBoxLayout();
        rotationWidget->SetLayout(layout);

        layout->setMargin(0);
        layout->setSpacing(1);

        DoubleSpinBox::Params minParams(params.accessor, params.ui, params.wndKey);
        minParams.fields[DoubleSpinBox::Fields::Value] = "minRotation";
        minParams.fields[DoubleSpinBox::Fields::Range] = "minRotationRange";
        rotationWidget->AddControl(new DoubleSpinBox(minParams, params.processor, model, rotationWidget->ToWidgetCast()));

        DoubleSpinBox::Params maxParams(params.accessor, params.ui, params.wndKey);
        maxParams.fields[DoubleSpinBox::Fields::Value] = "maxRotation";
        maxParams.fields[DoubleSpinBox::Fields::Range] = "maxRotationRange";
        rotationWidget->AddControl(new DoubleSpinBox(maxParams, params.processor, model, rotationWidget->ToWidgetCast()));

        builder.AddRow("Rotation random", rotationWidget);
    }

    // scale random controls
    {
        Widget* scaleWidget = new Widget(builder.GetWidget());
        QtHBoxLayout* layout = new QtHBoxLayout();
        scaleWidget->SetLayout(layout);

        layout->setMargin(0);
        layout->setSpacing(1);

        DoubleSpinBox::Params minParams(params.accessor, params.ui, params.wndKey);
        minParams.fields[DoubleSpinBox::Fields::Value] = "minScale";
        minParams.fields[DoubleSpinBox::Fields::Range] = "minScaleRange";
        scaleWidget->AddControl(new DoubleSpinBox(minParams, params.processor, model, scaleWidget->ToWidgetCast()));

        DoubleSpinBox::Params maxParams(params.accessor, params.ui, params.wndKey);
        maxParams.fields[DoubleSpinBox::Fields::Value] = "maxScale";
        maxParams.fields[DoubleSpinBox::Fields::Range] = "maxScaleRange";
        scaleWidget->AddControl(new DoubleSpinBox(maxParams, params.processor, model, scaleWidget->ToWidgetCast()));

        builder.AddRow("Scale random", scaleWidget);
    }

    {
        Widget* landInjectWidget = new Widget(builder.GetWidget());
        QtHBoxLayout* layout = new QtHBoxLayout();
        landInjectWidget->SetLayout(layout);

        layout->setMargin(0);
        layout->setSpacing(1);

        DoubleSpinBox::Params minParams(params.accessor, params.ui, params.wndKey);
        minParams.fields[DoubleSpinBox::Fields::Value] = "minLandInject";
        minParams.fields[DoubleSpinBox::Fields::Range] = "minLandInjectRange";
        landInjectWidget->AddControl(new DoubleSpinBox(minParams, params.processor, model, landInjectWidget->ToWidgetCast()));

        DoubleSpinBox::Params maxParams(params.accessor, params.ui, params.wndKey);
        maxParams.fields[DoubleSpinBox::Fields::Value] = "maxLandInject";
        maxParams.fields[DoubleSpinBox::Fields::Range] = "maxLandInjectRange";
        landInjectWidget->AddControl(new DoubleSpinBox(maxParams, params.processor, model, landInjectWidget->ToWidgetCast()));

        builder.AddRow("Z offset", landInjectWidget);
    }

    weakRootWidget = builder.GetWidget();
    return builder.GetWidget();
}

DAVA::RefPtr<DAVA::Texture> ObjectPlacementTool::GetCursorTexture() const
{
    return cursorTexture;
}

DAVA::Color ObjectPlacementTool::GetCursorColor() const
{
    if (GetMode() == MODE_SPAWN)
    {
        return BaseLandscapeTool::GetCursorColor();
    }

    return DAVA::Color(1.0f, 0.2f, 0.2f, 1.0f);
}

DAVA::Vector2 ObjectPlacementTool::GetBrushSize() const
{
    return DAVA::Vector2(brushSize, brushSize);
}

DAVA::float32 ObjectPlacementTool::GetBrushRotation() const
{
    return 0.0;
}

void ObjectPlacementTool::Deactivate(DAVA::PropertiesItem& settings)
{
    OnSaveBrushImpl(settings);

    for (Model* m : models)
    {
        delete m;
    }
    models.clear();
}

bool ObjectPlacementTool::GetLandscapeProjection(const DAVA::Vector2& landscapePosition, DAVA::Vector3& result) const
{
    DAVA::Landscape* landscape = system->GetEditedLandscape();
    DAVA::float32 height = landscape->GetLandscapeHeight();

    DAVA::SceneCollisionSystem* collision = system->GetEditedScene()->GetSystem<DAVA::SceneCollisionSystem>();

    DAVA::Vector3 from = DAVA::Vector3(landscapePosition, height);
    DAVA::Vector3 to(from.x, from.y, -height);

    return collision->LandRayTest(from, to, result);
}

DAVA::Rect ObjectPlacementTool::MapLandscapeRect(const DAVA::Rect& r) const
{
    DAVA::Landscape* landscape = system->GetEditedLandscape();
    DAVA::float32 landscapeSize = 0.5f * landscape->GetLandscapeSize();

    DAVA::float32 y = DAVA::Saturate(1.0f - r.y - r.dy);
    DAVA::float32 dy = r.dy;
    if (y + dy > 1.0f)
    {
        dy = 1.0f - y;
    }
    DAVA::Rect normalizedRect(2.0f * r.x - 1.0f, 2.0f * y - 1.0f, r.dx, dy);
    normalizedRect.x *= landscapeSize;
    normalizedRect.y *= landscapeSize;
    normalizedRect.dx *= 2.0f * landscapeSize;
    normalizedRect.dy *= 2.0f * landscapeSize;

    return normalizedRect;
}

bool ObjectPlacementTool::IsValidToAdd(const DAVA::Vector3& position, DAVA::float32 objSphereRadius, DAVA::float32 objDistance) const
{
    DAVA::SceneCollisionSystem* collision = system->GetEditedScene()->GetSystem<DAVA::SceneCollisionSystem>();
    bool objIntersect = collision->OverlapSphereTest(position, objSphereRadius);
    bool inLayerIntersect = collision->OverlapSphereTest(position, objSphereRadius + objDistance, 1);
    return objIntersect == false && inLayerIntersect == false;
}

DAVA::Vector<DAVA::Entity*> ObjectPlacementTool::LookupObjects(const DAVA::Rect& r) const
{
    DAVA::SelectableGroup::CollectionType collection;

    DAVA::SceneCollisionSystem* collision = system->GetEditedScene()->GetSystem<DAVA::SceneCollisionSystem>();
    DAVA::Vector3 landscapePos;
    if (GetLandscapeProjection(r.GetCenter(), landscapePos))
    {
        DAVA::Vector2 size = r.GetSize();
        DAVA::float32 radius = 0.5f * std::max(size.x, size.y);
        collision->OverlapSphereTest(landscapePos, radius, collection, 1);
    }

    DAVA::Vector<DAVA::Entity*> objects;
    objects.reserve(collection.size());
    for (const DAVA::Selectable& obj : collection)
    {
        if (obj.CanBeCastedTo<DAVA::Entity>() == true)
        {
            objects.push_back(obj.Cast<DAVA::Entity>());
        }
    }

    return objects;
}

DAVA::float32 ObjectPlacementTool::GetBrushRadius() const
{
    return brushSize * system->GetEditedLandscape()->GetLandscapeSize();
}

void ObjectPlacementTool::SetBrushRadius(const DAVA::float32& brushRadius)
{
    brushSize = DAVA::Saturate(brushRadius / system->GetEditedLandscape()->GetLandscapeSize());
}

ObjectPlacementTool::eMode ObjectPlacementTool::GetMode() const
{
    bool isPressed = inputController->IsModifierPressed(DAVA::eModifierKeys::CONTROL);
    if (isPressed == false)
    {
        return mode;
    }

    if (mode == MODE_SPAWN)
    {
        return MODE_REMOVE;
    }

    return MODE_SPAWN;
}

void ObjectPlacementTool::SetMode(eMode newMode)
{
    mode = newMode;
}

DAVA::float32 ObjectPlacementTool::GetMinRotation() const
{
    return params.minRotationRandom * 360.0f;
}

void ObjectPlacementTool::SetMinRotation(const DAVA::float32& v)
{
    params.minRotationRandom = v / 360.0f;
    params.maxRotationRandom = std::max(params.maxRotationRandom, params.minRotationRandom);
}

DAVA::float32 ObjectPlacementTool::GetMaxRotation() const
{
    return params.maxRotationRandom * 360.0f;
}

void ObjectPlacementTool::SetMaxRotation(const DAVA::float32& v)
{
    params.maxRotationRandom = v / 360.0f;
    params.minRotationRandom = std::min(params.maxRotationRandom, params.minRotationRandom);
}

DAVA::float32 ObjectPlacementTool::GetMinScale() const
{
    return params.minScaleRandom;
}

void ObjectPlacementTool::SetMinScale(const DAVA::float32& v)
{
    params.minScaleRandom = std::max(v, 0.01f);
    params.maxScaleRandom = std::max(params.minScaleRandom, params.maxScaleRandom);
}

DAVA::float32 ObjectPlacementTool::GetMaxScale() const
{
    return params.maxScaleRandom;
}

void ObjectPlacementTool::SetMaxScale(const DAVA::float32& v)
{
    params.maxScaleRandom = v;
    params.minScaleRandom = std::min(params.minScaleRandom, params.maxScaleRandom);
}

DAVA::float32 ObjectPlacementTool::GetMinLandscapeInjection() const
{
    return params.minLandscapeInjection;
}

void ObjectPlacementTool::SetMinLandscapeInjection(const DAVA::float32& v)
{
    params.minLandscapeInjection = v;
    params.maxLandscapeInjection = std::max(params.minLandscapeInjection, params.maxLandscapeInjection);
}

DAVA::float32 ObjectPlacementTool::GetMaxLandscapeInjection() const
{
    return params.maxLandscapeInjection;
}

void ObjectPlacementTool::SetMaxLandscapeInjection(const DAVA::float32& v)
{
    params.maxLandscapeInjection = v;
    params.minLandscapeInjection = std::min(params.maxLandscapeInjection, params.minLandscapeInjection);
}

DAVA::float32 ObjectPlacementTool::GetMinDistanceBetweenObjects() const
{
    return params.minDistanceBetweenObjects;
}

void ObjectPlacementTool::SetMinDistanceBetweenObjects(const DAVA::float32& v)
{
    params.minDistanceBetweenObjects = std::max(v, 0.0f);
}

void ObjectPlacementTool::OnRefreshModels()
{
    auto iter = models.begin();
    while (iter != models.end())
    {
        if ((*iter)->path.empty() == true)
        {
            delete (*iter);
            iter = models.erase(iter);
        }
        else
        {
            ++iter;
        }
    }

    Model* m = new Model();
    m->tool = this;
    models.push_back(m);
}

void ObjectPlacementTool::OnAddLayer()
{
    QString newLayerName = QInputDialog::getText(weakRootWidget.data(), "Type new layer name", "");
    if (newLayerName.isEmpty() == false)
    {
        DAVA::FastName layerName = DAVA::FastName(newLayerName.toStdString());
        DAVA::RefPtr<DAVA::Entity> layerEntity(new DAVA::Entity());
        layerEntity->SetName(layerName);

        MassObjectCreationLayer* layerComponent = new MassObjectCreationLayer();
        layerComponent->SetName(layerName);
        layerEntity->AddComponent(layerComponent);

        DAVA::SceneEditor2* scene = static_cast<DAVA::SceneEditor2*>(system->GetEditedScene());
        scene->Exec(std::make_unique<DAVA::EntityAddCommand>(layerEntity.Get(), scene));
        scene->GetSystem<MassObjectCreationSystem>()->SetEditedLayer(layerEntity.Get());
    }
}

void ObjectPlacementTool::OnLoadBrush()
{
    DAVA::FileDialogParams params;
    params.filters = "Brush params (*.mcb)";
    params.title = "Open brush file";
    QString path = DAVA::Deprecated::GetUI()->GetOpenFileName(DAVA::mainWindowKey, params);
    if (path.isEmpty() == false)
    {
        QFileInfo info(path);
        DAVA::String fileName = info.fileName().toStdString();
        DAVA::String directory = info.absolutePath().toStdString();
        DAVA::PropertiesHolder holder(fileName, directory);
        OnLoadBrushImpl(holder.CreateSubHolder("root"));
    }
}

void ObjectPlacementTool::OnLoadBrushImpl(const DAVA::PropertiesItem& settings)
{
    for (Model* m : models)
    {
        delete m;
    }
    models.clear();

    DAVA::ProjectManagerData* projectData = accessor->GetGlobalContext()->GetData<DAVA::ProjectManagerData>();

    DAVA::PropertiesItem h = settings.CreateSubHolder("ObjectPlacementTool");
    DAVA::PropertiesItem modelsNode = h.CreateSubHolder("models");
    DAVA::int32 modelsCount = modelsNode.Get<DAVA::int32>("count", 0);
    for (DAVA::int32 i = 0; i < modelsCount; ++i)
    {
        DAVA::String path(modelsNode.Get<DAVA::String>(DAVA::Format("model_%d", i)));
        path = projectData->GetDataSourcePath().GetAbsolutePathname() + path;
        Model* m = new Model();
        m->tool = this;
        m->SetPath(path);
        models.push_back(m);
    }

    DAVA::String layerName = h.Get<DAVA::String>("layerName", "");
    TrySetCurrentLayer(layerName);
    brushSize = h.Get<DAVA::float32>("brushSize", brushSize);
    mode = h.Get<eMode>("mode", mode);

    params.minRotationRandom = h.Get<DAVA::float32>("minRotationRandom", params.minRotationRandom);
    params.maxRotationRandom = h.Get<DAVA::float32>("maxRotationRandom", params.maxRotationRandom);
    params.minScaleRandom = h.Get<DAVA::float32>("minScaleRandom", params.minScaleRandom);
    params.maxScaleRandom = h.Get<DAVA::float32>("maxScaleRandom", params.maxScaleRandom);
    params.minLandscapeInjection = h.Get<DAVA::float32>("minLandscapeInjection", params.minLandscapeInjection);
    params.maxLandscapeInjection = h.Get<DAVA::float32>("maxLandscapeInjection", params.maxLandscapeInjection);
    params.minDistanceBetweenObjects = h.Get<DAVA::float32>("minDistanceBetweenObjects", params.minDistanceBetweenObjects);

    SetMinRotation(params.minRotationRandom * 360.0f);
    SetMaxRotation(params.maxRotationRandom * 360.0f);
    SetMinScale(params.minScaleRandom);
    SetMaxScale(params.maxScaleRandom);
    SetMinLandscapeInjection(params.minLandscapeInjection);
    SetMaxLandscapeInjection(params.maxLandscapeInjection);

    OnRefreshModels();
}

void ObjectPlacementTool::OnSaveBrush()
{
    DAVA::FileDialogParams params;
    params.filters = "Brush params (*.mcb)";
    params.title = "Open brush file";
    QString path = DAVA::Deprecated::GetUI()->GetSaveFileName(DAVA::mainWindowKey, params);
    if (path.isEmpty() == false)
    {
        QFileInfo info(path);
        DAVA::String fileName = info.fileName().toStdString();
        DAVA::String directory = info.absolutePath().toStdString();
        DAVA::PropertiesHolder holder(fileName, directory);
        DAVA::PropertiesItem h = holder.CreateSubHolder("root");
        OnSaveBrushImpl(h);
    }
}

void ObjectPlacementTool::OnSaveBrushImpl(DAVA::PropertiesItem& settings)
{
    DAVA::ProjectManagerData* projectData = accessor->GetGlobalContext()->GetData<DAVA::ProjectManagerData>();
    DAVA::PropertiesItem h = settings.CreateSubHolder("ObjectPlacementTool");
    DAVA::PropertiesItem modelsNode = h.CreateSubHolder("models");

    DVASSERT(models.empty() == false);
    modelsNode.Set("count", static_cast<DAVA::int32>(models.size() - 1));
    for (DAVA::int32 i = 0; i < models.size() - 1; ++i)
    {
        DAVA::String path = DAVA::FilePath(models[i]->path).GetRelativePathname(projectData->GetDataSourcePath().GetAbsolutePathname());
        modelsNode.Set(DAVA::Format("model_%d", i), path);
    }

    DAVA::String layerName;
    DAVA::Entity* currentLayer = GetCurrentLayer();
    for (auto layerNode : GetLayers().values)
    {
        if (layerNode.first == currentLayer)
        {
            layerName = layerNode.second;
            break;
        }
    }
    h.Set("layerName", layerName);
    h.Set("brushSize", brushSize);
    h.Set("mode", mode);

    h.Set("minRotationRandom", params.minRotationRandom);
    h.Set("maxRotationRandom", params.maxRotationRandom);
    h.Set("minScaleRandom", params.minScaleRandom);
    h.Set("maxScaleRandom", params.maxScaleRandom);
    h.Set("maxLandscapeInjection", params.maxLandscapeInjection);
    h.Set("minLandscapeInjection", params.minLandscapeInjection);
    h.Set("maxLandscapeInjection", params.maxLandscapeInjection);
    h.Set("minDistanceBetweenObjects", params.minDistanceBetweenObjects);
}

const DAVA::ReflectedPairsVector<DAVA::Entity*, DAVA::String>& ObjectPlacementTool::GetLayers() const
{
    MassObjectCreationSystem* massObjSystem = system->GetEditedScene()->GetSystem<MassObjectCreationSystem>();

    return massObjSystem->GetLayers();
}

DAVA::Entity* ObjectPlacementTool::GetCurrentLayer() const
{
    MassObjectCreationSystem* massObjSystem = system->GetEditedScene()->GetSystem<MassObjectCreationSystem>();
    return massObjSystem->GetEditedLayer();
}

void ObjectPlacementTool::SetCurrentLayer(DAVA::Entity* layer)
{
    ResetFilterData();
    MassObjectCreationSystem* massObjSystem = system->GetEditedScene()->GetSystem<MassObjectCreationSystem>();
    massObjSystem->SetEditedLayer(layer);
    ApplyFilterData();
}

void ObjectPlacementTool::TrySetCurrentLayer(const DAVA::String& layerName)
{
    for (auto layerNode : GetLayers().values)
    {
        if (layerNode.second == layerName)
        {
            SetCurrentLayer(layerNode.first);
            break;
        }
    }
}

void ObjectPlacementTool::ResetFilterData()
{
    DAVA::Scene* scene = system->GetEditedScene();
    MassObjectCreationSystem* massObjSystem = scene->GetSystem<MassObjectCreationSystem>();
    DAVA::SceneCollisionSystem* collisionSystem = scene->GetSystem<DAVA::SceneCollisionSystem>();

    const DAVA::Set<DAVA::Entity*>& objects = massObjSystem->GetEntitiesInLayer(massObjSystem->GetEditedLayer());
    for (DAVA::Entity* e : objects)
    {
        DAVA::QueryObjectDataComponent* queryData = e->GetComponent<DAVA::QueryObjectDataComponent>();
        DVASSERT(queryData != nullptr);
        queryData->SetData(0);
    }
}

void ObjectPlacementTool::ApplyFilterData()
{
    DAVA::Set<DAVA::String> modelPathes;
    for (Model* m : models)
    {
        modelPathes.insert(m->path);
    }

    DAVA::Scene* scene = system->GetEditedScene();

    MassObjectCreationSystem* massObjSystem = scene->GetSystem<MassObjectCreationSystem>();
    DAVA::SceneCollisionSystem* collisionSystem = scene->GetSystem<DAVA::SceneCollisionSystem>();

    const DAVA::Set<DAVA::Entity*>& objects = massObjSystem->GetEntitiesInLayer(massObjSystem->GetEditedLayer());
    for (DAVA::Entity* e : objects)
    {
        MassCreatedObjectComponent* component = e->GetComponent<MassCreatedObjectComponent>();
        DAVA::uint32 filterData = 0;
        if (modelPathes.count(component->GetSourceModelPath().GetAbsolutePathname()) > 0)
        {
            filterData = 1;
        }

        DAVA::QueryObjectDataComponent* queryData = e->GetComponent<DAVA::QueryObjectDataComponent>();
        DVASSERT(queryData != nullptr);
        queryData->SetData(filterData);
    }
}

DAVA_VIRTUAL_REFLECTION_IMPL(ObjectPlacementTool)
{
    DAVA::ReflectionRegistrator<ObjectPlacementTool>::Begin()
    .ConstructorByPointer<DAVA::LandscapeEditorSystemV2*>()
    .Method("onLoadBrush", &ObjectPlacementTool::OnLoadBrush)
    .Method("onSaveBrush", &ObjectPlacementTool::OnSaveBrush)
    .Field("layers", &ObjectPlacementTool::GetLayers, nullptr)
    .Field("currentLayer", &ObjectPlacementTool::GetCurrentLayer, &ObjectPlacementTool::SetCurrentLayer)
    .Method("onAddLayer", &ObjectPlacementTool::OnAddLayer)
    .Field("models", &ObjectPlacementTool::models)
    .Field("mode", &ObjectPlacementTool::GetMode, &ObjectPlacementTool::SetMode)[DAVA::M::EnumT<ObjectPlacementTool::eMode>()]
    .Field("brushSize", &ObjectPlacementTool::brushSize)[DAVA::M::Range(0.0, 1.0, 0.001)]
    .Field("brushRadius", &ObjectPlacementTool::GetBrushRadius, &ObjectPlacementTool::SetBrushRadius)
    .Field("minRotation", &ObjectPlacementTool::GetMinRotation, &ObjectPlacementTool::SetMinRotation)
    .Field("maxRotation", &ObjectPlacementTool::GetMaxRotation, &ObjectPlacementTool::SetMaxRotation)
    .Field("minScale", &ObjectPlacementTool::GetMinScale, &ObjectPlacementTool::SetMinScale)
    .Field("maxScale", &ObjectPlacementTool::GetMaxScale, &ObjectPlacementTool::SetMaxScale)
    .Field("minLandInject", &ObjectPlacementTool::GetMinLandscapeInjection, &ObjectPlacementTool::SetMinLandscapeInjection)
    .Field("maxLandInject", &ObjectPlacementTool::GetMaxLandscapeInjection, &ObjectPlacementTool::SetMaxLandscapeInjection)
    .Field("minDistanceBetweenObjects", &ObjectPlacementTool::GetMinDistanceBetweenObjects, &ObjectPlacementTool::SetMinDistanceBetweenObjects)[DAVA::M::Range(0.0, 100.0, 0.01f)]
    .End();
}

const DAVA::String& ObjectPlacementTool::Model::GetName() const
{
    return name;
}

void ObjectPlacementTool::Model::SetPath(const DAVA::String& path_)
{
    name = DAVA::String();
    path = path_;
    bbox = DAVA::AABBox3();
    DAVA::FilePath dataSource = DAVA::ProjectManagerData::GetDataSourcePath(path);

    if (this->path.empty() == false && dataSource.IsEmpty() == false)
    {
        DVASSERT(tool != nullptr);
        name = DAVA::String("~res:/") + DAVA::FilePath(path).GetRelativePathname(dataSource);
        model.Set(tool->system->GetEditedScene()->GetSystem<DAVA::StructureSystem>()->Load(path));

        if (model.Get() == nullptr)
        {
            this->path = DAVA::String();
        }
        else
        {
            DAVA::RenderComponent* component = model->GetComponent<DAVA::RenderComponent>();
            if (component != nullptr)
            {
                bbox = component->GetRenderObject()->GetBoundingBox();
            }
        }
    }
    else
    {
        model = DAVA::RefPtr<DAVA::Entity>();
    }

    tool->OnRefreshModels();
}

DAVA_REFLECTION_IMPL(ObjectPlacementTool::Model)
{
    DAVA::ReflectionRegistrator<ObjectPlacementTool::Model>::Begin()
    .Field("path", &ObjectPlacementTool::Model::GetName, &ObjectPlacementTool::Model::SetPath)
    .End();
}