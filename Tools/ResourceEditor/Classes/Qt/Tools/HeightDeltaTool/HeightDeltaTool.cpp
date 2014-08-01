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

HeightDeltaTool::HeightDeltaTool(QDoubleSpinBox* tb)
{
    thresholdBox = tb;
}

void HeightDeltaTool::SetThreshold(double thresholdValue)
{
    thresholdBox->setValue(thresholdValue);
}

double HeightDeltaTool::GetThreshold() const
{
    return thresholdBox->value();
}

double HeightDeltaTool::GetThresholdInMeters(double unitSize)
{
    double angle = GetThreshold();
    DAVA::float32 radAngle = DAVA::DegToRad((DAVA::float32)angle);
    
    double tangens = tan(radAngle);
    
    double delta = DAVA::Abs(unitSize * (DAVA::float32)tangens);
    return delta;
}

bool HeightDeltaTool::GenerateHeightDeltaImage(const QString& srcPath,
                                               DAVA::Landscape* landscape,
                                               DAVA::FilePath& outResultPath)
{
    QImageReader imageReader(srcPath);
    
    const DAVA::AABBox3& bbox = landscape->GetBoundingBox();
    DAVA::Heightmap* heightmap = landscape->GetHeightmap();
    
    if(heightmap != NULL)
    {
        double unitSize = (bbox.max.x - bbox.min.x) / heightmap->Size();
        
        QSize imageSize = imageReader.size();
        double threshold = GetThresholdInMeters(unitSize);
        
        DAVA::FilePath dstImagePath = srcPath.toStdString();
        DAVA::String baseName = dstImagePath.GetBasename();
        DAVA::String extension = dstImagePath.GetExtension();
        
        baseName += "_heightmask";
        
        dstImagePath = dstImagePath.GetDirectory() + baseName + extension;
        
        PaintHeightDeltaAction* action = new PaintHeightDeltaAction(dstImagePath,
                                                                    (DAVA::float32)threshold,
                                                                    heightmap,
                                                                    imageSize.width(),
                                                                    imageSize.height(),
                                                                    bbox.max.z - bbox.min.z);
        
        action->Redo();
        
        DAVA::SafeDelete(action);
        
        outResultPath = dstImagePath;
    }

    return (heightmap != NULL);
}