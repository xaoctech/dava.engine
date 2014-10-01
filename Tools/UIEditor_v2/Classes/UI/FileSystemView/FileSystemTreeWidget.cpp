//
//  FileSystemTreeWidget.cpp
//  UIEditor
//
//  Created by Dmitry Belsky on 9.9.14.
//
//
#include "FileSystemTreeWidget.h"

#include "ui_FileSystemTreeWidget.h"

FileSystemTreeWidget::FileSystemTreeWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::FileSystemTreeWidget)
    , model(NULL)
{
    model = new QFileSystemModel(this);
    ui->setupUi(this);
    
    model->setFilter(QDir::Files | QDir::AllDirs | QDir::NoDotAndDotDot);
    QStringList filters;
    filters << "*.yaml";
    model->setNameFilters(filters);
    model->setNameFilterDisables(false);
    
    connect(ui->treeView, SIGNAL(doubleClicked (const QModelIndex &)), this, SLOT(onDoubleClicked(const QModelIndex &)));
}

FileSystemTreeWidget::~FileSystemTreeWidget()
{
    delete ui;
    ui = NULL;
}

void FileSystemTreeWidget::SetProjectDir(const QString &path)
{
    QDir dir(path);
    dir.cdUp();
    QString p = dir.path() + "/Data/UI";
    model->setRootPath(p);
    ui->treeView->setModel(model);
    ui->treeView->setRootIndex(model->setRootPath(p));
    ui->treeView->hideColumn(1);
    ui->treeView->hideColumn(2);
    ui->treeView->hideColumn(3);
}

void FileSystemTreeWidget::onDoubleClicked(const QModelIndex &index)
{
    emit OpenPackageFile(model->filePath(index));
}
