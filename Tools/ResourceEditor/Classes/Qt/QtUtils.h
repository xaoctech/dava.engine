#ifndef __QT_UTILS_H__
#define __QT_UTILS_H__

#define QSTRING_TO_DAVASTRING(str)   (str).toStdString().data()

#include <QObject>
#include <QVariant>
#include <QMetaType>
#include <QDataStream>

//template <class PointerType>
//class PointerHolder: public QObject
//{
//    Q_OBJECT
//    Q_DECLARE_METATYPE(PointerHolder<PointerType>*)
//
//public:
//    
//    PointerHolder(PointerType *pointer)
//    {
//        storedPointer = pointer;
//    }
//
//    PointerHolder(const PointerHolder &holder)
//    {
//        storedPointer = holder.storedPointer;
//    }
//
//    ~PointerHolder()
//    {
//    }
//    
//    
//    PointerType * operator()()
//    {
//        return storedPointer;
//    }
//    
//private:
//    
//    PointerType * storedPointer;
//};
//
//
//
//template <class PointerType>
//QVariant PointerToQVariant(PointerType pointer)
//{
//    return qVariantFromValue(PointerHolder<PointerType>(pointer));
//}
//
//template <class PointerType>
//PointerType QVariantToPointer(const QVariant &qvariant)
//{
//    PointerHolder<PointerType> holder = qvariant_cast<PointerHolder<PointerType> >(qvariant);
//    return holder();
//}


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
