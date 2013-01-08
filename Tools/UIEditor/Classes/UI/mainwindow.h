#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ScreenWrapper.h"
#include "EditorSettings.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
protected:
	virtual void resizeEvent(QResizeEvent *);
	virtual void showEvent(QShowEvent * event);

private slots:
    void on_scaleSpin_valueChanged(double arg1);
	void OnSliderMoved();
    void on_scaleSlider_valueChanged(int value);
    void OnOpenFontManager();
    void OnOpenLocalizationManager();
    void OnShowHelpContents();
	
	void OnNewProject();
	void OnSaveProject();
    void OnOpenProject();
	void OnCloseProject();
	void OnNewPlatform();
	void OnNewScreen(HierarchyTreeNode::HIERARCHYTREENODEID id = HierarchyTreeNode::HIERARCHYTREENODEID_EMPTY);
	
	void OnProjectCreated();
	void OnSelectedScreenChanged();
	
	void OnUpdateScaleRequest(float scaleDelta);
	void OnUpdateScreenPositionRequest(const QPoint& posDelta);
	
	void FileMenuTriggered(QAction *resentScene);
	void MenuFileWillShow();

	void OnUndoRequested();
	void OnRedoRequested();
	
	void OnUndoRedoAvailabilityChanged();

private:
	bool CloseProject();
	
	void UpdateSliders();
	void UpdateScreenPosition();
	
	void InitMenu();
	void UpdateMenu();
	void UpdateProjectSettings(const QString& filename);

private:
    Ui::MainWindow *ui;
	QAction *recentPojectActions[EditorSettings::RECENT_FILES_COUNT];
	
	bool screenChangeUpdate;
};

#endif // MAINWINDOW_H
