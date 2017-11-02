#ifndef __RESOURCEEDITORQT__SELECENTITYTPATHWIDGET__
#define __RESOURCEEDITORQT__SELECENTITYTPATHWIDGET__

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

    void ConvertQMimeDataFromSceneTree(const QMimeData* mimeData, DAVA::List<DAVA::Entity*>&);

    void ConvertQMimeDataFromFilePath(const QMimeData* mimeData, DAVA::List<DAVA::Entity*>&,
                                      DAVA::SceneEditor2* sceneEditor = NULL);

    void ConvertFromMimeData(const QMimeData* mimeData, DAVA::List<DAVA::Entity*>& retList, DAVA::SceneEditor2* sceneEditor);

    void SetEntities(const DAVA::List<DAVA::Entity*>& list, bool perfromRertain);

    DAVA::List<DAVA::Entity*> entitiesToHold;
};

#endif /* defined(__RESOURCEEDITORQT__SELECENTITYTPATHWIDGET__) */