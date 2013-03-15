#include "HangingObjectsView.h"
#include "ui_HangingObjectsView.h"
#include <stdlib.h> 
#include "Project/ProjectManager.h"
#include "Classes/Qt/Main/QtMainWindowHandler.h"
#include "../SceneEditor/EditorConfig.h"

HangingObjectsView::HangingObjectsView(QWidget* parent)
:	QWidget(parent),
	ui(new Ui::HangingObjectsView)
{
	ui->setupUi(this);
	
	Init();
}

HangingObjectsView::~HangingObjectsView()
{
	delete ui;
}

void HangingObjectsView::Init()
{
	/*QtMainWindowHandler* handler = QtMainWindowHandler::Instance();
	connect(this, SIGNAL(Clicked(DAVA::uint32, DAVA::SetSwitchIndexHelper::eSET_SWITCH_INDEX)), handler, SLOT(ToggleSetSwitchIndex(DAVA::uint32, DAVA::SetSwitchIndexHelper::eSET_SWITCH_INDEX)));
	connect(ui->btnOK, SIGNAL(clicked()), this, SLOT(Clicked()));
	
	ui->btnOK->blockSignals(true);
	QtMainWindowHandler::Instance()->RegisterSetSwitchIndexWidgets(ui->spinBox,
		ui->rBtnSelection,
		ui->rBtnScene,
		ui->btnOK);

	handler->SetSwitchIndexWidgetsState(true);
	*/
}

