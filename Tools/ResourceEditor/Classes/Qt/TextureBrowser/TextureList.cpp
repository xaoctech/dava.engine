#include "TextureList.h"

TextureList::TextureList(QWidget *parent /* = 0 */)
	: QListView(parent)
{ }

void TextureList::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
	QListView::selectionChanged(selected, deselected);
	emit this->selected(selected.begin()->topLeft());
}
