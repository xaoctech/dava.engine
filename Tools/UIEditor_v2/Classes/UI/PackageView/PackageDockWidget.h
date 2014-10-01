//
//  PackageTreeWidget.h
//  UIEditor
//
//  Created by Dmitry Belsky on 10.9.14.
//
//

#ifndef __UI_EDITOR_UI_PACKAGE_WIDGET__
#define __UI_EDITOR_UI_PACKAGE_WIDGET__

#include <QWidget>
#include <QDockWidget>
#include <QItemSelectionModel>

#include "UI/UIControl.h"

namespace Ui {
    class PackageDockWidget;
}

class PackageDocument;

class PackageDockWidget : public QDockWidget
{
    Q_OBJECT
public:
    explicit PackageDockWidget(QWidget *parent = 0);
    virtual ~PackageDockWidget();

    void SetDocument(PackageDocument *newDocument);

private slots:
    void OnSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);
    void filterTextChanged(const QString &);

signals:
    void SelectionRootControlChanged(const QList<DAVA::UIControl *> &activatedRootControls, const QList<DAVA::UIControl *> &deactivatedRootControls);
    void SelectionControlChanged(const QList<DAVA::UIControl *> &activatedControls, const QList<DAVA::UIControl *> &deactivatedControls);

private:
    Ui::PackageDockWidget *ui;
    PackageDocument *document;
};

#endif // __UI_EDITOR_UI_PACKAGE_WIDGET__
