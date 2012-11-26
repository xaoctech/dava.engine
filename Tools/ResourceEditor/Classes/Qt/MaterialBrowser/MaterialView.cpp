#include "MaterialBrowser/MaterialView.h"

MaterialView::MaterialView(QWidget *parent /* = 0 */)
{
	setViewMode(QListView::IconMode);
}

void MaterialView::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{

}
