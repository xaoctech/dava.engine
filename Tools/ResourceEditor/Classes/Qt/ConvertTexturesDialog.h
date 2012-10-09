#ifndef CONVERTTEXTURESDIALOG_H
#define CONVERTTEXTURESDIALOG_H

#include <QDialog>
#include "DAVAEngine.h"

class QModelIndex;

namespace Ui {
class ConvertTexturesDialog;
}

class ConvertTexturesDialog : public QDialog
{
    Q_OBJECT
    
public:
    explicit ConvertTexturesDialog(QWidget *parent = 0);
    ~ConvertTexturesDialog();

	void setScene(DAVA::Scene *scene);

private:
    Ui::ConvertTexturesDialog *ui;

	void setupTextureListToolbar();
	void setupTextureToolbar();
	void setupTexturesList();
	void setupImagesScrollAreas();

private slots:
	void texturePressed(const QModelIndex & index);
};

#endif // CONVERTTEXTURESDIALOG_H
