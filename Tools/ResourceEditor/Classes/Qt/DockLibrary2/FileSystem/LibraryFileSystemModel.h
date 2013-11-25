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



#ifndef __LIBRARY_FILESYSTEM_MODEL_H__
#define __LIBRARY_FILESYSTEM_MODEL_H__

#include "../LibraryBaseModel.h"
#include "DAVAEngine.h"

#include <QFileInfo>

class LibraryFileSystemModel: public LibraryBaseModel
{
    Q_OBJECT
    
public:
    LibraryFileSystemModel(const QString & modelName);
    
    virtual void TreeItemSelected(const QItemSelection & selection);
    virtual void ListItemSelected(const QItemSelection & selection);

    virtual void SetProjectPath(const QString & path);
    
    virtual const QModelIndex GetTreeRootIndex() const;
    virtual const QModelIndex GetListRootIndex() const;

    virtual bool PrepareTreeContextMenu(QMenu & contextMenu, const QModelIndex & index) const;
    virtual bool PrepareListContextMenu(QMenu & contextMenu, const QModelIndex & index) const;

protected slots:
    
    void OnRevealAtFolder();
	void SetNameFilters();
    
protected:
    
    QAction * CreateAction(const QIcon & icon, const QString & hint, const QString &dataString);

    virtual void TreeFileSelected(const QFileInfo & fileInfo) = 0;
    virtual void ListFileSelected(const QFileInfo & fileInfo) = 0;
    
    virtual bool PrepareTreeContextMenuInternal(QMenu &contextMenu, const QFileInfo & fileInfo) const = 0;
    virtual bool PrepareListContextMenuInternal(QMenu &contextMenu, const QFileInfo & fileInfo) const = 0;
    
protected:
    
    QString treeRootPath;
    QString listRootPath;
};

#endif // __LIBRARY_FILESYSTEM_MODEL_H__
