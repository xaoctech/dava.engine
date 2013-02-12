#include "controlpropertygridwidget.h"
#include "ui_controlpropertygridwidget.h"

#include <QObject>
#include <QMetaObject>
#include <QMetaProperty>

#include "Base/BaseTypes.h"
#include "UI/UIControl.h"

#include "BaseMetadata.h"

using namespace DAVA;

static const QString CONTROL_PROPERTY_BLOCK_NAME = "Control";

ControlPropertyGridWidget::ControlPropertyGridWidget(QWidget *parent) :
    BasePropertyGridWidget(parent),
    ui(new Ui::ControlPropertyGridWidget)
{
    ui->setupUi(this);
    SetPropertyBlockName(CONTROL_PROPERTY_BLOCK_NAME);
}

ControlPropertyGridWidget::~ControlPropertyGridWidget()
{
    delete ui;
}

void ControlPropertyGridWidget::Initialize(BaseMetadata* activeMetadata)
{
    BasePropertyGridWidget::Initialize(activeMetadata);
    
    // Build the properties map to make the properties search faster.
    PROPERTIESMAP propertiesMap = BuildMetadataPropertiesMap();

    // Initialize the widgets. TODO - understand how to re-use property names!
    RegisterLineEditWidgetForProperty(propertiesMap, "UIControlClassName", ui->classNameLineEdit);
    RegisterLineEditWidgetForProperty(propertiesMap, "Name", ui->objectNameLineEdit, true);
    RegisterLineEditWidgetForProperty(propertiesMap, "Tag", ui->tagLineEdit);
}

void ControlPropertyGridWidget::Cleanup()
{
    BasePropertyGridWidget::Cleanup();

    UnregisterLineEditWidget(ui->objectNameLineEdit);
    UnregisterLineEditWidget(ui->tagLineEdit);
}
