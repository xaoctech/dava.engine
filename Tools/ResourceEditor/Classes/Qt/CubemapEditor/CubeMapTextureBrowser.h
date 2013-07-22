#ifndef CUBEMAPTEXTUREBROWSER_H
#define CUBEMAPTEXTUREBROWSER_H

#include <QDialog>
#include <QListWidget>

#include "CubemapEditor/CubeListItemDelegate.h"
#include "Base/BaseTypes.h"

namespace Ui {
class CubeMapTextureBrowser;
}

class CubeMapTextureBrowser : public QDialog
{
    Q_OBJECT
    
public:
    explicit CubeMapTextureBrowser(QWidget *parent = 0);
    ~CubeMapTextureBrowser();
	
protected:
	
	CubeListItemDelegate cubeListItemDelegate;
	
protected:
	
	void ReloadTexturesFromUI(QString& path);
	void ReloadTextures(const DAVA::String& rootPath);
	void ConnectSignals();
	void RestoreListSelection(int currentRow);
	
protected slots:
	
	void OnChooseDirectoryClicked();
	void OnReloadClicked();
	void OnCreateCubemapClicked();
	void OnListItemDoubleClicked(QListWidgetItem* item);
    
private:
    Ui::CubeMapTextureBrowser *ui;
};

#endif // CUBEMAPTEXTUREBROWSER_H
