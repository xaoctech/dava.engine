#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "DAVAEngine.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    
    
    if(DAVA::Core::Instance())
    {
        DAVA::KeyedArchive *options = DAVA::Core::Instance()->GetOptions();
        if(options)
        {
            QString titleStr(options->GetString("title", "Project Title").c_str());
            
            this->setWindowTitle(titleStr);
        }
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

