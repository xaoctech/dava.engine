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



#include "LibraryComplexView.h"
#include "LibraryBaseModel.h"
#include "LibraryFilteringModel.h"

#include <QToolBar>
#include <QSplitter>
#include <QTreeView>
#include <QListView>
#include <QVBoxLayout>
#include <QAction>
#include <QLineEdit>

LibraryComplexView::LibraryComplexView(QWidget *parent /* = 0 */)
	: QWidget(parent)
    , model(NULL)
{
    filteringModel = new LibraryFilteringModel(this);

    SetupToolbar();

    splitter = new QSplitter(this);
    

    SetupViews();
    SetupLayout();
    
    ViewAsList();
}

LibraryComplexView::~LibraryComplexView()
{
    
}

void LibraryComplexView::SetupToolbar()
{
    toolbar = new QToolBar(this);
    toolbar->setIconSize(QSize(16, 16));
    toolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    toolbar->setMovable(false);

    searchFilter = new QLineEdit(toolbar);
    searchFilter->setToolTip("Search something at right list view");
    searchFilter->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred));
    
    QIcon resetIcon(QString::fromUtf8(":/QtIcons/reset.png"));
    QAction *actionResetFilter = new QAction(resetIcon, "Reset search filter", toolbar);

    QIcon asListIcon(QString::fromUtf8(":/QtIconsTextureDialog/view_list.png"));
    actionViewAsList = new QAction(asListIcon, "View as list", toolbar);
    actionViewAsList->setCheckable(true);
    actionViewAsList->setChecked(true);

    QIcon asIconIcon(QString::fromUtf8(":/QtIconsTextureDialog/view_pictures.png"));
    actionViewAsIcons = new QAction(asIconIcon, "View as icons", toolbar);
    actionViewAsIcons->setCheckable(true);
    actionViewAsIcons->setChecked(false);

    QObject::connect(searchFilter, SIGNAL(textChanged(const QString &)), this, SLOT(SetFilter(const QString &)));
    QObject::connect(actionResetFilter, SIGNAL(triggered()), this, SLOT(ResetFilter()));
    QObject::connect(actionViewAsList, SIGNAL(triggered()), this, SLOT(ViewAsList()));
    QObject::connect(actionViewAsIcons, SIGNAL(triggered()), this, SLOT(ViewAsIcons()));
    
    toolbar->addWidget(searchFilter);
    toolbar->addAction(actionResetFilter);
    toolbar->addSeparator();
    toolbar->addAction(actionViewAsList);
    toolbar->addAction(actionViewAsIcons);
}

void LibraryComplexView::SetupViews()
{
    leftTree = new QTreeView(splitter);
    
    rightList = new QListView(splitter);
    rightList->setContextMenuPolicy(Qt::CustomContextMenu);
    rightList->setDragDropMode(QAbstractItemView::DragOnly);
	rightList->setDragEnabled(true);
    
    QObject::connect(rightList, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(ShowListContextMenu(const QPoint &)));
}

void LibraryComplexView::SetupLayout()
{
    // put tab bar and davawidget into vertical layout
	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(toolbar);
	layout->addWidget(splitter);
	layout->setMargin(0);
	layout->setSpacing(1);
	setLayout(layout);
}


void LibraryComplexView::ViewAsList()
{
    rightList->setViewMode(QListView::ListMode);
    rightList->setGridSize(QSize());

    actionViewAsList->setChecked(true);
    actionViewAsIcons->setChecked(false);
}

void LibraryComplexView::ViewAsIcons()
{
    rightList->setViewMode(QListView::IconMode);
    rightList->setGridSize(QSize(100, 100));

    actionViewAsList->setChecked(false);
    actionViewAsIcons->setChecked(true);
}

void LibraryComplexView::SetModel(LibraryBaseModel * newModel)
{
    if(model)
    {
        QObject::disconnect(leftTree->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), this, SLOT(TreeSelectionChanged(const QItemSelection &, const QItemSelection &)));
        
        QObject::disconnect(rightList->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), this, SLOT(ListSelectionChanged(const QItemSelection &, const QItemSelection &)));
        
    }
    
    model = newModel;
    
    leftTree->setModel(newModel->GetTreeModel());
    leftTree->setRootIndex(newModel->GetTreeRootIndex());
    
    filteringModel->SetModel(newModel->GetListModel());
    
    rightList->setModel(filteringModel);
    rightList->setRootIndex(filteringModel->mapFromSource(newModel->GetListRootIndex()));
    
    // hide columns with size/modif date etc.
	for(int i = 1; i < newModel->GetTreeModel()->columnCount(); ++i)
	{
        leftTree->setColumnHidden(i, true);
	}
    
    QObject::connect(leftTree->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), this, SLOT(TreeSelectionChanged(const QItemSelection &, const QItemSelection &)));
    
    QObject::connect(rightList->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), this, SLOT(ListSelectionChanged(const QItemSelection &, const QItemSelection &)));

}

void LibraryComplexView::TreeSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    if(0 == selected.count()) return;
    
    model->TreeItemSelected(selected);
    rightList->setRootIndex(filteringModel->mapFromSource(model->GetListRootIndex()));
}

void LibraryComplexView::ListSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    if(0 == selected.count()) return;

    model->TreeItemSelected(filteringModel->mapSelectionToSource(selected));
}

void LibraryComplexView::ShowListContextMenu(const QPoint & point)
{
    QModelIndex index = rightList->indexAt(point);
    
	if(!index.isValid()) return;

    QMenu contextMenu(this);
    
    if(model->PrepareListContextMenu(contextMenu, filteringModel->mapToSource(index)))
    {
        contextMenu.exec(rightList->mapToGlobal(point));
    }
}

void LibraryComplexView::SetFilter(const QString &filter)
{
    filteringModel->setFilterRegExp(QRegExp(filter, Qt::CaseInsensitive, QRegExp::FixedString));
    rightList->setRootIndex(filteringModel->mapFromSource(model->GetListRootIndex()));
}

void LibraryComplexView::ResetFilter()
{
    searchFilter->setText("");
}


