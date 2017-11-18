/*
Copyright 2014 Autodesk, Inc.  All rights reserved.
Use of this software is subject to the terms of the Autodesk license agreement
provided at the time of installation or download, or which otherwise 
accompanies this software in either electronic or hard copy form.
*/

/** \file 
 * The api for specifying light sources in beast
 */ 
#ifndef BEASTLIGHTSOURCE_H
#define BEASTLIGHTSOURCE_H

#include "beastapitypes.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/**
 * Describes light types
 */
typedef enum {
    /**
	 * Directional light
	 */
    ILB_LST_DIRECTIONAL = 0,

    /**
	 * Point light
	 */
    ILB_LST_POINT,

    /**
	 * Area light
	 */
    ILB_LST_AREA,

    /**
	 * Spot light
	 */
    ILB_LST_SPOT,

    /**
	 * Window light
	 */
    ILB_LST_WINDOW,

    /**
	 * Sky light
	 */
    ILB_LST_SKY,

    /**
	 * Ambient light
	 */
    ILB_LST_AMBIENT,

} ILBLightType;

/**
 * Describes different falloff for light sources
 */
typedef enum {
    /**
	 * Computes falloff as <tt>(1.0f / distance) ^ exponent</tt>\n
	 * Note that an exponent of 0 gives no falloff
	 */
    ILB_FO_EXPONENT = 0,
    /**
	 * Computes falloff as 
	 <tt>(max((maxRange - distance), 0) / maxRange) ^ exponent</tt>\n
	 */
    ILB_FO_MAX_RANGE,

    /**
	 * OpenGL-type fall off. Not supported yet.
	 */
    ILB_FO_POLYNOMIAL

} ILBFalloffType;

/**
 * Describes different light volumes for light sources
 */
typedef enum {
    /**
	 * Specified the whole 3D space
	 */
    ILB_LVT_INFINITY = 0,

    /**
	 * A sphere volume
	 */
    ILB_LVT_SPHERE,

    /**
	 * A cube volume
	 */
    ILB_LVT_CUBE
} ILBLightVolumeType;

/**
 * Status bit masks for light sources
 */
typedef enum {
    /**
	 * Controls if the light source should be visible for camera rays
	 */
    ILB_LS_VISIBLE_FOR_EYE = 0x00000001,

    /**
	 * Controls if the light source should be visible for reflection rays
	 */
    ILB_LS_VISIBLE_FOR_REFLECTIONS = 0x00000002,

    /**
	 * Controls if the light source should be visible for refraction rays
	 */
    ILB_LS_VISIBLE_FOR_REFRACTIONS = 0x00000004,

    /**
	 * Controls if the light source should be visible for global illumination rays
	 */
    ILB_LS_VISIBLE_FOR_GI = 0x00000008,

} ILBLightStats;

/**
 * Selects if the light stats should be enabled or disabled
 */
typedef enum {
    /**
	 * Sets the light stats supplied to false
	 */
    ILB_LSOP_DISABLE,
    /**
	 * Sets the light stats supplied to true
	 */
    ILB_LSOP_ENABLE,
    /**
	 * Sets the light stats to the value of the light mask
	 */
    ILB_LSOP_SET,
} ILBLightStatOperation;

/**
 * Type representing multiple light stats.
 * Combine light stats using the or operator (|). \n Example:\n
 * ILBLightStatsMask lsMask = ILB_LS_VISIBLE_FOR_EYE | ILB_LS_VISIBLE_FOR_GI;
 */
typedef uint32 ILBLightStatsMask;

/** 
 * Add a point light to the scene
 * @param scene the scene the point light should be a part of
 * @param name the name of the point light, must be unique within the scene.
 * @param transform the object space to world space transform for the light
 * @param intensity the colored intensity of the light source
 * @param light the handle to store the generated light source in
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBCreatePointLight(ILBSceneHandle scene,
                                               ILBConstString name,
                                               const ILBMatrix4x4* transform,
                                               const ILBLinearRGB* intensity,
                                               ILBLightHandle* light);

/** 
 * Add a spot light to the scene. It points in the negative Y-direction
 * by default, use the matrix to point it in a different direction.
 * The default cone angle is 90 degrees.
 * @param scene the scene the light should be a part of
 * @param name the name of the light, must be unique within the scene.
 * @param transform the object space to world space transform for the light.
 * @param intensity the colored intensity of the light source
 * @param light the handle to store the generated light source in
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBCreateSpotLight(ILBSceneHandle scene,
                                              ILBConstString name,
                                              const ILBMatrix4x4* transform,
                                              const ILBLinearRGB* intensity,
                                              ILBLightHandle* light);

/** 
 * Add a directional light to the scene
 * It points in negative Y direction by default, use the matrix to change its
 * direction.
 * @param scene the scene the directional light should be a part of
 * @param name the name of the directional light, must be unique within the scene.
 * @param transform the object space to world space transform for the light. 
 * @param intensity the colored intensity of the light source
 * @param light the handle to store the generated light source in
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBCreateDirectionalLight(ILBSceneHandle scene,
                                                     ILBConstString name,
                                                     const ILBMatrix4x4* transform,
                                                     const ILBLinearRGB* intensity,
                                                     ILBLightHandle* light);

/** 
 * Add an area light to the scene
 * It points in negative Y direction by default, use the matrix to change its
 * direction.
 * The area light extends -1.0 to 1.0 in the X/Z dimensions, use scaling
 * to control its area
 * @param scene the scene the light should be a part of
 * @param name the name of the light, must be unique within the scene.
 * @param transform the object space to world space transform for the light. 
 * It controls the area of the light as well.
 * @param intensity the colored intensity of the light source
 * @param light the handle to store the generated light source in
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBCreateAreaLight(ILBSceneHandle scene,
                                              ILBConstString name,
                                              const ILBMatrix4x4* transform,
                                              const ILBLinearRGB* intensity,
                                              ILBLightHandle* light);

/** 
 * Add a window light to the scene
 * It points in negative Y direction by default, use the matrix to change its
 * direction.
 * The window light extends -1.0 to 1.0 in the X/Z dimensions, use scaling
 * to control its area
 * @param scene the scene the light should be a part of
 * @param name the name of the light, must be unique within the scene.
 * @param transform the object space to world space transform for the light. 
 * It controls the area of the light as well
 * @param intensity the colored intensity of the light source
 * @param light the handle to store the generated light source in
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBCreateWindowLight(ILBSceneHandle scene,
                                                ILBConstString name,
                                                const ILBMatrix4x4* transform,
                                                const ILBLinearRGB* intensity,
                                                ILBLightHandle* light);

/** 
 * Add a sky light to the scene
 * It illuminates the whole scene by default, but this can be altered by setting 
 * a light volume type to the sky light. Use the light transform matrix to 
 * change the position and size of the light volume.
 * @param scene the scene the light should be a part of
 * @param name the name of the light, must be unique within the scene.
 * @param transform the object space to world space transform for the light. 
 * It controls the light volume of the light as well
 * @param intensity the colored intensity of the light source
 * @param light the handle to store the generated light source in
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBCreateSkyLight(ILBSceneHandle scene,
                                             ILBConstString name,
                                             const ILBMatrix4x4* transform,
                                             const ILBLinearRGB* intensity,
                                             ILBLightHandle* light);

/** 
 * Add an ambient light to the scene.
 * It illuminates the whole scene by default, but this can be altered by setting 
 * a light volume type to the light. Use the light transform matrix to 
 * change the position and size of the light volume.
 * @param scene the scene the ambient light should be a part of
 * @param name the name of the ambient light, must be unique within the scene.
 * @param transform the object space to world space transform for the light
 * @param intensity the colored intensity of the light source
 * @param light the handle to store the generated light source in
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBCreateAmbientLight(ILBSceneHandle scene,
                                                 ILBConstString name,
                                                 const ILBMatrix4x4* transform,
                                                 const ILBLinearRGB* intensity,
                                                 ILBLightHandle* light);

/**
 * Sets the transform for a light source.
 * @param light the light source to change
 * @param transform the new transform
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBSetLightTransform(ILBLightHandle light,
                                                const ILBMatrix4x4* transform);

/**
 * Sets the color for a light source.
 * @param light the light source to change
 * @param color the new color
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBSetLightColor(ILBLightHandle light,
                                            const ILBLinearRGB* color);

/**
 * Sets light intensity for a light source.
 * @param light the light source to set intensity
 * @param intensity the light intensity
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBSetLightIntensity(ILBLightHandle light, float intensity);

/**
 * Flags whether the light cast shadows or not.
 * Enabled by default
 * @param light the light source in question.
 * @param castShadows sets if shadow casting should be enabled or not
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBSetCastShadows(ILBLightHandle light, ILBBool castShadows);

/**
 * Sets a radius for the light source as a shadow caster.
 * Only valid for point and spot lights.
 * The radius is 0 by default.
 * @param light the light source to set the radius on.
 * @param radius the radius.
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBSetShadowRadius(ILBLightHandle light, float radius);

/**
 * Sets the angle covered of the sky for a directional light 
 * or window light for shadow casting purposes.
 * The angle is 0 by default
 * @param light the light source to set the radius on.
 * @param angleRadians the angle in radians.
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBSetShadowAngle(ILBLightHandle light, float angleRadians);

/**
 * Sets the maximum number of shadow samples for the light source.
 * Set to 1 by default
 * @param light the light source to set the radius on.
 * @param samples the number of samples.
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBSetShadowSamples(ILBLightHandle light, int32 samples);

/** 
 * Adds a light centric light link list
 * @param light the light source to add light links to
 * @param mode sets whether to link or unlink the light to the instances
 * @param instances an array of instances to link to the light
 * @param count the number of instances present in the instances
 * array
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBAddLightLightLinks(ILBLightHandle light,
                                                 ILBLightLinkMode mode,
                                                 const ILBInstanceHandle* instances,
                                                 int32 count);

/**
 * Sets the falloff for a light source.
 * Not valid for directional lights, sky lights and ambient lights.
 * By default falloff is disabled.
 * By default clamping is enabled.
 * @param light the light source to set falloff for
 * @param type the falloff type to use
 * @param exponent sets the exponent for the falloff
 * @param cutoff sets the influence range for the light source.
 * It affects both falloff types, for exponent it's a hard cutoff
 * where the light stops affecting at all.
 * For max range it sets where the light intensity fades
 * to zero
 * @param clampToOne sets whether to clamp the falloff to be lower or equal
 * to one. If set to false, the falloff is allowed to scale the intensity of
 * the light up as well as down.
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBSetFalloff(ILBLightHandle light,
                                         ILBFalloffType type,
                                         float exponent,
                                         float cutoff,
                                         ILBBool clampToOne);

/**
 * Sets the falloff to type "Max Range" for a light source.
 * Not valid for directional lights, sky lights and ambient lights.
 * By default falloff is disabled.
 * By default clamping is enabled.
 * @param light the light source to set falloff for
 * @param cutoff sets where the light intensity reaches zero.
 * @param exponent sets the exponent for the falloff
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBSetLightMaxRangeFalloff(ILBLightHandle light,
                                                      float cutoff,
                                                      float exponent);

/**
 * Sets the falloff to type "Exponent" for a light source.
 * Not valid for directional lights, sky lights and ambient lights.
 * By default falloff is disabled.
 * By default clamping is enabled.
 * @param light the light source to set falloff for
 * @param cutoff hard cutoff range where the light stops affecting at all.
 * @param exponent sets the exponent for the falloff
 * @param clampToOne sets whether to clamp the falloff to be lower or equal
 * to one. If set to false, the falloff is allowed to scale the intensity of
 * the light up as well as down.
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBSetLightExponentFalloff(ILBLightHandle light,
                                                      float cutoff,
                                                      float exponent,
                                                      ILBBool clampToOne);

/**
 * Sets the falloff to type "Polynomial" for a light source.
 * Not valid for directional lights, sky lights and ambient lights.
 * By default falloff is disabled.
 * By default clamping is enabled.
 * @param light the light source to set falloff for
 * @param cutoff sets the influence range for the light source.
 * @param constant
 * @param linear
 * @param quadratic
 * @param clampToOne sets whether to clamp the falloff to be lower or equal
 * to one. If set to false, the falloff is allowed to scale the intensity of
 * the light up as well as down.
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBSetLightPolynomialFalloff(ILBLightHandle light,
                                                        float cutoff,
                                                        float constant,
                                                        float linear,
                                                        float quadratic,
                                                        ILBBool clampToOne);

/**
 * Sets the cone angle for a spotlight.
 * The cone is given in radians for the entire cone (as opposed to the angle 
 * towards the forward direction). The penumbra angle is the angle from the
 * edge of the cone over which the intensity falls off to zero.
 * The effective spread of the cone is
 * max( angleRadians, angleRadians + 2*penumbraAngleRadians ) since the
 * penumbra angle can be both positive and negative.
 * The default cone angle is PI / 2 (90 degrees), 
 * The default is penumbra angle 0
 * The default penumbra exponent is 1
 * @param light the light source to set cone angle for
 * @param angleRadians the angle in radians for the cone
 * @param penumbraAngleRadians the angle of the penumbra of the spot light.
 * It's given as the difference from the cone angle and can be both
 * negative and positive.
 * @param penumbraExponent the exponent for the gradient in the penumbra
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBSetSpotlightCone(ILBLightHandle light,
                                               float angleRadians,
                                               float penumbraAngleRadians,
                                               float penumbraExponent);

/**
 * Sets distance to spotlight clipping planes. Both light shading and shadow
 * casting is clipped by these planes but attenuation is still calculated from
 * light source position.\n
 * Only works for spot lights.
 * @param light the light source to set clipping distances for
 * @param nearDistance the distance to near clipping plane
 * @param farDistance the distance to far clipping plane
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBSetClippingPlanes(ILBLightHandle light, float nearDistance, float farDistance);

/**
 * Sets scale for direct and indirect light intensity for a light source.
 * @param light the light source to set intensities
 * @param directScale direct light intensity scale
 * @param indirectScale indirect light intensity scale
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBSetIntensityScale(ILBLightHandle light,
                                                float directScale,
                                                float indirectScale);

/**
 * Sets light status bits for a light source.
 * @param light the light source to set light stats for
 * @param stats the stats to modify. Can be multiple light stats or:ed together
 * @param operation selects whether to enable or disable the selected light stats.
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBSetLightStats(ILBLightHandle light,
                                            ILBLightStatsMask stats,
                                            ILBLightStatOperation operation);

/**
 * Adds a light ramp entry for falloff calculation.
 * The ramp extends from 0 to 1 in light space, use the transformation
 * matrix to control the scale.\n
 * Works on point and spot lights.
 * @param light light source to manipulate
 * @param position position in the ramp. Must be greater than 0 and 
 * greater than the last position.
 * @param value color of the given position in the ramp.
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBSetLightRampEntry(ILBLightHandle light,
                                                float position,
                                                const ILBLinearRGB* value);

/**
 * Sets a projected texture for a light source (gobo).\n
 * Only works for spot lights.
 * @param light light to add gobo on
 * @param texture texture to use as gobo
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBSetLightProjectedTexture(ILBLightHandle light,
                                                       ILBTextureHandle texture);

/** 
 * Sets the display name for the light source.
 * This name does not have to be unique within the scene and can be used by 
 * tools to give human understandable names to objects that has names that are
 * generated and for some reason isn't suitable to read
 * @param light the light to set the name for
 * @param displayName the display name to set
 */
ILB_DLL_FUNCTION ILBStatus ILBSetLightDisplayName(ILBLightHandle light,
                                                  ILBConstString displayName);

/**
 * Sets light volume type for a sky light.\n
 * Only works for sky lights
 * @deprecated Use ILBSetLightVolumeType() instead
 * @param light light to set light volume type on
 * @param type the volume type to use
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBSetSkyLightVolumeType(ILBLightHandle light,
                                                    ILBLightVolumeType type);

/**
 * Sets a texture for a sky light. The texture should be in Lat/Long format.\n
 * Only works for sky lights
 * @param light light to set texture on
 * @param texture texture to use
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBSetSkyLightTexture(ILBLightHandle light,
                                                 ILBTextureHandle texture);

/**
 * Sets texture filter size for a sky light.\n
 * Only works for sky lights
 * @param light light to set filter size on
 * @param filter the filter size to use
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBSetSkyLightTextureFilter(ILBLightHandle light,
                                                       float filter);

/**
 * Sets light volume type for a light source.\n
 * Only works for ambient lights and sky lights.
 * @param light light to set light volume type on
 * @param type the volume type to use
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBSetLightVolumeType(ILBLightHandle light,
                                                 ILBLightVolumeType type);

/**
 * Changes the type of a light source. Retains basic parameters like transform
 * and color but all light type specific parameters are reset to default.
 * @param light the light to change type
 * @param newType
 * @param newHandle
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBChangeLightType(ILBLightHandle light,
                                              ILBLightType newType,
                                              ILBLightHandle* newHandle);

/**
 * Gets the type of a light source. This is used for lights returned from an 
 * Ernst session.
 * @param light the light to get the type for
 * @param type the returned light type
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBGetLightType(ILBLightHandle light,
                                           ILBLightType* type);

/** 
 * Gets the transform of a light source. This is used for lights returned from
 * an Ernst session.
 * @param light the light to get the transform for
 * @param transform the returned light transform
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBGetLightTransform(ILBLightHandle light,
                                                ILBMatrix4x4* transform);

/** 
 * Gets the name of a light source. This is used for lights returned from
 * an Ernst session.
 * @param light the light to get the name for
 * @param name the returned light name
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBGetLightName(ILBLightHandle light,
                                           ILBStringHandle* name);

/** 
 * Gets the display name of a light source. Display names are not unique within 
 * the scene and should only be used by tools to give human understandable names to objects
 * This is used for lights returned from an Ernst session.
 * @param light the light to get the name for
 * @param displayName the returned light display name
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBGetLightDisplayName(ILBLightHandle light,
                                                  ILBStringHandle* displayName);

/** 
 * Gets the color of a light source.
 * This is used for lights returned from an Ernst session.
 * @param light the light to get the color for
 * @param color the returned light color
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBGetLightColor(ILBLightHandle light,
                                            ILBLinearRGB* color);

/**
 * Gets light intensity for a light source.
 * This is used for lights returned from an Ernst session.
 * @param light the light source to get intensity from
 * @param intensity the light intensity
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBGetLightIntensity(ILBLightHandle light,
                                                float* intensity);

/** 
 * Returns whether a light source cast shadows.
 * This is used for lights returned from an Ernst session.
 * @param light the light to check shadow status
 * @param castShadows is set to true if light source cast shadows, false otherwise
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBGetLightCastShadows(ILBLightHandle light,
                                                  ILBBool* castShadows);

/** 
 * Returns the number of shadow samples a light source will use.
 * This is used for lights returned from an Ernst session.
 * @param light the light to get shadow samples
 * @param shadowSamples the number of shadow samples
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBGetLightShadowSamples(ILBLightHandle light,
                                                    int32* shadowSamples);

/** 
 * Returns the falloff type from a light source.
 * This is used for lights returned from an Ernst session.
 * @param light the light to get shadow samples
 * @param type the returned falloff type
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBGetLightFalloffType(ILBLightHandle light,
                                                  ILBFalloffType* type);

/** 
 * Returns the falloff parameters from a light source with MaxRange falloff. 
 * This is used for lights returned from an Ernst session.
 * @param light the light to get shadow samples
 * @param cutoff the falloff cutoff
 * @param exponent the falloff exponent
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBGetLightMaxRangeFalloff(ILBLightHandle light,
                                                      float* cutoff,
                                                      float* exponent);

/** 
 * Returns the falloff parameters from a light source with Exponent falloff. 
 * This is used for lights returned from an Ernst session.
 * @param light the light to get shadow samples
 * @param cutoff the falloff cutoff
 * @param exponent the falloff exponent
 * @param clamp sets whether to clamp the falloff to be lower or equal
 * to one. If set to false, the falloff is allowed to scale the intensity of
 * the light up as well as down.
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBGetLightExponentFalloff(ILBLightHandle light,
                                                      float* cutoff,
                                                      float* exponent,
                                                      ILBBool* clamp);

/** 
 * Returns the falloff parameters from a light source with Polynomial falloff. 
 * This is used for lights returned from an Ernst session.
 * @param light the light to get shadow samples
 * @param cutoff the falloff cutoff
 * @param constant the falloff constant factor
 * @param linear the falloff linear factor
 * @param quadratic the falloff quadratic factor
 * @param clamp sets whether to clamp the falloff to be lower or equal
 * to one. If set to false, the falloff is allowed to scale the intensity of
 * the light up as well as down.
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBGetLightPolynomialFalloff(ILBLightHandle light,
                                                        float* cutoff,
                                                        float* constant,
                                                        float* linear,
                                                        float* quadratic,
                                                        ILBBool* clamp);

/**
 * Gets scale for direct and indirect light intensity for a light source.
 * This is used for lights returned from an Ernst session.
 * @param light the light source to get intensities
 * @param directScale direct light intensity scale
 * @param indirectScale indirect light intensity scale
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBGetLightIntensityScale(ILBLightHandle light,
                                                     float* directScale,
                                                     float* indirectScale);

/**
 * Query light stats for a light source.
 * @param light the light source to get light stats from.
 * @param stats the stats to query. Can be multiple light stats or:ed together.
 * @param result the result of the query. A bitwise AND operation is done on
 * the individual stats.
 * given stats are true the result is true, and false otherwise.
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBGetLightStats(ILBLightHandle light,
                                            ILBLightStatsMask stats,
                                            ILBLightStatsMask* result);

/**
 * Gets the cone angle for a spotlight.
 * The cone is given in radians for the entire cone (as opposed to the angle 
 * towards the forward direction). The penumbra angle is the angle from the
 * edge of the cone over which the intensity falls off to zero.
 * The effective spread of the cone is
 * max( angleRadians, angleRadians + 2*penumbraAngleRadians ) since the
 * penumbra angle can be both positive and negative.
 * This is used for lights returned from an Ernst session.
 * @param light the light source to get cone angle for
 * @param angleRadians the angle in radians for the cone
 * @param penumbraAngleRadians the angle of the penumbra of the spot light.
 * It's given as the difference from the cone angle and can be both
 * negative and positive.
 * @param penumbraExponent the exponent for the gradient in the penumbra
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBGetSpotlightCone(ILBLightHandle light,
                                               float* angleRadians,
                                               float* penumbraAngleRadians,
                                               float* penumbraExponent);

/**
* Gets distance to spotlight clipping planes. Both light shading and shadow
* casting is clipped by these planes but attenuation is still calculated from
* light source position.\n
* Only works for spot lights.
* @param light the light source to get clipping distances for
* @param nearDistance the distance to near clipping plane
* @param farDistance the distance to far clipping plane
* @return The result of the operation.
*/
ILB_DLL_FUNCTION ILBStatus ILBGetClippingPlanes(ILBLightHandle light, float* nearDistance, float* farDistance);

/**
 * Gets a radius for the light source as a shadow caster.
 * Only valid for point and spot lights.
 * This is used for lights returned from an Ernst session.
 * @param light the light source to get the radius on.
 * @param radius the radius.
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBGetShadowRadius(ILBLightHandle light, float* radius);

/**
 * Gets the angle covered of the sky for a directional light 
 * or window light for shadow casting purposes.
 * This is used for lights returned from an Ernst session.
 * @param light the light source to get the radius on.
 * @param angleRadians the angle in radians.
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBGetShadowAngle(ILBLightHandle light, float* angleRadians);

/**
 * Gets light volume type for a sky light.\n
 * Only valid for sky lights
 * This is used for lights returned from an Ernst session.
 * @deprecated Use ILBGetLightVolumeType() instead
 * @param light light to set light volume type on
 * @param type the volume type to use
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBGetSkyLightVolumeType(ILBLightHandle light, ILBLightVolumeType* type);

/**
 * Gets a texture from a sky light.\n
 * Only valid for sky lights
 * This is used for lights returned from an Ernst session.
 * @param light light to get texture from
 * @param texture the texture handle
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBGetSkyLightTexture(ILBLightHandle light, ILBTextureHandle* texture);

/**
 * Gets texture filter size from a sky light.\n
 * Only valid for sky lights
 * This is used for lights returned from an Ernst session.
 * @param light light to get filter size from
 * @param filter the filter size
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBGetSkyLightTextureFilter(ILBLightHandle light, float* filter);

/**
 * Gets light volume type for a light source.\n
 * Only valid for ambient lights and sky lights.
 * This is used for lights returned from an Ernst session.
 * @param light light to set light volume type on
 * @param type the volume type to use
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBGetLightVolumeType(ILBLightHandle light, ILBLightVolumeType* type);

/**
 * Deletes a light source from a scene.
 * Only possible for dynamic scenes, ie. scenes associated with a live ernst 
 * job.
 * @param light light to delete
 * @return The result of the operation.
 */
ILB_DLL_FUNCTION ILBStatus ILBDeleteLightSource(ILBLightHandle light);

#ifdef __cplusplus
}
#endif // __cplusplus


#endif // BEASTLIGHTSOURCE_H
