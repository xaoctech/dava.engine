#pragma once

#include <TArc/Controls/PropertyPanel/PropertiesView.h>

namespace DAVA
{
class ContextAccessor;
class UI;

class VisualScriptEditorPropertiesView
{
public:
    static PropertiesView* CreateProperties(ContextAccessor* accessor, UI* ui, std::shared_ptr<PropertiesView::Updater> updater);
};

} // DAVA
