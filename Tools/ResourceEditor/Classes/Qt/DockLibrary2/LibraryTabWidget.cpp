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



#include "LibraryTabWidget.h"
#include "LibraryComplexView.h"

#include "LibraryBaseModel.h"
#include "FileSystem/LibraryFileSystemModel.h"

#include "Project/ProjectManager.h"


#include <QVBoxLayout>

LibraryTabWidget::LibraryTabWidget(QWidget *parent)
	: QWidget(parent)
    , tabBar(NULL)
{
	// tab bar
    SetupTabbar();
    libraryView = new LibraryComplexView(this);
    
	// put tab bar and davawidget into vertical layout
	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(tabBar);
	layout->addWidget(libraryView);
	layout->setMargin(0);
	layout->setSpacing(1);
	setLayout(layout);

    //Add models
    AddModel(new LibraryFileSystemModel());
}

LibraryTabWidget::~LibraryTabWidget()
{
    for_each(models.begin(), models.end(), DAVA::SafeDelete<LibraryBaseModel>);
    models.clear();
}

void LibraryTabWidget::SetupTabbar()
{
    tabBar = new QTabBar(this);
	tabBar->setTabsClosable(false);
	tabBar->setMovable(true);
	tabBar->setUsesScrollButtons(true);
	tabBar->setExpanding(false);
	tabBar->setMinimumHeight(tabBar->sizeHint().height());
    
    QObject::connect(tabBar, SIGNAL(currentChanged(int)), this, SLOT(TabBarCurrentChanged(int)));

    SetCurrentTab(0);
}

void LibraryTabWidget::SetupSignals()
{
    QObject::connect(ProjectManager::Instance(), SIGNAL(ProjectOpened(const QString &)), this, SLOT(ProjectOpened(const QString &)));
	QObject::connect(ProjectManager::Instance(), SIGNAL(ProjectClosed()), this, SLOT(ProjectClosed()));
}

void LibraryTabWidget::AddModel(LibraryBaseModel *model)
{
    int tabIndex = 0;
    
    auto endIt = models.end();
    for(auto it = models.begin(); it != endIt; ++it, ++tabIndex)
    {
        if(*it == model)
        {
            SetCurrentTab(tabIndex);
            return;
        }
    }
    
    models.push_back(model);
    tabBar->addTab(model->GetName());
    
    SetCurrentTab(tabIndex);
}

void LibraryTabWidget::SetCurrentTab(int index)
{
	if(index >= 0 && index < tabBar->count())
	{
		tabBar->setCurrentIndex(index);
        libraryView->SetModel(GetModel(index));
	}
}

int LibraryTabWidget::GetCurrentTab() const
{
    return tabBar->currentIndex();
}

void LibraryTabWidget::TabBarCurrentChanged(int index)
{
	SetCurrentTab(index);
}

void LibraryTabWidget::ProjectOpened(const QString &path)
{
    libraryView->setEnabled(true);
    UpdateViewForProject(path);
}

void LibraryTabWidget::ProjectClosed()
{
    libraryView->setEnabled(false);
    UpdateViewForProject("");
}

void LibraryTabWidget::UpdateViewForProject(const QString &projectPath)
{
    auto endIt = models.end();
    for(auto it = models.begin(); it != endIt; ++it)
    {
        (*it)->SetProjectPath(projectPath);
    }
    
    SetCurrentTab(GetCurrentTab());
}


LibraryBaseModel * LibraryTabWidget::GetModel(int index)
{
    auto it = models.begin();
    std::advance(it, index);
    
    if(it != models.end())
    {
        return (*it);
    }
    
    return NULL;
}

