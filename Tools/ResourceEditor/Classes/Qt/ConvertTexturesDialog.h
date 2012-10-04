#ifndef CONVERTTEXTURESDIALOG_H
#define CONVERTTEXTURESDIALOG_H

#include <QDialog>

namespace Ui {
class ConvertTexturesDialog;
}

class ConvertTexturesDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit ConvertTexturesDialog(QWidget *parent = 0);
    ~ConvertTexturesDialog();

private:
    Ui::ConvertTexturesDialog *ui;
};

#endif // CONVERTTEXTURESDIALOG_H
