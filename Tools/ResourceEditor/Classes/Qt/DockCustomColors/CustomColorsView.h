#ifndef __RESOURCEEDITORQT__CUSTOMCOLORSVIEW__
#define __RESOURCEEDITORQT__CUSTOMCOLORSVIEW__

#include <QWidget>

namespace Ui
{
	class CustomColorsView;
}

class CustomColorsView: public QWidget
{
	Q_OBJECT

public:
	explicit CustomColorsView(QWidget* parent = 0);
	~CustomColorsView();

	void Init();
	void InitColors();

private slots:
	void ProjectOpened(const QString &path);

private:
	Ui::CustomColorsView *ui;
};

#endif /* defined(__RESOURCEEDITORQT__CUSTOMCOLORSVIEW__) */
