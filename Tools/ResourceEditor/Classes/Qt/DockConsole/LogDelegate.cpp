#include "LogDelegate.h"

#include <QAbstractItemView>


LogDelegate::LogDelegate(QAbstractItemView* _view, QObject* parent)
    : QStyledItemDelegate( parent )
    , view( _view )
{
    Q_ASSERT( _view );
    view->setItemDelegate(this);
}

LogDelegate::~LogDelegate()
{
}
