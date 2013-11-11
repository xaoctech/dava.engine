/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/



#ifndef __MIMEDATA_HELPER2_H__
#define __MIMEDATA_HELPER2_H__

#include "DAVAEngine.h"
#include <QMimeData>

template <class T>
class MimeDataHelper2
{
public:
    
    static const char * mimeFormatMaterial;
	static const char * mimeFormatEntity;
	static const char * mimeFormatLayer;
	static const char * mimeFormatForce;
    
public:
    
    static QMimeData * EncodeMimeData(const QVector<T *> & data, const QString & format);
    static QMimeData * EncodeMimeData(const QVector<T *> & data);
    static QVector<T *> * DecodeMimeData(const QMimeData * data, const QString & format);

	static const QString GetSupportedTypeName();
};

template <class T>
const char * MimeDataHelper2<T>::mimeFormatMaterial = "application/dava.nmaterial";

template <class T>
const char * MimeDataHelper2<T>::mimeFormatEntity = "application/dava.entity";

template <class T>
const char * MimeDataHelper2<T>::mimeFormatLayer = "application/dava.particlelayer";

template <class T>
const char * MimeDataHelper2<T>::mimeFormatForce = "application/dava.particleforce";


template <class T>
QMimeData * MimeDataHelper2<T>::EncodeMimeData(const QVector<T *> & data, const QString & format)
{
	if(data.size() > 0)
	{
		QByteArray encodedData;
		QDataStream stream(&encodedData, QIODevice::WriteOnly);

        QMimeData *mimeData = new QMimeData();
		for (int i = 0; i < data.size(); ++i)
		{
			stream.writeRawData((char *) &data[i], sizeof(T *));
		}
        
		mimeData->setData(format, encodedData);
        return mimeData;
	}

    return NULL;
}

template <class T>
QMimeData * MimeDataHelper2<T>::EncodeMimeData(const QVector<T *> & data)
{
	if(data.size() > 0)
	{
		QByteArray encodedData;
		QDataStream stream(&encodedData, QIODevice::WriteOnly);
        
        QMimeData *mimeData = new QMimeData();
		for (int i = 0; i < data.size(); ++i)
		{
			stream.writeRawData((char *) &data[i], sizeof(T *));
		}
        
		mimeData->setData(GetSupportedTypeName(), encodedData);
        return mimeData;
	}
    
    return NULL;
}


template <class T>
QVector<T *> * MimeDataHelper2<T>::DecodeMimeData(const QMimeData * data, const QString & format)
{
	if(data->hasFormat(format))
	{
		QByteArray encodedData = data->data(format);
		QDataStream stream(&encodedData, QIODevice::ReadOnly);
        
		QVector<T *> * ret = new QVector<T *>();
		while(!stream.atEnd())
		{
            T * object = NULL;
			stream.readRawData((char *) &object, sizeof(T *));
			ret->push_back(object);
		}
        
        return ret;
	}
    
	return NULL;
}


template<>
const QString MimeDataHelper2<DAVA::Entity>::GetSupportedTypeName()
{
    return MimeDataHelper2<DAVA::Entity>::mimeFormatEntity;
}

template<>
const QString MimeDataHelper2<DAVA::NMaterial>::GetSupportedTypeName()
{
    return MimeDataHelper2<DAVA::NMaterial>::mimeFormatMaterial;
}

template<>
const QString MimeDataHelper2<DAVA::ParticleLayer>::GetSupportedTypeName()
{
    return MimeDataHelper2<DAVA::ParticleLayer>::mimeFormatLayer;
}

template<>
const QString MimeDataHelper2<DAVA::ParticleForce>::GetSupportedTypeName()
{
    return MimeDataHelper2<DAVA::ParticleForce>::mimeFormatForce;
}

template<class T>
const QString MimeDataHelper2<T>::GetSupportedTypeName()
{
    return "Not defined";
}



#endif  //#ifndef __MIMEDATA_HELPER2_H__