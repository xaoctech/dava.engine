/*
Copyright 2014 Autodesk, Inc.  All rights reserved.
Use of this software is subject to the terms of the Autodesk license agreement
provided at the time of installation or download, or which otherwise 
accompanies this software in either electronic or hard copy form.
*/

/*
Beast API Sample: Live Ernst example

The purpose of this sample is to demonstrate how to:
1. Start a live session in eRnsT
2. Change the properties of light sources
3. Receive the generated light maps back from Ernst
*/

#define _USE_MATH_DEFINES // for M_PI
#include <math.h>

#include <sstream>
#include <fstream>
#include <iostream>
#include <map>
#include <limits>

#include "objreader.h"
#include "primitives.h"
#include "textures.h"
#include "xmlwriter.h"
#include <beastapi/beaststring.h>
#include <beastapi/beastutils.h>
#include <beastapi/beastmanager.h>
#include <beastapi/beastscene.h>
#include <beastapi/beastinstance.h>
#include <beastapi/beastlightsource.h>
#include <beastapi/beastcamera.h>
#include <beastapi/beastmaterial.h>
#include <beastapi/beasttarget.h>
#include <beastapi/beastrenderpass.h>
#include <beastapi/beasttargetentity.h>

// Make sure we have a unicode safe cout define
#ifdef UNICODE
#define tcout std::wcout
#else
#define tcout std::cout
#endif

const unsigned int SPHERES = 50;
const float SPHERE_RADIUS = 1.0f;

static ILBMeshHandle constructMesh(const bex::tstring& name, const std::vector<bex::Vec3>& vertices, const std::vector<bex::Vec3>& normals, const std::vector<bex::Vec2>& uvs, bex::tstring mat, ILBManagerHandle manager)
{
    ILBMeshHandle mesh;
    bex::apiCall(ILBBeginMesh(manager, name.c_str(), &mesh));
    bex::apiCall(ILBAddVertexData(mesh, &vertices[0], &normals[0], static_cast<int32>(vertices.size())));
    bex::apiCall(ILBBeginMaterialGroup(mesh, mat.c_str()));
    std::vector<int32> indices;
    for (size_t idx = 0; idx < vertices.size(); idx++)
    {
        indices.push_back((int32)idx);
    }
    bex::apiCall(ILBAddTriangleData(mesh, &indices[0], static_cast<int32>(indices.size())));
    bex::apiCall(ILBEndMaterialGroup(mesh));
    bex::apiCall(ILBBeginUVLayer(mesh, _T("uv1")));
    bex::apiCall(ILBAddUVData(mesh, &uvs[0], static_cast<int32>(uvs.size())));
    bex::apiCall(ILBEndUVLayer(mesh));
    bex::apiCall(ILBEndMesh(mesh));

    return mesh;
}

class TargetEntity
{
public:
    TargetEntity(bex::Matrix4x4 transform, ILBMeshHandle mesh, const bex::tstring& name, ILBTargetEntityType type, int32 width = 0, int32 height = 0)
        :
        m_transform(transform)
        ,
        m_mesh(mesh)
        ,
        m_name(name.c_str())
        ,
        m_type(type)
        ,
        m_width(width)
        ,
        m_height(height)
    {
    }

    const bex::tstring& getName() const
    {
        return m_name;
    }

    void update(ILBTargetEntityType type, int32 width, int32 height)
    {
        m_type = type;
        m_width = width;
        m_height = height;
    }

    void createScene(ILBSceneHandle scene)
    {
        bex::apiCall(ILBCreateInstance(scene, m_mesh, m_name.c_str(), &m_transform, &m_instance));
        bex::apiCall(ILBSetRenderStats(m_instance, ILB_RS_SHADOW_BIAS, ILB_RSOP_ENABLE));
    }

    void create(ILBTargetHandle objVertexTarget, ILBTargetHandle objAtlasTarget) const
    {
        ILBTargetEntityHandle objEntity;
        if (m_type == ILB_TT_VERTEX)
        {
            bex::apiCall(ILBAddBakeInstance(objVertexTarget, m_instance, &objEntity));
        }
        else if (m_type == ILB_TT_TEXTURE)
        {
            bex::apiCall(ILBAddBakeInstance(objAtlasTarget, m_instance, &objEntity));
            if (m_width > 0 && m_height > 0)
            {
                bex::apiCall(ILBSetBakeResolution(objEntity, m_width, m_height));
            }
        }
    }

private:
    bex::Matrix4x4 m_transform;
    ILBTargetEntityType m_type;
    ILBInstanceHandle m_instance;
    ILBMeshHandle m_mesh;
    bex::tstring m_name;
    int32 m_width;
    int32 m_height;
};

class TargetManager
{
private:
    typedef std::map<bex::tstring, TargetEntity*> TargetMap;

public:
    ~TargetManager()
    {
        TargetMap::iterator it = m_targets.begin();
        for (; it != m_targets.end(); it++)
        {
            delete it->second;
        }
    }

    void addTarget(TargetEntity* target)
    {
        TargetMap::iterator it = m_targets.find(target->getName());
        if (it != m_targets.end())
        {
            delete it->second;
            m_targets.erase(it);
        }
        m_targets[target->getName()] = target;
    }

    void update(const bex::tstring& name, ILBTargetEntityType type, int32 width, int32 height)
    {
        TargetMap::iterator it = m_targets.find(name);
        if (it != m_targets.end())
        {
            it->second->update(type, width, height);
        }
    }

    void createScene(ILBSceneHandle scene)
    {
        TargetMap::const_iterator it = m_targets.begin();
        for (; it != m_targets.end(); it++)
        {
            it->second->createScene(scene);
        }
    }

    void create(ILBJobHandle job) const
    {
        ILBTargetHandle objVertexTarget;
        ILBTargetHandle objAtlasTarget;
        bex::apiCall(ILBCreateVertexTarget(job, _T("objVertexTarget"), &objVertexTarget));
        bex::apiCall(ILBCreateTextureTarget(job, _T("objTextureTarget"), 64, 64, &objAtlasTarget));
        TargetMap::const_iterator it = m_targets.begin();
        for (; it != m_targets.end(); it++)
        {
            it->second->create(objVertexTarget, objAtlasTarget);
        }
    }

private:
    TargetMap m_targets;
};

class Camera
{
public:
    Camera(const bex::tstring& name,
           const bex::Matrix4x4& transform,
           float horizontalFov,
           float pixelAspectRatio)
        :
        m_name(name)
        ,
        m_transform(transform)
        ,
        m_horizontalFov(horizontalFov)
        ,
        m_pixelAspectRatio(pixelAspectRatio)
    {
    }

    ILBCameraHandle create(ILBSceneHandle scene) const
    {
        ILBCameraHandle camera;
        bex::apiCall(ILBCreatePerspectiveCamera(scene, m_name.c_str(), &m_transform, &camera));
        bex::apiCall(ILBSetFov(camera, m_horizontalFov, m_pixelAspectRatio));
        return camera;
    }

private:
    bex::tstring m_name;
    bex::Matrix4x4 m_transform;
    float m_horizontalFov;
    float m_pixelAspectRatio;
};

class LightSource
{
public:
    LightSource(const bex::tstring& name,
                const bex::Matrix4x4& transform,
                const bex::ColorRGB& color)
        :
        m_name(name)
        ,
        m_displayName(name)
        ,
        m_transform(transform)
        ,
        m_color(color)
        ,
        m_intensity(1.0f)
        ,
        m_castShadows(true)
        ,
        m_shadowSamples(1)
        ,
        m_directScale(1.0f)
        ,
        m_indirectScale(1.0f)
        ,
        m_visibleForEye(true)
        ,
        m_visibleForRefl(true)
        ,
        m_visibleForRefr(true)
        ,
        m_visibleForGI(true)
    {
    }

    virtual ~LightSource()
    {
    }

    void setDisplayName(const bex::tstring& displayName)
    {
        m_displayName = displayName;
    }
    void setTransform(const bex::Matrix4x4& transform)
    {
        m_transform = transform;
    }
    void setColor(const bex::ColorRGB& color)
    {
        m_color = color;
    }
    void setIntensity(float intensity)
    {
        m_intensity = intensity;
    }
    void setCastShadows(bool castShadows)
    {
        m_castShadows = castShadows;
    }
    void setShadowSamples(int32 shadowSamples)
    {
        m_shadowSamples = shadowSamples;
    }
    void setIntensityScale(float directScale, float indirectScale)
    {
        m_directScale = directScale;
        m_indirectScale = indirectScale;
    }
    void setVisibleForEye(bool v)
    {
        m_visibleForEye = v;
    }
    void setVisibleForRefl(bool v)
    {
        m_visibleForRefl = v;
    }
    void setVisibleForRefr(bool v)
    {
        m_visibleForRefr = v;
    }
    void setVisibleForGI(bool v)
    {
        m_visibleForGI = v;
    }
    const bex::tstring& getName() const
    {
        return m_name;
    }
    ILBLightHandle getHandle() const
    {
        return m_handle;
    }
    virtual ILBLightHandle create(ILBManagerHandle beastManager, ILBSceneHandle scene) = 0;

protected:
    virtual void setBasicParameters(ILBLightHandle light) const
    {
        ILBSetLightDisplayName(light, m_displayName.c_str());
        ILBSetLightIntensity(light, m_intensity);
        ILBSetCastShadows(light, m_castShadows);
        ILBSetShadowSamples(light, m_shadowSamples);
        ILBSetIntensityScale(light, m_directScale, m_indirectScale);
        ILBSetLightStats(light, ILB_LS_VISIBLE_FOR_EYE, m_visibleForEye ? ILB_LSOP_ENABLE : ILB_LSOP_DISABLE);
        ILBSetLightStats(light, ILB_LS_VISIBLE_FOR_REFLECTIONS, m_visibleForRefl ? ILB_LSOP_ENABLE : ILB_LSOP_DISABLE);
        ILBSetLightStats(light, ILB_LS_VISIBLE_FOR_REFRACTIONS, m_visibleForRefr ? ILB_LSOP_ENABLE : ILB_LSOP_DISABLE);
        ILBSetLightStats(light, ILB_LS_VISIBLE_FOR_GI, m_visibleForGI ? ILB_LSOP_ENABLE : ILB_LSOP_DISABLE);
    }
    bex::tstring m_name;
    bex::tstring m_displayName;
    bex::Matrix4x4 m_transform;
    bex::ColorRGB m_color;
    bool m_castShadows;
    int32 m_shadowSamples;
    float m_intensity;
    float m_directScale;
    float m_indirectScale;
    bool m_visibleForEye;
    bool m_visibleForRefl;
    bool m_visibleForRefr;
    bool m_visibleForGI;
    ILBLightHandle m_handle;
};

class FalloffLightSource : public LightSource
{
public:
    FalloffLightSource(const bex::tstring& name,
                       const bex::Matrix4x4& transform,
                       const bex::ColorRGB& color)
        :
        LightSource(name, transform, color)
        ,
        m_falloffType(EXPONENT_FALLOFF)
        ,
        m_exponent(0.0f)
        ,
        m_constant(0.0f)
        ,
        m_linear(0.0f)
        ,
        m_quadratic(1.0f)
        ,
        m_cutoff(std::numeric_limits<float>::max())
        ,
        m_clamp(true)
    {
    }
    void setExponentFalloff(float cutoff, float exponent, bool clamp)
    {
        m_falloffType = EXPONENT_FALLOFF;
        m_cutoff = cutoff;
        m_exponent = exponent;
        m_clamp = clamp;
    }
    void setMaxrangeFalloff(float cutoff, float exponent)
    {
        m_falloffType = MAXRANGE_FALLOFF;
        m_cutoff = cutoff;
        m_exponent = exponent;
    }
    void setPolynomialFalloff(float cutoff, float constant, float linear, float quadratic, bool clamp)
    {
        m_falloffType = POLYNOMIAL_FALLOFF;
        m_cutoff = cutoff;
        m_constant = constant;
        m_linear = linear;
        m_quadratic = quadratic;
        m_clamp = clamp;
    }

protected:
    void setBasicParameters(ILBLightHandle light) const
    {
        LightSource::setBasicParameters(light);
        switch (m_falloffType)
        {
        case EXPONENT_FALLOFF:
            bex::apiCall(ILBSetLightExponentFalloff(light, m_cutoff, m_exponent, m_clamp));
            break;
        case MAXRANGE_FALLOFF:
            bex::apiCall(ILBSetLightMaxRangeFalloff(light, m_cutoff, m_exponent));
            break;
        case POLYNOMIAL_FALLOFF:
            bex::apiCall(ILBSetLightPolynomialFalloff(light, m_cutoff, m_constant, m_linear, m_quadratic, m_clamp));
            break;
        }
    }

    enum FallOffType
    {
        EXPONENT_FALLOFF,
        MAXRANGE_FALLOFF,
        POLYNOMIAL_FALLOFF
    };

    FallOffType m_falloffType;
    float m_cutoff;
    float m_exponent;
    float m_constant;
    float m_linear;
    float m_quadratic;
    bool m_clamp;
};

class PointLightSource : public FalloffLightSource
{
public:
    PointLightSource(const bex::tstring& name,
                     const bex::Matrix4x4& transform,
                     const bex::ColorRGB& color)
        :
        FalloffLightSource(name, transform, color)
        ,
        m_shadowRadius(0.0f)
    {
    }
    virtual ILBLightHandle create(ILBManagerHandle beastManager, ILBSceneHandle scene)
    {
        ILBLightHandle light;
        bex::apiCall(ILBCreatePointLight(scene, m_name.c_str(), &m_transform, &m_color, &light));
        setBasicParameters(light);
        m_handle = light;
        return light;
    }
    void setShadowRadius(float shadowRadius)
    {
        m_shadowRadius = shadowRadius;
    }

protected:
    void setBasicParameters(ILBLightHandle light) const
    {
        FalloffLightSource::setBasicParameters(light);
        bex::apiCall(ILBSetShadowRadius(light, m_shadowRadius));
    }

    float m_shadowRadius;
};

class SpotLightSource : public PointLightSource
{
public:
    SpotLightSource(const bex::tstring& name,
                    const bex::Matrix4x4& transform,
                    const bex::ColorRGB& color)
        :
        PointLightSource(name, transform, color)
        ,
        m_angle((float)(M_PI / 2.0f))
        ,
        m_penumbraAngle(0.0f)
        ,
        m_penumbraExponent(1.0f)
    {
    }
    virtual ILBLightHandle create(ILBManagerHandle beastManager, ILBSceneHandle scene)
    {
        ILBLightHandle light;
        bex::apiCall(ILBCreateSpotLight(scene, m_name.c_str(), &m_transform, &m_color, &light));
        setBasicParameters(light);
        m_handle = light;
        return light;
    }
    void setCone(float angle, float penumbraAngle, float penumbraExponent)
    {
        m_angle = angle;
        m_penumbraAngle = penumbraAngle;
        m_penumbraExponent = penumbraExponent;
    }

protected:
    void setBasicParameters(ILBLightHandle light) const
    {
        PointLightSource::setBasicParameters(light);
        bex::apiCall(ILBSetSpotlightCone(light, m_angle, m_penumbraAngle, m_penumbraExponent));
    }
    float m_angle;
    float m_penumbraAngle;
    float m_penumbraExponent;
};

class AreaLightSource : public FalloffLightSource
{
public:
    AreaLightSource(const bex::tstring& name,
                    const bex::Matrix4x4& transform,
                    const bex::ColorRGB& color)
        :
        FalloffLightSource(name, transform, color)
    {
    }
    virtual ILBLightHandle create(ILBManagerHandle beastManager, ILBSceneHandle scene)
    {
        ILBLightHandle light;
        bex::apiCall(ILBCreateAreaLight(scene, m_name.c_str(), &m_transform, &m_color, &light));
        setBasicParameters(light);
        m_handle = light;
        return light;
    }
};

class DirectionalLightSource : public LightSource
{
public:
    DirectionalLightSource(const bex::tstring& name,
                           const bex::Matrix4x4& transform,
                           const bex::ColorRGB& color)
        :
        LightSource(name, transform, color)
        ,
        m_shadowAngle(0.0f)
    {
    }

    void setShadowAngle(float shadowAngle)
    {
        m_shadowAngle = shadowAngle;
    }

    virtual ILBLightHandle create(ILBManagerHandle beastManager, ILBSceneHandle scene)
    {
        ILBLightHandle light;
        bex::apiCall(ILBCreateDirectionalLight(scene, m_name.c_str(), &m_transform, &m_color, &light));
        setBasicParameters(light);
        m_handle = light;
        return light;
    }

protected:
    void setBasicParameters(ILBLightHandle light) const
    {
        LightSource::setBasicParameters(light);
        bex::apiCall(ILBSetShadowAngle(light, m_shadowAngle));
    }
    float m_shadowAngle;
};

class SkyLightSource : public LightSource
{
public:
    SkyLightSource(const bex::tstring& name,
                   const bex::Matrix4x4& transform,
                   const bex::ColorRGB& color)
        :
        LightSource(name, transform, color)
        ,
        m_volumeType(ILB_LVT_INFINITY)
        ,
        m_texture(L"")
    {
    }

    virtual ILBLightHandle create(ILBManagerHandle beastManager, ILBSceneHandle scene)
    {
        ILBLightHandle light;
        bex::apiCall(ILBCreateSkyLight(scene, m_name.c_str(), &m_transform, &m_color, &light));
        setBasicParameters(light);
        if (m_texture.size())
        {
            ILBTextureHandle tex;
            if (ILBFindTexture(beastManager, m_texture.c_str(), &tex) != ILB_ST_SUCCESS)
            {
                bex::apiCall(ILBReferenceTexture(beastManager, m_texture.c_str(), m_texture.c_str(), &tex));
            }
            bex::apiCall(ILBSetSkyLightTexture(light, tex));
        }
        m_handle = light;
        return light;
    }

    void setVolumeType(ILBLightVolumeType volumeType)
    {
        m_volumeType = volumeType;
    }

    void setTexture(const bex::tstring& texture)
    {
        m_texture = texture;
    }

protected:
    void setBasicParameters(ILBLightHandle light) const
    {
        LightSource::setBasicParameters(light);
        bex::apiCall(ILBSetSkyLightVolumeType(light, m_volumeType));
    }
    ILBLightVolumeType m_volumeType;
    bex::tstring m_texture;
};

class LightManager
{
private:
    typedef std::map<bex::tstring, LightSource*> LightMap;

public:
    ~LightManager()
    {
        LightMap::iterator it = m_lights.begin();
        for (; it != m_lights.end(); it++)
        {
            delete it->second;
        }
    }
    void addLight(LightSource* light)
    {
        LightMap::iterator it = m_lights.find(light->getName());
        if (it != m_lights.end())
        {
            delete it->second;
            m_lights.erase(it);
        }
        m_lights[light->getName()] = light;
    }
    void deleteLight(const bex::tstring& name)
    {
        LightMap::iterator it = m_lights.find(name);
        if (it != m_lights.end())
        {
            delete it->second;
            m_lights.erase(it);
        }
    }
    void create(ILBManagerHandle beastManager, ILBSceneHandle scene)
    {
        LightMap::iterator it = m_lights.begin();
        for (; it != m_lights.end(); it++)
        {
            it->second->create(beastManager, scene);
        }
    }

private:
    LightMap m_lights;
};

int main(char argc, char** argv)
{
    try
    {
        ILBManagerHandle bmh;

        // Route errors to stderr
        bex::apiCall(ILBSetLogTarget(ILB_LT_ERROR, ILB_LS_STDERR, 0));

#if defined(WIN32)
        // Route general info to debug output (i.e output window in
        // visual studio)
        bex::apiCall(ILBSetLogTarget(ILB_LT_INFO, ILB_LS_DEBUG_OUTPUT, 0));
#else
        // Route general info to std output
        bex::apiCall(ILBSetLogTarget(ILB_LT_INFO, ILB_LS_STDOUT, 0));
#endif

        // Setup our beast manager
        bex::apiCall(ILBCreateManager(L"../../../temp/liveernst", ILB_CS_LOCAL, bex::getLicenseKey().c_str(), &bmh));

        // Set the path to the Beast binaries
        bex::apiCall(ILBSetBeastPath(bmh, L"../../../bin"));

        // Waste the cache from previous runs if present
        bex::apiCall(ILBClearCache(bmh));

        bex::OBJReader objReader;
        std::wstring objFileName = L"../../data/hangar.obj";
        tcout << "Starting to load file" << std::endl;
        objReader.loadObjFile(objFileName);
        tcout << "Done Loading File " << objFileName << std::endl;

        typedef std::pair<std::string, ILBMeshHandle> MeshNameHandle;
        std::vector<MeshNameHandle> meshHandles;
        std::vector<std::string> allMeshes;

        objReader.getMeshNames(allMeshes);
        for (unsigned int i = 0; i < allMeshes.size(); i++)
        {
            std::vector<bex::Vec3> vertices;
            std::vector<bex::Vec3> normals;
            std::vector<bex::Vec2> uvs;
            std::string materialName;
            objReader.getMeshData(allMeshes[i], vertices, normals, uvs, materialName);
            if (vertices.size() != 0)
            {
                const std::string& name = allMeshes[i];
                ILBMeshHandle handle = constructMesh(bex::string2tstring(name), vertices, normals, uvs, bex::string2tstring(materialName), bmh);
                meshHandles.push_back(MeshNameHandle(name, handle));
            }
        }

        LightManager lightManager;

        DirectionalLightSource* sun = new DirectionalLightSource(L"Sun",
                                                                 bex::translation(bex::Vec3(0.0f, 7.5f, -20.0f)) * bex::directionalLightOrientation(normalize(bex::Vec3(-1.0f, -1.0f, -0.25f))),
                                                                 bex::ColorRGB(0.6f, 0.6f, .5f));
        sun->setCastShadows(true);
        sun->setShadowSamples(32);
        sun->setShadowAngle(.025f);
        lightManager.addLight(sun);

        SkyLightSource* sky = new SkyLightSource(L"Sky",
                                                 bex::translation(bex::Vec3(0.0f, 5.0f, -20.0f)),
                                                 bex::ColorRGB(0.25f, 0.3f, 0.4f));
        lightManager.addLight(sky);

        ILBSceneUpVector sceneUpVector = ILB_UP_POS_Y;
        float sceneScale = 1.0f;

        Camera perspCamera(_T("Camera"), bex::translation(bex::Vec3(-40.0f, 20.0f, -50.0f)) * bex::cameraOrientation(normalize(bex::Vec3(1.0f, -0.5f, 1.0f)), bex::Vec3(0.0f, 1.0f, 0.0f)), (float)M_PI_2, 1.0f);

        TargetManager targetManager;

        bex::Matrix4x4 objTrans = bex::scaleTranslation(bex::Vec3(1.0f, 1.0f, 1.0f), bex::Vec3(0.0f, 0.0f, 0.0f));

        for (size_t i = 0; i < meshHandles.size(); ++i)
        {
            if (meshHandles[i].first == "hangar:Ground")
            {
                // Vertex bake ground
                targetManager.addTarget(new TargetEntity(objTrans, meshHandles[i].second, bex::string2tstring(meshHandles[i].first), ILB_TT_VERTEX));
            }
            else
            {
                targetManager.addTarget(new TargetEntity(objTrans, meshHandles[i].second, bex::string2tstring(meshHandles[i].first), ILB_TT_TEXTURE));
            }
        }

        // Create a scene
        ILBSceneHandle scene;
        bex::apiCall(ILBBeginScene(bmh, L"ErnstScene", &scene));

        ILBSetSceneUpVector(scene, sceneUpVector);
        ILBSetMeterPerWorldUnit(scene, sceneScale);

        lightManager.create(bmh, scene);
        targetManager.createScene(scene);

        // Create materials from obj file
        const std::map<std::string, bex::OBJReader::Material>& map = objReader.getMaterials();
        std::map<std::string, bex::OBJReader::Material>::const_iterator it = map.begin();
        for (; it != map.end(); ++it)
        {
            ILBMaterialHandle mat;
            bex::apiCall(ILBCreateMaterial(scene, bex::string2tstring(it->first).c_str(), &mat));
            bex::apiCall(ILBSetMaterialColor(mat, ILB_CC_DIFFUSE, &(it->second.diffuse)));
            bex::apiCall(ILBSetMaterialColor(mat, ILB_CC_EMISSIVE, &(it->second.emissive)));
            if (it->second.shininess > 0.0f)
            {
                bex::apiCall(ILBSetMaterialColor(mat, ILB_CC_SPECULAR, &(it->second.specular)));
                bex::apiCall(ILBSetMaterialScale(mat, ILB_CC_SHININESS, it->second.shininess));
            }
        }

        // Create the camera
        ILBCameraHandle cameraHandle = perspCamera.create(scene);

        // Finalize the scene
        bex::apiCall(ILBEndScene(scene));

        // Create Ernst job
        ILBJobHandle ernstJob;
        bex::apiCall(ILBCreateLiveErnstJob(bmh, L"ErnstJob", scene, &ernstJob));
        targetManager.create(ernstJob);

        // Run Ernst
        bex::apiCall(ILBStartJob(ernstJob, ILB_SR_CLOSE_WHEN_DONE, ILB_RD_AUTODETECT));
        bex::apiCall(ILBSetErnstGIBakingMode(ernstJob, ILB_BM_CAMERA_FRUSTUM));

        ILBLightHandle light2;
        bex::apiCall(ILBCreatePointLight(scene, L"Point", &bex::translation(bex::Vec3(-2.0, 10.0f, 1.0f)), &bex::ColorRGB(1.0f, 0.0f, 0.0f), &light2));
        bex::apiCall(ILBSetCastShadows(light2, true));

        DWORD updateFrequencyTicks = 10000;
        DWORD nextUpdateTicks = GetTickCount();
        DWORD updateIndex = 0;

        while (true)
        {
            bex::apiCall(ILBWaitJobDone(ernstJob, 500));
            ILBBool isRunning;
            bex::apiCall(ILBIsJobRunning(ernstJob, &isRunning));
            if (!isRunning)
            {
                break;
            }

            DWORD ticks = GetTickCount();
            if (ticks >= nextUpdateTicks)
            {
                nextUpdateTicks += updateFrequencyTicks;
                updateIndex += 1;

                float x = 50.0f * sinf(updateIndex * 30.0f * float(M_PI) / 180.0f);
                float y = 20.0f;
                float z = 50.0f * cosf(updateIndex * 30.0f * float(M_PI) / 180.0f);
                bex::apiCall(ILBSetLightTransform(light2, &bex::translation(bex::Vec3(x, y, z))));
            }

            while (true)
            {
                ILBJobUpdateHandle uh;
                ILBBool hasUpdate;
                bex::apiCall(ILBGetJobUpdate(ernstJob, &hasUpdate, &uh));
                if (!hasUpdate)
                {
                    break;
                }

                ILBUpdateType updateType;
                bex::apiCall(ILBGetJobUpdateType(uh, &updateType));
                switch (updateType)
                {
                case ILB_UT_UPDATE_TEXTURE:
                {
                    ILBFramebufferHandle fb;
                    int32 channels, width, height;
                    bex::apiCall(ILBGetUpdateFramebuffer(uh, &fb));
                    bex::apiCall(ILBGetChannelCount(fb, &channels));
                    bex::apiCall(ILBGetResolution(fb, &width, &height));
                    ILBTargetEntityHandle te;
                    bex::apiCall(ILBGetFramebufferTargetEntity(fb, &te));
                    ILBStringHandle sh;
                    bex::apiCall(ILBGetTargetEntityInstance(te, &sh));
                    bex::tstring instanceName = bex::convertStringHandle(sh);
                    tcout << "FBT: " << width << "x" << height << " " << instanceName << std::endl;
                    break;
                }
                case ILB_UT_UPDATE_VERTEX:
                {
                    ILBFramebufferHandle fb;
                    int32 channels, width, height;
                    bex::apiCall(ILBGetUpdateFramebuffer(uh, &fb));
                    bex::apiCall(ILBGetChannelCount(fb, &channels));
                    bex::apiCall(ILBGetResolution(fb, &width, &height));
                    ILBTargetEntityHandle te;
                    bex::apiCall(ILBGetFramebufferTargetEntity(fb, &te));
                    ILBStringHandle sh;
                    bex::apiCall(ILBGetTargetEntityInstance(te, &sh));
                    bex::tstring instanceName = bex::convertStringHandle(sh);
                    tcout << "FBV: " << width << "x" << height << " " << instanceName << std::endl;
                    break;
                }
                }
                bex::apiCall(ILBDestroyUpdate(uh));
            }
        }
        ILBJobStatus jobStatus;
        bex::apiCall(ILBGetJobResult(ernstJob, &jobStatus));

        switch (jobStatus)
        {
        case ILB_JS_INVALID_LICENSE:
            tcout << L"Could not find license for Ernst" << std::endl;
            break;
        case ILB_JS_CRASH:
            tcout << L"Ernst crashed" << std::endl;
            break;
        }

        bex::apiCall(ILBDestroyJob(ernstJob));
        bex::apiCall(ILBReleaseScene(scene));
        return 0;
    }
    catch (bex::Exception& ex)
    {
        ILBStringHandle errorString;
        ILBStringHandle extendedError;
        ILBErrorToString(ex.status, &errorString);
        ILBGetExtendErrorInformation(&extendedError);
        tcout << L"Beast API error" << std::endl;
        tcout << L"Error: " << bex::convertStringHandle(errorString) << std::endl;
        tcout << L"Info: " << bex::convertStringHandle(extendedError) << std::endl;
        return 1;
    }
    catch (std::exception& ex)
    {
        tcout << L"Standard exception" << std::endl;
        tcout << L"Error: " << ex.what() << std::endl;
        ;
        return 1;
    }
}
