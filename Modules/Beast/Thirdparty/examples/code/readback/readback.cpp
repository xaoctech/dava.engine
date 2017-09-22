/*
Copyright 2014 Autodesk, Inc.  All rights reserved.
Use of this software is subject to the terms of the Autodesk license agreement
provided at the time of installation or download, or which otherwise 
accompanies this software in either electronic or hard copy form.
*/

/*
Beast API Sample: Readback

The purpose of this sample is to demonstrate how to use the API to:
 1. Set up a scene for texure baking and bake lightmaps
 2. Set up a scene for vertex baking and bake vertex colors
 3. Read back lightmaps and vertex colors
 4. Render camera views of the scene with freshly baked lightmaps
     and vertex colors
*/

#define _USE_MATH_DEFINES // for M_PI
#include <math.h>

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
#include <beastapi/beastframebuffer.h>
#include <sstream>
#include <fstream>
#include <iostream>
#include <map>

// This is the vertex resolution of the plane, affects the quality
// of the baked vertex colors. (Will give planeRes^2 vertices in the plane)
const int planeRes = 32;
// This is the vertex resolution of the sphere, affects the quality
// of the baked vertex colors. (Will give sphereRes^2 vertices in the sphere)
const int sphereRes = 16;
// This is the lightmap resolution
const int lightmapRes = 128;

// A simple scene with a sphere on a plane
class SampleScene
{
public:
    SampleScene(ILBManagerHandle bmh, const std::string& name)
    {
        m_scene = 0;
        bex::apiCall(ILBBeginScene(bmh, name.c_str(), &m_scene));
    }
    void createInstances(ILBMeshHandle plane, ILBMeshHandle sphere)
    {
        // Create an instance of the plane that will be a floor
        bex::Matrix4x4 floorTrans = bex::scaleTranslation(bex::Vec3(10.0f, 1.0f, 10.0f),
                                                          bex::Vec3(0.0f, -5.0f, 0.0f));
        bex::apiCall(ILBCreateInstance(m_scene, plane, "FloorInstance", &floorTrans, &m_floorInstance));

        // Create the sphere on the plane
        bex::Matrix4x4 trans = bex::scaleTranslation(bex::Vec3(SPHERE_RADIUS, SPHERE_RADIUS, SPHERE_RADIUS),
                                                     bex::Vec3(0.0f, SPHERE_RADIUS - 5.0f + 0.1f, 0.0f));
        bex::apiCall(ILBCreateInstance(m_scene, sphere, "SphereInstance", &trans, &m_sphereInstance));
        bex::apiCall(ILBSetRenderStats(m_sphereInstance, ILB_RS_SHADOW_BIAS, ILB_RSOP_ENABLE));
    }

    void createLight()
    {
        // Create a directional light that casts soft shadows
        ILBLightHandle light;
        bex::apiCall(ILBCreateDirectionalLight(m_scene,
                                               "Sun",
                                               &bex::directionalLightOrientation(bex::Vec3(1.0, -1.0f, -1.0f)),
                                               &bex::ColorRGB(1.0f, 1.0f, .8f),
                                               &light));
        bex::apiCall(ILBSetCastShadows(light, true));
        bex::apiCall(ILBSetShadowSamples(light, 32));
        bex::apiCall(ILBSetShadowAngle(light, .1f));
        ILBLightHandle skyLight;
        bex::apiCall(ILBCreateSkyLight(m_scene,
                                       "SkyLight",
                                       &bex::identity(),
                                       &bex::ColorRGB(1.0f, 0.0f, 0.0f),
                                       &skyLight));
    }

    void createDiffuseMaterial(const std::string& name, const bex::ColorRGBA& diffuseCol)
    {
        ILBMaterialHandle mat;
        bex::apiCall(ILBCreateMaterial(m_scene, name.c_str(), &mat));
        bex::apiCall(ILBSetMaterialColor(mat, ILB_CC_DIFFUSE, &diffuseCol));
    }

    void createEmissiveMaterial(const std::string& name, ILBTextureHandle texture)
    {
        ILBMaterialHandle mat;
        bex::apiCall(ILBCreateMaterial(m_scene, name.c_str(), &mat));
        bex::apiCall(ILBSetMaterialTexture(mat, ILB_CC_EMISSIVE, texture));
    }

    void createVertexColorMaterial(const std::string& name)
    {
        ILBMaterialHandle mat;
        bex::apiCall(ILBCreateMaterial(m_scene, name.c_str(), &mat));
        bex::apiCall(ILBSetMaterialUseVertexColors(mat, ILB_CC_EMISSIVE));
    }

    void createCamera()
    {
        // Setup a camera to render from
        bex::apiCall(ILBCreatePerspectiveCamera(m_scene, _T("Camera"), &bex::translation(bex::Vec3(0.0f, 0.0f, 10.0)), &m_camera));
    }

    void finalize()
    {
        ILBEndScene(m_scene);
    }

    ILBSceneHandle get()
    {
        return m_scene;
    }

    ILBInstanceHandle getFloorInstance()
    {
        return m_floorInstance;
    }

    ILBInstanceHandle getSphereInstance()
    {
        return m_sphereInstance;
    }

    ILBCameraHandle getCamera()
    {
        return m_camera;
    }

private:
    const static float SPHERE_RADIUS;

    ILBSceneHandle m_scene;
    ILBInstanceHandle m_floorInstance, m_sphereInstance;
    ILBCameraHandle m_camera;
};

// A job and some common operations
class SampleJob
{
public:
    SampleJob(ILBManagerHandle bmh, const std::string& name, SampleScene& scene, const std::string& xmlFile)
        : m_bmh(bmh)
    {
        bex::apiCall(ILBCreateJob(bmh, name.c_str(), scene.get(), xmlFile.c_str(), &m_job));
        createFullShadingPass();
    }

    void createCameraTarget(SampleScene& scene, unsigned int xres, unsigned int yres)
    {
        ILBTargetHandle target;
        bex::apiCall(ILBCreateCameraTarget(m_job, _T("cameraTarget"), scene.getCamera(), xres, yres, &target));
        m_targets.push_back(target);
    }

    void createTextureTarget(const std::string& name, unsigned int xres, unsigned int yres, ILBInstanceHandle instance)
    {
        ILBTargetHandle target;
        bex::apiCall(ILBCreateTextureTarget(m_job, name.c_str(), xres, yres, &target));
        ILBTargetEntityHandle entity;
        bex::apiCall(ILBAddBakeInstance(target, instance, &entity));
        m_targets.push_back(target);
        m_textureTargets[name] = target;
    }

    void createVertexTarget(const std::string& name, ILBInstanceHandle instance)
    {
        ILBTargetHandle target;
        bex::apiCall(ILBCreateVertexTarget(m_job, name.c_str(), &target));
        ILBTargetEntityHandle entity;
        bex::apiCall(ILBAddBakeInstance(target, instance, &entity));
        m_targets.push_back(target);
        m_vertexTargets[name] = std::pair<ILBTargetHandle, ILBTargetEntityHandle>(target, entity);
    }

    int run(bool returnWhenComplete, bool destroyJob)
    {
        addPassToTargets();
        if (!bex::renderJob(m_job, std::cout, returnWhenComplete, destroyJob))
        {
            return 1;
        }
        return 0;
    }

    ILBTextureHandle createTextureFromTarget(const std::string& name)
    {
        TextureTargetMap::iterator it = m_textureTargets.find(name);
        if (it == m_textureTargets.end())
        {
            std::cout << "Could not find target " << name << std::endl;
            return 0;
        }
        return bex::copyFrameBuffer(m_bmh, (*it).second, m_pass, name, true);
    }

    void readVertexColors(const std::string& name, std::vector<float>& colors)
    {
        VertexTargetMap::iterator it = m_vertexTargets.find(name);
        if (it == m_vertexTargets.end())
        {
            std::cout << "Could not find target " << name << std::endl;
            return;
        }
        bex::copyVertexBuffer((*it).second.first, m_pass, (*it).second.second, colors);
    }

private:
    void addPassToTargets()
    {
        for (size_t i = 0; i < m_targets.size(); i++)
        {
            bex::apiCall(ILBAddPassToTarget(m_targets[i], m_pass));
        }
    }

    void createFullShadingPass()
    {
        bex::apiCall(ILBCreateFullShadingPass(m_job, _T("fullShading"), &m_pass));
    }

    typedef std::map<std::string, ILBTargetHandle> TextureTargetMap;
    typedef std::map<std::string, std::pair<ILBTargetHandle, ILBTargetEntityHandle>> VertexTargetMap;

    ILBManagerHandle m_bmh;
    ILBJobHandle m_job;
    ILBRenderPassHandle m_pass;
    std::vector<ILBTargetHandle> m_targets;
    TextureTargetMap m_textureTargets;
    VertexTargetMap m_vertexTargets;
};

const float SampleScene::SPHERE_RADIUS = 4.0f;

// Creates a simple Beast XML file with selectable GI support.
void createXML(const std::string& filename, bool gi)
{
    {
        using namespace bex;
        std::ofstream ofs(filename.c_str(), std::ios_base::out | std::ios_base::trunc);
        XMLWriter xml(ofs);

        {
            ScopedTag _x(xml, "ILConfig");
            {
                ScopedTag _x(xml, "AASettings");
                xml.data("minSampleRate", 0);
                xml.data("maxSampleRate", 2);
            }
            {
                ScopedTag _x(xml, "RenderSettings");
                xml.data("bias", 0.00001f);
            }
            if (gi)
            {
                {
                    ScopedTag _x(xml, "GISettings");
                    xml.data("enableGI", true);
                    xml.data("fgRays", 1000);
                    xml.data("fgContrastThreshold", 0.1);
                    xml.data("fgInterpolationPoints", 15);
                    xml.data("primaryIntegrator", "FinalGather");
                    xml.data("secondaryIntegrator", "None");
                }
            }
        }
    }
}

// Creates a scene and renders it from a camera view. If
// lightmap is true then materials showing lightmaps will be
// created. Otherwise materials showing vertex colors will be created.
void displayResults(ILBManagerHandle bmh,
                    ILBMeshHandle floorMesh,
                    const std::string& floorMatName,
                    ILBTextureHandle floorLightmap,
                    ILBMeshHandle sphereMesh,
                    const std::string& sphereMatName,
                    ILBTextureHandle sphereLightmap,
                    const std::string& xmlFile,
                    bool lightmaps)
{
    std::string suffix = lightmaps ? "Lightmap" : "Vertexcolors";

    SampleScene scene(bmh, "RenderScene" + suffix);
    scene.createInstances(floorMesh, sphereMesh);
    scene.createCamera();

    if (lightmaps)
    {
        scene.createEmissiveMaterial(floorMatName, floorLightmap);
        scene.createEmissiveMaterial(sphereMatName, sphereLightmap);
    }
    else
    {
        scene.createVertexColorMaterial(floorMatName);
        scene.createVertexColorMaterial(sphereMatName);
    }
    scene.finalize();

    SampleJob job(bmh, "RenderJob" + suffix, scene, xmlFile);
    job.createCameraTarget(scene, 640, 480);
    job.run(false, true);
}

int main(char argc, char** argv)
{
    try
    {
        // Route errors to stderr
        bex::apiCall(ILBSetLogTarget(ILB_LT_ERROR, ILB_LS_STDERR, 0));

#if defined(WIN32)
        // Route general info to debug output (i.e output window in
        // visual studio)
        bex::apiCall(ILBSetLogTarget(ILB_LT_INFO, ILB_LS_DEBUG_OUTPUT, 0));
#else
        // Route general info to std output
        bex::apiCall(ILBSetLogTarget(ILB_LT_INFO, ILB_LS_NULL, 0));
#endif

        // Setup our beast manager
        ILBManagerHandle bmh;
        bex::apiCall(ILBCreateManager("../../../temp/readback", ILB_CS_LOCAL, bex::getLicenseKey().c_str(), &bmh));

        // Set the path to the Beast binaries
        bex::apiCall(ILBSetBeastPath(bmh, "../../../bin"));

        // Waste the cache from previous runs if present
        bex::apiCall(ILBClearCache(bmh));

        // Create a sphere and a plane meshes
        const std::string sphereMatName = "SphereMaterial";
        const std::string floorMatName = "FloorMaterial";
        ILBMeshHandle sphereMesh = bex::createSphere(bmh, "Sphere", sphereMatName, sphereRes, sphereRes);
        ILBMeshHandle floorMesh = bex::createPlane(bmh, "Floor", floorMatName, planeRes, planeRes);

        // Create a scene for baking light maps and vertex colors
        SampleScene bakingScene(bmh, "BakingScene");
        bakingScene.createInstances(floorMesh, sphereMesh);
        bakingScene.createLight();
        bakingScene.createCamera();

        // Create the floor and sphere material
        bakingScene.createDiffuseMaterial(floorMatName, bex::ColorRGBA(0.7f, 0.7f, 0.7f, 1.0f));
        bakingScene.createDiffuseMaterial(sphereMatName, bex::ColorRGBA(.9f, .9f, .9f, 1.0f));
        bakingScene.finalize();

        // Create a Beast XML file for baking with GI
        std::string xmlFileNameGI = "../../data/bakingGI.xml";
        createXML(xmlFileNameGI, true);

        // Create a job which bakes light maps and vertex colors
        SampleJob bakeJob(bmh, "BakeJob", bakingScene, xmlFileNameGI);
        bakeJob.createTextureTarget("floorTextureTarget", lightmapRes, lightmapRes, bakingScene.getFloorInstance());
        bakeJob.createTextureTarget("sphereTextureTarget", lightmapRes, lightmapRes, bakingScene.getSphereInstance());
        bakeJob.createVertexTarget("floorVertexTarget", bakingScene.getFloorInstance());
        bakeJob.createVertexTarget("sphereVertexTarget", bakingScene.getSphereInstance());

        if (bakeJob.run(true, false))
        {
            return 1;
        }

        // Read back the lightmaps as textures
        ILBTextureHandle lightmapFloor = bakeJob.createTextureFromTarget("floorTextureTarget");
        ILBTextureHandle lightmapSphere = bakeJob.createTextureFromTarget("sphereTextureTarget");

        // Read back the vertex colors
        std::vector<float> floorVertexColors;
        bakeJob.readVertexColors("floorVertexTarget", floorVertexColors);
        std::vector<float> sphereVertexColors;
        bakeJob.readVertexColors("sphereVertexTarget", sphereVertexColors);

        // The reason we re-create the meshes is that we need to add the baked vertex colors.
        ILBMeshHandle floorMeshColors = bex::createPlane(bmh, "FloorColor", floorMatName, planeRes, planeRes, &floorVertexColors);
        ILBMeshHandle sphereMeshColors = bex::createSphere(bmh, "SphereColor", sphereMatName, sphereRes, sphereRes, &sphereVertexColors);

        // Create a Beast XML file for rendering with displaying the results with the GI baked
        // to lightmaps or vertex colors.
        std::string xmlFileNameNoGI = "../../data/bakingNoGI.xml";
        createXML(xmlFileNameNoGI, false);

        // Render a scene showing the baked lightmaps
        displayResults(bmh, floorMeshColors, floorMatName, lightmapFloor,
                       sphereMeshColors, sphereMatName, lightmapSphere, xmlFileNameNoGI, true);

        // Render a scene showing the baked vertex colors
        displayResults(bmh, floorMeshColors, floorMatName, lightmapFloor,
                       sphereMeshColors, sphereMatName, lightmapSphere, xmlFileNameNoGI, false);

        return 0;
    }
    catch (bex::Exception& ex)
    {
        ILBStringHandle errorString;
        ILBStringHandle extendedError;
        ILBErrorToString(ex.status, &errorString);
        ILBGetExtendErrorInformation(&extendedError);
        std::cout << "Beast API error" << std::endl;
        std::cout << "Error: " << bex::convertStringHandle(errorString) << std::endl;
        std::cout << "Info: " << bex::convertStringHandle(extendedError) << std::endl;
        return 1;
    }
    catch (std::exception& ex)
    {
        std::cout << "Standard exception" << std::endl;
        std::cout << "Error: " << ex.what() << std::endl;
        ;
        return 1;
    }
}
