//
//  UIPackageDocument.cpp
//  UIEditor
//
//  Created by Alexey Strokachuk on 9/17/14.
//
//

#include "PackageDocument.h"
#include <DAVAEngine.h>
#include <QLineEdit>
#include <QAction>

#include "UI/PackageView/UIPackageModel.h"
#include "UI/PackageView/UIFilteredPackageModel.h"
#include "UI/DavaGLWidget.h"
#include "UI/GraphicView/GraphicsViewContext.h"
#include "UI/PropertiesView/PropertiesViewContext.h"

using namespace DAVA;

PackageDocument::PackageDocument(DAVA::UIPackage *_package, QObject *parent)
: QObject(parent)
, package(SafeRetain(_package))
, graphicsContext(NULL)
{
    undoStack = new QUndoStack(this);
    undoAction = undoStack->createUndoAction(undoStack);
    undoAction->setShortcuts(QKeySequence::Undo);
    undoAction->setIcon(QIcon(":/Icons/edit_undo.png"));

    redoAction = undoStack->createRedoAction(undoStack);
    redoAction->setShortcuts(QKeySequence::Redo);
    redoAction->setIcon(QIcon(":/Icons/edit_redo.png"));

    treeContext.model = new UIPackageModel(package, this);
    treeContext.proxyModel = new UIFilteredPackageModel(this);
    treeContext.proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    treeContext.proxyModel->setSourceModel(treeContext.model);

    graphicsContext = new GraphicsViewContext();
    connect(this, SIGNAL(activeRootControlsChanged(const QList<DAVA::UIControl *> &, const QList<DAVA::UIControl *> &)), graphicsContext, SLOT(OnActiveRootControlsChanged(const QList<DAVA::UIControl *> &, const QList<DAVA::UIControl *> &)));

    propertiesContext = new PropertiesViewContext(this);
    //libraryContext;

    for (size_t index = 0; index < package->GetControlsCount(); ++index)
    {
        activeRootControls.push_back(package->GetControl(index));
    }

    if (!activeRootControls.empty())
        emit activeRootControlsChanged(activeRootControls, QList<UIControl*>());
}

PackageDocument::~PackageDocument()
{
    treeContext.proxyModel->setSourceModel(NULL);

    SafeDelete(treeContext.model);
    SafeDelete(treeContext.proxyModel);
    disconnect(this, SIGNAL(activeRootControlsChanged(const QList<DAVA::UIControl *> &, const QList<DAVA::UIControl *> &)), graphicsContext, SLOT(OnActiveRootControlsChanged(const QList<DAVA::UIControl *> &, const QList<DAVA::UIControl *> &)));
    SafeDelete(graphicsContext);
    SafeDelete(propertiesContext);
    
    SafeRelease(package);
}

bool PackageDocument::IsModified() const
{
    return !undoStack->isClean();
}

void PackageDocument::ClearModified()
{
    undoStack->clear();
}

const DAVA::FilePath &PackageDocument::PackageFilePath() const
{
    return package->getFilePath();
}

void PackageDocument::OnSelectionRootControlChanged(const QList<DAVA::UIControl *> &activatedRootControls, const QList<DAVA::UIControl *> &deactivatedRootControls)
{
    activeRootControls.clear();
    foreach(DAVA::UIControl *control, activatedRootControls)
    {
        activeRootControls.push_back(control);
    }

    emit activeRootControlsChanged(activeRootControls, deactivatedRootControls);
}

void PackageDocument::OnSelectionControlChanged(const QList<DAVA::UIControl *> &activatedControls, const QList<DAVA::UIControl *> &deactivatedControls)
{
    selectedControls.clear();
    
    foreach(DAVA::UIControl *control, activatedControls)
    {
        selectedControls.push_back(control);
    }

    emit controlsSelectionChanged(activatedControls, deactivatedControls);
}