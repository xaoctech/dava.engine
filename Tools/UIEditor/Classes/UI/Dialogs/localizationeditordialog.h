#ifndef LOCALIZATIONEDITORDIALOG_H
#define LOCALIZATIONEDITORDIALOG_H

#include <QDialog>

namespace Ui {
class LocalizationEditorDialog;
}
    
class LocalizationEditorDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit LocalizationEditorDialog(QWidget *parent = 0);
    ~LocalizationEditorDialog();
    
private:
    Ui::LocalizationEditorDialog *ui;
    void FillLocaleComboBox();
    void ConnectToSignals();
    void SetLocalizationDirectoryPath();
    void SetDefaultLanguage();

    void ReinitializeLocalizationSystem(const QString& localizationDirectory);

private slots:
    void OnOpenLocalizationFileButtonClicked();
    void OnCurrentLocaleChanged(int index);
    
};

#endif // LOCALIZATIONEDITORDIALOG_H
