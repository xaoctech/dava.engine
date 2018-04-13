#include "DynamicBindings.h"

#include "Math/AABBox3.h"
#include "Time/SystemTimer.h"
#include "Render/RenderBase.h"

#include "Engine/Engine.h"

namespace DAVA
{
namespace
{
Array<FastName, DynamicBindings::DYNAMIC_PARAMETERS_COUNT> DYNAMIC_PARAM_NAMES;

void InitDynamicParamNames()
{
    if (DYNAMIC_PARAM_NAMES[0].IsValid() == false)
    {
        DYNAMIC_PARAM_NAMES[DynamicBindings::UNKNOWN_SEMANTIC] = FastName("unknownSemantic");

        DYNAMIC_PARAM_NAMES[DynamicBindings::PARAM_WORLD] = FastName("worldMatrix");
        DYNAMIC_PARAM_NAMES[DynamicBindings::PARAM_INV_WORLD] = FastName("invWorldMatrix");
        DYNAMIC_PARAM_NAMES[DynamicBindings::PARAM_WORLD_VIEW_OBJECT_CENTER] = FastName("worldViewObjectCenter");
        DYNAMIC_PARAM_NAMES[DynamicBindings::PARAM_WORLD_VIEW_PROJ] = FastName("worldViewProjMatrix");
        DYNAMIC_PARAM_NAMES[DynamicBindings::PARAM_INV_WORLD_VIEW_PROJ] = FastName("invWorldViewProjMatrix");
        DYNAMIC_PARAM_NAMES[DynamicBindings::PARAM_WORLD_VIEW_INV_TRANSPOSE] = FastName("worldViewInvTransposeMatrix"); //PARAM_NORMAL, // NORMAL MATRIX
        DYNAMIC_PARAM_NAMES[DynamicBindings::PARAM_WORLD_INV_TRANSPOSE] = FastName("worldInvTransposeMatrix");
        DYNAMIC_PARAM_NAMES[DynamicBindings::PARAM_WORLD_SCALE] = FastName("worldScale");

        DYNAMIC_PARAM_NAMES[DynamicBindings::PARAM_VIEW] = FastName("viewMatrix");
        DYNAMIC_PARAM_NAMES[DynamicBindings::PARAM_INV_VIEW] = FastName("invViewMatrix");
        DYNAMIC_PARAM_NAMES[DynamicBindings::PARAM_WORLD_VIEW] = FastName("worldViewMatrix");
        DYNAMIC_PARAM_NAMES[DynamicBindings::PARAM_INV_WORLD_VIEW] = FastName("invWorldViewMatrix");

        DYNAMIC_PARAM_NAMES[DynamicBindings::PARAM_VIEW_PROJ] = FastName("viewProjMatrix");
        DYNAMIC_PARAM_NAMES[DynamicBindings::PARAM_INV_VIEW_PROJ] = FastName("invViewProjMatrix");

        DYNAMIC_PARAM_NAMES[DynamicBindings::PARAM_PROJ] = FastName("projMatrix");
        DYNAMIC_PARAM_NAMES[DynamicBindings::PARAM_INV_PROJ] = FastName("invProjMatrix");

        DYNAMIC_PARAM_NAMES[DynamicBindings::PARAM_LOCAL_BOUNDING_BOX] = FastName("localBoundingBox");
        DYNAMIC_PARAM_NAMES[DynamicBindings::PARAM_BOUNDING_BOX_SIZE] = FastName("boundingBoxSize");

        DYNAMIC_PARAM_NAMES[DynamicBindings::PARAM_PROJECTION_FLIPPED] = FastName("projectionFlipped");

        DYNAMIC_PARAM_NAMES[DynamicBindings::PARAM_PREV_WORLD] = FastName("prevWorldMatrix");
        DYNAMIC_PARAM_NAMES[DynamicBindings::PARAM_PREV_VIEW_PROJ] = FastName("prevViewProjMatrix");

        DYNAMIC_PARAM_NAMES[DynamicBindings::PARAM_GLOBAL_TIME] = FastName("globalTime");

        DYNAMIC_PARAM_NAMES[DynamicBindings::PARAM_CAMERA_POS] = FastName("cameraPosition");
        DYNAMIC_PARAM_NAMES[DynamicBindings::PARAM_CAMERA_DIR] = FastName("cameraDirection");
        DYNAMIC_PARAM_NAMES[DynamicBindings::PARAM_CAMERA_UP] = FastName("cameraUp");

        DYNAMIC_PARAM_NAMES[DynamicBindings::PARAM_LIGHT0_POSITION] = FastName("lightPosition0");
        DYNAMIC_PARAM_NAMES[DynamicBindings::PARAM_LIGHT0_COLOR] = FastName("lightColor0");

        DYNAMIC_PARAM_NAMES[DynamicBindings::PARAM_JOINT_POSITIONS] = FastName("jointPositions"); // Skinned animation.
        DYNAMIC_PARAM_NAMES[DynamicBindings::PARAM_JOINT_QUATERNIONS] = FastName("jointQuaternions");
        DYNAMIC_PARAM_NAMES[DynamicBindings::PARAM_PREV_JOINT_POSITIONS] = FastName("prevJointPositions");
        DYNAMIC_PARAM_NAMES[DynamicBindings::PARAM_PREV_JOINT_QUATERNIONS] = FastName("prevJointQuaternions");
        DYNAMIC_PARAM_NAMES[DynamicBindings::PARAM_JOINTS_COUNT] = FastName("jointsCount");

        DYNAMIC_PARAM_NAMES[DynamicBindings::PARAM_VIEWPORT_SIZE] = FastName("viewportSize");
        DYNAMIC_PARAM_NAMES[DynamicBindings::PARAM_RCP_VIEWPORT_SIZE] = FastName("rcpViewportSize");
        DYNAMIC_PARAM_NAMES[DynamicBindings::PARAM_VIEWPORT_OFFSET] = FastName("viewportOffset");

        DYNAMIC_PARAM_NAMES[DynamicBindings::PARAM_HEIGHTMAP_SIZE] = FastName("heightmapSize");

        DYNAMIC_PARAM_NAMES[DynamicBindings::PARAM_SHADOW_COLOR] = FastName("shadowColor");

        DYNAMIC_PARAM_NAMES[DynamicBindings::PARAM_SHADOW_VIEW] = FastName("shadowView");
        DYNAMIC_PARAM_NAMES[DynamicBindings::PARAM_SHADOW_PARAMETERS] = FastName("shadowParameters");

        DYNAMIC_PARAM_NAMES[DynamicBindings::PARAM_SHADOW_PROJECTION_SCALE] = FastName("directionalShadowMapProjectionScale");
        DYNAMIC_PARAM_NAMES[DynamicBindings::PARAM_SHADOW_PROJECTION_OFFSET] = FastName("directionalShadowMapProjectionOffset");
        DYNAMIC_PARAM_NAMES[DynamicBindings::PARAM_SHADOW_LIGHTING_PARAMETERS] = FastName("lightingParameters");
        DYNAMIC_PARAM_NAMES[DynamicBindings::PARAM_SHADOW_PARAMS] = FastName("shadowMapParameters");

        DYNAMIC_PARAM_NAMES[DynamicBindings::PARAM_POINT_LIGHTS] = FastName("pointLights");
        DYNAMIC_PARAM_NAMES[DynamicBindings::PARAM_POINT_SHADOW_MAP_FACE_SIZE] = FastName("pointLightFaceSize");

        DYNAMIC_PARAM_NAMES[DynamicBindings::LOCAL_PROBE_CAPTURE_POSITION_IN_WORLDSPACE] = FastName("localProbeCapturePositionInWorldSpace");
        DYNAMIC_PARAM_NAMES[DynamicBindings::LOCAL_PROBE_CAPTURE_WORLD_TO_LOCAL_MATRIX] = FastName("localProbeCaptureWorldToLocalMatrix");

        DYNAMIC_PARAM_NAMES[DynamicBindings::PARAM_WIND] = FastName("wind");
        DYNAMIC_PARAM_NAMES[DynamicBindings::PARAM_PREV_WIND] = FastName("prevWind");
        DYNAMIC_PARAM_NAMES[DynamicBindings::PARAM_FLEXIBILITY] = FastName("flexibility");

        DYNAMIC_PARAM_NAMES[DynamicBindings::PARAM_RENDER_TARGET_SIZE] = FastName("renderTargetSize");

        DYNAMIC_PARAM_NAMES[DynamicBindings::PARAM_FOG_VALUES] = FastName("fogParameters");

        DYNAMIC_PARAM_NAMES[DynamicBindings::PARAM_GLOBAL_DIFFUSE_SPHERICAL_HARMONICS] = FastName("sphericalHarmonics");

        DYNAMIC_PARAM_NAMES[DynamicBindings::PARAM_GLOBAL_LUMINANCE_SCALE] = FastName("globalLuminanceScale");

        DYNAMIC_PARAM_NAMES[DynamicBindings::PARAM_VT_PAGE_INFO] = FastName("vtPageInfo");
        DYNAMIC_PARAM_NAMES[DynamicBindings::PARAM_VT_POS] = FastName("vtPos");
        DYNAMIC_PARAM_NAMES[DynamicBindings::PARAM_VT_BASIS] = FastName("vtBasis");

        DYNAMIC_PARAM_NAMES[DynamicBindings::PARAM_DISTANT_DEPTH_VALUE] = FastName("distantDepthValue");

        DYNAMIC_PARAM_NAMES[DynamicBindings::PARAM_TESSELLATION_HEIGHT] = FastName("tessellationHeight");

        DYNAMIC_PARAM_NAMES[DynamicBindings::PARAM_CAMERA_DYNAMIC_RANGE] = FastName("cameraDynamicRange");
        DYNAMIC_PARAM_NAMES[DynamicBindings::PARAM_CAMERA_TARGET_LUMINANCE] = FastName("cameraTargetLuminance");

        DYNAMIC_PARAM_NAMES[DynamicBindings::PARAM_CAMERA_PROJ_JITTER_PREV_CURR] = FastName("cameraProjJitterPrevCurr");

        DYNAMIC_PARAM_NAMES[DynamicBindings::PARAM_NDC_TO_Z_MAPPING] = FastName("ndcToZMapping");
    }
};

const FastName DYNAMIC_TEXTURE_NAMES[DynamicBindings::DYNAMIC_TEXTURE_COUNT] =
{
  FastName("globalReflection"),
  FastName("localReflection"),
  FastName("dynamicTextureSrc0"),
  FastName("dynamicTextureSrc1"),
  FastName("dynamicTextureSrc2"),
  FastName("dynamicTextureSrc3"),
  FastName("dynamicTextureLdrHistory"),
  FastName("dynamicTextureLdrCurrent")
};
}

DynamicBindings::DynamicBindings()
{
    DVASSERT(DEPENDENT_SEMANTIC_END < 64, "This is mere message for a future generations of dava render programmers - dynamicParamersRequireUpdate is a 64bit value.\
         Dynamic param with id 64 and more would not work with Compute[XXX]IfRequired, as no flag would be set for it.\
         It's now up to you to solve this problem (just hope this moment is reached when 128bit architecture is everywhere :))");

    globalLuminanceScale = GetGlobalLuminanceScale();
    SetDynamicParam(PARAM_FOG_VALUES, defaultFogValues.data, UPDATE_SEMANTIC_ALWAYS);
    SetDynamicParam(PARAM_GLOBAL_LUMINANCE_SCALE, &globalLuminanceScale, reinterpret_cast<DAVA::pointer_size>(&globalLuminanceScale));
}

DynamicBindings::eUniformSemantic DynamicBindings::GetUniformSemanticByName(const FastName& name)
{
    InitDynamicParamNames();

    for (int32 k = 0; k < DYNAMIC_PARAMETERS_COUNT; ++k)
        if (name == DYNAMIC_PARAM_NAMES[k])
            return static_cast<eUniformSemantic>(k);

    return UNKNOWN_SEMANTIC;
}

void DynamicBindings::SetDynamicParam(DynamicBindings::eUniformSemantic shaderSemantic, const void* value, pointer_size _updateSemantic)
{
    if (_updateSemantic == UPDATE_SEMANTIC_ALWAYS || dynamicParameters[shaderSemantic].updateSemantic != _updateSemantic)
    {
        if (_updateSemantic == UPDATE_SEMANTIC_ALWAYS)
            dynamicParameters[shaderSemantic].updateSemantic++;
        else
            dynamicParameters[shaderSemantic].updateSemantic = _updateSemantic;

        dynamicParameters[shaderSemantic].value = value;

        if (shaderSemantic < DEPENDENT_SEMANTIC_END)
        {
            dynamicParamersRequireUpdate &= ~(1ull << shaderSemantic);

            switch (shaderSemantic)
            {
            case PARAM_WORLD:
                dynamicParamersRequireUpdate |= ((1ull << PARAM_INV_WORLD) | (1ull << PARAM_WORLD_VIEW) | (1ull << PARAM_INV_WORLD_VIEW) | (1ull << PARAM_WORLD_VIEW_OBJECT_CENTER) | (1ull << PARAM_WORLD_VIEW_PROJ) | (1ull << PARAM_INV_WORLD_VIEW_PROJ) | (1ull << PARAM_WORLD_VIEW_INV_TRANSPOSE) | (1ull << PARAM_WORLD_INV_TRANSPOSE) | (1ull << PARAM_WORLD_SCALE));
                break;
            case PARAM_VIEW:
                dynamicParamersRequireUpdate |= ((1ull << PARAM_INV_VIEW) | (1ull << PARAM_WORLD_VIEW) | (1ull << PARAM_INV_WORLD_VIEW) | (1ull << PARAM_WORLD_VIEW_PROJ) | (1ull << PARAM_INV_WORLD_VIEW_PROJ) | (1ull << PARAM_VIEW_PROJ) | (1ull << PARAM_INV_VIEW_PROJ) | (1ull << PARAM_WORLD_VIEW_INV_TRANSPOSE) | (1ull << PARAM_WORLD_VIEW_OBJECT_CENTER));
                break;
            case PARAM_PROJ:
                dynamicParamersRequireUpdate |= ((1ull << PARAM_INV_PROJ) | (1ull << PARAM_VIEW_PROJ) | (1ull << PARAM_INV_VIEW_PROJ) | (1ull << PARAM_WORLD_VIEW_PROJ) | (1ull << PARAM_INV_WORLD_VIEW_PROJ));
                break;
            case PARAM_LOCAL_BOUNDING_BOX:
                dynamicParamersRequireUpdate |= (1ull << PARAM_BOUNDING_BOX_SIZE) | (1ull << PARAM_WORLD_VIEW_OBJECT_CENTER);
                break;
            case PARAM_PROJECTION_FLIPPED: //store projection flipped locally
                projectionFlipped = *(reinterpret_cast<const float32*>(value));
                dynamicParameters[shaderSemantic].value = &projectionFlipped;
                break;
            default:
                break;
            }
        }
    }
}

void DynamicBindings::ComputeWorldViewMatrixIfRequired()
{
    if (dynamicParamersRequireUpdate & (1 << PARAM_WORLD_VIEW))
    {
        worldViewMatrix = GetDynamicParamMatrix(PARAM_WORLD) * GetDynamicParamMatrix(PARAM_VIEW);
        SetDynamicParam(PARAM_WORLD_VIEW, &worldViewMatrix, UPDATE_SEMANTIC_ALWAYS);
    }
}

void DynamicBindings::ComputeWorldViewObjectCenterIfRequired()
{
    ComputeWorldViewMatrixIfRequired();
    if (dynamicParamersRequireUpdate & (1 << PARAM_WORLD_VIEW_OBJECT_CENTER))
    {
        const AABBox3* objectBox = reinterpret_cast<const AABBox3*>(GetDynamicParam(PARAM_LOCAL_BOUNDING_BOX));
        const Matrix4& worldView = GetDynamicParamMatrix(PARAM_WORLD_VIEW);
        worldViewObjectCenter = objectBox->GetCenter() * worldView;
        SetDynamicParam(PARAM_WORLD_VIEW_OBJECT_CENTER, &worldViewObjectCenter, UPDATE_SEMANTIC_ALWAYS);
    }
}

void DynamicBindings::ComputeWorldScaleIfRequired()
{
    if (dynamicParamersRequireUpdate & (1 << PARAM_WORLD_SCALE))
    {
        worldScale = GetDynamicParamMatrix(PARAM_WORLD).GetScaleVector();

        SetDynamicParam(PARAM_WORLD_SCALE, &worldScale, UPDATE_SEMANTIC_ALWAYS);
    }
}

inline void DynamicBindings::ComputeLocalBoundingBoxSizeIfRequired()
{
    if (dynamicParamersRequireUpdate & (1 << PARAM_BOUNDING_BOX_SIZE))
    {
        const AABBox3* objectBox = reinterpret_cast<const AABBox3*>(GetDynamicParam(PARAM_LOCAL_BOUNDING_BOX));
        boundingBoxSize = objectBox->GetSize();

        SetDynamicParam(PARAM_BOUNDING_BOX_SIZE, &boundingBoxSize, UPDATE_SEMANTIC_ALWAYS);
    }
}

inline void DynamicBindings::ComputeViewProjMatrixIfRequired()
{
    if (dynamicParamersRequireUpdate & (1 << PARAM_VIEW_PROJ))
    {
        viewProjMatrix = GetDynamicParamMatrix(PARAM_VIEW) * GetDynamicParamMatrix(PARAM_PROJ);
        SetDynamicParam(PARAM_VIEW_PROJ, &viewProjMatrix, UPDATE_SEMANTIC_ALWAYS);
    }
}

inline void DynamicBindings::ComputeWorldViewProjMatrixIfRequired()
{
    if (dynamicParamersRequireUpdate & (1 << PARAM_WORLD_VIEW_PROJ))
    {
        ComputeViewProjMatrixIfRequired();
        worldViewProjMatrix = GetDynamicParamMatrix(PARAM_WORLD) * GetDynamicParamMatrix(PARAM_VIEW_PROJ);
        SetDynamicParam(PARAM_WORLD_VIEW_PROJ, &worldViewProjMatrix, UPDATE_SEMANTIC_ALWAYS);
    }
}

inline void DynamicBindings::ComputeInvWorldViewMatrixIfRequired()
{
    if (dynamicParamersRequireUpdate & (1 << PARAM_INV_WORLD_VIEW))
    {
        ComputeWorldViewMatrixIfRequired();
        worldViewMatrix.GetInverse(invWorldViewMatrix);
        SetDynamicParam(PARAM_INV_WORLD_VIEW, &invWorldViewMatrix, UPDATE_SEMANTIC_ALWAYS);
    }
}

inline void DynamicBindings::ComputeWorldViewInvTransposeMatrixIfRequired()
{
    if (dynamicParamersRequireUpdate & (1 << PARAM_WORLD_VIEW_INV_TRANSPOSE))
    {
        ComputeInvWorldViewMatrixIfRequired();
        normalMatrix = invWorldViewMatrix;
        normalMatrix.Transpose();
        SetDynamicParam(PARAM_WORLD_VIEW_INV_TRANSPOSE, &normalMatrix, UPDATE_SEMANTIC_ALWAYS);
    }
}

inline void DynamicBindings::ComputeInvWorldMatrixIfRequired()
{
    if (dynamicParamersRequireUpdate & (1 << PARAM_INV_WORLD))
    {
        const Matrix4& worldMatrix = GetDynamicParamMatrix(PARAM_WORLD);
        worldMatrix.GetInverse(invWorldMatrix);
        SetDynamicParam(PARAM_INV_WORLD, &invWorldMatrix, UPDATE_SEMANTIC_ALWAYS);
    }
}

inline void DynamicBindings::ComputeWorldInvTransposeMatrixIfRequired()
{
    if (dynamicParamersRequireUpdate & (1 << PARAM_WORLD_INV_TRANSPOSE))
    {
        ComputeInvWorldMatrixIfRequired();
        worldInvTransposeMatrix = invWorldMatrix;
        worldInvTransposeMatrix.Transpose();
        SetDynamicParam(PARAM_WORLD_INV_TRANSPOSE, &worldInvTransposeMatrix, UPDATE_SEMANTIC_ALWAYS);
    }
}

uint32 DynamicBindings::GetDynamicParamArraySize(DynamicBindings::eUniformSemantic shaderSemantic, uint32 defaultValue)
{
    if ((shaderSemantic == PARAM_JOINT_POSITIONS) || (shaderSemantic == PARAM_JOINT_QUATERNIONS)
        || (shaderSemantic == PARAM_PREV_JOINT_POSITIONS) || (shaderSemantic == PARAM_PREV_JOINT_QUATERNIONS)) // may be different at editor.
        return *(reinterpret_cast<const uint32*>(GetDynamicParam(PARAM_JOINTS_COUNT)));
    else
        return defaultValue;
}

const void* DynamicBindings::GetDynamicParam(eUniformSemantic shaderSemantic)
{
    switch (shaderSemantic)
    {
    case PARAM_WORLD_VIEW_PROJ:
        ComputeWorldViewProjMatrixIfRequired();
        break;
    case PARAM_INV_WORLD:
        ComputeInvWorldMatrixIfRequired();
        break;
    case PARAM_WORLD_VIEW:
        ComputeWorldViewMatrixIfRequired();
        break;
    case PARAM_WORLD_SCALE:
        ComputeWorldScaleIfRequired();
        break;
    case PARAM_INV_WORLD_VIEW:
        ComputeInvWorldViewMatrixIfRequired();
        break;
    case PARAM_WORLD_VIEW_INV_TRANSPOSE:
        ComputeWorldViewInvTransposeMatrixIfRequired();
        break;
    case PARAM_WORLD_INV_TRANSPOSE:
        ComputeWorldInvTransposeMatrixIfRequired();
        break;
    case PARAM_WORLD_VIEW_OBJECT_CENTER:
        ComputeWorldViewObjectCenterIfRequired();
        break;
    case PARAM_BOUNDING_BOX_SIZE:
        ComputeLocalBoundingBoxSizeIfRequired();
        break;
    default:
        break;
    }

    DVASSERT(dynamicParameters[shaderSemantic].value != 0);
    if (dynamicParameters[shaderSemantic].value == nullptr)
    {
        const static float emptyData[16] = {};
        dynamicParameters[shaderSemantic].value = emptyData;
    }
    return dynamicParameters[shaderSemantic].value;
}

pointer_size DynamicBindings::GetDynamicParamUpdateSemantic(eUniformSemantic shaderSemantic)
{
    return dynamicParameters[shaderSemantic].updateSemantic;
}

const FastName& DynamicBindings::GetDynamicTextureName(eTextureSemantic textureSemantic)
{
    return DYNAMIC_TEXTURE_NAMES[textureSemantic];
}

bool DynamicBindings::IsDynamicTexture(const FastName& name)
{
    for (uint32 k = 0; k < DynamicBindings::DYNAMIC_TEXTURE_COUNT; ++k)
    {
        if (name == DYNAMIC_TEXTURE_NAMES[k])
            return true;
    }
    return false;
}
}
