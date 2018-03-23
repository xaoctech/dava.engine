#include "REPlatform/Global/MenuScheme.h"

#include <TArc/WindowSubSystem/ActionUtils.h>

#include <Base/StaticSingleton.h>
#include <Base/BaseTypes.h>

#include <QList>
#include <QAction>

#include <algorithm>

namespace DAVA
{
const QString ProjectMenuName = "Project";
const QString SceneMenuName = "Scene";

namespace MenuSchemeDetails
{
class MenuBuilder : public DAVA::StaticSingleton<MenuBuilder>
{
public:
    MenuBuilder()
    {
        Emplace(ProjectMenuName);
        Emplace(SceneMenuName);
    }

    void Emplace(const QString& menuName, int32 pos = -1)
    {
        static uint32 counter = 0;
        uint32 insertPos = static_cast<uint32>(pos);
        if (pos == -1)
        {
            insertPos = counter++;
        }

        DVASSERT(std::find(positions.begin(), positions.end(), menuName) == positions.end());

        auto insertIter = positions.begin();
        if (insertPos >= positions.size())
        {
            insertIter = positions.end();
        }
        else
        {
            std::advance(insertIter, insertPos);
        }

        positions.insert(insertIter, menuName);
    }

    QUrl CreateMenuPoint(const QString menuName) const
    {
        auto iter = std::find(positions.begin(), positions.end(), menuName);
        DVASSERT(iter != positions.end());

        if (iter == positions.begin())
        {
            return DAVA::CreateMenuPoint("", InsertionParams(InsertionParams::eInsertionMethod::BeforeItem));
        }

        auto nextIter = std::next(iter);
        if (nextIter == positions.end())
        {
            return DAVA::CreateMenuPoint("", InsertionParams(InsertionParams::eInsertionMethod::AfterItem));
        }

        return DAVA::CreateMenuPoint("", InsertionParams(InsertionParams::eInsertionMethod::BeforeItem, *nextIter));
    }

private:
    Vector<QString> positions;
};
} // namespace MenuSchemeDetails

QUrl CreateREMenuPoint(const QString& menuName)
{
    using namespace MenuSchemeDetails;

    MenuBuilder* builder = MenuBuilder::Instance();
    return builder->CreateMenuPoint(menuName);
}

void BuildMenuPrositionsFromLegacy(QMenuBar* menuBar)
{
    using namespace MenuSchemeDetails;

    MenuBuilder* builder = MenuBuilder::Instance();
    QList<QAction*> actions = menuBar->actions();
    foreach (QAction* action, actions)
    {
        builder->Emplace(action->text());
    }
}

} // namespace DAVA
