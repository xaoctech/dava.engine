#pragma once

namespace DAVA
{
namespace TArc
{
class QtAction;
}
}

namespace Interfaces
{
class PackageActionsInterface
{
public:
    virtual DAVA::TArc::QtAction* GetCutAction() = 0;
    virtual DAVA::TArc::QtAction* GetCopyAction() = 0;
    virtual DAVA::TArc::QtAction* GetPasteAction() = 0;
    virtual DAVA::TArc::QtAction* GetDuplicateAction() = 0;
    virtual DAVA::TArc::QtAction* GetDeleteAction() = 0;
    virtual DAVA::TArc::QtAction* GetJumpToPrototypeAction() = 0;
    virtual DAVA::TArc::QtAction* GetFindPrototypeInstancesAction() = 0;
};
}