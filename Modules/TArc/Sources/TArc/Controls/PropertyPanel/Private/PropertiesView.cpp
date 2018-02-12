#include "TArc/Controls/PropertyPanel/PropertiesView.h"
#include "TArc/Controls/PropertyPanel/Private/ReflectedPropertyModel.h"
#include "TArc/Controls/PropertyPanel/Private/PropertiesViewDelegate.h"
#include "TArc/Controls/PropertyPanel/Private/PropertiesTreeView.h"
#include "TArc/Controls/ComboBox.h"
#include "TArc/Controls/CheckBox.h"
#include "TArc/WindowSubSystem/UI.h"
#include "TArc/Core/ContextAccessor.h"
#include "TArc/Utils/ScopedValueGuard.h"
#include "TArc/Utils/Utils.h"

#include <Reflection/Reflection.h>
#include <Base/BaseTypes.h>

#include <QVBoxLayout>
#include <QHeaderView>
#include <QTimer>
#include <QToolBar>
#include <QAction>
#include <QPersistentModelIndex>
#include <QAction>

namespace DAVA
{
namespace PropertiesViewDetail
{
const char* SeparatorPositionKey = "SeparatorPosition";
const char* isFavoritesViewOnlyKey = "isFavoritesViewOnly";
const int ToolBarHeight = 34;

String RegularTreeDescription(const Any& v)
{
    if (v.Get<bool>() == true)
        return "Regular Tree on";
    else
        return "Regular Tree off";
}

String FavoritesDescription(const Any& v)
{
    if (v.Get<bool>() == true)
        return "Favorites on";
    else
        return "Favorites off";
}

String DeveloperModeDescription(const Any& v)
{
    if (v.Get<bool>() == true)
        return "Developer mode on";
    else
        return "Developer mode off";
}

} // namespace PropertiesViewDetail

DAVA_REFLECTION_IMPL(PropertiesView)
{
    ReflectionRegistrator<PropertiesView>::Begin()
    .Field("regularTreeShown", &PropertiesView::IsRegularTreeShown, &PropertiesView::SetRegularTreeShown)[M::ValueDescription(&PropertiesViewDetail::RegularTreeDescription)]
    .Field("favoritesShown", &PropertiesView::IsFavoritesShown, &PropertiesView::SetFavoritesShown)[M::ValueDescription(&PropertiesViewDetail::FavoritesDescription)]
    .Field("devMode", &PropertiesView::IsInDeveloperMode, &PropertiesView::SetDeveloperMode)[M::ValueDescription(&PropertiesViewDetail::DeveloperModeDescription)]
    .End();
}

PropertiesView::PropertiesView(const Params& params_)
    : binder(params_.accessor)
    , params(params_)
{
    binder.BindField(params.objectsField, MakeFunction(this, &PropertiesView::OnObjectsChanged));
    model.reset(new ReflectedPropertyModel(params.wndKey, params.accessor, params.invoker, params.ui));

    SetupUI();

    std::shared_ptr<Updater> lockedUpdater = params.updater.lock();
    if (lockedUpdater != nullptr)
    {
        lockedUpdater->update.Connect(this, &PropertiesView::Update);
    }

    PropertiesItem viewItem = params.accessor->CreatePropertiesNode(params.settingsNodeName);
    int columnWidth = viewItem.Get(PropertiesViewDetail::SeparatorPositionKey, view->columnWidth(0));
    view->setColumnWidth(0, columnWidth);

    model->LoadState(viewItem);
    if (params.showToolBar == true)
    {
        viewMode = static_cast<eViewMode>(viewItem.Get(PropertiesViewDetail::isFavoritesViewOnlyKey, static_cast<int32>(VIEW_MODE_REGULAR_TREE)));
    }

    QObject::connect(view, &QTreeView::expanded, this, &PropertiesView::OnExpanded);
    QObject::connect(view, &QTreeView::collapsed, this, &PropertiesView::OnCollapsed);
    QObject::connect(view->selectionModel(), &QItemSelectionModel::currentChanged, this, &PropertiesView::OnCurrentChanged);

    model->SetDeveloperMode(params.isInDevMode);
}

PropertiesView::~PropertiesView()
{
    std::shared_ptr<Updater> lockedUpdater = params.updater.lock();
    if (lockedUpdater != nullptr)
    {
        lockedUpdater->update.Disconnect(this);
    }

    PropertiesItem viewSettings = params.accessor->CreatePropertiesNode(params.settingsNodeName);
    viewSettings.Set(PropertiesViewDetail::SeparatorPositionKey, view->columnWidth(0));
    viewSettings.Set(PropertiesViewDetail::isFavoritesViewOnlyKey, static_cast<int32>(viewMode));

    model->SaveState(viewSettings);
}

void PropertiesView::RegisterExtension(const std::shared_ptr<ExtensionChain>& extension)
{
    model->RegisterExtension(extension);
}

void PropertiesView::UnregisterExtension(const std::shared_ptr<ExtensionChain>& extension)
{
    model->UnregisterExtension(extension);
}

void PropertiesView::SetupUI()
{
    // Main layout setup.
    // Toolbar + TreeView
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);
    setLayout(layout);

    if (params.showToolBar == true)
    {
        QToolBar* toolBar = new QToolBar(this);
        toolBar->setFixedHeight(PropertiesViewDetail::ToolBarHeight);
        layout->addWidget(toolBar);

        // Toolbar setup
        QAction* favoriteModeAction = new QAction(toolBar);
        favoriteModeAction->setCheckable(true);
        QIcon icon;
        icon.addFile(QStringLiteral(":/QtIcons/star.png"), QSize(), QIcon::Normal, QIcon::Off);
        favoriteModeAction->setIcon(icon);
        toolBar->addAction(favoriteModeAction);
        connections.AddConnection(favoriteModeAction, &QAction::toggled, MakeFunction(this, &PropertiesView::OnFavoritesEditChanged));

        Reflection thisModel = Reflection::Create(ReflectedObject(this));
        {
            CheckBox::Params controlParams(params.accessor, params.ui, params.wndKey);
            controlParams.fields[CheckBox::Fields::Checked] = "regularTreeShown";
            CheckBox* checkBox = new CheckBox(controlParams, params.accessor, thisModel, toolBar);
            toolBar->addWidget(checkBox->ToWidgetCast());
        }
        {
            CheckBox::Params controlParams(params.accessor, params.ui, params.wndKey);
            controlParams.fields[CheckBox::Fields::Checked] = "favoritesShown";
            CheckBox* checkBox = new CheckBox(controlParams, params.accessor, thisModel, toolBar);
            toolBar->addWidget(checkBox->ToWidgetCast());
        }
        {
            CheckBox::Params controlParams(params.accessor, params.ui, params.wndKey);
            controlParams.fields[CheckBox::Fields::Checked] = "devMode";
            CheckBox* checkBox = new CheckBox(controlParams, params.accessor, thisModel, toolBar);
            toolBar->addWidget(checkBox->ToWidgetCast());
        }
    }

    view = new PropertiesTreeView(this);
    view->setIndentation(view->indentation() >> 1);
    view->setObjectName(QString("%1_propertiesview").arg(QString::fromStdString(params.settingsNodeName)));
    view->setEditTriggers(QAbstractItemView::CurrentChanged | QAbstractItemView::SelectedClicked | QAbstractItemView::EditKeyPressed);
    layout->addWidget(view);

    view->setModel(model.get());
    view->setRootIndex(QModelIndex());
    view->setItemDelegate(new PropertiesViewDelegate(view, model.get(), this));

    QHeaderView* headerView = view->header();
    connections.AddConnection(headerView, &QHeaderView::sectionResized, MakeFunction(this, &PropertiesView::OnColumnResized));
}

void PropertiesView::OnObjectsChanged(const Any& objects)
{
    currentIndexPath.clear();
    view->setRootIndex(QModelIndex());
    if (objects.IsEmpty())
    {
        model->SetObjects(Vector<Reflection>());
    }
    else
    {
        if (objects.CanCast<Vector<Reflection>>())
        {
            model->SetObjects(objects.Cast<Vector<Reflection>>());
        }
        else if (objects.CanCast<Reflection>())
        {
            Vector<Reflection> modelData(1, objects.Cast<Reflection>());
            model->SetObjects(modelData);
        }
        else
        {
            DVASSERT(false);
        }
    }
    UpdateViewRootIndex();
    UpdateExpanded();
}

void PropertiesView::OnColumnResized(int columnIndex, int oldSize, int newSize)
{
    PropertiesViewDelegate* d = qobject_cast<PropertiesViewDelegate*>(view->itemDelegate());
    DVASSERT(d != nullptr);
    d->UpdateSizeHints(columnIndex, newSize);
}

void PropertiesView::Update(UpdatePolicy policy)
{
    ScopedValueGuard<bool> guard(isModelUpdate, true);

    switch (policy)
    {
    case PropertiesView::FullUpdate:
        model->Update();
        break;
    case PropertiesView::FastUpdate:
        model->UpdateFast();
        break;
    default:
        DVASSERT(false, "Unimplemented update policy have been received");
        break;
    }

    UpdateExpanded();
    QModelIndex currentIndex = model->LookIndex(currentIndexPath);
    if (currentIndex.isValid())
    {
        QModelIndex viewCurrent = view->currentIndex();
        if (currentIndex.row() != viewCurrent.row() || currentIndex.internalPointer() != viewCurrent.internalPointer())
        {
            view->setCurrentIndex(currentIndex);
        }
    }
    else
    {
        view->clearSelection();
    }
}

void PropertiesView::UpdateExpanded()
{
    ScopedValueGuard<bool> guard(isExpandUpdate, true);
    QModelIndexList expandedList = model->GetExpandedList(view->rootIndex());
    foreach (const QModelIndex& index, expandedList)
    {
        view->expand(index);
    }
}

void PropertiesView::OnExpanded(const QModelIndex& index)
{
    SCOPED_VALUE_GUARD(bool, isExpandUpdate, true, void());
    model->SetExpanded(true, index);

    QModelIndexList expandedList = model->GetExpandedChildren(index);
    foreach (const QModelIndex& index, expandedList)
    {
        view->expand(index);
    }
}

void PropertiesView::OnCollapsed(const QModelIndex& index)
{
    SCOPED_VALUE_GUARD(bool, isExpandUpdate, true, void());
    model->SetExpanded(false, index);
}

void PropertiesView::OnFavoritesEditChanged(bool isChecked)
{
    view->SetFavoritesEditMode(isChecked);
    UpdateViewRootIndex();
    UpdateExpanded();
}

bool PropertiesView::IsRegularTreeShown() const
{
    return (viewMode & VIEW_MODE_REGULAR_TREE) == VIEW_MODE_REGULAR_TREE;
}

void PropertiesView::SetRegularTreeShown(bool isShown)
{
    if (isShown == true)
        viewMode |= VIEW_MODE_REGULAR_TREE;
    else
        viewMode &= (~VIEW_MODE_REGULAR_TREE);

    UpdateViewRootIndex();
    UpdateExpanded();
}

bool PropertiesView::IsFavoritesShown() const
{
    return (viewMode & VIEW_MODE_FAVORITES) == VIEW_MODE_FAVORITES;
}

void PropertiesView::SetFavoritesShown(bool isShown)
{
    if (isShown == true)
        viewMode |= VIEW_MODE_FAVORITES;
    else
        viewMode &= (~VIEW_MODE_FAVORITES);

    UpdateViewRootIndex();
    UpdateExpanded();
}

bool PropertiesView::IsInDeveloperMode() const
{
    return model->IsDeveloperMode();
}

void PropertiesView::SetDeveloperMode(bool isDevMode)
{
    if (isDevMode == true)
    {
        ModalMessageParams p;
        p.defaultButton = ModalMessageParams::Cancel;
        p.icon = ModalMessageParams::Warning;
        p.title = "Switch to developer mode";
        p.message = "You are trying to switch into developer mode. It can be unsafe.\nAre you sure?";
        if (params.ui->ShowModalMessage(DAVA::mainWindowKey, p) == ModalMessageParams::Cancel)
        {
            return;
        }
    }

    model->SetDeveloperMode(isDevMode);
}

void PropertiesView::UpdateViewRootIndex()
{
    QModelIndex rootIndex;
    bool isFavoritesShown = IsFavoritesShown();
    bool isRegularShown = IsRegularTreeShown();
    if ((isFavoritesShown == true && isRegularShown == true) || view->IsInFavoritesEditMode())
    {
        rootIndex = model->GetRootIndex();
    }
    else if (isFavoritesShown == true)
    {
        rootIndex = model->GetFavoriteRootIndex();
    }
    else if (isRegularShown == true)
    {
        rootIndex = model->GetRegularRootIndex();
    }

    view->setRootIndex(rootIndex);
}

void PropertiesView::OnCurrentChanged(const QModelIndex& index, const QModelIndex& prev)
{
    if (isModelUpdate == false)
    {
        currentIndexPath = model->GetIndexPath(index);
    }
}
} // namespace DAVA
