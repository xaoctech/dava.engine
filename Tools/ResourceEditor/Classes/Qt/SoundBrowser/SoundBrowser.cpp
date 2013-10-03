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

#include "SoundBrowser.h"
#include "ui_soundbrowser.h"

#include <QTreeWidget>
#include <QMessageBox>
#include <QLabel>
#include <QSlider>
#include <QToolTip>

SoundBrowser::SoundBrowser(const DAVA::FilePath & dirPath, DAVA::SoundComponent * editComponent, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SoundBrowser),
    component(editComponent),
    selectedEvent(0)
{
    ui->setupUi(this);

    QList<int> sizes = QList<int>() << 260 << 390;
    ui->splitter->setSizes(sizes);

    ui->playButton->setIcon(QIcon(":/QtIcons/play.png"));
    ui->stopButton->setIcon(QIcon(":/QtIcons/stop.png"));

    ui->playButton->setDisabled(true);
    ui->stopButton->setDisabled(true);
    ui->setButton->setDisabled(true);

    LoadAllFEVsRecursive(dirPath);

    DVASSERT(component);

    DAVA::Vector<DAVA::String> names;
    DAVA::SoundSystem::Instance()->GetAllEventsNames(names);
    FillEventsTree(names, component->GetEventName());

    QObject::connect(ui->treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(OnEventDoubleClicked(QTreeWidgetItem*, int)));
    QObject::connect(ui->treeWidget, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(OnEventSelected(QTreeWidgetItem*, int)));

    QObject::connect(ui->playButton, SIGNAL(clicked()), this, SLOT(OnPlay()));
    QObject::connect(ui->stopButton, SIGNAL(clicked()), this, SLOT(OnStop()));
    QObject::connect(ui->setButton, SIGNAL(clicked()), this, SLOT(OnSetEvent()));
}

SoundBrowser::~SoundBrowser()
{
    SafeRelease(selectedEvent);

    DAVA::SoundSystem::Instance()->UnloadProjects();

    delete ui;
}

void SoundBrowser::OnEventDoubleClicked(QTreeWidgetItem * item, int column)
{
    QVariant data = item->data(0, Qt::UserRole);
    if(!data.isNull())
    {
        selectedEventPath = data.toString().toStdString();
        OnSetEvent();
    }
}

void SoundBrowser::OnEventSelected(QTreeWidgetItem * item, int column)
{
    QVariant data = item->data(0, Qt::UserRole);
    if(!data.isNull())
        EventSelected(data.toString().toStdString());
}

void SoundBrowser::EventSelected(const DAVA::String & eventPath)
{
    if(selectedEventPath == eventPath)
        return;

    SafeRelease(selectedEvent);
    selectedEvent = DAVA::SoundSystem::Instance()->CreateSoundEvent(eventPath);

    selectedEventPath = eventPath;

    FillEventParamsFrame();

    ui->playButton->setDisabled(false);
    ui->stopButton->setDisabled(false);
    ui->setButton->setDisabled(false);
}

void SoundBrowser::FillEventParamsFrame()
{
    ClearParamsFrame();

    ui->eventNameLabel->setText(QString(selectedEventPath.c_str()));

    if(!selectedEvent)
        return;

    DAVA::Vector<DAVA::SoundEvent::SoundEventParameterInfo> params;
    selectedEvent->GetEventParametersInfo(params);
    for(int i = 0; i < params.size(); i++)
    {
        DAVA::SoundEvent::SoundEventParameterInfo & param = params[i];
        AddSliderWidget(param.name, param.minValue, param.maxValue);
    }
}

void SoundBrowser::OnPlay()
{
    if(selectedEvent)
        selectedEvent->Play();
}

void SoundBrowser::OnStop()
{
    if(selectedEvent)
        selectedEvent->Stop();
}

void SoundBrowser::OnSetEvent()
{
    component->SetEventName(selectedEventPath);
    close();
}

void SoundBrowser::OnSliderPressed()
{
    QSlider * slider = dynamic_cast<QSlider *>(QObject::sender());

    DAVA::float32 minValue = slider->property("minValue").toFloat();
    DAVA::float32 maxValue = slider->property("maxValue").toFloat();

    DAVA::float32 curParamValue = slider->value() / 1000.f * (maxValue - minValue) + minValue;

    QToolTip::showText(QCursor::pos(), QString("%1").arg(curParamValue), slider);
}

void SoundBrowser::OnSliderMoved(int value)
{
    QSlider * slider = dynamic_cast<QSlider *>(QObject::sender());

    DAVA::String paramName = slider->property("paramName").toString().toStdString();
    DAVA::float32 minValue = slider->property("minValue").toFloat();
    DAVA::float32 maxValue = slider->property("maxValue").toFloat();

    DAVA::float32 newParamValue = value / 1000.f * (maxValue - minValue) + minValue;

    QToolTip::showText(QCursor::pos(), QString("%1").arg(newParamValue), slider);

    if(selectedEvent)
        selectedEvent->SetParameterValue(paramName, newParamValue);
}

void SoundBrowser::AddSliderWidget( const DAVA::String & paramName, DAVA::float32 minValue, DAVA::float32 maxValue )
{
    QGridLayout * layout = dynamic_cast<QGridLayout *>(ui->paramsFrame->layout());
    
    QLabel * nameLabel = new QLabel(QString(paramName.c_str()));
    QLabel * minLabel = new QLabel(QString(DAVA::Format("%.1f", minValue)));
    minLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    QLabel * maxLabel = new QLabel(QString(DAVA::Format("%.1f", maxValue)));
    QSlider * slider = new QSlider(Qt::Horizontal);
    slider->setMinimum(0);
    slider->setMaximum(1000);
    slider->setValue(0);
    slider->setProperty("paramName", QString(paramName.c_str()));
    slider->setProperty("minValue", minValue);
    slider->setProperty("maxValue", maxValue);
    QObject::connect(slider, SIGNAL(valueChanged(int)), this, SLOT(OnSliderMoved(int)));
    QObject::connect(slider, SIGNAL(sliderPressed()), this, SLOT(OnSliderPressed()));

    layout->addWidget(nameLabel);
    layout->addWidget(minLabel);
    layout->addWidget(slider);
    layout->addWidget(maxLabel);
}

void SoundBrowser::ClearParamsFrame()
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
}

void SoundBrowser::LoadAllFEVsRecursive(const DAVA::FilePath & dirPath)
{
    DVASSERT(dirPath.IsDirectoryPathname());

    DAVA::FileList list(dirPath);
    DAVA::int32 entriesCount = list.GetCount();
    for(DAVA::int32 i = 0; i < entriesCount; i++)
    {
        if(list.IsDirectory(i))
        {
            if(!list.IsNavigationDirectory(i))
                LoadAllFEVsRecursive(list.GetPathname(i));
        }
        else
        {
            const DAVA::FilePath & filePath = list.GetPathname(i);
            
            if(filePath.GetExtension() == ".fev")
                DAVA::SoundSystem::Instance()->LoadFEV(filePath);
        }
    }
}

void SoundBrowser::FillEventsTree(const DAVA::Vector<DAVA::String> & names, const DAVA::String & eventToSelect)
{
    DAVA::int32 eventsCount = names.size();
    for(DAVA::int32 i = 0; i < eventsCount; i++)
    {
        const DAVA::String & eventPath = names[i];

        bool isItemToShow = (eventToSelect == eventPath);

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

                    if(isItemToShow)
                    {
                        findedItem->setSelected(true);
                        EventSelected(eventToSelect);
                    }
                }
                else
                {
                    findedItem->setIcon(0, QIcon(":/QtIcons/sound_group.png"));

                    if(isItemToShow)
                        findedItem->setExpanded(true);
                }
            }
            currentItem = findedItem;
        }
    }
}