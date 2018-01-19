#pragma once

#include "SelectPathWidgetBase.h"

#include <QWidget>
#include <QMimeData>
#include <QLineEdit>
#include <QToolButton>

#include "DAVAEngine.h"
namespace DAVA
{
class SceneEditor2;
} // namespace DAVA

class SelectEntityPathWidget : public SelectPathWidgetBase
{
    Q_OBJECT

public:
    explicit SelectEntityPathWidget(QWidget* parent = 0, DAVA::String openDialoDefualtPath = "", DAVA::String relativPath = "");

    ~SelectEntityPathWidget();

    DAVA::Entity* GetOutputEntity(DAVA::SceneEditor2*);

protected:
    void dragEnterEvent(QDragEnterEvent* event);

    DAVA::Entity* ConvertQMimeDataFromFilePath(DAVA::SceneEditor2* sceneEditor = NULL);
    DAVA::Entity* ConvertFromMimeData(DAVA::SceneEditor2* sceneEditor);

    void SetEntities(DAVA::Entity* entity, bool perfromRetain);

    DAVA::List<DAVA::Entity*> entitiesToHold;
};
