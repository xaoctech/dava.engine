/********************************************************************************
** Form generated from reading UI file 'davaglwidget.ui'
**
** Created: Thu Jul 26 13:48:54 2012
**      by: Qt User Interface Compiler version 4.8.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_DAVAGLWIDGET_H
#define UI_DAVAGLWIDGET_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_DavaGLWidget
{
public:

    void setupUi(QWidget *DavaGLWidget)
    {
        if (DavaGLWidget->objectName().isEmpty())
            DavaGLWidget->setObjectName(QString::fromUtf8("DavaGLWidget"));
        DavaGLWidget->resize(353, 239);
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(DavaGLWidget->sizePolicy().hasHeightForWidth());
        DavaGLWidget->setSizePolicy(sizePolicy);

        retranslateUi(DavaGLWidget);

        QMetaObject::connectSlotsByName(DavaGLWidget);
    } // setupUi

    void retranslateUi(QWidget *DavaGLWidget)
    {
        DavaGLWidget->setWindowTitle(QApplication::translate("DavaGLWidget", "Form", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class DavaGLWidget: public Ui_DavaGLWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_DAVAGLWIDGET_H
