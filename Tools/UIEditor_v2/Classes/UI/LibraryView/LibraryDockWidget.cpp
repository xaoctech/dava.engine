//
//  LibraryDockWidget.cpp
//  UIEditor
//
//  Created by Alexey Strokachuk on 9/27/14.
//
//
#include "LibraryDockWidget.h"
#include "ui_LibraryDockWidget.h"

LibraryDockWidget::LibraryDockWidget(QWidget *parent)
    : QDockWidget(parent)
    , ui(new Ui::LibraryDockWidget())
{
    ui->setupUi(this);
}

LibraryDockWidget::~LibraryDockWidget()
{
    delete ui;
}

