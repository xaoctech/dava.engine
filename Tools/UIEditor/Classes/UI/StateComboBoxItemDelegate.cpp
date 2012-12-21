#include "StateComboBoxItemDelegate.h"

StateComboBoxItemDelegate::StateComboBoxItemDelegate(QObject *parent) :
    QItemDelegate(parent)
{
}

void StateComboBoxItemDelegate::SetBoldTextIndexesList(const QList<int>& textIndexesList)
{
    this->boldTextIndexesList = textIndexesList;
}

// Paint reimplementation
void StateComboBoxItemDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    // Current value text
    QString valueString = index.model()->data(index, Qt::DisplayRole).toString();
    //Check if index is in "bold text" list
  
    /*
    if (this->boldTextIndexesList.isEmpty() || !this->boldTextIndexesList.contains(index.row()))
    {
        QItemDelegate::paint (painter, option, index);
    }
    else
     */
    {
        //Save current state
        painter->save();
        
        // Set the "bold" flag for the font depending on the index
        QFont activeFont = painter->font();
        activeFont.setBold(this->boldTextIndexesList.contains(index.row()));
        painter->setFont(activeFont);
        
        //Get position of item
        QRect point = option.rect;
        //Shift to right from left border
        point.setX(point.x() + 5);
        
        if (option.state & QStyle::State_Selected)
        {
            //Highlight text
            painter->fillRect(option.rect, option.palette.highlight());
            //Set white color for selected text
            QPen penHText(Qt::white);
            painter->setPen(penHText);
        }
        else
        {
            //Set black color for non-selected text
            QPen penHText(Qt::black);
            painter->setPen(penHText);
        }

        //Draw text
        painter->drawText(point, Qt::AlignLeft | Qt::TextSingleLine, valueString);
        //Restore painter state
        painter->restore();
    }
}
