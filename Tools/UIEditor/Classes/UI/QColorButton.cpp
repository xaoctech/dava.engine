#include "QColorButton.h"
#include "FileSystem/FileSystem.h"
#include "ResourcesManageHelper.h"
#include <QFile>

using namespace DAVA;

QColorButton::QColorButton(QWidget *parent) :
    QPushButton(parent)
{
    //Set default background color as white
    SetBackgroundColor(QColor(255,255,255));
}

QColor QColorButton::GetBackgroundColor() const
{ 
    return currentBackgroundColor;
}

void QColorButton::SetBackgroundColor(const QColor& color)
{
    //Create style string for background color
    QString style = QString("* { border: 1px solid grey; background-color: %1;} ").arg(color.name());
    this->currentBackgroundColor = color;
    //Reset style
    this->setStyleSheet("");
    this->setStyleSheet(style);
    //Call signal each time control background color is changed
    emit backgroundColorChanged(color);
}

void QColorButton::SetBackgroundImage(const QString &imagePath)
{
    //Check if background image file realy exist  
    if(QFile::exists(imagePath))
    {
        //Get current background color
        QString currentBgColor = this->currentBackgroundColor.name();
        //Create style string for background image. Keep current background color
        QString style = QString("* {border: 1px solid grey; background-color: %1; background-image: url(%2);} ").arg(currentBgColor,imagePath);
        //Reset style
        this->setStyleSheet("");
        this->setStyleSheet(style);
    }    
}

void QColorButton::SetDisplayMultipleColors(const bool needSetbackgroundImage)
{
    if (needSetbackgroundImage)
    {
        SetBackgroundImage(ResourcesManageHelper::GetButtonBackgroundImagePath());
    }
}