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



#include "LibraryWidget.h"
#include "LibraryFilteringModel.h"

#include "Main/mainwindow.h"
#include "Project/ProjectManager.h"
#include "Scene/SceneTabWidget.h"
#include "Scene/SceneEditor2.h"

#include "Commands2/DAEConvertAction.h"


#include <QToolBar>
#include <QLineEdit>
#include <QComboBox>
#include <QTreeView>
#include <QHeaderView>
#include <QVBoxLayout>
#include <QProcess>
#include <QFileSystemModel>
#include <QMenu>
#include <QAction>
#include <QStringList>

Q_DECLARE_METATYPE( QFileInfo )



struct FileType
{
    FileType() {}
    
    FileType(const QString &n, const QString &f)
    {
        name = n;
        filter << f;
    }

    FileType(const QString &n, const QString &f1, const QString &f2)
    {
        name = n;
        filter << f1;
        filter << f2;
    }

    QString name;
    QStringList filter;
};

QVector<FileType> fileTypeValues;

LibraryWidget::LibraryWidget(QWidget *parent /* = 0 */)
	: QWidget(parent)
{
    fileTypeValues.push_back(FileType("All files", "*"));
    fileTypeValues.push_back(FileType("Models", "*.dae", "*.sc2"));
    fileTypeValues.push_back(FileType("Textures", "*.png", "*.tex"));
    fileTypeValues.push_back(FileType("DAE", "*.dae"));
    fileTypeValues.push_back(FileType("PNG", "*.png"));
    fileTypeValues.push_back(FileType("SC2", "*.sc2"));
    fileTypeValues.push_back(FileType("TEX", "*.tex"));
    
    SetupToolbar();

    SetupView();
    SetupLayout();
    
    ViewAsList();
    OnFilesTypeChanged(0);
}

LibraryWidget::~LibraryWidget()
{
    
}

void LibraryWidget::SetupSignals()
{
    QObject::connect(ProjectManager::Instance(), SIGNAL(ProjectOpened(const QString &)), this, SLOT(ProjectOpened(const QString &)));
	QObject::connect(ProjectManager::Instance(), SIGNAL(ProjectClosed()), this, SLOT(ProjectClosed()));
    
    QObject::connect(filesView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)), this, SLOT(SelectionChanged(const QItemSelection &, const QItemSelection &)));
    QObject::connect(filesView, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(ShowContextMenu(const QPoint &)));
}

void LibraryWidget::SetupToolbar()
{
    toolbar = new QToolBar(this);
    toolbar->setIconSize(QSize(16, 16));
    toolbar->setToolButtonStyle(Qt::ToolButtonIconOnly);
    toolbar->setMovable(false);

    searchFilter = new QLineEdit(toolbar);
    searchFilter->setToolTip("Search something at right list view");
    searchFilter->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred));
    
    filesTypeFilter = new QComboBox(toolbar);
    filesTypeFilter->setEditable(false);
    filesTypeFilter->setMinimumWidth(100);
    filesTypeFilter->setMaximumWidth(100);
    for(int i = 0; i < fileTypeValues.size(); ++i)
    {
        filesTypeFilter->addItem(fileTypeValues[i].name);
    }
    filesTypeFilter->setCurrentIndex(0);
    
    
    QIcon resetIcon(QString::fromUtf8(":/QtIcons/reset.png"));
    QAction *actionResetFilter = new QAction(resetIcon, "Reset search filter", toolbar);

    QIcon asListIcon(QString::fromUtf8(":/QtIconsTextureDialog/view_list.png"));
    actionViewAsList = new QAction(asListIcon, "View as list", toolbar);
    actionViewAsList->setCheckable(true);
    actionViewAsList->setChecked(true);

    QIcon asDetailedIcon(QString::fromUtf8(":/QtIcons/all.png"));
    actionViewDetailed = new QAction(asDetailedIcon, "View detailed", toolbar);
    actionViewDetailed->setCheckable(true);
    actionViewDetailed->setChecked(false);

    QObject::connect(searchFilter, SIGNAL(textChanged(const QString &)), this, SLOT(SetFilter(const QString &)));
    QObject::connect(actionResetFilter, SIGNAL(triggered()), this, SLOT(ResetFilter()));
    QObject::connect(filesTypeFilter, SIGNAL(currentIndexChanged(int)), this, SLOT(OnFilesTypeChanged(int)));
    QObject::connect(actionViewAsList, SIGNAL(triggered()), this, SLOT(ViewAsList()));
    QObject::connect(actionViewDetailed, SIGNAL(triggered()), this, SLOT(ViewDetailed()));
    
    toolbar->addWidget(searchFilter);
    toolbar->addAction(actionResetFilter);
    toolbar->addSeparator();
    toolbar->addWidget(filesTypeFilter);
    
    toolbar->addAction(actionViewAsList);
    toolbar->addAction(actionViewDetailed);
}

void LibraryWidget::SetupView()
{
    filesModel = new QFileSystemModel(this);
    filesModel->setFilter(QDir::NoDotAndDotDot | QDir::AllEntries | QDir::AllDirs);
    
    proxyModel = new LibraryFilteringModel(this);
    proxyModel->SetModel(filesModel);
    proxyModel->setFilterKeyColumn(0);
    
    filesView = new QTreeView(this);
    filesView->setContextMenuPolicy(Qt::CustomContextMenu);
    filesView->header()->setVisible(false);
    filesView->setDragDropMode(QAbstractItemView::DragOnly);
	filesView->setDragEnabled(true);
    filesView->setUniformRowHeights(true);
    
    filesView->setModel(proxyModel);
}

void LibraryWidget::SetupLayout()
{
    // put tab bar and davawidget into vertical layout
	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(toolbar);
	layout->addWidget(filesView);
	layout->setMargin(0);
	layout->setSpacing(1);
	setLayout(layout);
}


void LibraryWidget::ViewAsList()
{
    HideDetailedColumnsAtFilesView(true);
    
    actionViewAsList->setChecked(true);
    actionViewDetailed->setChecked(false);
}

void LibraryWidget::ViewDetailed()
{
    // Magic trick for MacOS: call funciton twice
    HideDetailedColumnsAtFilesView(false);
    HideDetailedColumnsAtFilesView(false);
    //EndOftrick

    actionViewAsList->setChecked(false);
    actionViewDetailed->setChecked(true);
}

void LibraryWidget::HideDetailedColumnsAtFilesView(bool hide)
{
    int columns = (hide) ? 1 : proxyModel->columnCount();
    int width = filesView->geometry().width() / columns;
    
    if(!hide)
    {
        filesView->setColumnWidth(0, width);
    }
    
    for(int i = 1; i < proxyModel->columnCount(); ++i)
	{
        filesView->setColumnHidden(i, hide);
        filesView->setColumnWidth(i, width);
	}
}


void LibraryWidget::SelectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    if(0 == selected.count()) return;

    const QItemSelection & realSelection = proxyModel->mapSelectionToSource(selected);
    
    const QModelIndex & index = realSelection.indexes().first();
    QFileInfo fileInfo = filesModel->fileInfo(index);

    if(0 == fileInfo.suffix().compare("sc2", Qt::CaseInsensitive))
    {
        ShowPreview(fileInfo.filePath());
    }
    else
    {
        HidePreview();
    }
}


void LibraryWidget::ShowContextMenu(const QPoint & point)
{
    HidePreview();
    
    const QModelIndex & index = filesView->indexAt(point);
    
	if(!index.isValid()) return;

    const QModelIndex & realIndex = proxyModel->mapToSource(index);
    
    QFileInfo fileInfo = filesModel->fileInfo(realIndex);
    if(!fileInfo.isFile()) return;

    QMenu contextMenu(this);
    QVariant fileInfoAsVariant = QVariant::fromValue<QFileInfo>(fileInfo);

    DAVA::FilePath pathname = fileInfo.absoluteFilePath().toStdString();
    if(pathname.IsEqualToExtension(".sc2"))
    {
        QAction * actionAdd = contextMenu.addAction("Add Model", this, SLOT(OnAddModel()));
        QAction * actionEdit = contextMenu.addAction("Edit Model", this, SLOT(OnEditModel()));
        
        actionAdd->setData(fileInfoAsVariant);
        actionEdit->setData(fileInfoAsVariant);
    }
    else if(pathname.IsEqualToExtension(".dae"))
    {
        QAction * actionConvert = contextMenu.addAction("Convert", this, SLOT(OnConvertDae()));
        QAction * actionConvertGeometry = contextMenu.addAction("Convert geometry", this, SLOT(OnConvertGeometry()));
        
        actionConvert->setData(fileInfoAsVariant);
        actionConvertGeometry->setData(fileInfoAsVariant);
    }
    else if(pathname.IsEqualToExtension(".tex"))
    {
        QAction * actionEdit = contextMenu.addAction("Edit", this, SLOT(OnEditTextureDescriptor()));
        actionEdit->setData(fileInfoAsVariant);
    }
    else if(pathname.IsEqualToExtension(".png"))
    {
        QAction * actionEdit = contextMenu.addAction("Edit", this, SLOT(OnEditTextureDescriptor()));
        actionEdit->setData(fileInfoAsVariant);
    }
    
    
    contextMenu.addSeparator();
    QAction * actionRevealAt = contextMenu.addAction("Reveal at folder", this, SLOT(OnRevealAtFolder()));
    actionRevealAt->setData(fileInfoAsVariant);

    
    contextMenu.exec(filesView->mapToGlobal(point));
}

void LibraryWidget::SetFilter(const QString &filter)
{
    proxyModel->setFilterRegExp(QRegExp(filter, Qt::CaseInsensitive, QRegExp::FixedString));

    if(filter.isEmpty())
    {
        
    }
    else
    {
        
    }
}

void LibraryWidget::ResetFilter()
{
    searchFilter->setText("");
}


void LibraryWidget::OnFilesTypeChanged(int typeIndex)
{
    QStringList nameFilters;
    nameFilters << fileTypeValues[typeIndex].filter;

    filesModel->setNameFilters(nameFilters);
    filesModel->setNameFilterDisables(false);

    filesModel->setFilter(QDir::Files);
    filesModel->setFilter(QDir::NoDotAndDotDot | QDir::AllEntries | QDir::AllDirs);
    
    proxyModel->invalidate();
}


void LibraryWidget::ProjectOpened(const QString &path)
{
    ActivateProject(path);
}

void LibraryWidget::ProjectClosed()
{
    ActivateProject("");
}


void LibraryWidget::ActivateProject(const QString &projectPath)
{
    rootPathname = projectPath + "/DataSource/3d/";
    
    QDir rootDir(rootPathname);
    
    filesModel->setRootPath(rootPathname);
    proxyModel->invalidate();
 
    proxyModel->SetSourceRoot(filesModel->index(rootPathname));

    filesView->setRootIndex(proxyModel->mapFromSource(filesModel->index(rootPathname)));
}


void LibraryWidget::OnAddModel()
{
    QVariant indexAsVariant = ((QAction *)sender())->data();
    const QFileInfo fileInfo = indexAsVariant.value<QFileInfo>();
    
    SceneEditor2 *scene = QtMainWindow::Instance()->GetCurrentScene();
    if(NULL != scene)
    {
        QtMainWindow::Instance()->WaitStart("Add object to scene", fileInfo.absoluteFilePath());
        
        scene->structureSystem->Add(fileInfo.absoluteFilePath().toStdString());
        
        QtMainWindow::Instance()->WaitStop();
    }
}

void LibraryWidget::OnEditModel()
{
    QVariant indexAsVariant = ((QAction *)sender())->data();
    const QFileInfo fileInfo = indexAsVariant.value<QFileInfo>();
    
    QtMainWindow::Instance()->OpenScene(fileInfo.absoluteFilePath());
}

void LibraryWidget::OnConvertDae()
{
    QVariant indexAsVariant = ((QAction *)sender())->data();
    const QFileInfo fileInfo = indexAsVariant.value<QFileInfo>();
    
    QtMainWindow::Instance()->WaitStart("DAE to SC2 conversion", fileInfo.absoluteFilePath());
    
    Command2 *daeCmd = new DAEConvertAction(fileInfo.absoluteFilePath().toStdString());
    daeCmd->Redo();
    delete daeCmd;
    
    QtMainWindow::Instance()->WaitStop();
}

void LibraryWidget::OnConvertGeometry()
{
    QVariant indexAsVariant = ((QAction *)sender())->data();
    const QFileInfo fileInfo = indexAsVariant.value<QFileInfo>();
    
    QtMainWindow::Instance()->WaitStart("DAE to SC2 conversion of geometry", fileInfo.absoluteFilePath());
    
    Command2 *daeCmd = new DAEConvertWithSettingsAction(fileInfo.absoluteFilePath().toStdString());
    daeCmd->Redo();
    delete daeCmd;
    
    QtMainWindow::Instance()->WaitStop();
}

void LibraryWidget::OnEditTextureDescriptor()
{
    
}

void LibraryWidget::OnRevealAtFolder()
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

void LibraryWidget::HidePreview() const
{
    SceneTabWidget *widget = QtMainWindow::Instance()->GetSceneWidget();
    widget->HideScenePreview();
}

void LibraryWidget::ShowPreview(const QString & pathname) const
{
    SceneTabWidget *widget = QtMainWindow::Instance()->GetSceneWidget();
    widget->ShowScenePreview(pathname.toStdString());
}

