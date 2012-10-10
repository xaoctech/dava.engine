#ifndef CONVERTTEXTURESDIALOG_H
#define CONVERTTEXTURESDIALOG_H

#include <QDialog>
#include <QMap>
#include "DAVAEngine.h"

class QModelIndex;
class TextureListDelegate;
class TextureListModel;
class QAbstractItemDelegate;

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
	TextureListModel *textureListModel;
	TextureListDelegate *textureListImagesDelegate;
	QAbstractItemDelegate *textureListDefaultDelegate;
	QMap<QString, int> textureListSortModes;

	void setupTextureListToolbar();
	void setupTextureToolbar();
	void setupTexturesList();
	void setupImagesScrollAreas();
	void setupTextureListFilter();

private slots:
	void textureListViewImages(bool checked);
	void textureListViewText(bool checked);
	void textureListFilterChanged(const QString &text);
	void textureListSortChanged(const QString &text);
	void texturePressed(const QModelIndex & index);
};

#endif // CONVERTTEXTURESDIALOG_H
