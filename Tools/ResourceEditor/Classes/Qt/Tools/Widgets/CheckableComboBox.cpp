#include "CheckableComboBox.h"

#include <QAbstractItemView>
#include <QEvent>
#include <QListView>
#include <QStylePainter>
#include <QDebug>


CheckableComboBox::CheckableComboBox(QWidget* parent)
    : QComboBox(parent)
{
    connect( model(), SIGNAL( rowsInserted( const QModelIndex&, int, int ) ), SLOT( OnRowsInserted( const QModelIndex&, int, int ) ) );

    QListView *v = new QListView();
    setView(v);

    view()->viewport()->installEventFilter(this);
}

CheckableComboBox::~CheckableComboBox()
{
}

QStringList CheckableComboBox::GetSelectedItems() const
{
    const QModelIndexList& indexes = GetCheckedIndexes();
    QStringList list;
    
    for ( int i = 0; i < indexes.size(); i++ )
    {
        list << indexes[i].data( Qt::DisplayRole ).toString();
    }

    return list;
}

QList<QVariant> CheckableComboBox::GetSelectedUserData() const
{
    const QModelIndexList& indexes = GetCheckedIndexes();
    QVariantList list;
    
    for ( int i = 0; i < indexes.size(); i++ )
    {
        list << indexes[i].data( Qt::UserRole );
    }

    return list;
}

void CheckableComboBox::SelectUserData(const QList<QVariant>& dataList)
{
    QAbstractItemModel *m = model();
    const int n = m->rowCount();
    for ( int i = 0; i < n; i++ )
    {
        const QModelIndex index =  m->index( i, 0, QModelIndex() );
        const Qt::CheckState checkState = dataList.contains( index.data( Qt::UserRole ) ) ? Qt::Checked : Qt::Unchecked;
        m->setData( index, checkState, Qt::CheckStateRole );
    }
}

void CheckableComboBox::OnRowsInserted(const QModelIndex& parent, int start, int end)
{
    QStandardItemModel *m = qobject_cast<QStandardItemModel *>( model() );
    if (m == NULL)
        return ;

    for (int i = start; i <= end; i++)
    {
        QStandardItem *item = m->item(i);
        item->setCheckable(true);
    }
}

QModelIndexList CheckableComboBox::GetCheckedIndexes() const
{
    QModelIndexList list;

    QAbstractItemModel *m = model();
    const int n = m->rowCount();
    for ( int i = 0; i < n; i++ )
    {
        const QModelIndex index =  m->index( i, 0, QModelIndex() );
        const bool isChecked = (index.data( Qt::CheckStateRole ).toInt() == Qt::Checked );
        if ( isChecked )
        {
            list << index;
        }
    }

    return list;
}

bool CheckableComboBox::eventFilter(QObject* obj, QEvent* e)
{
    if (obj == view()->viewport())
    {
        switch (e->type())
        {
        case QEvent::MouseButtonRelease:
            {
                QAbstractItemView *v = view();
                QAbstractItemModel *m = v->model();
                const QModelIndex index = v->currentIndex();
                const bool isChecked = ( m->data(index, Qt::CheckStateRole).toInt() == Qt::Checked );
                m->setData( index, isChecked ? Qt::Unchecked : Qt::Checked, Qt::CheckStateRole);
            }
            return true;

        case QEvent::Hide:
            {
                emit done();
            }
            break;

        default:
            break;
        }
    }

    return QComboBox::eventFilter(obj, e);
}

void CheckableComboBox::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QStylePainter p(this);
    p.setPen(palette().color(QPalette::Text));

    QStyleOptionComboBox option;
    initStyleOption(&option);

    p.drawComplexControl(QStyle::CC_ComboBox, option);

    const QRect textRect = style()->subControlRect( QStyle::CC_ComboBox, &option, QStyle::SC_ComboBoxEditField );
    const QFontMetrics metrics(font());
    const QString elidedText = metrics.elidedText( QString(), Qt::ElideRight, textRect.width() );

    p.drawText(textRect, Qt::AlignVCenter, elidedText);
}
