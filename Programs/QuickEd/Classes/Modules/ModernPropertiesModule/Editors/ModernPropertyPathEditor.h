#pragma once

#include "Modules/ModernPropertiesModule/Editors/ModernPropertyDefaultEditor.h"

class QLineEdit;
class QToolButton;
class QHBoxLayout;

class ModernPropertyPathEditor : public ModernPropertyDefaultEditor
{
    Q_OBJECT

public:
    ModernPropertyPathEditor(const std::shared_ptr<ModernPropertyContext>& context, ValueProperty* property,
                             const QList<QString>& extensions, const QString& resourceSubDir, bool allowAnyExtension);
    ~ModernPropertyPathEditor() override;

    void AddToGrid(QGridLayout* layout, int row, int col, int colSpan) override;

protected:
    void OnPropertyChanged() override;
    void ResetProperty() override;

private:
    void OnButtonClicked();
    void OnEditingFinished();
    void OnTextChanged(const QString& text);
    bool IsPathValid(const QString& path) const;
    void ClearError();

    QLineEdit* line = nullptr;
    QToolButton* button = nullptr;

    QList<QString> resourceExtensions;
    QString resourceSubDir;
    bool allowAnyExtension = false;

    QHBoxLayout* layout = nullptr;
};
