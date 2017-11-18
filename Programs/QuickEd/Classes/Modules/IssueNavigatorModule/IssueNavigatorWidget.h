#pragma once

#include <Base/BaseTypes.h>
#include <QWidget>
#include <QStandardItemModel>

#include <FileSystem/FilePath.h>

namespace DAVA
{
class Any;
namespace TArc
{
class ContextAccessor;
class FieldBinder;
}
}

namespace Ui
{
class IssueNavigatorWidget;
}

struct Issue
{
    DAVA::int32 sectionId;
    DAVA::int32 issueId;
    DAVA::String message;
    DAVA::String packagePath;
    DAVA::String pathToControl;
    DAVA::String propertyName;
};

class IssueNavigatorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit IssueNavigatorWidget(DAVA::TArc::ContextAccessor* accessor, QWidget* parent = nullptr);
    ~IssueNavigatorWidget() override;

    void AddIssue(const Issue& issue);
    void ChangeMessage(DAVA::int32 sectionId, DAVA::int32 id, const DAVA::String& message);
    void ChangePathToControl(DAVA::int32 sectionId, DAVA::int32 id, const DAVA::String& pathToControlMsg);
    void RemoveIssue(DAVA::int32 sectionId, DAVA::int32 issueId);

signals:
    void JumpToControl(const DAVA::FilePath& packagePath, const DAVA::String& controlName);
    void JumpToPackage(const DAVA::FilePath& packagePath);

private slots:
    void OnActivated(const QModelIndex& index);

private:
    int GetIssueRow(const DAVA::int32 issueId, const DAVA::int32 sectionId);
    bool eventFilter(QObject* obj, QEvent* event) override;

    std::unique_ptr<Ui::IssueNavigatorWidget> ui;
    QStandardItemModel* model = nullptr;

    enum
    {
        PACKAGE_DATA = Qt::UserRole + 1,
        CONTROL_DATA,
        ISSUE_SECTION_DATA,
        ISSUE_ID_DATA
    };
};
