#pragma once

#include <TArc/Qt/QtIcon.h>
#include <TArc/Qt/QtString.h>

#include <FileSystem/FilePath.h>
#include <Functional/Function.h>

namespace DAVA
{
class REFileOperation
{
public:
    enum class eOperationType
    {
        IMPORT,
        EXPORT
    };
    virtual QIcon GetIcon() const = 0;
    virtual QString GetName() const = 0;
    virtual eOperationType GetType() const = 0;
    virtual QString GetTargetFileFilter() const = 0; // In QFileDialog filters format
    virtual void Apply(const DAVA::FilePath& filePath) const = 0;
};

class RESimpleFileOperation : public REFileOperation
{
public:
    using TCallback = DAVA::Function<void(const DAVA::FilePath&)>;
    RESimpleFileOperation(const QIcon& icon, const QString& name, eOperationType type, const QString& filter, const TCallback& callback);

    QIcon GetIcon() const override;
    QString GetName() const override;
    eOperationType GetType() const override;
    QString GetTargetFileFilter() const override;
    void Apply(const DAVA::FilePath& filePath) const override;

private:
    QIcon icon;
    QString name;
    eOperationType type;
    QString filter;
    TCallback callback;
};

class REFileOperationsInterface
{
public:
    virtual void RegisterFileOperation(std::shared_ptr<REFileOperation> fileOperation) = 0;
    virtual void UnregisterFileOperation(std::shared_ptr<REFileOperation> fileOperation) = 0;
};

} // namespace DAVA
