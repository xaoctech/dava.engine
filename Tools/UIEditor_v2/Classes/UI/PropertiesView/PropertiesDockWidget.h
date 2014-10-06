//
//  PropertiesTreeWidget.h
//  UIEditor
//
//  Created by Dmitry Belsky on 12.9.14.
//
//

#ifndef __UI_EDITOR_PROPERTIES_TREE_WIDGET_H__
#define __UI_EDITOR_PROPERTIES_TREE_WIDGET_H__

#include <QWidget>
#include <QDockWidget>

#include "DAVAEngine.h"

namespace Ui {
    class PropertiesDockWidget;
}

class PropertiesViewContext;

class PropertiesDockWidget : public QDockWidget
{
    Q_OBJECT
public:
    PropertiesDockWidget(QWidget *parent = NULL);
    virtual ~PropertiesDockWidget();
    
    void SetContext(PropertiesViewContext *newContext);

protected:
    void SetControl(DAVA::UIControl *control);
    
private slots:
    void OnControlsSelectionChanged(const QList<DAVA::UIControl *> &activatedControls, const QList<DAVA::UIControl *> &deactivatedControls);

private:
    Ui::PropertiesDockWidget *ui;
    PropertiesViewContext *context;
};

#endif // __UI_EDITOR_PROPERTIES_TREE_WIDGET_H__
