/*==================================================================================
    Copyright (c) 2008, DAVA, INC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA, INC nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA, INC AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL DAVA, INC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#ifndef QTDAVAVECTORPROPERTYMANAGER_H
#define QTDAVAVECTORPROPERTYMANAGER_H

#include "QtPropertyBrowser/qtpropertybrowser.h"
#include "QtPropertyBrowser/qtpropertymanager.h"
#include "Base/BaseMath.h"
#include "Math/Vector.h"

#include <QMap>

class QtVector4PropertyManager;

class QtVector4PropertyManagerPrivate
{
	QtVector4PropertyManager *q_ptr;
	Q_DECLARE_PUBLIC(QtVector4PropertyManager)
public:

	void slotDoubleChanged(QtProperty *property, double value);
	void slotPropertyDestroyed(QtProperty *property);

	struct Data
	{
		Data() : val(0, 0, 0, 0)
		{}

		DAVA::Vector4 val;
	};

	typedef QMap<const QtProperty *, Data> PropertyValueMap;
	PropertyValueMap m_values;
	QtDoublePropertyManager *m_doublePropertyManager;

	QMap<const QtProperty *, QtProperty *> m_propertyToX;
	QMap<const QtProperty *, QtProperty *> m_propertyToY;
	QMap<const QtProperty *, QtProperty *> m_propertyToZ;
	QMap<const QtProperty *, QtProperty *> m_propertyToW;

	QMap<const QtProperty *, QtProperty *> m_xToProperty;
	QMap<const QtProperty *, QtProperty *> m_yToProperty;
	QMap<const QtProperty *, QtProperty *> m_zToProperty;
	QMap<const QtProperty *, QtProperty *> m_wToProperty;
};

class QT_QTPROPERTYBROWSER_EXPORT QtVector4PropertyManager : public QtAbstractPropertyManager
{
	Q_OBJECT
public:
	QtVector4PropertyManager(QObject *parent = 0);
	~QtVector4PropertyManager();

	QtDoublePropertyManager *subDoublePropertyManager() const;
	DAVA::Vector4 value(const QtProperty *property) const;

public Q_SLOTS:
	void setValue(QtProperty *property, const DAVA::Vector4 &val);

Q_SIGNALS:
	void valueChanged(QtProperty *property, const DAVA::Vector4 &val);

protected:
	QString valueText(const QtProperty *property) const;
	virtual void initializeProperty(QtProperty *property);
	virtual void uninitializeProperty(QtProperty *property);

private:
	QtVector4PropertyManagerPrivate *d_ptr;
	Q_DECLARE_PRIVATE(QtVector4PropertyManager)
	Q_DISABLE_COPY(QtVector4PropertyManager)
	Q_PRIVATE_SLOT(d_func(), void slotDoubleChanged(QtProperty *, double))
	Q_PRIVATE_SLOT(d_func(), void slotPropertyDestroyed(QtProperty *))
};

class QtVector3PropertyManager;

class QtVector3PropertyManagerPrivate
{
	QtVector3PropertyManager *q_ptr;
	Q_DECLARE_PUBLIC(QtVector3PropertyManager)
public:

	void slotDoubleChanged(QtProperty *property, double value);
	void slotPropertyDestroyed(QtProperty *property);

	struct Data
	{
		Data() : val(0, 0, 0)
		{}

		DAVA::Vector3 val;
	};

	typedef QMap<const QtProperty *, Data> PropertyValueMap;
	PropertyValueMap m_values;
	QtDoublePropertyManager *m_doublePropertyManager;

	QMap<const QtProperty *, QtProperty *> m_propertyToX;
	QMap<const QtProperty *, QtProperty *> m_propertyToY;
	QMap<const QtProperty *, QtProperty *> m_propertyToZ;

	QMap<const QtProperty *, QtProperty *> m_xToProperty;
	QMap<const QtProperty *, QtProperty *> m_yToProperty;
	QMap<const QtProperty *, QtProperty *> m_zToProperty;
};

class QT_QTPROPERTYBROWSER_EXPORT QtVector3PropertyManager : public QtAbstractPropertyManager
{
	Q_OBJECT
public:
	QtVector3PropertyManager(QObject *parent = 0);
	~QtVector3PropertyManager();

	QtDoublePropertyManager *subDoublePropertyManager() const;
	DAVA::Vector3 value(const QtProperty *property) const;

	public Q_SLOTS:
		void setValue(QtProperty *property, const DAVA::Vector3 &val);

Q_SIGNALS:
		void valueChanged(QtProperty *property, const DAVA::Vector3 &val);

protected:
	QString valueText(const QtProperty *property) const;
	virtual void initializeProperty(QtProperty *property);
	virtual void uninitializeProperty(QtProperty *property);

private:
	QtVector3PropertyManagerPrivate *d_ptr;
	Q_DECLARE_PRIVATE(QtVector3PropertyManager)
	Q_DISABLE_COPY(QtVector3PropertyManager)
	Q_PRIVATE_SLOT(d_func(), void slotDoubleChanged(QtProperty *, double))
	Q_PRIVATE_SLOT(d_func(), void slotPropertyDestroyed(QtProperty *))
};

class QtVector2PropertyManager;

class QtVector2PropertyManagerPrivate
{
	QtVector2PropertyManager *q_ptr;
	Q_DECLARE_PUBLIC(QtVector2PropertyManager)
public:

	void slotDoubleChanged(QtProperty *property, double value);
	void slotPropertyDestroyed(QtProperty *property);

	struct Data
	{
		Data() : val(0, 0)
		{}

		DAVA::Vector2 val;
	};

	typedef QMap<const QtProperty *, Data> PropertyValueMap;
	PropertyValueMap m_values;
	QtDoublePropertyManager *m_doublePropertyManager;

	QMap<const QtProperty *, QtProperty *> m_propertyToX;
	QMap<const QtProperty *, QtProperty *> m_propertyToY;

	QMap<const QtProperty *, QtProperty *> m_xToProperty;
	QMap<const QtProperty *, QtProperty *> m_yToProperty;
};

class QT_QTPROPERTYBROWSER_EXPORT QtVector2PropertyManager : public QtAbstractPropertyManager
{
	Q_OBJECT
public:
	QtVector2PropertyManager(QObject *parent = 0);
	~QtVector2PropertyManager();

	QtDoublePropertyManager *subDoublePropertyManager() const;
	DAVA::Vector2 value(const QtProperty *property) const;

	public Q_SLOTS:
		void setValue(QtProperty *property, const DAVA::Vector2 &val);

Q_SIGNALS:
		void valueChanged(QtProperty *property, const DAVA::Vector2 &val);

protected:
	QString valueText(const QtProperty *property) const;
	virtual void initializeProperty(QtProperty *property);
	virtual void uninitializeProperty(QtProperty *property);

private:
	QtVector2PropertyManagerPrivate *d_ptr;
	Q_DECLARE_PRIVATE(QtVector2PropertyManager)
		Q_DISABLE_COPY(QtVector2PropertyManager)
		Q_PRIVATE_SLOT(d_func(), void slotDoubleChanged(QtProperty *, double))
		Q_PRIVATE_SLOT(d_func(), void slotPropertyDestroyed(QtProperty *))
};

#endif 