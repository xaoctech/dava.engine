#pragma once

#include "EditorSystems/BaseEditorSystem.h"
#include "EditorSystems/EditorSystemsManager.h"

#include <TArc/Qt/QtString.h>

#include <QMap>
#include <QPixmap>
#include <QCursor>

class CursorSystem final : public BaseEditorSystem
{
public:
    explicit CursorSystem(DAVA::ContextAccessor* accessor);
    ~CursorSystem() override = default;

private:
    void OnDragStateChanged(EditorSystemsManager::eDragState currentState, EditorSystemsManager::eDragState previousState) override;
    eSystems GetOrder() const override;

    void OnActiveAreaChanged(const HUDAreaInfo& areaInfo);

    QPixmap CreatePixmapForArea(float angle, const HUDAreaInfo::eArea area) const;
    QPixmap CreatePixmap(const QString& address) const;

    static QMap<QString, QPixmap> cursorpixes;
};
