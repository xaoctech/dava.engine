#pragma once

#include <TArc/Core/ClientModule.h>

#include <Render/Highlevel/LandscapeSubdivision.h>
#include <Render/Highlevel/DecorationData.h>
#include <Reflection/Reflection.h>
#include <Base/BaseTypes.h>

#include <QWidget>
#include <QModelIndex>
#include <QItemDelegate>

namespace DAVA
{
class Any;
class FieldBinder;
class SceneEditor2;
}

class QGroupBox;
class QSpinBox;
class QListView;
class QStandardItemModel;
class QItemSelection;
class QTableView;
class QCheckBox;
class QDoubleSpinBox;
class QStandardItem;
class QLabel;

class FilePathBrowser;

class DecorationEditor : public QWidget
{
    Q_OBJECT

public:
    explicit DecorationEditor(DAVA::ContextAccessor* contextAccessor, DAVA::UI* ui, QWidget* parent = nullptr);
    ~DecorationEditor() override = default;

private:
    void OnSelectionChanged(const DAVA::Any& selection);

    DAVA::FilePath GetDecorationPath() const;
    void SetDecorationPath(const DAVA::FilePath& decorationPath);

    bool HasDecorationData() const;

private slots:
    void OnLayerSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
    void OnVariationSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

    void OnSpinBoxChanged(int value);
    void OnCheckBoxChanged(int state);
    void OnDoubleSpinBoxChanged(double value);
    void OnDensityTableChanged(const QModelIndex& topLeft, const QModelIndex& bottomRight, const QVector<int>& roles);

private:
    void SetupUI();

    void MarkSceneChanged();

    void UpdateDecorationSettings();
    void UpdateLayerSettings();
    void UpdateVariationSettings();
    void UpdateLevelDistances();

    DAVA::DecorationData* decorationData = nullptr;
    DAVA::LandscapeSubdivision* subdivision = nullptr;

    std::unique_ptr<DAVA::FieldBinder> selectionFieldBinder;
    DAVA::ContextAccessor* contextAccessor = nullptr;
    DAVA::UI* ui = nullptr;

    //UI elements
    QWidget* editorContent = nullptr;
    QGroupBox* decorationSettingsWidget = nullptr;
    QGroupBox* layerSettingsWidget = nullptr;
    QGroupBox* variationSettingsWidget = nullptr;

    FilePathBrowser* pathBrowser = nullptr;
    QSpinBox* baseLevelEdit = nullptr;
    QSpinBox* levelCountEdit = nullptr;

    QLabel* maxDistanceLabel = nullptr;

    QListView* layerListWidget = nullptr;
    QListView* variationListWidget = nullptr;
    QTableView* densityTableWidget = nullptr;

    QStandardItemModel* layerListModel = nullptr;
    QStandardItemModel* variationListModel = nullptr;
    QStandardItemModel* densityTableModel = nullptr;

    QCheckBox* maskCheckBox[4];
    QCheckBox* cullfaceCheckBox = nullptr;
    QCheckBox* orientCheckbox = nullptr;
    QCheckBox* tintCheckBox = nullptr;
    QCheckBox* collisionCheckBox = nullptr;
    QCheckBox* enabledCheckBox = nullptr;
    QDoubleSpinBox* orientEdit = nullptr;
    QDoubleSpinBox* tintHeightEdit = nullptr;
    QDoubleSpinBox* scaleMinEdit = nullptr;
    QDoubleSpinBox* scaleMaxEdit = nullptr;
    QDoubleSpinBox* pitchMaxEdit = nullptr;
    QSpinBox* collisionGroupEdit = nullptr;
    QDoubleSpinBox* collisionRadiusEdit = nullptr;
    QDoubleSpinBox* densityEdit = nullptr;

    QModelIndexList selectedLayer;
    QModelIndexList selectedVariations;

    DAVA::Vector<DAVA::float32> levelDistances;

    bool uiUpdateState = false;

    DAVA_REFLECTION(DecorationEditor);
};

//////////////////////////////////////////////////////////////////////////

class DensityTableItemDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    explicit DensityTableItemDelegate(QObject* parent = 0);

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
    void setEditorData(QWidget* editor, const QModelIndex& index) const override;
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override;
    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
};

//////////////////////////////////////////////////////////////////////////

class DecorationEditorModule : public DAVA::ClientModule
{
protected:
    void PostInit() override;

    DAVA_VIRTUAL_REFLECTION(LibraryModule, DAVA::ClientModule);
};
