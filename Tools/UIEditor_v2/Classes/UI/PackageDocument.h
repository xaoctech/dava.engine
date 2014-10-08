//
//  UIPackageDocument.h
//  UIEditor
//
//  Created by Alexey Strokachuk on 9/17/14.
//
//

#ifndef __UIEditor__UIPackageDocument__
#define __UIEditor__UIPackageDocument__

#include <QWidget>
#include <QVariant>
#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"

namespace Ui {
    class PackageDocument;
}

namespace DAVA {
    class UIPackage;
    class FilePath;
    class UIControl;
    class UIScreen;
}

class QAbstractItemModel;
class QSortFilterProxyModel;
class QUndoStack;

class UIPackageModel;
class DavaGLWidget;

class TreeViewContext
{
public:
    QPoint scrollPosition;
    //QModelIndexList expandedItems;
    QAbstractItemModel *model;
    QSortFilterProxyModel *proxyModel;
    QString filterString;
};

class LibraryViewContext
{
public:
    QPoint scrollPosition;
    //QModelIndexList expandedItems;
};

class GraphicsViewContext;
class PropertiesViewContext;

class PackageDocument: public QObject
{
    Q_OBJECT
public:
    PackageDocument(DAVA::UIPackage *package, QObject *parent = NULL);
    ~PackageDocument();
    
    bool IsModified() const;
    void ClearModified();
    const DAVA::FilePath &PackageFilePath() const;
    DAVA::UIPackage *Package() const {return package;}
    
    const QList<DAVA::UIControl *> &GetSelectedControls() const { return selectedControls; }
    const QList<DAVA::UIControl *> &GetActiveRootControls() const { return activeRootControls; }
    
    const TreeViewContext *GetTreeContext() const { return &treeContext; };
    const GraphicsViewContext *GetGraphicsContext() const {return graphicsContext; };
    const PropertiesViewContext *GetPropertiesContext() const {return propertiesContext; };
    const LibraryViewContext *GetLibraryContext() const {return &libraryContext; };

    TreeViewContext *GetTreeContext() { return &treeContext; };
    GraphicsViewContext *GetGraphicsContext() {return graphicsContext; };
    PropertiesViewContext *GetPropertiesContext() {return propertiesContext; };
    LibraryViewContext *GetLibraryContext() {return &libraryContext; };

    QUndoStack *UndoStack() const { return undoStack; }

signals:
    void controlsSelectionChanged(const QList<DAVA::UIControl *> &activatedControls, const QList<DAVA::UIControl *> &deactivatedControls);
    void activeRootControlsChanged(const QList<DAVA::UIControl *> &activatedRootControls, const QList<DAVA::UIControl *> &deactivatedRootControls);
    
public slots:
    void OnSelectionRootControlChanged(const QList<DAVA::UIControl *> &activatedRootControls, const QList<DAVA::UIControl *> &deactivatedRootControls);
    void OnSelectionControlChanged(const QList<DAVA::UIControl *> &activatedControls, const QList<DAVA::UIControl *> &deactivatedControls);

private:
    void UpdateControlCanvas();
    
private:
    DAVA::UIPackage *package;
    QList<DAVA::UIControl *> selectedControls;
    QList<DAVA::UIControl *> activeRootControls;
    TreeViewContext treeContext;
    GraphicsViewContext *graphicsContext;
    PropertiesViewContext *propertiesContext;
    LibraryViewContext libraryContext;

    QUndoStack *undoStack;
};

Q_DECLARE_METATYPE(PackageDocument *);

#endif /* defined(__UIEditor__UIPackageDocument__) */
