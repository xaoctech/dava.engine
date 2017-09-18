#include "IssueNavigatorWidget.h"

#include "ui_IssueNavigatorWidget.h"

#include <TArc/Core/FieldBinder.h>

#include <QKeyEvent>

IssueNavigatorWidget::IssueNavigatorWidget(DAVA::TArc::ContextAccessor* accessor, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::IssueNavigatorWidget())
{
    ui->setupUi(this);

    model = new QStandardItemModel();
    model->setHorizontalHeaderLabels(QStringList() << "Error"
                                                   << "Path To Control"
                                                   << "Package Path"
                                                   << "Property Name");

    ui->treeView->setModel(model);
    connect(ui->treeView, &QTreeView::activated, this, &IssueNavigatorWidget::OnActivated);
    ui->treeView->installEventFilter(this); // Enter key doesn't invoke activated signal on mac.
}

IssueNavigatorWidget::~IssueNavigatorWidget()
{
}

void IssueNavigatorWidget::AddIssue(const Issue& issue)
{
    QList<QStandardItem*> items;

    items << new QStandardItem(QString::fromStdString(issue.message));
    items << new QStandardItem(QString::fromStdString(issue.pathToControl));
    items << new QStandardItem(QString::fromStdString(issue.packagePath));
    items << new QStandardItem(QString::fromStdString(issue.propertyName));

    for (QStandardItem* item : items)
    {
        item->setEditable(false);
        item->setData(issue.sectionId, ISSUE_SECTION_DATA);
        item->setData(issue.issueId, ISSUE_ID_DATA);
        item->setData(QString::fromStdString(issue.packagePath), PACKAGE_DATA);
        item->setData(QString::fromStdString(issue.pathToControl), CONTROL_DATA);
    }

    model->appendRow(items);
}

void IssueNavigatorWidget::ChangeMessage(DAVA::int32 sectionId_, DAVA::int32 issueId_, const DAVA::String& message)
{
    int row = GetIssueRow(issueId_, sectionId_);
    if (row > -1)
    {
        QStandardItem* item = model->item(row, 0);
        item->setText(QString::fromStdString(message));
    }
}

void IssueNavigatorWidget::ChangePathToControl(DAVA::int32 sectionId_, DAVA::int32 issueId_, const DAVA::String& pathToControlMsg)
{
    int row = GetIssueRow(issueId_, sectionId_);
    if (row > -1)
    {
        QString path = QString::fromStdString(pathToControlMsg);
        QStandardItem* item = model->item(row, 1);
        item->setText(path);

        for (int column = 0; column < model->columnCount(); ++column)
        {
            QStandardItem* item = model->item(row, column);
            item->setData(path, CONTROL_DATA);
        }
    }
}

void IssueNavigatorWidget::RemoveIssue(DAVA::int32 sectionId_, DAVA::int32 issueId_)
{
    for (int row = 0; row < model->rowCount();)
    {
        QStandardItem* item = model->item(row);
        DAVA::int32 sectionId = item->data(ISSUE_SECTION_DATA).toInt();
        DAVA::int32 issueId = item->data(ISSUE_ID_DATA).toInt();
        if (issueId == issueId_ && sectionId == sectionId_)
        {
            model->removeRows(row, 1);
        }
        else
        {
            row++;
        }
    }
}

void IssueNavigatorWidget::OnActivated(const QModelIndex& index)
{
    QString path = index.data(PACKAGE_DATA).toString();
    if (index.data(CONTROL_DATA).isValid())
    {
        QString control = index.data(CONTROL_DATA).toString();
        emit JumpToControl(DAVA::FilePath(path.toStdString()), control.toStdString());
    }
    else
    {
        emit JumpToPackage(DAVA::FilePath(path.toStdString()));
    }
}

int IssueNavigatorWidget::GetIssueRow(const DAVA::int32 issueId_, const DAVA::int32 sectionId_)
{
    for (int row = 0; row < model->rowCount(); row++)
    {
        QStandardItem* item = model->item(row, 0);
        DAVA::int32 sectionId = item->data(ISSUE_SECTION_DATA).toInt();
        DAVA::int32 issueId = item->data(ISSUE_ID_DATA).toInt();
        if (issueId_ == issueId && sectionId_ == sectionId)
        {
            return row;
        }
    }
    return -1;
}

bool IssueNavigatorWidget::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter)
        {
            if (ui->treeView->selectionModel()->currentIndex().isValid())
            {
                OnActivated(ui->treeView->selectionModel()->currentIndex());
                return true;
            }
        }
    }

    return QWidget::eventFilter(obj, event);
}
