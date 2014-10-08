//
//  FileSystemTreeWidget.cpp
//  UIEditor
//
//  Created by Dmitry Belsky on 9.9.14.
//
//
#include "FileSystemDockWidget.h"

#include "ui_FileSystemDockWidget.h"

FileSystemDockWidget::FileSystemDockWidget(QWidget *parent)
    : QDockWidget(parent)
    , ui(new Ui::FileSystemDockWidget())
    , model(NULL)
{
    model = new QFileSystemModel(this);
    proxyModel = new FilteredFileSystemModel(this);
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    ui->setupUi(this);
    
    model->setFilter(QDir::Files | QDir::AllDirs | QDir::NoDotAndDotDot);
    QStringList filters;
    filters << "*.yaml";
    model->setNameFilters(filters);
    model->setNameFilterDisables(false);

    proxyModel->setSourceModel(model);
    proxyModel->setFilterKeyColumn(0);

    connect(ui->treeView, SIGNAL(doubleClicked (const QModelIndex &)), this, SLOT(onDoubleClicked(const QModelIndex &)));
    //connect(ui->filterLine, SIGNAL(textChanged(const QString &)), this, SLOT(filterTextChanged(const QString &)));
    connect(ui->filterLine, SIGNAL(textChanged(const QString &)), proxyModel, SLOT(setFilterFixedString(const QString &)));
    
}

FileSystemDockWidget::~FileSystemDockWidget()
{
    delete ui;
    ui = NULL;
}

void FileSystemDockWidget::SetProjectDir(const QString &path)
{
    QDir dir(path);
    dir.cdUp();
    QString p = dir.path() + "/Data/UI";
    QModelIndex rootIndex = model->setRootPath(p);
    ui->treeView->setModel(proxyModel);
    ui->treeView->setRootIndex(proxyModel->mapFromSource(rootIndex));
    ui->treeView->hideColumn(1);
    ui->treeView->hideColumn(2);
    ui->treeView->hideColumn(3);
}

void FileSystemDockWidget::onDoubleClicked(const QModelIndex &index)
{
    emit OpenPackageFile(model->filePath(proxyModel->mapToSource(index)));
}

FilteredFileSystemModel::FilteredFileSystemModel( QObject *parent /*= NULL*/ )
    : QSortFilterProxyModel(parent)
{}

FilteredFileSystemModel::~FilteredFileSystemModel()
{}

bool FilteredFileSystemModel::filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const
{
    QFileSystemModel *sm = qobject_cast<QFileSystemModel*>(sourceModel());

    QModelIndex rootIndex = sm->index(sm->rootPath());
    QModelIndex testIndex = sourceParent;
    while (testIndex.isValid() && testIndex != rootIndex)
    {
        testIndex = testIndex.parent();
    }
    if (testIndex.isValid())
    {
        return filterAcceptsRowSelf(sourceRow, sourceParent);
    }
    return true;
}

bool FilteredFileSystemModel::filterAcceptsRowSelf( int sourceRow, const QModelIndex &sourceParent ) const
{
    if (QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent))
    {
        return true;
    }
    return hasAcceptedChildren(sourceRow, sourceParent);
}

bool FilteredFileSystemModel::hasAcceptedChildren( int sourceRow, const QModelIndex &sourceParent ) const
{
    QModelIndex item = sourceModel()->index(sourceRow, 0, sourceParent);

    while(sourceModel()->canFetchMore(item))
    {
        sourceModel()->fetchMore(item);
    }

    int rowCount = sourceModel()->rowCount(item);

    if (rowCount == 0)
    {
        return false;
    }

    for (int row = 0; row < rowCount; ++row)
    {
        if (filterAcceptsRowSelf(row, item))
            return true;
    }

    return false;
}
