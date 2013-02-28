//
//  PropertiesHelper.h
//  UIEditor
//
//  Created by Yuri Coder on 10/25/12.
//
//

#ifndef __UIEditor__PropertiesHelper__
#define __UIEditor__PropertiesHelper__

#include <QString>
#include <QVariant>

#include "BaseMetadata.h"
#include "BaseMetadataParams.h"

namespace DAVA {
    
// Helper class to get/set Metadata Properties.
class PropertiesHelper
{
public:
    // Get/set the property value for the particular param index.
    template <typename T>
        static inline T GetPropertyValue(BaseMetadata* activeMetadata, const QString& propertyName,
                                     BaseMetadataParams::METADATAPARAMID paramID);
    template <typename T>
        static inline void SetPropertyValue(BaseMetadata* activeMetadata, const QString& propertyName,
                                     BaseMetadataParams::METADATAPARAMID paramID, const T& value);
    
    // Get all property values. isPropertyValueDiffers will be set to TRUE in case the values are different
    // for different properties.
	template<typename T>
		static inline T GetAllPropertyValues(BaseMetadata* activeMetadata, const QString& propertyName);

    template<typename T>
        static inline T GetAllPropertyValues(BaseMetadata* activeMetadata, const QString& propertyName,
                                             bool& isPropertyValueDiffers);

    template<typename T>
        static inline void SetAllPropertyValues(BaseMetadata* activeMetadata, const QString& propertyName, const T& value);
};

// Implementation.
template<typename T> inline T PropertiesHelper::GetPropertyValue(BaseMetadata* activeMetadata, const QString& propertyName,
                                                                 BaseMetadataParams::METADATAPARAMID paramID)
{
    if (activeMetadata == NULL)
    {
        return T();
    }

    activeMetadata->SetActiveParamID(paramID);
    const T& paramValue = activeMetadata->property(propertyName.toStdString().c_str()).value<T>();

    return paramValue;
}

template<typename T>
    inline void PropertiesHelper::SetPropertyValue(BaseMetadata* activeMetadata, const QString& propertyName,
                                                BaseMetadataParams::METADATAPARAMID paramID,
                                                const T& value)
{
    if (activeMetadata == NULL)
    {
        return;
    }

    BaseMetadataParams::METADATAPARAMID currentParamID = activeMetadata->GetActiveParamID();
    activeMetadata->SetActiveParamID(paramID);
    activeMetadata->setProperty(propertyName.toStdString().c_str(), QVariant::fromValue<T>(value));
    activeMetadata->SetActiveParamID(currentParamID);
}

template<typename T>
    inline T PropertiesHelper::GetAllPropertyValues(BaseMetadata* activeMetadata,
                                                    const QString& propertyName)
{
	bool propertyValueDiffers = false;
	return GetAllPropertyValues<T>(activeMetadata, propertyName, propertyValueDiffers);
}


template<typename T>
    inline T PropertiesHelper::GetAllPropertyValues(BaseMetadata* activeMetadata,
                                                    const QString& propertyName,
                                                    bool& isPropertyValueDiffers)
{
    if (activeMetadata == NULL)
    {
        return T();
    }
        
    // Get the first control value.
    activeMetadata->SetActiveParamID(0);
    const T& firstValue = activeMetadata->property(propertyName.toStdString().c_str()).value<T>();
        
    // Look for the other values - start from 1, since the value #0 is already processed.
    int paramsCount = activeMetadata->GetParamsCount();
    for (BaseMetadataParams::METADATAPARAMID i = 1; i < paramsCount; i ++)
    {
        activeMetadata->SetActiveParamID(i);
        const T& curValue = activeMetadata->property(propertyName.toStdString().c_str()).value<T>();
        if (curValue != firstValue)
        {
            isPropertyValueDiffers = true;
            break;
        }
    }

    return firstValue;
}
    
template<typename T>
    inline void PropertiesHelper::SetAllPropertyValues(BaseMetadata* activeMetadata,
                                                       const QString& propertyName,
                                                       const T& value)
{
    if (activeMetadata == NULL)
    {
        return;
    }

    // Set the same property for all Params inside the metadata.
    int paramsCount = activeMetadata->GetParamsCount();
    for (BaseMetadataParams::METADATAPARAMID i = 0; i < paramsCount; i ++)
    {
        activeMetadata->SetActiveParamID(i);
        activeMetadata->setProperty(propertyName.toStdString().c_str(), QVariant::fromValue<T>(value));
    }
}

}
#include <iostream>

#endif /* defined(__UIEditor__PropertiesHelper__) */
