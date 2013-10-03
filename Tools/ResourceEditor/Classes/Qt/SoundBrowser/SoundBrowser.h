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

#ifndef __SOUND_BROWSER_H__
#define __SOUND_BROWSER_H__

#include <QDialog>
#include <QTreeWidgetItem>
#include "DAVAEngine.h"

namespace Ui {
class SoundBrowser;
}

class SoundBrowser : public QDialog
{
    Q_OBJECT
    
public:
    explicit SoundBrowser(const DAVA::FilePath & dirPath, DAVA::SoundComponent * editComponent, QWidget *parent = 0);
    ~SoundBrowser();
    
private slots:
    void OnEventDoubleClicked(QTreeWidgetItem * item, int column);
    void OnEventSelected(QTreeWidgetItem * item, int column);

    void OnSliderMoved(int value);
    void OnSliderPressed();

    void OnPlay();
    void OnStop();
    void OnSetEvent();

private:
    struct ParamSliderData : public QObjectUserData
    {
        DAVA::String paramName;
        DAVA::float32 minValue;
        DAVA::float32 maxValue;
    };

    void LoadAllFEVsRecursive(const DAVA::FilePath & dirPath);
    void FillEventsTree(const DAVA::Vector<DAVA::String> & names, const DAVA::String & eventToSelect);

    void AddSliderWidget(const DAVA::String & paramName, DAVA::float32 minValue, DAVA::float32 maxValue);
    void ClearParamsFrame();

    void EventSelected(const DAVA::String & eventPath);
    void FillEventParamsFrame();

    DAVA::SoundComponent * component;
    DAVA::SoundEvent * selectedEvent;
    DAVA::String selectedEventPath;

    Ui::SoundBrowser *ui;
};

#endif // SOUNDBROWSER_H
