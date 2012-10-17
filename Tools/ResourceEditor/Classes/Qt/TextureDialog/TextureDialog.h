#ifndef __TEXTURE_DIALOG_H__
#define __TEXTURE_DIALOG_H__

#include <QDialog>
#include <QMap>
#include "DAVAEngine.h"

class QModelIndex;
class TextureListDelegate;
class TextureListModel;
class QAbstractItemDelegate;
class QStatusBar;

namespace Ui {
class TextureDialog;
}

class TextureDialog : public QDialog
{
    Q_OBJECT

public:
	enum TextureView
	{
		ViewNothing,
		ViewPVR,
		ViewDXT
	};

public:
    explicit TextureDialog(QWidget *parent = 0);
    ~TextureDialog();

	void setScene(DAVA::Scene *scene);
	void setTextureView(TextureView view);

private:
    Ui::TextureDialog *ui;
	TextureListModel *textureListModel;
	TextureListDelegate *textureListImagesDelegate;
	QAbstractItemDelegate *textureListDefaultDelegate;
	QStatusBar *statusBar;
	
	QMap<QString, int> textureListSortModes;
	DAVA::Texture *curTextureOriginal;
	TextureView curTextureView;

	void setupTextureListToolbar();
	void setupTextureToolbar();
	void setupTexturesList();
	void setupImagesScrollAreas();
	void setupTextureListFilter();
	void setupStatusBar();
	void setupTextureProperties();
	void setupTextureViewToolbar();

private slots:
	void textureListViewImages(bool checked);
	void textureListViewText(bool checked);
	void textureListFilterChanged(const QString &text);
	void textureListSortChanged(const QString &text);
	void texturePressed(const QModelIndex & index);
	void textureColorChannelPressed(bool checked);
	void textureFormatPVRChanged(const DAVA::PixelFormat &newFormat);
	void textureFormatDXTChanged(const DAVA::PixelFormat &newFormat);
	void textureViewPVR(bool checked);
	void textureViewDXT(bool checked);
};

#endif // __TEXTURE_DIALOG_H__
