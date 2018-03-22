#pragma once

#include <UI/Styles/UIStyleSheetPropertyDataBase.h>

#include <Base/RefPtr.h>
#include <QWidget>
#include <QLabel>
#include <QGridLayout>

class AbstractProperty;
class ValueProperty;
class RootProperty;

namespace DAVA
{
class Any;
class UIControl;
class ContextAccessor;
}

class ModernPropertyContext final
{
public:
    ModernPropertyContext(RootProperty* root, DAVA::ContextAccessor* accessor, QWidget* parent);
    ~ModernPropertyContext();

    RootProperty* GetRoot() const;
    DAVA::ContextAccessor* GetAccessor() const;
    QWidget* GetParent() const;

private:
    DAVA::RefPtr<RootProperty> root;
    DAVA::ContextAccessor* accessor = nullptr;
    QWidget* parent = nullptr;
};

class ModernPropertyEditor : public QObject
{
    Q_OBJECT
public:
    ModernPropertyEditor(const std::shared_ptr<ModernPropertyContext>& context, ValueProperty* property);
    virtual ~ModernPropertyEditor();

    virtual void AddToGrid(QGridLayout* layout, int row, int col, int colSpan) = 0;

protected:
    void ChangeProperty(const DAVA::Any& value);
    void ApplyStyleToWidget(QWidget* widget);
    virtual void OnPropertyChanged();
    bool eventFilter(QObject* o, QEvent* e);

    RootProperty* GetRootProperty() const;
    DAVA::ContextAccessor* GetAccessor() const;
    QWidget* GetParentWidget() const;

    virtual void ResetProperty();

    DAVA::RefPtr<ValueProperty> property;
    QAction* resetAction = nullptr;
    QAction* forceOverrideAction = nullptr;
    QLabel* propertyName = nullptr;

    bool overriden = false;
    bool setByStyle = false;
    bool inherited = false;

private:
    std::shared_ptr<ModernPropertyContext> context;

    void PropertyChanged(AbstractProperty* property);
    void OnStylePropertiesChanged(DAVA::UIControl* control, const DAVA::UIStyleSheetPropertySet& properties);
    void OnControlLayouted(DAVA::UIControl* control);

    void ForceOverride();

    bool isLayoutSensitive = false;
};
