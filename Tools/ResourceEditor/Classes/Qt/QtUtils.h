#ifndef __QT_UTILS_H__
#define __QT_UTILS_H__

#define QSTRING_TO_DAVASTRING(str)   (str).toStdString().data()

#include <QObject>
#include <QVariant>
#include <QMetaType>
#include <QDataStream>


class GraphItem;
class PointerHolder
{
public:
    
    PointerHolder();
    PointerHolder(const PointerHolder &fromHolder);
    virtual ~PointerHolder();

    static QVariant ToQVariant(GraphItem *item);
    static GraphItem * ToGraphItem(const QVariant &variant);

    void SetPointer(GraphItem *pointer);
    GraphItem *GetPointer() const;
    
    GraphItem * storedPointer;
};


QDataStream& operator<<(QDataStream& ostream, const PointerHolder& ph);
QDataStream& operator>>(QDataStream& istream, PointerHolder& ph);

Q_DECLARE_METATYPE(PointerHolder);
Q_DECLARE_METATYPE(QList<PointerHolder>)


#endif // __QT_UTILS_H__
