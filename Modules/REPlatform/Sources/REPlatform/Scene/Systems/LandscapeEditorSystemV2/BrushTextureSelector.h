#pragma once

#include <TArc/Controls/ControlProxy.h>
#include <TArc/Qt/QtIcon.h>
#include <TArc/Qt/QtString.h>
#include <TArc/Utils/QtConnections.h>

#include <Base/RefPtr.h>
#include <Base/BaseTypes.h>
#include <Render/Image/Image.h>
#include <Reflection/Reflection.h>

#include <QWidget>
#include <QFileSystemWatcher>

namespace DAVA
{
class BrushTextureSelector : public ControlProxyImpl<QWidget>
{
    using TBase = ControlProxyImpl<QWidget>;

public:
    enum class Fields
    {
        Value, //String
        FieldCount
    };

    DECLARE_CONTROL_PARAMS(Fields);

    BrushTextureSelector(const Params& params, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent = nullptr);

    void ForceUpdate() override;
    void TearDown() override;

    struct BrushTexture
    {
        RefPtr<Image> brush;
        QIcon preview;
        String imagePath;
        String imageName;

        void SetImagePath(const String& imagePath);

        bool operator==(const BrushTexture& other) const
        {
            return imagePath == other.imagePath;
        }
    };

protected:
    void UpdateControl(const ControlDescriptor& changedFields) override;

    void OnDirectoryChanged(const QString& dir);

    int32 GetBrush() const;
    void SetBrush(int32 brushIndex);

    Vector<BrushTexture> textures;

    QtConnections connections;
    std::unique_ptr<QFileSystemWatcher> watcher;
    Vector<ControlProxy*> children;

    static bool castRegistered;

    DAVA_REFLECTION(BrushTextureSelector);
};
} // namespace DAVA
