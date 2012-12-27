#include "QtPropertyBrowser/Dava/qtDavaVectorPropertyManager.h"

// ------------------------------------------------------------------------------------------------------------------------------------------
// Vector4
// ------------------------------------------------------------------------------------------------------------------------------------------

void QtVector4PropertyManagerPrivate::slotDoubleChanged(QtProperty *property, double value)
{
    if (QtProperty *prop = m_xToProperty.value(property, 0)) {
        DAVA::Vector4 r = m_values[prop].val;
        q_ptr->setValue(prop, r);
    } else if (QtProperty *prop = m_yToProperty.value(property, 0)) {
        DAVA::Vector4 r = m_values[prop].val;
        q_ptr->setValue(prop, r);
    } else if (QtProperty *prop = m_zToProperty.value(property, 0)) {
		DAVA::Vector4 r = m_values[prop].val;
		q_ptr->setValue(prop, r);
    } else if (QtProperty *prop = m_wToProperty.value(property, 0)) {
		DAVA::Vector4 r = m_values[prop].val;
		q_ptr->setValue(prop, r);
    }
}

void QtVector4PropertyManagerPrivate::slotPropertyDestroyed(QtProperty *property)
{
    if (QtProperty *pointProp = m_xToProperty.value(property, 0)) {
        m_propertyToX[pointProp] = 0;
        m_xToProperty.remove(property);
    } else if (QtProperty *pointProp = m_yToProperty.value(property, 0)) {
        m_propertyToY[pointProp] = 0;
        m_yToProperty.remove(property);
    } else if (QtProperty *pointProp = m_zToProperty.value(property, 0)) {
        m_propertyToZ[pointProp] = 0;
        m_zToProperty.remove(property);
    } else if (QtProperty *pointProp = m_wToProperty.value(property, 0)) {
        m_propertyToW[pointProp] = 0;
        m_wToProperty.remove(property);
    }
}


/*!
    Creates a manager with the given \a parent.
*/
QtVector4PropertyManager::QtVector4PropertyManager(QObject *parent)
    : QtAbstractPropertyManager(parent)
{
    d_ptr = new QtVector4PropertyManagerPrivate;
    d_ptr->q_ptr = this;

    d_ptr->m_doublePropertyManager = new QtDoublePropertyManager(this);
    connect(d_ptr->m_doublePropertyManager, SIGNAL(valueChanged(QtProperty *, double)), this, SLOT(slotDoubleChanged(QtProperty *, double)));
    connect(d_ptr->m_doublePropertyManager, SIGNAL(propertyDestroyed(QtProperty *)), this, SLOT(slotPropertyDestroyed(QtProperty *)));
}

/*!
    Destroys this manager, and all the properties it has created.
*/
QtVector4PropertyManager::~QtVector4PropertyManager()
{
    clear();
    delete d_ptr;
}

/*!
    Returns the manager that creates the nested \e x, \e y, \e width
    and \e height subproperties.

    In order to provide editing widgets for the mentioned
    subproperties in a property browser widget, this manager must be
    associated with an editor factory.

    \sa QtAbstractPropertyBrowser::setFactoryForManager()
*/
QtDoublePropertyManager *QtVector4PropertyManager::subDoublePropertyManager() const
{
    return d_ptr->m_doublePropertyManager;
}

/*!
    Returns the given \a property's value.

    If the given \a property is not managed by this manager, this
    function returns an invalid rectangle.

    \sa setValue(), constraint()
*/
DAVA::Vector4 QtVector4PropertyManager::value(const QtProperty *property) const
{
	DAVA::Vector4 v;
	QtVector4PropertyManagerPrivate::PropertyValueMap::const_iterator it = d_ptr->m_values.find(property);

	if (it != d_ptr->m_values.constEnd())
	{
		v = it->val;
	}

	return v;
}

/*!
    \reimp
*/
QString QtVector4PropertyManager::valueText(const QtProperty *property) const
{
	return QString("Vector4 to string not implemented\n");
}

/*!
    \fn void QtRectFPropertyManager::setValue(QtProperty *property, const QRectF &value)

    Sets the value of the given \a property to \a value. Nested
    properties are updated automatically.

    If the specified \a value is not inside the given \a property's
    constraining rectangle, the value is adjusted accordingly to fit
    within the constraint.

    \sa value(), setConstraint(), valueChanged()
*/
void QtVector4PropertyManager::setValue(QtProperty *property, const DAVA::Vector4 &val)
{
    const QtVector4PropertyManagerPrivate::PropertyValueMap::iterator it = d_ptr->m_values.find(property);
    if (it == d_ptr->m_values.end())
        return;

    QtVector4PropertyManagerPrivate::Data data = it.value();

    if (data.val == val)
        return;

    data.val = val;

    it.value() = data;
    d_ptr->m_doublePropertyManager->setValue(d_ptr->m_propertyToX[property], val.x);
    d_ptr->m_doublePropertyManager->setValue(d_ptr->m_propertyToY[property], val.y);
    d_ptr->m_doublePropertyManager->setValue(d_ptr->m_propertyToZ[property], val.z);
    d_ptr->m_doublePropertyManager->setValue(d_ptr->m_propertyToW[property], val.w);

    emit propertyChanged(property);
    emit valueChanged(property, data.val);
}

/*!
    \reimp
*/
void QtVector4PropertyManager::initializeProperty(QtProperty *property)
{
    d_ptr->m_values[property] = QtVector4PropertyManagerPrivate::Data();

    QtProperty *xProp = d_ptr->m_doublePropertyManager->addProperty();
    xProp->setPropertyName(tr("X"));
    d_ptr->m_doublePropertyManager->setValue(xProp, 0);
    d_ptr->m_propertyToX[property] = xProp;
    d_ptr->m_xToProperty[xProp] = property;
    property->addSubProperty(xProp);

    QtProperty *yProp = d_ptr->m_doublePropertyManager->addProperty();
    yProp->setPropertyName(tr("Y"));
    d_ptr->m_doublePropertyManager->setValue(yProp, 0);
    d_ptr->m_propertyToY[property] = yProp;
    d_ptr->m_yToProperty[yProp] = property;
    property->addSubProperty(yProp);

	QtProperty *zProp = d_ptr->m_doublePropertyManager->addProperty();
	zProp->setPropertyName(tr("Z"));
	d_ptr->m_doublePropertyManager->setValue(zProp, 0);
	d_ptr->m_doublePropertyManager->setMinimum(zProp, 0);
	d_ptr->m_propertyToZ[property] = zProp;
	d_ptr->m_zToProperty[zProp] = property;
	property->addSubProperty(zProp);

    QtProperty *wProp = d_ptr->m_doublePropertyManager->addProperty();
    wProp->setPropertyName(tr("W"));
    d_ptr->m_doublePropertyManager->setValue(wProp, 0);
    d_ptr->m_doublePropertyManager->setMinimum(wProp, 0);
    d_ptr->m_propertyToW[property] = wProp;
    d_ptr->m_wToProperty[wProp] = property;
    property->addSubProperty(wProp);
}

/*!
    \reimp
*/
void QtVector4PropertyManager::uninitializeProperty(QtProperty *property)
{
    QtProperty *xProp = d_ptr->m_propertyToX[property];
    if (xProp) {
        d_ptr->m_xToProperty.remove(xProp);
        delete xProp;
    }
    d_ptr->m_propertyToX.remove(property);

    QtProperty *yProp = d_ptr->m_propertyToY[property];
    if (yProp) {
        d_ptr->m_yToProperty.remove(yProp);
        delete yProp;
    }
    d_ptr->m_propertyToY.remove(property);

    QtProperty *zProp = d_ptr->m_propertyToZ[property];
    if (zProp) {
        d_ptr->m_zToProperty.remove(zProp);
        delete zProp;
    }
    d_ptr->m_propertyToZ.remove(property);
	
	QtProperty *wProp = d_ptr->m_propertyToW[property];
	if (wProp) {
		d_ptr->m_wToProperty.remove(wProp);
		delete wProp;
	}
	d_ptr->m_propertyToW.remove(property);

    d_ptr->m_values.remove(property);
}




// ------------------------------------------------------------------------------------------------------------------------------------------
// Vector3
// ------------------------------------------------------------------------------------------------------------------------------------------


void QtVector3PropertyManagerPrivate::slotDoubleChanged(QtProperty *property, double value)
{
    if (QtProperty *prop = m_xToProperty.value(property, 0)) {
        DAVA::Vector3 r = m_values[prop].val;
        q_ptr->setValue(prop, r);
    } else if (QtProperty *prop = m_yToProperty.value(property, 0)) {
        DAVA::Vector3 r = m_values[prop].val;
        q_ptr->setValue(prop, r);
    } else if (QtProperty *prop = m_zToProperty.value(property, 0)) {
		DAVA::Vector3 r = m_values[prop].val;
		q_ptr->setValue(prop, r);
	}
}

void QtVector3PropertyManagerPrivate::slotPropertyDestroyed(QtProperty *property)
{
    if (QtProperty *pointProp = m_xToProperty.value(property, 0)) {
        m_propertyToX[pointProp] = 0;
        m_xToProperty.remove(property);
    } else if (QtProperty *pointProp = m_yToProperty.value(property, 0)) {
        m_propertyToY[pointProp] = 0;
        m_yToProperty.remove(property);
    } else if (QtProperty *pointProp = m_zToProperty.value(property, 0)) {
        m_propertyToZ[pointProp] = 0;
        m_zToProperty.remove(property);
	}
}


/*!
    Creates a manager with the given \a parent.
*/
QtVector3PropertyManager::QtVector3PropertyManager(QObject *parent)
    : QtAbstractPropertyManager(parent)
{
    d_ptr = new QtVector3PropertyManagerPrivate;
    d_ptr->q_ptr = this;

    d_ptr->m_doublePropertyManager = new QtDoublePropertyManager(this);
    connect(d_ptr->m_doublePropertyManager, SIGNAL(valueChanged(QtProperty *, double)), this, SLOT(slotDoubleChanged(QtProperty *, double)));
    connect(d_ptr->m_doublePropertyManager, SIGNAL(propertyDestroyed(QtProperty *)), this, SLOT(slotPropertyDestroyed(QtProperty *)));
}

/*!
    Destroys this manager, and all the properties it has created.
*/
QtVector3PropertyManager::~QtVector3PropertyManager()
{
    clear();
    delete d_ptr;
}

/*!
    Returns the manager that creates the nested \e x, \e y, \e width
    and \e height subproperties.

    In order to provide editing widgets for the mentioned
    subproperties in a property browser widget, this manager must be
    associated with an editor factory.

    \sa QtAbstractPropertyBrowser::setFactoryForManager()
*/
QtDoublePropertyManager *QtVector3PropertyManager::subDoublePropertyManager() const
{
    return d_ptr->m_doublePropertyManager;
}

/*!
    Returns the given \a property's value.

    If the given \a property is not managed by this manager, this
    function returns an invalid rectangle.

    \sa setValue(), constraint()
*/
DAVA::Vector3 QtVector3PropertyManager::value(const QtProperty *property) const
{
	DAVA::Vector3 v;
	QtVector3PropertyManagerPrivate::PropertyValueMap::const_iterator it = d_ptr->m_values.find(property);

	if (it != d_ptr->m_values.constEnd())
	{
		v = it->val;
	}

	return v;
}

/*!
    \reimp
*/
QString QtVector3PropertyManager::valueText(const QtProperty *property) const
{
	return QString("Vector3 to string not implemented\n");
}

/*!
    \fn void QtRectFPropertyManager::setValue(QtProperty *property, const QRectF &value)

    Sets the value of the given \a property to \a value. Nested
    properties are updated automatically.

    If the specified \a value is not inside the given \a property's
    constraining rectangle, the value is adjusted accordingly to fit
    within the constraint.

    \sa value(), setConstraint(), valueChanged()
*/
void QtVector3PropertyManager::setValue(QtProperty *property, const DAVA::Vector3 &val)
{
    const QtVector3PropertyManagerPrivate::PropertyValueMap::iterator it = d_ptr->m_values.find(property);
    if (it == d_ptr->m_values.end())
        return;

    QtVector3PropertyManagerPrivate::Data data = it.value();

    if (data.val == val)
        return;

    data.val = val;

    it.value() = data;
    d_ptr->m_doublePropertyManager->setValue(d_ptr->m_propertyToX[property], val.x);
    d_ptr->m_doublePropertyManager->setValue(d_ptr->m_propertyToY[property], val.y);
    d_ptr->m_doublePropertyManager->setValue(d_ptr->m_propertyToZ[property], val.z);

    emit propertyChanged(property);
    emit valueChanged(property, data.val);
}

/*!
    \reimp
*/
void QtVector3PropertyManager::initializeProperty(QtProperty *property)
{
    d_ptr->m_values[property] = QtVector3PropertyManagerPrivate::Data();

    QtProperty *xProp = d_ptr->m_doublePropertyManager->addProperty();
    xProp->setPropertyName(tr("X"));
    d_ptr->m_doublePropertyManager->setValue(xProp, 0);
    d_ptr->m_propertyToX[property] = xProp;
    d_ptr->m_xToProperty[xProp] = property;
    property->addSubProperty(xProp);

    QtProperty *yProp = d_ptr->m_doublePropertyManager->addProperty();
    yProp->setPropertyName(tr("Y"));
    d_ptr->m_doublePropertyManager->setValue(yProp, 0);
    d_ptr->m_propertyToY[property] = yProp;
    d_ptr->m_yToProperty[yProp] = property;
    property->addSubProperty(yProp);

	QtProperty *zProp = d_ptr->m_doublePropertyManager->addProperty();
	zProp->setPropertyName(tr("Z"));
	d_ptr->m_doublePropertyManager->setValue(zProp, 0);
	d_ptr->m_doublePropertyManager->setMinimum(zProp, 0);
	d_ptr->m_propertyToZ[property] = zProp;
	d_ptr->m_zToProperty[zProp] = property;
	property->addSubProperty(zProp);
}

/*!
    \reimp
*/
void QtVector3PropertyManager::uninitializeProperty(QtProperty *property)
{
    QtProperty *xProp = d_ptr->m_propertyToX[property];
    if (xProp) {
        d_ptr->m_xToProperty.remove(xProp);
        delete xProp;
    }
    d_ptr->m_propertyToX.remove(property);

    QtProperty *yProp = d_ptr->m_propertyToY[property];
    if (yProp) {
        d_ptr->m_yToProperty.remove(yProp);
        delete yProp;
    }
    d_ptr->m_propertyToY.remove(property);

    QtProperty *zProp = d_ptr->m_propertyToZ[property];
    if (zProp) {
        d_ptr->m_zToProperty.remove(zProp);
        delete zProp;
    }
    d_ptr->m_propertyToZ.remove(property);

    d_ptr->m_values.remove(property);
}
