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

#include "DockProperties.h"
#include "Main/mainwindow.h"

DockProperties::DockProperties(QWidget *parent /* = NULL */)
	: QDockWidget(parent)
{ }

DockProperties::~DockProperties()
{ }

void DockProperties::Init()
{
	Ui::MainWindow* ui = QtMainWindow::Instance()->GetUI();

	// toggle favorites edit mode
	QObject::connect(ui->actionFavoritesEdit, SIGNAL(triggered()), this, SLOT(ActionFavoritesEdit()));
	ui->propertyEditor->SetFavoritesEditMode(ui->actionFavoritesEdit->isChecked());

	// filter
	QObject::connect(ui->propertiesFilterClear, SIGNAL(pressed()), ui->propertiesFilterEdit, SLOT(clear()));
	QObject::connect(ui->propertiesFilterEdit, SIGNAL(textChanged(const QString &)), ui->propertyEditor, SLOT(SetFilter(const QString &)));
}

void DockProperties::ActionFavoritesEdit()
{
	QAction *favoritesEditAction = dynamic_cast<QAction *>(QObject::sender());
	if(NULL != favoritesEditAction)
	{
		QtMainWindow::Instance()->GetUI()->propertyEditor->SetFavoritesEditMode(favoritesEditAction->isChecked());
	}
}
