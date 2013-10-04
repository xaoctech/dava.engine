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

#include "FMODSoundBrowser.h"
#include "Qt/Project/ProjectManager.h"
#include "ui_soundbrowser.h"

#include <QTreeWidget>
#include <QMessageBox>
#include <QLabel>
#include <QSlider>
#include <QToolTip>

FMODSoundBrowser::FMODSoundBrowser(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FMODSoundBrowser),
    component(0)
{
    ui->setupUi(this);

    QList<int> sizes = QList<int>() << 260 << 390;
    ui->splitter->setSizes(sizes);

    ui->playButton->setIcon(QIcon(":/QtIcons/play.png"));
    ui->stopButton->setIcon(QIcon(":/QtIcons/stop.png"));

    ui->playButton->setDisabled(true);
    ui->stopButton->setDisabled(true);

    QObject::connect(ui->treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(OnEventSelected(QTreeWidgetItem*, int)));

    QObject::connect(ui->playButton, SIGNAL(clicked()), this, SLOT(OnPlay()));
    QObject::connect(ui->stopButton, SIGNAL(clicked()), this, SLOT(OnStop()));

    QObject::connect(ProjectManager::Instance(), SIGNAL(ProjectOpened(const QString &)), this, SLOT(OnProjectOpened(const QString &)));
    QObject::connect(ProjectManager::Instance(), SIGNAL(ProjectClosed()), this, SLOT(OnProjectClosed()));

    setModal(true);
}

FMODSoundBrowser::~FMODSoundBrowser()
{
    DAVA::FMODSoundSystem::GetFMODSoundSystem()->UnloadProjects();

    delete ui;
}

void FMODSoundBrowser::SetEditableComponent(DAVA::FMODSoundComponent * editComponent)
{
    component = editComponent;
    DVASSERT(component);

    ui->treeWidget->collapseAll();
    ui->treeWidget->clearSelection();
    
    SelectItemAndExpandTreeByEventName(component->GetEventName());

    EventSelected(component->GetEventName());
}

void FMODSoundBrowser::OnProjectOpened(const QString & projectPath)
{
    DAVA::FMODSoundSystem * soundsystem = DAVA::FMODSoundSystem::GetFMODSoundSystem();
    
    soundsystem->UnloadProjects();

    DAVA::FilePath sfxPath = projectPath.toStdString() + "/Data/Sfx/";
    soundsystem->LoadAllFEVsRecursive(sfxPath);

    DAVA::Vector<DAVA::String> names;
    soundsystem->GetAllEventsNames(names);

    FillEventsTree(names);
}

void FMODSoundBrowser::OnProjectClosed()
{
    DAVA::FMODSoundSystem::GetFMODSoundSystem()->UnloadProjects();

    ui->treeWidget->clear();
}

void FMODSoundBrowser::OnEventSelected(QTreeWidgetItem * item, int column)
{
    QVariant data = item->data(0, Qt::UserRole);
    if(!data.isNull())
        EventSelected(data.toString().toStdString());
}

void FMODSoundBrowser::EventSelected(const DAVA::String & eventPath)
{
    ClearParamsFrame();

    if(component && eventPath != "")
    {
        component->SetEventName(eventPath);

        FillEventParamsFrame();
    }
}

void FMODSoundBrowser::FillEventParamsFrame()
{
    if(!component)
        return;

    ui->eventNameLabel->setText(QString(component->GetEventName().c_str()));

    ui->playButton->setDisabled(false);
    ui->stopButton->setDisabled(false);

    DAVA::Vector<DAVA::FMODSoundComponent::SoundEventParameterInfo> params;
    component->GetEventParametersInfo(params);
    DAVA::int32 paramsCount = params.size();
    for(DAVA::int32 i = 0; i < paramsCount; i++)
    {
        DAVA::FMODSoundComponent::SoundEventParameterInfo & param = params[i];
        if(param.name != "(distance)")
            AddSliderWidget(param);
    }
}

void FMODSoundBrowser::OnPlay()
{
    component->Trigger();
}

void FMODSoundBrowser::OnStop()
{
    component->Stop();
}

void FMODSoundBrowser::OnSliderPressed()
{
    QSlider * slider = dynamic_cast<QSlider *>(QObject::sender());

    DAVA::float32 minValue = slider->property("minValue").toFloat();
    DAVA::float32 maxValue = slider->property("maxValue").toFloat();

    DAVA::float32 curParamValue = slider->value() / 1000.f * (maxValue - minValue) + minValue;

    QToolTip::showText(QCursor::pos(), QString("%1").arg(curParamValue), slider);
}

void FMODSoundBrowser::OnSliderMoved(int value)
{
    QSlider * slider = dynamic_cast<QSlider *>(QObject::sender());

    DAVA::String paramName = slider->property("paramName").toString().toStdString();
    DAVA::float32 minValue = slider->property("minValue").toFloat();
    DAVA::float32 maxValue = slider->property("maxValue").toFloat();

    DAVA::float32 newParamValue = value / 1000.f * (maxValue - minValue) + minValue;

    QToolTip::showText(QCursor::pos(), QString("%1").arg(newParamValue), slider);

    if(component)
        component->SetParameter(paramName, newParamValue);
}

void FMODSoundBrowser::AddSliderWidget(const DAVA::FMODSoundComponent::SoundEventParameterInfo & param)
{
    QGridLayout * layout = dynamic_cast<QGridLayout *>(ui->paramsFrame->layout());
    
    QLabel * nameLabel = new QLabel(QString(param.name.c_str()));
    QLabel * minLabel = new QLabel(QString(DAVA::Format("%.1f", param.minValue)));
    minLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    QLabel * maxLabel = new QLabel(QString(DAVA::Format("%.1f", param.maxValue)));
    QSlider * slider = new QSlider(Qt::Horizontal);
    slider->setMinimum(0);
    slider->setMaximum(1000);
    slider->setValue(0);
    slider->setProperty("paramName", QString(param.name.c_str()));
    slider->setProperty("minValue", param.minValue);
    slider->setProperty("maxValue", param.maxValue);

    if(component)
    {
        int currentValue = (param.currentValue - param.minValue) / (param.maxValue - param.minValue) * 1000;
        slider->setValue(currentValue);
    }

    QObject::connect(slider, SIGNAL(valueChanged(int)), this, SLOT(OnSliderMoved(int)));
    QObject::connect(slider, SIGNAL(sliderPressed()), this, SLOT(OnSliderPressed()));

    layout->addWidget(nameLabel);
    layout->addWidget(minLabel);
    layout->addWidget(slider);
    layout->addWidget(maxLabel);
}

void FMODSoundBrowser::ClearParamsFrame()
{
    QGridLayout * layout = dynamic_cast<QGridLayout *>(ui->paramsFrame->layout());

    QLayoutItem* item = 0;
    while((item = layout->takeAt(0)))
    {
        delete item->widget();
        delete item;
    }

    layout->setColumnStretch(0, 3);
    layout->setColumnStretch(1, 1);
    layout->setColumnStretch(2, 8);
    layout->setColumnStretch(3, 1);

    ui->playButton->setDisabled(true);
    ui->stopButton->setDisabled(true);

    ui->eventNameLabel->clear();
}

void FMODSoundBrowser::SelectItemAndExpandTreeByEventName(const DAVA::String & eventName)
{
    DAVA::Vector<DAVA::String> tokens;
    DAVA::Split(eventName, "/", tokens);
    DAVA::int32 tokensCount = tokens.size();
    QTreeWidgetItem * currentItem = ui->treeWidget->invisibleRootItem();
    for(DAVA::int32 i = 0; i < tokensCount; i++)
    {
        QString currentToken = QString(tokens[i].c_str());
        DAVA::int32 childrenCount = currentItem->childCount();
        QTreeWidgetItem * findedItem = 0;
        for(DAVA::int32 k = 0; k < childrenCount; k++)
        {
            QTreeWidgetItem * currentChild = currentItem->child(k);
            if(currentChild->text(0) == currentToken)
            {
                findedItem = currentChild;
                findedItem->setExpanded(true);
                break;
            }
        }
        if(!findedItem)
            return;

        currentItem = findedItem;
    }
    currentItem->setSelected(true);
}

void FMODSoundBrowser::FillEventsTree(const DAVA::Vector<DAVA::String> & names)
{
    ui->treeWidget->clear();

    DAVA::int32 eventsCount = names.size();
    for(DAVA::int32 i = 0; i < eventsCount; i++)
    {
        const DAVA::String & eventPath = names[i];

        DAVA::Vector<DAVA::String> tokens;
        DAVA::Split(eventPath, "/", tokens);

        DAVA::int32 tokensCount = tokens.size();
        QTreeWidgetItem * currentItem = ui->treeWidget->invisibleRootItem();
        for(DAVA::int32 j = 0; j < tokensCount; j++)
        {
            QString currentToken = QString(tokens[j].c_str());
            DAVA::int32 childrenCount = currentItem->childCount();
            QTreeWidgetItem * findedItem = 0;
            for(DAVA::int32 k = 0; k < childrenCount; k++)
            {
                QTreeWidgetItem * currentChild = currentItem->child(k);
                if(currentChild->text(0) == currentToken)
                {
                    findedItem = currentChild;
                    break;
                }
            }

            bool isEvent = (j == tokensCount-1);

            if(findedItem == 0)
            {
                findedItem = new QTreeWidgetItem(currentItem);
                currentItem->addChild(findedItem);
                findedItem->setText(0, currentToken);

                if(isEvent)
                {
                    findedItem->setIcon(0, QIcon(":/QtIcons/sound.png"));
                    findedItem->setData(0, Qt::UserRole, QString(eventPath.c_str()));
                }
                else
                {
                    findedItem->setIcon(0, QIcon(":/QtIcons/sound_group.png"));
                }
            }
            currentItem = findedItem;
        }
    }
}