#pragma once

#include <TArc/DataProcessing/DataNode.h>

#include <Math/Vector.h>

class QWidget;

class CentralWidgetData : public DAVA::TArc::DataNode
{
public:
    CentralWidgetData(QWidget* renderWidget, QWidget* hRulerWidget, QWidget* vRulerWidget);

    static DAVA::FastName viewSizePropertyName;
    static DAVA::FastName guidesPosPropertyName;
    static DAVA::FastName guidesSizePropertyName;

    DAVA::Vector2 GetViewSize() const;
    DAVA::Vector2 GetGuidesPos() const;
    DAVA::Vector2 GetGuidesSize() const;

private:
    QWidget* renderWidget = nullptr;
    QWidget* hRulerWidget = nullptr;
    QWidget* vRulerWidget = nullptr;

    DAVA_VIRTUAL_REFLECTION(CentralWidgetData, DAVA::TArc::DataNode);
};
