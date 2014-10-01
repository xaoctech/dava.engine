//
//  FileSystemTreeWidget.h
//  UIEditor
//
//  Created by Dmitry Belsky on 9.9.14.
//
//

#ifndef __UI_EDITOR_FILE_SYSTEM_TREE_WIDGET_H__
#define __UI_EDITOR_FILE_SYSTEM_TREE_WIDGET_H__

#include <QWidget>
#include <QFileSystemModel>

namespace Ui {
    class FileSystemTreeWidget;
}

class QFileSystemModel;

class FileSystemTreeWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit FileSystemTreeWidget(QWidget *parent = 0);
    virtual ~FileSystemTreeWidget();

    void SetProjectDir(const QString &path);
signals:
    void OpenPackageFile(const QString &path);

private slots:
    void onDoubleClicked(const QModelIndex &index);
    
private:
    Ui::FileSystemTreeWidget *ui;
    QFileSystemModel *model;
};

#endif // __UI_EDITOR_FILE_SYSTEM_TREE_WIDGET_H__
