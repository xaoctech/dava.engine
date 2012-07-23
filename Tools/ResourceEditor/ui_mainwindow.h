/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created: Mon Jul 23 22:30:28 2012
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QMainWindow>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>
#include "davaglwidget.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionNewScene;
    QAction *actionOpenScene;
    QAction *actionSaveScene;
    QAction *actionOpenProject;
    QAction *actionPNG;
    QAction *actionPVR;
    QAction *actionDXT;
    QAction *actionLandscape;
    QAction *actionLight;
    QAction *actionServiceNode;
    QAction *actionBox;
    QAction *actionSphere;
    QAction *actionCamera;
    QAction *actionImposter;
    QAction *actionUserNode;
    QAction *actionMaterialEditor;
    QAction *actionHeightMapEditor;
    QAction *actionTileMapEditor;
    QAction *actionTextureConverter;
    QAction *actionIPhone;
    QAction *actionRetina;
    QAction *actionIPad;
    QAction *actionDefault;
    QAction *actionParticleEmitter;
    QWidget *centralWidget;
    QVBoxLayout *verticalLayout;
    DavaGLWidget *davaGlWidget;
    QMenuBar *menuBar;
    QMenu *menuFile;
    QMenu *menuResentScenes;
    QMenu *menuExport;
    QMenu *menuCreateNode;
    QMenu *menuTools;
    QMenu *menuViewPort;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(1065, 660);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(MainWindow->sizePolicy().hasHeightForWidth());
        MainWindow->setSizePolicy(sizePolicy);
        MainWindow->setMinimumSize(QSize(300, 200));
        actionNewScene = new QAction(MainWindow);
        actionNewScene->setObjectName(QString::fromUtf8("actionNewScene"));
        actionOpenScene = new QAction(MainWindow);
        actionOpenScene->setObjectName(QString::fromUtf8("actionOpenScene"));
        actionSaveScene = new QAction(MainWindow);
        actionSaveScene->setObjectName(QString::fromUtf8("actionSaveScene"));
        actionOpenProject = new QAction(MainWindow);
        actionOpenProject->setObjectName(QString::fromUtf8("actionOpenProject"));
        actionPNG = new QAction(MainWindow);
        actionPNG->setObjectName(QString::fromUtf8("actionPNG"));
        actionPVR = new QAction(MainWindow);
        actionPVR->setObjectName(QString::fromUtf8("actionPVR"));
        actionDXT = new QAction(MainWindow);
        actionDXT->setObjectName(QString::fromUtf8("actionDXT"));
        actionLandscape = new QAction(MainWindow);
        actionLandscape->setObjectName(QString::fromUtf8("actionLandscape"));
        actionLight = new QAction(MainWindow);
        actionLight->setObjectName(QString::fromUtf8("actionLight"));
        actionServiceNode = new QAction(MainWindow);
        actionServiceNode->setObjectName(QString::fromUtf8("actionServiceNode"));
        actionBox = new QAction(MainWindow);
        actionBox->setObjectName(QString::fromUtf8("actionBox"));
        actionSphere = new QAction(MainWindow);
        actionSphere->setObjectName(QString::fromUtf8("actionSphere"));
        actionCamera = new QAction(MainWindow);
        actionCamera->setObjectName(QString::fromUtf8("actionCamera"));
        actionImposter = new QAction(MainWindow);
        actionImposter->setObjectName(QString::fromUtf8("actionImposter"));
        actionUserNode = new QAction(MainWindow);
        actionUserNode->setObjectName(QString::fromUtf8("actionUserNode"));
        actionMaterialEditor = new QAction(MainWindow);
        actionMaterialEditor->setObjectName(QString::fromUtf8("actionMaterialEditor"));
        actionHeightMapEditor = new QAction(MainWindow);
        actionHeightMapEditor->setObjectName(QString::fromUtf8("actionHeightMapEditor"));
        actionTileMapEditor = new QAction(MainWindow);
        actionTileMapEditor->setObjectName(QString::fromUtf8("actionTileMapEditor"));
        actionTextureConverter = new QAction(MainWindow);
        actionTextureConverter->setObjectName(QString::fromUtf8("actionTextureConverter"));
        actionIPhone = new QAction(MainWindow);
        actionIPhone->setObjectName(QString::fromUtf8("actionIPhone"));
        actionRetina = new QAction(MainWindow);
        actionRetina->setObjectName(QString::fromUtf8("actionRetina"));
        actionIPad = new QAction(MainWindow);
        actionIPad->setObjectName(QString::fromUtf8("actionIPad"));
        actionDefault = new QAction(MainWindow);
        actionDefault->setObjectName(QString::fromUtf8("actionDefault"));
        actionParticleEmitter = new QAction(MainWindow);
        actionParticleEmitter->setObjectName(QString::fromUtf8("actionParticleEmitter"));
        centralWidget = new QWidget(MainWindow);
        centralWidget->setObjectName(QString::fromUtf8("centralWidget"));
        verticalLayout = new QVBoxLayout(centralWidget);
        verticalLayout->setSpacing(6);
        verticalLayout->setContentsMargins(0, 0, 0, 0);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        davaGlWidget = new DavaGLWidget(centralWidget);
        davaGlWidget->setObjectName(QString::fromUtf8("davaGlWidget"));
        davaGlWidget->setEnabled(true);
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(davaGlWidget->sizePolicy().hasHeightForWidth());
        davaGlWidget->setSizePolicy(sizePolicy1);
        davaGlWidget->setMinimumSize(QSize(300, 200));
        davaGlWidget->setAutoFillBackground(true);

        verticalLayout->addWidget(davaGlWidget);

        MainWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(MainWindow);
        menuBar->setObjectName(QString::fromUtf8("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 1065, 22));
        menuFile = new QMenu(menuBar);
        menuFile->setObjectName(QString::fromUtf8("menuFile"));
        menuResentScenes = new QMenu(menuFile);
        menuResentScenes->setObjectName(QString::fromUtf8("menuResentScenes"));
        menuExport = new QMenu(menuFile);
        menuExport->setObjectName(QString::fromUtf8("menuExport"));
        menuCreateNode = new QMenu(menuBar);
        menuCreateNode->setObjectName(QString::fromUtf8("menuCreateNode"));
        menuTools = new QMenu(menuBar);
        menuTools->setObjectName(QString::fromUtf8("menuTools"));
        menuViewPort = new QMenu(menuBar);
        menuViewPort->setObjectName(QString::fromUtf8("menuViewPort"));
        MainWindow->setMenuBar(menuBar);

        menuBar->addAction(menuFile->menuAction());
        menuBar->addAction(menuCreateNode->menuAction());
        menuBar->addAction(menuTools->menuAction());
        menuBar->addAction(menuViewPort->menuAction());
        menuFile->addAction(actionNewScene);
        menuFile->addSeparator();
        menuFile->addAction(actionOpenScene);
        menuFile->addAction(actionOpenProject);
        menuFile->addAction(menuResentScenes->menuAction());
        menuFile->addSeparator();
        menuFile->addAction(actionSaveScene);
        menuFile->addAction(menuExport->menuAction());
        menuExport->addAction(actionPNG);
        menuExport->addAction(actionPVR);
        menuExport->addAction(actionDXT);
        menuCreateNode->addAction(actionLandscape);
        menuCreateNode->addAction(actionLight);
        menuCreateNode->addAction(actionServiceNode);
        menuCreateNode->addAction(actionBox);
        menuCreateNode->addAction(actionSphere);
        menuCreateNode->addAction(actionCamera);
        menuCreateNode->addAction(actionImposter);
        menuCreateNode->addAction(actionParticleEmitter);
        menuCreateNode->addAction(actionUserNode);
        menuTools->addAction(actionMaterialEditor);
        menuTools->addAction(actionTextureConverter);
        menuTools->addSeparator();
        menuTools->addAction(actionHeightMapEditor);
        menuTools->addAction(actionTileMapEditor);
        menuViewPort->addAction(actionIPhone);
        menuViewPort->addAction(actionRetina);
        menuViewPort->addAction(actionIPad);
        menuViewPort->addAction(actionDefault);

        retranslateUi(MainWindow);

        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "MainWindow", 0, QApplication::UnicodeUTF8));
        actionNewScene->setText(QApplication::translate("MainWindow", "New Scene", 0, QApplication::UnicodeUTF8));
        actionOpenScene->setText(QApplication::translate("MainWindow", "Open Scene", 0, QApplication::UnicodeUTF8));
        actionSaveScene->setText(QApplication::translate("MainWindow", "Save Scene", 0, QApplication::UnicodeUTF8));
        actionOpenProject->setText(QApplication::translate("MainWindow", "Open Project", 0, QApplication::UnicodeUTF8));
        actionPNG->setText(QApplication::translate("MainWindow", "PNG", 0, QApplication::UnicodeUTF8));
        actionPVR->setText(QApplication::translate("MainWindow", "PVR", 0, QApplication::UnicodeUTF8));
        actionDXT->setText(QApplication::translate("MainWindow", "DXT", 0, QApplication::UnicodeUTF8));
        actionLandscape->setText(QApplication::translate("MainWindow", "Landscape", 0, QApplication::UnicodeUTF8));
        actionLight->setText(QApplication::translate("MainWindow", "Light", 0, QApplication::UnicodeUTF8));
        actionServiceNode->setText(QApplication::translate("MainWindow", "Service Node", 0, QApplication::UnicodeUTF8));
        actionBox->setText(QApplication::translate("MainWindow", "Box", 0, QApplication::UnicodeUTF8));
        actionSphere->setText(QApplication::translate("MainWindow", "Sphere", 0, QApplication::UnicodeUTF8));
        actionCamera->setText(QApplication::translate("MainWindow", "Camera", 0, QApplication::UnicodeUTF8));
        actionImposter->setText(QApplication::translate("MainWindow", "Imposter", 0, QApplication::UnicodeUTF8));
        actionUserNode->setText(QApplication::translate("MainWindow", "User Node", 0, QApplication::UnicodeUTF8));
        actionMaterialEditor->setText(QApplication::translate("MainWindow", "Material Editor", 0, QApplication::UnicodeUTF8));
        actionHeightMapEditor->setText(QApplication::translate("MainWindow", "Height Map Editor", 0, QApplication::UnicodeUTF8));
        actionTileMapEditor->setText(QApplication::translate("MainWindow", "Tile Map Editor", 0, QApplication::UnicodeUTF8));
        actionTextureConverter->setText(QApplication::translate("MainWindow", "Texture Converter", 0, QApplication::UnicodeUTF8));
        actionIPhone->setText(QApplication::translate("MainWindow", "iPhone", 0, QApplication::UnicodeUTF8));
        actionRetina->setText(QApplication::translate("MainWindow", "Retina", 0, QApplication::UnicodeUTF8));
        actionIPad->setText(QApplication::translate("MainWindow", "iPad", 0, QApplication::UnicodeUTF8));
        actionDefault->setText(QApplication::translate("MainWindow", "Default", 0, QApplication::UnicodeUTF8));
        actionParticleEmitter->setText(QApplication::translate("MainWindow", "Particle Emitter", 0, QApplication::UnicodeUTF8));
        menuFile->setTitle(QApplication::translate("MainWindow", "File", 0, QApplication::UnicodeUTF8));
        menuResentScenes->setTitle(QApplication::translate("MainWindow", "Resent Scenes", 0, QApplication::UnicodeUTF8));
        menuExport->setTitle(QApplication::translate("MainWindow", "Export", 0, QApplication::UnicodeUTF8));
        menuCreateNode->setTitle(QApplication::translate("MainWindow", "Create Node", 0, QApplication::UnicodeUTF8));
        menuTools->setTitle(QApplication::translate("MainWindow", "Tools", 0, QApplication::UnicodeUTF8));
        menuViewPort->setTitle(QApplication::translate("MainWindow", "View Port", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
