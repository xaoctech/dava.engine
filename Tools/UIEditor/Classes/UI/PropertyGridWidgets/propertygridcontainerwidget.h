#ifndef PROPERTYGRIDCONTAINERWIDGET_H
#define PROPERTYGRIDCONTAINERWIDGET_H

#include <QWidget>

#include "PropertiesGridController.h"
#include "PropertyGridWidgetsFactory.h"

namespace Ui {
class PropertyGridContainerWidget;
}

class PropertyGridContainerWidget : public QWidget
{
    Q_OBJECT
    
public:
    explicit PropertyGridContainerWidget(QWidget *parent = 0);
    ~PropertyGridContainerWidget();
    
public slots:
    // Emitted by Controller when Properties Grid is updated.
    void OnPropertiesGridUpdated();
    
protected:
    // Connect/disconnect to the signals.
    void ConnectToSignals();

    // Build the Properties Grid for the single control and for the vector of controls.
    void BuildPropertiesGrid(UIControl* activeControl, BaseMetadata* metaData,
                             HierarchyTreeNode::HIERARCHYTREENODEID activeNodeID);
    void BuildPropertiesGrid(BaseMetadata* metaData,
                             const METADATAPARAMSVECT& params);

    // Build the Properties Grid for the controls selected.
    void BuildPropertiesGridList();
    
    // Cleanup the existing Properties Grid.
    void CleanupPropertiesGrid();

    // Work with Active Metadata.
    BaseMetadata* GetActiveMetadata(const HierarchyTreeNode* activeTreeNode);
    BaseMetadata* GetActiveMetadata(const HierarchyTreeController::SELECTEDCONTROLNODES activeNodes);

    void CleanupActiveMetadata();

private:
    Ui::PropertyGridContainerWidget *ui;
    
    // Properties Grid Widgets Factory.
    PropertyGridWidgetsFactory widgetsFactory;
    
    // Active Property Grid Widgets.
    PropertyGridWidgetsFactory::PROPERTYGRIDWIDGETSLIST activeWidgetsList;
    
    // Active Metadata.
    BaseMetadata* activeMetadata;
};


#endif // PROPERTYGRIDCONTAINERWIDGET_H
