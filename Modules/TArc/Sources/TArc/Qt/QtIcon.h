#pragma once

#include <Base/Type.h>

#include <QIcon>
#include <QList>
#include <QSize>

namespace DAVA
{
template <>
struct TypeDetails::IsEqualComparable<QIcon> : std::true_type
{
};

template <>
inline Type::CompareOp TypeDetails::GetEqualIfComparable<QIcon>(std::true_type)
{
    static auto op = [](const void* data1, const void* data2) -> bool
    {
        const QIcon& v1 = *static_cast<const QIcon*>(data1);
        const QIcon& v2 = *static_cast<const QIcon*>(data2);

        QList<QSize> s1 = v1.availableSizes();
        QList<QSize> s2 = v2.availableSizes();

        if (s1 != s2)
            return false;

        if (s1.size() == 0)
            return true;

        QPixmap p1 = v1.pixmap(s1.at(0));
        QPixmap p2 = v2.pixmap(s2.at(0));

        return (p1.toImage() == p2.toImage());
    };

    return static_cast<Type::CompareOp>(op);
}
}
