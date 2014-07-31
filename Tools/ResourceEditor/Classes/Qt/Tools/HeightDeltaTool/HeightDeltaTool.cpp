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

#include "HeightDeltaTool.h"
#include "Commands2/PaintHeightDeltaAction.h"
#include "Qt/MAin/QtUtils.h"

#include <QImageReader>

HeightDeltaTool::HeightDeltaTool(QDoubleSpinBox* tb,
                QToolButton* cb)
{
    thresholdBox = tb;
    colorButton = cb;
}

void HeightDeltaTool::SetSelectedColor(const QColor& color)
{
    selectedColor = color;
    
    QString styleSheet = QString("background:%1").arg(selectedColor.name());
    colorButton->setStyleSheet(styleSheet);
}

const QColor& HeightDeltaTool::GetSelectedColor()
{
    return selectedColor;
}

void HeightDeltaTool::SetThreshold(double thresholdValue)
{
    thresholdBox->setValue(thresholdValue);
}

double HeightDeltaTool::GetThreshold() const
{
    return thresholdBox->value();
}

bool HeightDeltaTool::GenerateHeightDeltaImage(const QString& srcPath,
                                               DAVA::Landscape* landscape)
{
    QImageReader imageReader(srcPath);
    
    QSize imageSize = imageReader.size();
    QColor selectedColor = GetSelectedColor();
    double threshold = GetThreshold();
    
    DAVA::FilePath dstImagePath = srcPath.toStdString();
    DAVA::String baseName = dstImagePath.GetBasename();
    DAVA::String extension = dstImagePath.GetExtension();
    
    QString colorName = selectedColor.name();
    colorName = colorName.replace("#", "");
    colorName.prepend("_");
    
    baseName += colorName.toStdString();
    
    dstImagePath = dstImagePath.GetDirectory() + baseName + extension;
    
    const DAVA::AABBox3& bbox = landscape->GetBoundingBox();
    PaintHeightDeltaAction* action = new PaintHeightDeltaAction(dstImagePath,
                                                                QColorToColor(selectedColor),
                                                                (DAVA::float32)threshold,
                                                                landscape->GetHeightmap(),
                                                                imageSize.width(),
                                                                imageSize.height(),
                                                                bbox.max.z - bbox.min.z);
    
    action->Redo();
    
    DAVA::SafeDelete(action);

    return true;
}