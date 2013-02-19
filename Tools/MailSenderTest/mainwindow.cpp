#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "DAVAEngine.h"
#include "Network/MailSender.h"

using namespace DAVA;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
	new MailSender();
    connect(ui->sendButton, SIGNAL(clicked()), this, SLOT(OnSendButtonPressed()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::OnSendButtonPressed()
{
   bool one =  MailSender::Instance()->SendEmail("mavisson@yandex.ru",
   													"hello master",
  												 "I want to say hallo to my dear master of music.");
}
