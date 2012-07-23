#ifndef __GUI_ACTION_MANAGER_H__
#define __GUI_ACTION_MANAGER_H__

#include <QObject>
#include "DAVAEngine.h"

class GUIActionHandler: public QObject
{
    Q_OBJECT
    
public:
    GUIActionHandler(QObject *parent = 0);
    virtual ~GUIActionHandler();
    

public slots:
    //menu

    //File
    void NewScene();
    void OpenScene();
    void OpenProject();
    void OpenResentScene(DAVA::int32 index);
    void SaveScene();
    void ExportAsPNG();
    void ExportAsPVR();
    void ExportAsDXT();

    //create node
    void CreateLandscape();
    void CreateLight();
    void CreateServiceNode();
    void CreateBox();
    void CreateSphere();
    void CreateCamera();
    void CreateImposter();
    void CreateUserNode();
    
    //tools
    void Materials();
    void ConvertTextures();
    void HeightmapEditor();
    void TilemapEditor();
    
    //viewport
    void ViewportiPhone();
    void VeiwportRetina();
    void ViewportiPad();
    void ViewportDefault();
    
    
};

#endif // __GUI_ACTION_MANAGER_H__
