#include "REPlatform/Global/REFileOperationsInterface.h"

namespace DAVA
{
RESimpleFileOperation::RESimpleFileOperation(const QIcon& icon_, const QString& name_, eOperationType type_,
                                             const QString& filter_, const TCallback& callback_)
    : icon(icon_)
    , name(name_)
    , type(type_)
    , filter(filter_)
    , callback(callback_)
{
}

QIcon RESimpleFileOperation::GetIcon() const
{
    return icon;
}

QString RESimpleFileOperation::GetName() const
{
    return name;
}

REFileOperation::eOperationType RESimpleFileOperation::GetType() const
{
    return type;
}

QString RESimpleFileOperation::GetTargetFileFilter() const
{
    return filter;
}

void RESimpleFileOperation::Apply(const DAVA::FilePath& filePath) const
{
    callback(filePath);
}

} // namespace DAVA
