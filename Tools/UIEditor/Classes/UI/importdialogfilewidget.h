#ifndef __UIEDITOR__IMPORTDIALOGFILEWIDGET__
#define __UIEDITOR__IMPORTDIALOGFILEWIDGET__

#include <QWidget>
#include "importdialog.h"

namespace Ui {
	class ImportDialogFileWidget;
}

class ImportDialogFileWidget : public QWidget
{
	Q_OBJECT

public:
	explicit ImportDialogFileWidget(uint32 id, QWidget *parent = 0);
	~ImportDialogFileWidget();

	ImportDialog::eAction GetSelectedAction() const;
	void SetAction(ImportDialog::eAction action);

	QSize GetSize() const;

	QString GetFilename() const;
	void SetFilename(const QString& filename);

	void InitWithFilenameAndAction(const QString& filename, ImportDialog::eAction action);
	void SetSizeWidgetShowable(bool showable);
	void SetUpperIconsShowable(bool showable);

signals:
	void ActionChanged(uint32 id);
	void SizeChanged();

public slots:
	void ResetAggregatorSize();

private slots:
	void UpdateState(int action);
	void OnSizeChanged();

private:
	Ui::ImportDialogFileWidget *ui;

	uint32 id;
	ImportDialog::eAction activeAction;
	QString filename;
	bool neverShowSizeWidget;
	bool showUpperIcons;
};

#endif /* defined(__UIEDITOR__IMPORTDIALOGFILEWIDGET__) */
