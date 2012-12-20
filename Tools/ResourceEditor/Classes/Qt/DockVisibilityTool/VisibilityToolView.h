#ifndef __RESOURCEEDITORQT__VISIBILITYTOOLVIEW__
#define __RESOURCEEDITORQT__VISIBILITYTOOLVIEW__

#include <QWidget>

namespace Ui
{
	class VisibilityToolView;
}

class VisibilityToolView: public QWidget
{
	Q_OBJECT

public:
	explicit VisibilityToolView(QWidget* parent = 0);
	~VisibilityToolView();

	void Init();

private:
	Ui::VisibilityToolView *ui;
};

#endif /* defined(__RESOURCEEDITORQT__VISIBILITYTOOLVIEW__) */
