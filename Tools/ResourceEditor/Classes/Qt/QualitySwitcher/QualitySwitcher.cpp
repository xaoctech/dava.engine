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

#include "QualitySwitcher.h"
#include "Project/ProjectManager.h"

#include <QComboBox>
#include <QPushButton>
#include <QGridLayout>
#include <QLabel>

QualitySwitcher::QualitySwitcher(QWidget *parent /* = NULL */)
: QDialog(parent, Qt::Popup | Qt::FramelessWindowHint)
, defBtn(NULL)
{
    int i = 0;
    int height = 10;
    const int spacing = 5;

    QGridLayout *gridLay = new QGridLayout();

    const QVector<ProjectManager::AvailableMaterialQuality>* qualities = ProjectManager::Instance()->GetAvailableMaterialQualities();
    if(NULL != qualities)
    {
        for(; i < qualities->size(); ++i)
        {
            const ProjectManager::AvailableMaterialQuality &qual = qualities->at(i);

            if(qual.values.size() > 0)
            {
                QLabel *lab = new QLabel(qual.name + ":", this);
                QComboBox *combo = new QComboBox(this);

                gridLay->addWidget(lab, i, 0, Qt::AlignRight);
                gridLay->addWidget(combo, i, 1);

                height += combo->geometry().height() + spacing;

                // TODO: get current value

                for(int j = 0; j < qual.values.size(); ++j)
                {
                    QString qualityValue = qual.prefix + qual.values[j];
                    combo->addItem(qual.values[j], qualityValue);
                }
            }
        }
    }

    defBtn = new QPushButton("Ok", this);
    gridLay->addWidget(defBtn, i, 1);

    gridLay->setSpacing(spacing);
    gridLay->setMargin(5);

    setLayout(gridLay);
    adjustSize();

    setAttribute(Qt::WA_DeleteOnClose, true);
}

QualitySwitcher::~QualitySwitcher()
{ }

void QualitySwitcher::Show()
{
    QualitySwitcher *sw = new QualitySwitcher(NULL);
    sw->show();
}