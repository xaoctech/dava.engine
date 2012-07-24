#ifndef __QT_DEFINES_H__
#define __QT_DEFINES_H__


//Used toAscii because toStdString crashed onto Win32 because of different _HAS_ITERATOR_DEBUGGING values at editor and qtcore
#define QSTRING_TO_DAVASTRING(str)   (str).toAscii().data()


#endif // __QT_DEFINES_H__
