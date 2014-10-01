//
//  PropertiesTreeWidget.cpp
//  UIEditor
//
//  Created by Dmitry Belsky on 12.9.14.
//
//

#include "PropertiesDockWidget.h"

#include "ui_PropertiesDockWidget.h"
#include "PropertiesTreeModel.h"
#include "UI/PackageDocument.h"
#include "UI/PropertiesView/PropertiesTreeItemDelegate.h"
#include <qitemeditorfactory.h>
#include "QtControls/lineeditext.h"
#include <qstyleditemdelegate.h>
#include "QtControls/Vector2Edit.h"

using namespace DAVA;

PropertiesDockWidget::PropertiesDockWidget(QWidget *parent)
    : QDockWidget(parent)
    , ui(new Ui::PropertiesDockWidget())
    , document(NULL)
{
    ui->setupUi(this);
}

PropertiesDockWidget::~PropertiesDockWidget()
{
    delete ui;
    ui = NULL;
}

void PropertiesDockWidget::SetDocument(PackageDocument *newDocument)
{
    if (document)
    {
        disconnect(document, SIGNAL(controlsSelectionChanged(const QList<DAVA::UIControl *> &, const QList<DAVA::UIControl *> &)), this, SLOT(OnControlsSelectionChanged(const QList<DAVA::UIControl *> &, const QList<DAVA::UIControl *> &)));
        ui->treeView->setModel(NULL);
        //ui->filterLine->setEnabled(false);
        //ui->treeView->setEnabled(false);
    }
    
    document = newDocument;
    
    if (document)
    {
        //ui->treeView->setModel(document->GetTreeContext()->proxyModel);
        //ui->filterLine->setText(document->GetTreeContext()->filterString);
        connect(document, SIGNAL(controlsSelectionChanged(const QList<DAVA::UIControl *> &, const QList<DAVA::UIControl *> &)), this, SLOT(OnControlsSelectionChanged(const QList<DAVA::UIControl *> &, const QList<DAVA::UIControl *> &)));
        //ui->filterLine->setEnabled(true);
        //ui->treeView->setEnabled(true);
    }
}

void PropertiesDockWidget::OnControlsSelectionChanged(const QList<DAVA::UIControl *> &activatedControls, const QList<DAVA::UIControl *> &deactivatedControls)
{
    if (!activatedControls.empty())
        SetControl(activatedControls.front());
    else
        SetControl(NULL);
}

void PropertiesDockWidget::SetControl(DAVA::UIControl *control)
{
    if (control)
    {
        QStyledItemDelegate *itemDelegate = new PropertiesTreeItemDelegate(this);
        PropertiesTreeModel *model = new PropertiesTreeModel(control, this);
//        QItemEditorFactory *itemEditorFactory = new QItemEditorFactory();
//        itemEditorFactory->registerEditor( QVariant::Vector2D, new QStandardItemEditorCreator<Vector2Edit>());
//        itemDelegate->setItemEditorFactory(itemEditorFactory);

        //QItemEditorFactory::setDefaultFactory(itemEditorFactory);
        ui->treeView->setItemDelegate(itemDelegate);
        ui->treeView->setModel(model);
        ui->treeView->expandToDepth(0);
        ui->treeView->resizeColumnToContents(0);
    }
    else
    {
        ui->treeView->setModel(NULL);
    }
}
