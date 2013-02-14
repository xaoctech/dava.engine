#ifndef __RESOURCEEDITORQT__MODIFICATIONWIDGET__
#define __RESOURCEEDITORQT__MODIFICATIONWIDGET__

#include <QWidget>

namespace Ui
{
	class ModificationWidget;
}

class ModificationWidget: public QWidget
{
	Q_OBJECT

public:
	explicit ModificationWidget(QWidget* parent = 0);
	~ModificationWidget();

signals:
	void ApplyModification(double x, double y, double z);

private slots:
	void OnEditingFinished();

private:
	Ui::ModificationWidget *ui;

	void ResetSpinBoxes();

protected:
	virtual void keyPressEvent(QKeyEvent* event);
};

#endif /* defined(__RESOURCEEDITORQT__MODIFICATIONWIDGET__) */
