/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/



#include "LibraryFileSystemModel.h"
#include "LibraryFileSystemFilteringModel.h"

#include "Main/QtUtils.h"

#include <QFileSystemModel>
#include <QProcess>


LibraryFileSystemModel::LibraryFileSystemModel(const QString &modelName)
    : LibraryBaseModel(modelName)
{
    treeModel = new QFileSystemModel(this);
    listModel = new QFileSystemModel(this);
    filteringModel = new LibraryFileSystemFilteringModel(this);
	filteringModel->SetModel(listModel);

	((QFileSystemModel *)treeModel)->setFilter(QDir::Dirs|QDir::Drives|QDir::NoDotAndDotDot|QDir::AllDirs);
	((QFileSystemModel *)listModel)->setFilter(QDir::Files|QDir::NoDotAndDotDot);
}

void LibraryFileSystemModel::TreeItemSelected(const QItemSelection & selection)
{
    const QModelIndex index = selection.indexes().first();

    QFileInfo fileInfo = ((QFileSystemModel *)treeModel)->fileInfo(index);
	listRootPath = fileInfo.filePath();

    ((QFileSystemModel *)listModel)->setRootPath(listRootPath);
	filteringModel->invalidate();
	((LibraryFileSystemFilteringModel *)filteringModel)->SetSourceRoot(((QFileSystemModel *)listModel)->index(listRootPath));
    
    TreeFileSelected(fileInfo);
}

void LibraryFileSystemModel::ListItemSelected(const QItemSelection & selection)
{
	QItemSelection listSelection = filteringModel->mapSelectionToSource(selection);

    const QModelIndex index = listSelection.indexes().first();
	if(index.isValid())
    {
        QFileInfo fileInfo = ((QFileSystemModel *)listModel)->fileInfo(index);
        TreeFileSelected(fileInfo);
    }
}


void LibraryFileSystemModel::SetProjectPath(const QString & path)
{
    treeRootPath = path + "/DataSource/3d/";
    listRootPath = treeRootPath;

    QDir rootDir(treeRootPath);
    
    ((QFileSystemModel *)treeModel)->setRootPath(rootDir.canonicalPath());
    ((QFileSystemModel *)listModel)->setRootPath(rootDir.canonicalPath());
	((LibraryFileSystemFilteringModel *)filteringModel)->SetSourceRoot(((QFileSystemModel *)listModel)->index(listRootPath));
}

const QModelIndex LibraryFileSystemModel::GetTreeRootIndex() const
{
    return ((QFileSystemModel *)treeModel)->index(treeRootPath);
}

const QModelIndex LibraryFileSystemModel::GetListRootIndex() const
{
	const QModelIndex index = ((QFileSystemModel *)listModel)->index(listRootPath);
    return filteringModel->mapFromSource(index);
}

bool LibraryFileSystemModel::PrepareTreeContextMenu(QMenu &contextMenu, const QModelIndex &index) const
{
    if(index.isValid())
    {
        QFileInfo fileInfo = ((QFileSystemModel *)treeModel)->fileInfo(index);
        return PrepareTreeContextMenuInternal(contextMenu, fileInfo);
    }

    return false;
}

bool LibraryFileSystemModel::PrepareListContextMenu(QMenu &contextMenu, const QModelIndex &index) const
{
	const QModelIndex listIndex = filteringModel->mapToSource(index);
    QFileInfo fileInfo = ((QFileSystemModel *)listModel)->fileInfo(listIndex);
    if(fileInfo.isFile())
    {
        QVariant fileInfoAsVariant = QVariant::fromValue<QFileInfo>(fileInfo);
        PrepareListContextMenuInternal(contextMenu, fileInfo);

        contextMenu.addSeparator();
        QAction * actionRevealAt = contextMenu.addAction("Reveal at folder", this, SLOT(OnRevealAtFolder()));
        actionRevealAt->setData(fileInfoAsVariant);

        return true;
    }

    return false;
}

void LibraryFileSystemModel::OnRevealAtFolder()
{
    QVariant indexAsVariant = ((QAction *)sender())->data();
    const QFileInfo fileInfo = indexAsVariant.value<QFileInfo>();
    
#if defined (Q_WS_MAC)
    QStringList args;
    args << "-e";
    args << "tell application \"Finder\"";
    args << "-e";
    args << "activate";
    args << "-e";
    args << "select POSIX file \""+fileInfo.absoluteFilePath()+"\"";
    args << "-e";
    args << "end tell";
    QProcess::startDetached("osascript", args);
#elif defined (Q_WS_WIN)
    QStringList args;
    args << "/select," << QDir::toNativeSeparators(fileInfo.absoluteFilePath());
    QProcess::startDetached("explorer", args);
#endif//
}


QAction * LibraryFileSystemModel::CreateAction(const QIcon & icon, const QString & hint, const QString &dataString)
{
    QAction *action = new QAction(icon, hint, this);
	action->setCheckable(true);
	action->setChecked(true);
    action->setData(dataString);
    
    QObject::connect(action, SIGNAL(triggered()), this, SLOT(SetNameFilters()));
    
	actions.push_back(action);
    return action;
}


void LibraryFileSystemModel::SetNameFilters()
{
	QStringList nameFilters;

    auto endIt = actions.end();
    for(auto it = actions.begin(); it != endIt; ++it)
    {
		if((*it)->isChecked())
		{
			nameFilters << (*it)->data().toString();
		}
    }
    
    ((QFileSystemModel *)listModel)->setNameFilters(nameFilters);
    ((QFileSystemModel *)listModel)->setNameFilterDisables(false);

    if(nameFilters.size())
    {
        ((QFileSystemModel *)listModel)->setFilter(QDir::Files|QDir::NoDotAndDotDot);
    }
    else
    {
        ((QFileSystemModel *)listModel)->setFilter(QDir::NoDotAndDotDot);
    }
}

