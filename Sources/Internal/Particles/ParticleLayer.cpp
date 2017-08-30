#include "Particles/ParticleLayer.h"
#include "Particles/ParticleEmitter.h"
#include "Utils/StringFormat.h"
#include "Render/Image/Image.h"
#include "FileSystem/FileSystem.h"
#include "Logger/Logger.h"
#include "Particles/ParticleDragForce.h"

#include "Reflection/ReflectionRegistrator.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(ParticleLayer)
{
    ReflectionRegistrator<ParticleLayer>::Begin()
        .End();
}

using ForceShape = ParticleDragForce::eShape;
using ForceTimingType = ParticleDragForce::eTimingType;
using ForceType = ParticleDragForce::eType;

namespace ParticleLayerDetail
{
struct ShapeMap
{
    ForceShape elemType;
    String name;
};
const Array<ShapeMap, 2> shapeMap =
{ {
{ ForceShape::BOX, "box" },
{ ForceShape::SPHERE, "sphere" }
} };

struct TimingTypeMap
{
    ForceTimingType elemType;
    String name;
};
const Array<TimingTypeMap, 3> timingTypesMap =
{ {
{ ForceTimingType::CONSTANT, "const" },
{ ForceTimingType::OVER_LAYER_LIFE, "ovr_layer" },
{ ForceTimingType::OVER_PARTICLE_LIFE, "ovr_prt" }
} };

struct ForceTypeMap
{
    ForceType elemType;
    String name;
};
const Array<ForceTypeMap, 6> forceTypesMap =
{ {
{ ForceType::DRAG_FORCE, "drag" },
{ ForceType::LORENTZ_FORCE, "lorentz" },
{ ForceType::POINT_GRAVITY, "pointgr"},
{ ForceType::BOX_WRAP, "boxwr"},
{ ForceType::GRAVITY, "grav" },
{ ForceType::WIND, "wind"}
} };

template <typename T, typename U, size_t sz>
T StringToType(const String& typeName, T defaultVal, const Array<U, sz> map)
{
    for (const auto& e : map)
    {
        if (e.name == typeName)
            return e.elemType;
    }

    return defaultVal;
}

template <typename T, typename U, size_t sz>
String TypeToString(T type, const String& defaultName, const Array<U, sz> map)
{
    for (const auto& e : map)
    {
        if (e.elemType == type)
            return e.name;
    }
    return defaultName;
}
}

const ParticleLayer::LayerTypeNamesInfo ParticleLayer::layerTypeNamesInfoMap[] =
{
  { TYPE_SINGLE_PARTICLE, "single" },
  { TYPE_PARTICLES, "particles" },
  { TYPE_PARTICLE_STRIPE, "particlesStripe" },
  { TYPE_SUPEREMITTER_PARTICLES, "superEmitter" }
};

/*the following code is legacy compatibility to load original particle blending nodes*/
enum eBlendMode
{
    BLEND_NONE = 0,
    BLEND_ZERO,
    BLEND_ONE,
    BLEND_DST_COLOR,
    BLEND_ONE_MINUS_DST_COLOR,
    BLEND_SRC_ALPHA,
    BLEND_ONE_MINUS_SRC_ALPHA,
    BLEND_DST_ALPHA,
    BLEND_ONE_MINUS_DST_ALPHA,
    BLEND_SRC_ALPHA_SATURATE,
    BLEND_SRC_COLOR,
    BLEND_ONE_MINUS_SRC_COLOR,

    BLEND_MODE_COUNT,
};
const char* BLEND_MODE_NAMES[BLEND_MODE_COUNT] =
{
  "BLEND_NONE",
  "BLEND_ZERO",
  "BLEND_ONE",
  "BLEND_DST_COLOR",
  "BLEND_ONE_MINUS_DST_COLOR",
  "BLEND_SRC_ALPHA",
  "BLEND_ONE_MINUS_SRC_ALPHA",
  "BLEND_DST_ALPHA",
  "BLEND_ONE_MINUS_DST_ALPHA",
  "BLEND_SRC_ALPHA_SATURATE",
  "BLEND_SRC_COLOR",
  "BLEND_ONE_MINUS_SRC_COLOR"
};

eBlendMode GetBlendModeByName(const String& blendStr)
{
    for (uint32 i = 0; i < BLEND_MODE_COUNT; i++)
        if (blendStr == BLEND_MODE_NAMES[i])
            return static_cast<eBlendMode>(i);

    return BLEND_MODE_COUNT;
}

/*end of legacy compatibility code*/

ParticleLayer::ParticleLayer()
{
    activeLODS.resize(4, true);
}

ParticleLayer::~ParticleLayer()
{
    SafeRelease(innerEmitter);

    CleanupForces();
    // dynamic cache automatically delete all particles
}

ParticleLayer* ParticleLayer::Clone()
{
    ParticleLayer* dstLayer = new ParticleLayer();

    dstLayer->stripeLifetime = stripeLifetime;
    dstLayer->stripeVertexSpawnStep = stripeVertexSpawnStep;
    dstLayer->stripeStartSize = stripeStartSize;
    dstLayer->stripeUScrollSpeed = stripeUScrollSpeed;
    dstLayer->stripeVScrollSpeed = stripeVScrollSpeed;
    dstLayer->stripeFadeDistanceFromTop = stripeFadeDistanceFromTop;
    dstLayer->alphaOverLife = alphaOverLife;

    if (stripeSizeOverLife)
        dstLayer->stripeSizeOverLife.Set(stripeSizeOverLife->Clone());

    if (stripeTextureTileOverLife)
        dstLayer->stripeTextureTileOverLife.Set(stripeTextureTileOverLife->Clone());

    if (stripeNoiseUScrollSpeedOverLife)
        dstLayer->stripeNoiseUScrollSpeedOverLife.Set(stripeNoiseUScrollSpeedOverLife->Clone());

    if (stripeNoiseVScrollSpeedOverLife)
        dstLayer->stripeNoiseVScrollSpeedOverLife.Set(stripeNoiseVScrollSpeedOverLife->Clone());

    if (stripeColorOverLife)
        dstLayer->stripeColorOverLife.Set(stripeColorOverLife->Clone());

    if (flowSpeed)
        dstLayer->flowSpeed.Set(flowSpeed->Clone());
    if (flowSpeedVariation)
        dstLayer->flowSpeedVariation.Set(flowSpeedVariation->Clone());

    if (flowOffset)
        dstLayer->flowOffset.Set(flowOffset->Clone());
    if (flowOffsetVariation)
        dstLayer->flowOffsetVariation.Set(flowOffsetVariation->Clone());

    if (noiseScale)
        dstLayer->noiseScale.Set(noiseScale->Clone());
    if (noiseScaleVariation)
        dstLayer->noiseScaleVariation.Set(noiseScaleVariation->Clone());
    if (noiseScaleOverLife)
        dstLayer->noiseScaleOverLife.Set(noiseScaleOverLife->Clone());

    if (noiseUScrollSpeed)
        dstLayer->noiseUScrollSpeed.Set(noiseUScrollSpeed->Clone());
    if (noiseUScrollSpeedVariation)
        dstLayer->noiseUScrollSpeedVariation.Set(noiseUScrollSpeedVariation->Clone());
    if (noiseUScrollSpeedOverLife)
        dstLayer->noiseUScrollSpeedOverLife.Set(noiseUScrollSpeedOverLife->Clone());

    if (noiseVScrollSpeed)
        dstLayer->noiseVScrollSpeed.Set(noiseVScrollSpeed->Clone());
    if (noiseVScrollSpeedVariation)
        dstLayer->noiseVScrollSpeedVariation.Set(noiseVScrollSpeedVariation->Clone());
    if (noiseVScrollSpeedOverLife)
        dstLayer->noiseVScrollSpeedOverLife.Set(noiseVScrollSpeedOverLife->Clone());

    if (life)
        dstLayer->life.Set(life->Clone());

    if (lifeVariation)
        dstLayer->lifeVariation.Set(lifeVariation->Clone());

    if (number)
        dstLayer->number.Set(number->Clone());

    if (numberVariation)
        dstLayer->numberVariation.Set(numberVariation->Clone());

    if (size)
        dstLayer->size.Set(size->Clone());

    if (sizeVariation)
        dstLayer->sizeVariation.Set(sizeVariation->Clone());

    if (sizeOverLifeXY)
        dstLayer->sizeOverLifeXY.Set(sizeOverLifeXY->Clone());

    if (velocity)
        dstLayer->velocity.Set(velocity->Clone());

    if (velocityVariation)
        dstLayer->velocityVariation.Set(velocityVariation->Clone());

    if (velocityOverLife)
        dstLayer->velocityOverLife.Set(velocityOverLife->Clone());

    // Copy the forces.
    dstLayer->CleanupForces();
    dstLayer->forces.reserve(forces.size());
    for (size_t f = 0; f < forces.size(); ++f)
    {
        ParticleForce* clonedForce = this->forces[f]->Clone();
        dstLayer->AddForce(clonedForce);
        clonedForce->Release();
    }

    dstLayer->CleanupDrag();
    dstLayer->dragForces.reserve(dragForces.size());
    for (size_t f = 0; f < dragForces.size(); ++f)
    {
        ParticleDragForce* clonedForce = dragForces[f]->Clone();
        dstLayer->AddDrag(clonedForce);
        clonedForce->Release();
    }

    if (spin)
        dstLayer->spin.Set(spin->Clone());

    if (spinVariation)
        dstLayer->spinVariation.Set(spinVariation->Clone());

    if (spinOverLife)
        dstLayer->spinOverLife.Set(spinOverLife->Clone());
    dstLayer->randomSpinDirection = randomSpinDirection;

    if (animSpeedOverLife)
        dstLayer->animSpeedOverLife.Set(animSpeedOverLife->Clone());

    if (colorOverLife)
        dstLayer->colorOverLife.Set(colorOverLife->Clone());

    if (colorRandom)
        dstLayer->colorRandom.Set(colorRandom->Clone());

    if (alphaOverLife)
        dstLayer->alphaOverLife.Set(alphaOverLife->Clone());

    if (angle)
        dstLayer->angle.Set(angle->Clone());

    if (angleVariation)
        dstLayer->angleVariation.Set(angleVariation->Clone());

    SafeRelease(dstLayer->innerEmitter);
    if (innerEmitter)
        dstLayer->innerEmitter = static_cast<ParticleEmitter*>(innerEmitter->Clone());
    dstLayer->innerEmitterPath = innerEmitterPath;

    dstLayer->layerName = layerName;

    dstLayer->enableFlow = enableFlow;
    dstLayer->enableFlowAnimation = enableFlowAnimation;

    dstLayer->enableNoise = enableNoise;
    dstLayer->enableNoiseScroll = enableNoiseScroll;

    dstLayer->blending = blending;
    dstLayer->enableFog = enableFog;
    dstLayer->enableFrameBlend = enableFrameBlend;
    dstLayer->inheritPosition = inheritPosition;
    dstLayer->stripeInheritPositionOnlyForBaseVertex = stripeInheritPositionOnlyForBaseVertex;
    dstLayer->usePerspectiveMapping = usePerspectiveMapping;
    dstLayer->startTime = startTime;
    dstLayer->endTime = endTime;

    dstLayer->isLooped = isLooped;
    dstLayer->deltaTime = deltaTime;
    dstLayer->deltaVariation = deltaVariation;
    dstLayer->loopVariation = loopVariation;
    dstLayer->loopEndTime = loopEndTime;

    dstLayer->isDisabled = isDisabled;

    dstLayer->type = type;
    dstLayer->degradeStrategy = degradeStrategy;
    dstLayer->sprite = sprite;
    dstLayer->flowmap = flowmap;
    dstLayer->noise = noise;
    dstLayer->alphaRemapSprite = alphaRemapSprite;
    dstLayer->layerPivotPoint = layerPivotPoint;
    dstLayer->layerPivotSizeOffsets = layerPivotSizeOffsets;

    dstLayer->frameOverLifeEnabled = frameOverLifeEnabled;
    dstLayer->frameOverLifeFPS = frameOverLifeFPS;
    dstLayer->randomFrameOnStart = randomFrameOnStart;
    dstLayer->loopSpriteAnimation = loopSpriteAnimation;
    dstLayer->particleOrientation = particleOrientation;

    dstLayer->scaleVelocityBase = scaleVelocityBase;
    dstLayer->scaleVelocityFactor = scaleVelocityFactor;

    dstLayer->spritePath = spritePath;
    dstLayer->flowmapPath = flowmapPath;
    dstLayer->activeLODS = activeLODS;
    dstLayer->isLong = isLong;
    dstLayer->useFresnelToAlpha = useFresnelToAlpha;
    dstLayer->fresnelToAlphaBias = fresnelToAlphaBias;
    dstLayer->fresnelToAlphaPower = fresnelToAlphaPower;
    dstLayer->noisePath = noisePath;
    dstLayer->enableNoise = enableNoise;
    dstLayer->enableNoiseScroll = enableNoiseScroll;

    dstLayer->alphaRemapPath = alphaRemapPath;
    if (alphaRemapOverLife)
        dstLayer->alphaRemapOverLife.Set(alphaRemapOverLife->Clone());
    dstLayer->enableAlphaRemap = enableAlphaRemap;
    dstLayer->alphaRemapLoopCount = alphaRemapLoopCount;

    return dstLayer;
}

bool ParticleLayer::IsLodActive(int32 lod)
{
    if ((lod >= 0) && (lod < static_cast<int32>(activeLODS.size())))
        return activeLODS[lod];

    return false;
}

void ParticleLayer::SetLodActive(int32 lod, bool active)
{
    if ((lod >= 0) && (lod < static_cast<int32>(activeLODS.size())))
        activeLODS[lod] = active;
}

template <class T>
void UpdatePropertyLineKeys(PropertyLine<T>* line, float32 startTime, float32 translateTime, float32 endTime)
{
    if (!line)
        return;
    Vector<typename PropertyLine<T>::PropertyKey>& keys = line->GetValues();
    int32 size = static_cast<int32>(keys.size());
    int32 i;
    for (i = 0; i < size; ++i)
    {
        keys[i].t += translateTime;
        if (keys[i].t > endTime)
            break;
    }
    if (i == 0)
        i += 1; //keep at least 1
    keys.erase(keys.begin() + i, keys.end());
    if (keys.size() == 1)
    {
        keys[0].t = startTime;
    }
}

template <class T>
void UpdatePropertyLineOnLoad(PropertyLine<T>* line, float32 startTime, float32 endTime)
{
    if (!line)
        return;
    Vector<typename PropertyLine<T>::PropertyKey>& keys = line->GetValues();
    int32 size = static_cast<int32>(keys.size());
    int32 i;
    /*drop keys before*/
    for (i = 0; i < size; ++i)
    {
        if (keys[i].t >= startTime)
            break;
    }
    if (i != 0)
    {
        T v0 = line->GetValue(startTime);
        keys.erase(keys.begin(), keys.begin() + i);
        typename PropertyLine<T>::PropertyKey key;
        key.t = startTime;
        key.value = v0;
        keys.insert(keys.begin(), key);
    }

    /*drop keys after*/
    size = static_cast<int32>(keys.size());
    for (i = 0; i < size; i++)
    {
        if (keys[i].t > endTime)
            break;
    }
    if (i != size)
    {
        T v1 = line->GetValue(endTime);
        keys.erase(keys.begin() + i, keys.end());
        typename PropertyLine<T>::PropertyKey key;
        key.t = endTime;
        key.value = v1;
        keys.push_back(key);
    }
}

void ParticleLayer::UpdateLayerTime(float32 startTime, float32 endTime)
{
    float32 translateTime = startTime - this->startTime;
    this->startTime = startTime;
    this->endTime = endTime;
    /*validate all time depended property lines*/
    UpdatePropertyLineKeys(PropertyLineHelper::GetValueLine(life).Get(), startTime, translateTime, endTime);
    UpdatePropertyLineKeys(PropertyLineHelper::GetValueLine(lifeVariation).Get(), startTime, translateTime, endTime);
    UpdatePropertyLineKeys(PropertyLineHelper::GetValueLine(number).Get(), startTime, translateTime, endTime);
    UpdatePropertyLineKeys(PropertyLineHelper::GetValueLine(numberVariation).Get(), startTime, translateTime, endTime);
    UpdatePropertyLineKeys(PropertyLineHelper::GetValueLine(size).Get(), startTime, translateTime, endTime);
    UpdatePropertyLineKeys(PropertyLineHelper::GetValueLine(sizeVariation).Get(), startTime, translateTime, endTime);
    UpdatePropertyLineKeys(PropertyLineHelper::GetValueLine(velocity).Get(), startTime, translateTime, endTime);
    UpdatePropertyLineKeys(PropertyLineHelper::GetValueLine(velocityVariation).Get(), startTime, translateTime, endTime);
    UpdatePropertyLineKeys(PropertyLineHelper::GetValueLine(spin).Get(), startTime, translateTime, endTime);
    UpdatePropertyLineKeys(PropertyLineHelper::GetValueLine(spinVariation).Get(), startTime, translateTime, endTime);
    UpdatePropertyLineKeys(PropertyLineHelper::GetValueLine(angle).Get(), startTime, translateTime, endTime);
    UpdatePropertyLineKeys(PropertyLineHelper::GetValueLine(angleVariation).Get(), startTime, translateTime, endTime);
    UpdatePropertyLineKeys(PropertyLineHelper::GetValueLine(flowSpeed).Get(), startTime, translateTime, endTime);
    UpdatePropertyLineKeys(PropertyLineHelper::GetValueLine(flowSpeedVariation).Get(), startTime, translateTime, endTime);
    UpdatePropertyLineKeys(PropertyLineHelper::GetValueLine(flowOffset).Get(), startTime, translateTime, endTime);
    UpdatePropertyLineKeys(PropertyLineHelper::GetValueLine(flowOffsetVariation).Get(), startTime, translateTime, endTime);
    UpdatePropertyLineKeys(PropertyLineHelper::GetValueLine(noiseScale).Get(), startTime, translateTime, endTime);
    UpdatePropertyLineKeys(PropertyLineHelper::GetValueLine(noiseScaleVariation).Get(), startTime, translateTime, endTime);
    UpdatePropertyLineKeys(PropertyLineHelper::GetValueLine(noiseUScrollSpeed).Get(), startTime, translateTime, endTime);
    UpdatePropertyLineKeys(PropertyLineHelper::GetValueLine(noiseUScrollSpeedVariation).Get(), startTime, translateTime, endTime);
    UpdatePropertyLineKeys(PropertyLineHelper::GetValueLine(noiseVScrollSpeed).Get(), startTime, translateTime, endTime);
    UpdatePropertyLineKeys(PropertyLineHelper::GetValueLine(noiseVScrollSpeedVariation).Get(), startTime, translateTime, endTime);
}

void ParticleLayer::SetSprite(const FilePath& path)
{
    spritePath = path;
    if (type != TYPE_SUPEREMITTER_PARTICLES)
        sprite.reset(Sprite::Create(spritePath));
}

void ParticleLayer::SetPivotPoint(Vector2 pivot)
{
    layerPivotPoint = pivot;
    layerPivotSizeOffsets = Vector2(1 + std::abs(layerPivotPoint.x), 1 + std::abs(layerPivotPoint.y));
    layerPivotSizeOffsets *= 0.5f;
}

void ParticleLayer::SetFlowmap(const FilePath& spritePath_)
{
    flowmapPath = spritePath_;
    if (type != TYPE_SUPEREMITTER_PARTICLES)
        flowmap.reset(Sprite::Create(flowmapPath));
}

void ParticleLayer::SetNoise(const FilePath& spritePath_)
{
    noisePath = spritePath_;
    if (type != TYPE_SUPEREMITTER_PARTICLES)
        noise.reset(Sprite::Create(noisePath));
}

void ParticleLayer::SetAlphaRemap(const FilePath& spritePath_)
{
    alphaRemapPath = spritePath_;
    if (type != TYPE_SUPEREMITTER_PARTICLES)
        alphaRemapSprite.reset(Sprite::Create(alphaRemapPath));
}

void ParticleLayer::LoadFromYaml(const FilePath& configPath, const YamlNode* node, bool preserveInheritPosition)
{
    stripeSizeOverLife = PropertyLineYamlReader::CreatePropertyLine<float32>(node->Get("stripeSizeOverLifeProp"));
    stripeTextureTileOverLife = PropertyLineYamlReader::CreatePropertyLine<float32>(node->Get("stripeTextureTileOverLife"));
    stripeColorOverLife = PropertyLineYamlReader::CreatePropertyLine<Color>(node->Get("stripeColorOverLife"));
    stripeNoiseUScrollSpeedOverLife = PropertyLineYamlReader::CreatePropertyLine<float32>(node->Get("stripeNoiseUScrollSpeedOverLife"));
    stripeNoiseVScrollSpeedOverLife = PropertyLineYamlReader::CreatePropertyLine<float32>(node->Get("stripeNoiseVScrollSpeedOverLife"));

    stripeLifetime = 0.0f;
    const YamlNode* stripeLifetimeNode = node->Get("stripeLifetime");
    if (stripeLifetimeNode)
    {
        stripeLifetime = stripeLifetimeNode->AsFloat();
    }
    stripeVertexSpawnStep = 1.0f;
    const YamlNode* stripeVertexSpawnStepNode = node->Get("stripeVertexSpawnStep");
    if (stripeVertexSpawnStepNode)
    {
        stripeVertexSpawnStep = stripeVertexSpawnStepNode->AsFloat();
    }
    stripeStartSize = 0.0f;
    const YamlNode* stripeStartSizeNode = node->Get("stripeStartSize");
    if (stripeStartSizeNode)
    {
        stripeStartSize = stripeStartSizeNode->AsFloat();
    }
    stripeUScrollSpeed = 0.0f;
    const YamlNode* stripeUScrollSpeedNode = node->Get("stripeUScrollSpeed");
    if (stripeUScrollSpeedNode)
    {
        stripeUScrollSpeed = stripeUScrollSpeedNode->AsFloat();
    }
    stripeVScrollSpeed = 0.0f;
    const YamlNode* stripeVScrollSpeedNode = node->Get("stripeVScrollSpeed");
    if (stripeVScrollSpeedNode)
    {
        stripeVScrollSpeed = stripeVScrollSpeedNode->AsFloat();
    }

    stripeFadeDistanceFromTop = 0.0f;
    const YamlNode* stripeFadeDistanceFromTopNode = node->Get("stripeFadeDistanceFromTop");
    if (stripeFadeDistanceFromTopNode)
    {
        stripeFadeDistanceFromTop = stripeFadeDistanceFromTopNode->AsFloat();
    }

    // format processing
    int32 format = 0;
    const YamlNode* formatNode = node->Get("effectFormat");
    if (formatNode)
    {
        format = formatNode->AsInt32();
    }

    type = TYPE_PARTICLES;
    const YamlNode* typeNode = node->Get("layerType");
    if (typeNode)
    {
        type = StringToLayerType(typeNode->AsString(), TYPE_PARTICLES);
    }

    degradeStrategy = DEGRADE_KEEP;
    const YamlNode* degradeNode = node->Get("degradeStrategy");
    if (degradeNode)
    {
        degradeStrategy = static_cast<eDegradeStrategy>(degradeNode->AsInt());
    }

    const YamlNode* nameNode = node->Get("name");
    if (nameNode)
    {
        layerName = nameNode->AsString();
    }

    const YamlNode* longNode = node->Get("isLong");
    if (longNode)
    {
        isLong = longNode->AsBool();
    }

    const YamlNode* useFresToAlphaNode = node->Get("useFresToAlpha");
    if (useFresToAlphaNode)
        useFresnelToAlpha = useFresToAlphaNode->AsBool();
    const YamlNode* fresToAlphaBiasNode = node->Get("fresToAlphaBias");
    if (fresToAlphaBiasNode)
        fresnelToAlphaBias = fresToAlphaBiasNode->AsFloat();
    const YamlNode* fresToAlphaPowerNode = node->Get("fresToAlphaPower");
    if (fresToAlphaPowerNode)
        fresnelToAlphaPower = fresToAlphaPowerNode->AsFloat();

    const YamlNode* pivotPointNode = node->Get("pivotPoint");

    const YamlNode* spriteNode = node->Get("sprite");
    if (spriteNode && !spriteNode->AsString().empty())
    {
        // Store the absolute path to sprite.
        FilePath spritePath = configPath.GetDirectory() + spriteNode->AsString();
        SetSprite(spritePath);
    }
    const YamlNode* flowmapNode = node->Get("flowmap");
    if (flowmapNode && !flowmapNode->AsString().empty())
    {
        FilePath flowPath = configPath.GetDirectory() + flowmapNode->AsString();
        SetFlowmap(flowPath);
    }
    const YamlNode* noiseNode = node->Get("noise");
    if (noiseNode && !noiseNode->AsString().empty())
    {
        FilePath noisePath = configPath.GetDirectory() + noiseNode->AsString();
        SetNoise(noisePath);
    }
    const YamlNode* alphaRemapNode = node->Get("alphaRemap");
    if (alphaRemapNode && !alphaRemapNode->AsString().empty())
    {
        FilePath alphaRemapPath = configPath.GetDirectory() + alphaRemapNode->AsString();
        SetAlphaRemap(alphaRemapPath);
    }

    if (pivotPointNode)
    {
        Vector2 _pivot = pivotPointNode->AsPoint();
        if ((format == 0) && sprite)
        {
            float32 ny = -_pivot.x / sprite->GetWidth() * 2;
            float32 nx = -_pivot.y / sprite->GetHeight() * 2;
            _pivot.Set(nx, ny);
        }

        SetPivotPoint(_pivot);
    }

    const YamlNode* lodsNode = node->Get("activeLODS");
    if (lodsNode)
    {
        const Vector<YamlNode*>& vec = lodsNode->AsVector();
        for (size_t i = 0; i < vec.size(); ++i)
            SetLodActive(static_cast<int32>(i), (vec[i]->AsInt()) != 0); //as AddToArray has no override for bool, flags are stored as int
    }

    colorOverLife = PropertyLineYamlReader::CreatePropertyLine<Color>(node->Get("colorOverLife"));
    colorRandom = PropertyLineYamlReader::CreatePropertyLine<Color>(node->Get("colorRandom"));
    alphaOverLife = PropertyLineYamlReader::CreatePropertyLine<float32>(node->Get("alphaOverLife"));

    const YamlNode* frameOverLifeEnabledNode = node->Get("frameOverLifeEnabled");
    if (frameOverLifeEnabledNode)
    {
        frameOverLifeEnabled = frameOverLifeEnabledNode->AsBool();
    }

    const YamlNode* randomFrameOnStartNode = node->Get("randomFrameOnStart");
    if (randomFrameOnStartNode)
    {
        randomFrameOnStart = randomFrameOnStartNode->AsBool();
    }
    const YamlNode* loopSpriteAnimationNode = node->Get("loopSpriteAnimation");
    if (loopSpriteAnimationNode)
    {
        loopSpriteAnimation = loopSpriteAnimationNode->AsBool();
    }

    const YamlNode* particleOrientationNode = node->Get("particleOrientation");
    if (particleOrientationNode)
    {
        particleOrientation = particleOrientationNode->AsInt32();
    }

    const YamlNode* frameOverLifeFPSNode = node->Get("frameOverLifeFPS");
    if (frameOverLifeFPSNode)
    {
        frameOverLifeFPS = frameOverLifeFPSNode->AsFloat();
    }

    const YamlNode* scaleVelocityBaseNode = node->Get("scaleVelocityBase");
    if (scaleVelocityBaseNode)
    {
        scaleVelocityBase = scaleVelocityBaseNode->AsFloat();
    }

    const YamlNode* scaleVelocityFactorNode = node->Get("scaleVelocityFactor");
    if (scaleVelocityFactorNode)
    {
        scaleVelocityFactor = scaleVelocityFactorNode->AsFloat();
    }

    alphaRemapLoopCount = 1.0f;
    const YamlNode* alphaRemapLoopCountNode = node->Get("alphaRemapLoopCount");
    if (alphaRemapLoopCountNode)
    {
        alphaRemapLoopCount = alphaRemapLoopCountNode->AsFloat();
    }

    alphaRemapOverLife = PropertyLineYamlReader::CreatePropertyLine<float32>(node->Get("alphaRemapOverLife"));

    flowSpeed = PropertyLineYamlReader::CreatePropertyLine<float32>(node->Get("flowSpeed"));
    flowSpeedVariation = PropertyLineYamlReader::CreatePropertyLine<float32>(node->Get("flowSpeedVariation"));
    flowOffset = PropertyLineYamlReader::CreatePropertyLine<float32>(node->Get("flowOffset"));
    flowOffsetVariation = PropertyLineYamlReader::CreatePropertyLine<float32>(node->Get("flowOffsetVariation"));

    noiseScale = PropertyLineYamlReader::CreatePropertyLine<float32>(node->Get("noiseScale"));
    noiseScaleVariation = PropertyLineYamlReader::CreatePropertyLine<float32>(node->Get("noiseScaleVariation"));
    noiseScaleOverLife = PropertyLineYamlReader::CreatePropertyLine<float32>(node->Get("noiseScaleOverLife"));
    noiseUScrollSpeed = PropertyLineYamlReader::CreatePropertyLine<float32>(node->Get("noiseUScrollSpeed"));
    noiseUScrollSpeedVariation = PropertyLineYamlReader::CreatePropertyLine<float32>(node->Get("noiseUScrollSpeedVariation"));
    noiseUScrollSpeedOverLife = PropertyLineYamlReader::CreatePropertyLine<float32>(node->Get("noiseUScrollSpeedOverLife"));
    noiseVScrollSpeed = PropertyLineYamlReader::CreatePropertyLine<float32>(node->Get("noiseVScrollSpeed"));
    noiseVScrollSpeedVariation = PropertyLineYamlReader::CreatePropertyLine<float32>(node->Get("noiseVScrollSpeedVariation"));
    noiseVScrollSpeedOverLife = PropertyLineYamlReader::CreatePropertyLine<float32>(node->Get("noiseVScrollSpeedOverLife"));

    life = PropertyLineYamlReader::CreatePropertyLine<float32>(node->Get("life"));
    lifeVariation = PropertyLineYamlReader::CreatePropertyLine<float32>(node->Get("lifeVariation"));

    number = PropertyLineYamlReader::CreatePropertyLine<float32>(node->Get("number"));
    numberVariation = PropertyLineYamlReader::CreatePropertyLine<float32>(node->Get("numberVariation"));

    size = PropertyLineYamlReader::CreatePropertyLine<Vector2>(node->Get("size"));

    sizeVariation = PropertyLineYamlReader::CreatePropertyLine<Vector2>(node->Get("sizeVariation"));

    sizeOverLifeXY = PropertyLineYamlReader::CreatePropertyLine<Vector2>(node->Get("sizeOverLifeXY"));

    // Yuri Coder, 2013/04/03. sizeOverLife is outdated and kept here for the backward compatibility only.
    // New property is sizeOverlifeXY and contains both X and Y components.
    RefPtr<PropertyLine<float32>> sizeOverLife = PropertyLineYamlReader::CreatePropertyLine<float32>(node->Get("sizeOverLife"));
    if (sizeOverLife)
    {
        if (sizeOverLifeXY)
        {
            // Both properties can't be present in the same config.
            Logger::Error("Both sizeOverlife and sizeOverlifeXY are defined for Particle Layer %s, taking sizeOverlifeXY as default",
                          configPath.GetAbsolutePathname().c_str());
            DVASSERT(false);
        }
        else
        {
            // Only the outdated sizeOverlife is defined - create sizeOverlifeXY property based on outdated one.
            FillSizeOverlifeXY(sizeOverLife);
        }
    }

    velocity = PropertyLineYamlReader::CreatePropertyLine<float32>(node->Get("velocity"));
    velocityVariation = PropertyLineYamlReader::CreatePropertyLine<float32>(node->Get("velocityVariation"));
    velocityOverLife = PropertyLineYamlReader::CreatePropertyLine<float32>(node->Get("velocityOverLife"));

    angle = PropertyLineYamlReader::CreatePropertyLine<float32>(node->Get("angle"));
    angleVariation = PropertyLineYamlReader::CreatePropertyLine<float32>(node->Get("angleVariation"));

    int32 forceCount = 0;
    const YamlNode* forceCountNode = node->Get("forceCount");
    if (forceCountNode)
        forceCount = forceCountNode->AsInt();

    for (int k = 0; k < forceCount; ++k)
    {
        // Any of the Force Parameters might be NULL, and this is acceptable.
        RefPtr<PropertyLine<Vector3>> force = PropertyLineYamlReader::CreatePropertyLine<Vector3>(node->Get(Format("force%d", k)));
        RefPtr<PropertyLine<float32>> forceOverLife = PropertyLineYamlReader::CreatePropertyLine<float32>(node->Get(Format("forceOverLife%d", k)));

        if (force && (!format)) //as no forceVariation anymore - add it directly to force
        {
            RefPtr<PropertyLine<Vector3>> forceVariation = PropertyLineYamlReader::CreatePropertyLine<Vector3>(node->Get(Format("forceVariation%d", k)));
            if (forceVariation)
            {
                Vector3 varriationToAdd = forceVariation->GetValue(0);
                Vector<typename PropertyLine<Vector3>::PropertyKey>& keys = force->GetValues();
                for (size_t i = 0, sz = keys.size(); i < sz; ++i)
                {
                    keys[i].value += varriationToAdd;
                }
            }
        }

        ParticleForce* particleForce = new ParticleForce(force, forceOverLife);
        AddForce(particleForce);
        particleForce->Release();
    }

    LoadForcesFromYaml(node);

    spin = PropertyLineYamlReader::CreatePropertyLine<float32>(node->Get("spin"));
    spinVariation = PropertyLineYamlReader::CreatePropertyLine<float32>(node->Get("spinVariation"));
    spinOverLife = PropertyLineYamlReader::CreatePropertyLine<float32>(node->Get("spinOverLife"));
    animSpeedOverLife = PropertyLineYamlReader::CreatePropertyLine<float32>(node->Get("animSpeedOverLife"));
    const YamlNode* randomSpinDirectionNode = node->Get("randomSpinDirection");
    if (randomSpinDirectionNode)
    {
        randomSpinDirection = randomSpinDirectionNode->AsBool();
    }

    blending = BLENDING_ALPHABLEND; //default

    //read blend node for backward compatibility with old effect files
    const YamlNode* blend = node->Get("blend");
    if (blend)
    {
        if (blend->AsString() == "alpha")
        {
            blending = BLENDING_ALPHABLEND;
        }
        if (blend->AsString() == "add")
        {
            blending = BLENDING_ALPHA_ADDITIVE;
        }
    }

    const YamlNode* blendSrcNode = node->Get("srcBlendFactor");
    const YamlNode* blendDestNode = node->Get("dstBlendFactor");

    if (blendSrcNode && blendDestNode)
    {
        eBlendMode srcBlendFactor = GetBlendModeByName(blendSrcNode->AsString());
        eBlendMode dstBlendFactor = GetBlendModeByName(blendDestNode->AsString());

        if ((srcBlendFactor == BLEND_ONE) && (dstBlendFactor == BLEND_ONE))
            blending = BLENDING_ADDITIVE;
        else if ((srcBlendFactor == BLEND_SRC_ALPHA) && (dstBlendFactor == BLEND_ONE))
            blending = BLENDING_ALPHA_ADDITIVE;
        else if ((srcBlendFactor == BLEND_ONE_MINUS_DST_COLOR) && (dstBlendFactor == BLEND_ONE))
            blending = BLENDING_SOFT_ADDITIVE;
        else if ((srcBlendFactor == BLEND_DST_COLOR) && (dstBlendFactor == BLEND_ZERO))
            blending = BLENDING_MULTIPLICATIVE;
        else if ((srcBlendFactor == BLEND_DST_COLOR) && (dstBlendFactor == BLEND_SRC_COLOR))
            blending = BLENDING_STRONG_MULTIPLICATIVE;
    }

    //end of legacy

    const YamlNode* blendingNode = node->Get("blending");
    if (blendingNode)
        blending = static_cast<eBlending>(blendingNode->AsInt());
    const YamlNode* fogNode = node->Get("enableFog");
    if (fogNode)
    {
        enableFog = fogNode->AsBool();
    }
    const YamlNode* enableFlowNode = node->Get("enableFlow");
    if (enableFlowNode)
    {
        enableFlow = enableFlowNode->AsBool();
    }
    const YamlNode* enableFlowAnimationNode = node->Get("enableFlowAnimation");
    if (enableFlowAnimationNode)
    {
        enableFlowAnimation = enableFlowAnimationNode->AsBool();
    }

    const YamlNode* enableNoiseNode = node->Get("enableNoise");
    if (enableNoiseNode)
    {
        enableNoise = enableNoiseNode->AsBool();
    }

    const YamlNode* useNoiseScrollNode = node->Get("useNoiseScroll");
    if (useNoiseScrollNode)
    {
        enableNoiseScroll = useNoiseScrollNode->AsBool();
    }

    const YamlNode* enableAlphaRemapNode = node->Get("enableAlphaRemap");
    if (enableAlphaRemapNode)
    {
        enableAlphaRemap = enableAlphaRemapNode->AsBool();
    }

    const YamlNode* frameBlendNode = node->Get("enableFrameBlend");
    if (frameBlendNode)
    {
        enableFrameBlend = frameBlendNode->AsBool();
    }

    startTime = 0.0f;
    endTime = 100000000.0f;
    const YamlNode* startTimeNode = node->Get("startTime");
    if (startTimeNode)
        startTime = startTimeNode->AsFloat();

    const YamlNode* endTimeNode = node->Get("endTime");
    if (endTimeNode)
        endTime = endTimeNode->AsFloat();

    isLooped = false;
    deltaTime = 0.0f;
    deltaVariation = 0.0f;
    loopVariation = 0.0f;

    const YamlNode* isLoopedNode = node->Get("isLooped");
    if (isLoopedNode)
        isLooped = isLoopedNode->AsBool();

    const YamlNode* deltaTimeNode = node->Get("deltaTime");
    if (deltaTimeNode)
        deltaTime = deltaTimeNode->AsFloat();

    const YamlNode* deltaVariationNode = node->Get("deltaVariation");
    if (deltaVariationNode)
        deltaVariation = deltaVariationNode->AsFloat();

    const YamlNode* loopVariationNode = node->Get("loopVariation");
    if (loopVariationNode)
        loopVariation = loopVariationNode->AsFloat();

    const YamlNode* loopEndTimeNode = node->Get("loopEndTime");
    if (loopEndTimeNode)
        loopEndTime = loopEndTimeNode->AsFloat();

    /*validate all time depended property lines*/
    UpdatePropertyLineOnLoad(stripeSizeOverLife.Get(), startTime, endTime);

    UpdatePropertyLineOnLoad(alphaRemapOverLife.Get(), startTime, endTime);

    UpdatePropertyLineOnLoad(flowSpeed.Get(), startTime, endTime);
    UpdatePropertyLineOnLoad(flowSpeedVariation.Get(), startTime, endTime);
    UpdatePropertyLineOnLoad(flowOffset.Get(), startTime, endTime);
    UpdatePropertyLineOnLoad(flowOffsetVariation.Get(), startTime, endTime);

    UpdatePropertyLineOnLoad(noiseScale.Get(), startTime, endTime);
    UpdatePropertyLineOnLoad(noiseScaleVariation.Get(), startTime, endTime);
    UpdatePropertyLineOnLoad(noiseScaleOverLife.Get(), startTime, endTime);
    UpdatePropertyLineOnLoad(noiseUScrollSpeed.Get(), startTime, endTime);
    UpdatePropertyLineOnLoad(noiseUScrollSpeedVariation.Get(), startTime, endTime);
    UpdatePropertyLineOnLoad(noiseUScrollSpeedOverLife.Get(), startTime, endTime);
    UpdatePropertyLineOnLoad(noiseVScrollSpeed.Get(), startTime, endTime);
    UpdatePropertyLineOnLoad(noiseVScrollSpeedVariation.Get(), startTime, endTime);
    UpdatePropertyLineOnLoad(noiseVScrollSpeedOverLife.Get(), startTime, endTime);

    UpdatePropertyLineOnLoad(life.Get(), startTime, endTime);
    UpdatePropertyLineOnLoad(lifeVariation.Get(), startTime, endTime);

    UpdatePropertyLineOnLoad(number.Get(), startTime, endTime);
    UpdatePropertyLineOnLoad(numberVariation.Get(), startTime, endTime);

    UpdatePropertyLineOnLoad(size.Get(), startTime, endTime);
    UpdatePropertyLineOnLoad(sizeVariation.Get(), startTime, endTime);

    UpdatePropertyLineOnLoad(velocity.Get(), startTime, endTime);
    UpdatePropertyLineOnLoad(velocityVariation.Get(), startTime, endTime);

    UpdatePropertyLineOnLoad(spin.Get(), startTime, endTime);
    UpdatePropertyLineOnLoad(spinVariation.Get(), startTime, endTime);

    UpdatePropertyLineOnLoad(angle.Get(), startTime, endTime);
    UpdatePropertyLineOnLoad(angleVariation.Get(), startTime, endTime);

    const YamlNode* inheritPositionNode = node->Get("inheritPosition");
    if (inheritPositionNode)
    {
        inheritPosition = inheritPositionNode->AsBool();
    }
    const YamlNode* stripeInheritPositionForBaseNode = node->Get("stripeInheritPositionForBase");
    if (stripeInheritPositionForBaseNode)
        stripeInheritPositionOnlyForBaseVertex = stripeInheritPositionForBaseNode->AsBool();

    const YamlNode* usePerspectiveMappingNode = node->Get("usePerspectiveMapping");
    if (usePerspectiveMappingNode)
        usePerspectiveMapping = usePerspectiveMappingNode->AsBool();

    // Load the Inner Emitter parameters.
    const YamlNode* innerEmitterPathNode = node->Get("innerEmitterPath");
    if ((type == TYPE_SUPEREMITTER_PARTICLES) && innerEmitterPathNode)
    {
        SafeRelease(innerEmitter);
        innerEmitter = new ParticleEmitter();
        // Since Inner Emitter path is stored as Relative, convert it to absolute when loading.
        String relativePath = innerEmitterPathNode->AsString();
        if (relativePath.empty())
        {
            Logger::Error("Failed to load inner emitter from empty config path");
        }
        else
        {
            innerEmitterPath = configPath.GetDirectory() + relativePath;
            if (innerEmitterPath == configPath) // prevent recursion
            {
                Logger::Error("Attempt to load inner emitter from super emitter's config will cause recursion");
            }
            else
            {
                innerEmitter->LoadFromYaml(this->innerEmitterPath, true);
            }
        }
    }
    if (format == 0) //update old stuff
    {
        UpdateSizeLine(size.Get(), true, !isLong);
        UpdateSizeLine(sizeVariation.Get(), true, !isLong);
        UpdateSizeLine(sizeOverLifeXY.Get(), false, !isLong);
        inheritPosition &= preserveInheritPosition;
    }
}

void ParticleLayer::UpdateSizeLine(PropertyLine<Vector2>* line, bool rescaleSize, bool swapXY)
{
    //conversion from old format
    if (!line)
        return;
    if ((!rescaleSize) && (!swapXY))
        return; //nothing to update

    Vector<typename PropertyLine<Vector2>::PropertyKey>& keys = PropertyLineHelper::GetValueLine(line)->GetValues();
    for (size_t i = 0, sz = keys.size(); i < sz; ++i)
    {
        if (rescaleSize)
        {
            keys[i].value.x *= 0.5f;
            keys[i].value.y *= 0.5f;
        }
        if (swapXY)
        {
            float x = keys[i].value.x;
            keys[i].value.x = keys[i].value.y;
            keys[i].value.y = x;
        }
    }
}

void ParticleLayer::SaveToYamlNode(const FilePath& configPath, YamlNode* parentNode, int32 layerIndex)
{
    YamlNode* layerNode = new YamlNode(YamlNode::TYPE_MAP);
    String layerNodeName = Format("layer%d", layerIndex);
    parentNode->AddNodeToMap(layerNodeName, layerNode);

    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "stripeSizeOverLifeProp", stripeSizeOverLife);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "stripeTextureTileOverLife", stripeTextureTileOverLife);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "stripeNoiseUScrollSpeedOverLife", stripeNoiseUScrollSpeedOverLife);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "stripeNoiseVScrollSpeedOverLife", stripeNoiseVScrollSpeedOverLife);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<Color>(layerNode, "stripeColorOverLife", stripeColorOverLife);

    PropertyLineYamlWriter::WritePropertyValueToYamlNode(layerNode, "stripeLifetime", stripeLifetime);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode(layerNode, "stripeVertexSpawnStep", stripeVertexSpawnStep);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode(layerNode, "stripeStartSize", stripeStartSize);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode(layerNode, "stripeUScrollSpeed", stripeUScrollSpeed);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode(layerNode, "stripeVScrollSpeed", stripeVScrollSpeed);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode(layerNode, "stripeFadeDistanceFromTop", stripeFadeDistanceFromTop);

    PropertyLineYamlWriter::WritePropertyValueToYamlNode<String>(layerNode, "name", layerName);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<String>(layerNode, "type", "layer");
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<String>(layerNode, "layerType",
                                                                 LayerTypeToString(type, "particles"));

    PropertyLineYamlWriter::WritePropertyValueToYamlNode<int32>(layerNode, "degradeStrategy", static_cast<int32>(degradeStrategy));
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<bool>(layerNode, "isLong", isLong);

    PropertyLineYamlWriter::WritePropertyValueToYamlNode<Vector2>(layerNode, "pivotPoint", layerPivotPoint);

    PropertyLineYamlWriter::WritePropertyValueToYamlNode(layerNode, "useFresToAlpha", useFresnelToAlpha);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode(layerNode, "fresToAlphaBias", fresnelToAlphaBias);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode(layerNode, "fresToAlphaPower", fresnelToAlphaPower);

    PropertyLineYamlWriter::WritePropertyValueToYamlNode(layerNode, "alphaRemapLoopCount", alphaRemapLoopCount);

    SaveSpritePath(spritePath, configPath, layerNode, "sprite");
    SaveSpritePath(flowmapPath, configPath, layerNode, "flowmap");
    SaveSpritePath(noisePath, configPath, layerNode, "noise");
    SaveSpritePath(alphaRemapPath, configPath, layerNode, "alphaRemap");

    layerNode->Add("enableAlphaRemap", enableAlphaRemap);

    layerNode->Add("blending", blending);

    layerNode->Add("enableFlow", enableFlow);
    layerNode->Add("enableFlowAnimation", enableFlowAnimation);

    layerNode->Add("enableNoise", enableNoise);
    layerNode->Add("useNoiseScroll", enableNoiseScroll);

    layerNode->Add("enableFog", enableFog);
    layerNode->Add("enableFrameBlend", enableFrameBlend);

    layerNode->Add("scaleVelocityBase", scaleVelocityBase);
    layerNode->Add("scaleVelocityFactor", scaleVelocityFactor);

    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "alphaRemapOverLife", this->alphaRemapOverLife);

    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "flowSpeed", this->flowSpeed);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "flowSpeedVariation", this->flowSpeedVariation);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "flowOffset", this->flowOffset);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "flowOffsetVariation", this->flowOffsetVariation);

    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "noiseScale", this->noiseScale);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "noiseScaleVariation", this->noiseScaleVariation);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "noiseScaleOverLife", this->noiseScaleOverLife);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "noiseUScrollSpeed", this->noiseUScrollSpeed);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "noiseUScrollSpeedVariation", this->noiseUScrollSpeedVariation);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "noiseUScrollSpeedOverLife", this->noiseUScrollSpeedOverLife);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "noiseVScrollSpeed", this->noiseVScrollSpeed);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "noiseVScrollSpeedVariation", this->noiseVScrollSpeedVariation);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "noiseVScrollSpeedOverLife", this->noiseVScrollSpeedOverLife);

    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "life", this->life);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "lifeVariation", this->lifeVariation);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "number", this->number);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "numberVariation", this->numberVariation);

    PropertyLineYamlWriter::WritePropertyLineToYamlNode<Vector2>(layerNode, "size", this->size);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<Vector2>(layerNode, "sizeVariation", this->sizeVariation);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<Vector2>(layerNode, "sizeOverLifeXY", this->sizeOverLifeXY);

    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "velocity", this->velocity);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "velocityVariation", this->velocityVariation);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "velocityOverLife", this->velocityOverLife);

    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "spin", this->spin);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "spinVariation", this->spinVariation);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "spinOverLife", this->spinOverLife);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "animSpeedOverLife", this->animSpeedOverLife);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<bool>(layerNode, "randomSpinDirection", this->randomSpinDirection);

    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "angle", this->angle);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "angleVariation", this->angleVariation);

    PropertyLineYamlWriter::WritePropertyLineToYamlNode<Color>(layerNode, "colorRandom", this->colorRandom);
    PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, "alphaOverLife", this->alphaOverLife);

    PropertyLineYamlWriter::WritePropertyLineToYamlNode<Color>(layerNode, "colorOverLife", this->colorOverLife);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<bool>(layerNode, "frameOverLifeEnabled", this->frameOverLifeEnabled);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(layerNode, "frameOverLifeFPS", this->frameOverLifeFPS);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<bool>(layerNode, "randomFrameOnStart", this->randomFrameOnStart);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<bool>(layerNode, "loopSpriteAnimation", this->loopSpriteAnimation);

    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(layerNode, "startTime", this->startTime);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(layerNode, "endTime", this->endTime);

    PropertyLineYamlWriter::WritePropertyValueToYamlNode<bool>(layerNode, "isLooped", this->isLooped);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(layerNode, "deltaTime", this->deltaTime);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(layerNode, "deltaVariation", this->deltaVariation);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(layerNode, "loopVariation", this->loopVariation);
    PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(layerNode, "loopEndTime", this->loopEndTime);

    layerNode->Set("inheritPosition", inheritPosition);
    layerNode->Set("stripeInheritPositionForBase", stripeInheritPositionOnlyForBaseVertex);
    layerNode->Set("usePerspectiveMapping", usePerspectiveMapping);

    layerNode->Set("particleOrientation", particleOrientation);

    YamlNode* lodsNode = new YamlNode(YamlNode::TYPE_ARRAY);
    for (int32 i = 0; i < 4; i++)
        lodsNode->Add(static_cast<int32>(activeLODS[i])); //as for now AddValueToArray has no bool type - force it to int
    layerNode->SetNodeToMap("activeLODS", lodsNode);

    if ((type == TYPE_SUPEREMITTER_PARTICLES) && innerEmitter)
    {
        String innerRelativePath = innerEmitterPath.GetRelativePathname(configPath.GetDirectory());
        PropertyLineYamlWriter::WritePropertyValueToYamlNode<String>(layerNode, "innerEmitterPath", innerRelativePath);
    }

    PropertyLineYamlWriter::WritePropertyValueToYamlNode<int32>(layerNode, "effectFormat", 1);

    // Now write the forces.
    SaveForcesToYamlNode(layerNode);

    SaveDragForcesToYamlNode(layerNode);
}

void ParticleLayer::SaveSpritePath(FilePath& path, const FilePath& configPath, YamlNode* layerNode, std::string name)
{
    if (!path.IsEmpty())
    {
        path.TruncateExtension();
        String relativePath = path.GetRelativePathname(configPath.GetDirectory());
        PropertyLineYamlWriter::WritePropertyValueToYamlNode<String>(layerNode, name, relativePath);
    }
}

void ParticleLayer::SaveForcesToYamlNode(YamlNode* layerNode)
{
    int32 forceCount = static_cast<int32>(this->forces.size());
    if (forceCount == 0)
    {
        // No forces to write.
        return;
    }

    PropertyLineYamlWriter::WritePropertyValueToYamlNode<int32>(layerNode, "forceCount", forceCount);
    for (int32 i = 0; i < forceCount; i++)
    {
        ParticleForce* currentForce = this->forces[i];

        String forceDataName = Format("force%d", i);
        PropertyLineYamlWriter::WritePropertyLineToYamlNode<Vector3>(layerNode, forceDataName, currentForce->force);

        forceDataName = Format("forceOverLife%d", i);
        PropertyLineYamlWriter::WritePropertyLineToYamlNode<float32>(layerNode, forceDataName, currentForce->forceOverLife);
    }
}

void ParticleLayer::SaveDragForcesToYamlNode(YamlNode* layerNode)
{
    using namespace ParticleLayerDetail;
    int32 forceCount = static_cast<int32>(dragForces.size());
    if (forceCount == 0)
    {
        // No forces to write.
        return;
    }

    PropertyLineYamlWriter::WritePropertyValueToYamlNode<int32>(layerNode, "dragForceCount", forceCount);
    for (int32 i = 0; i < forceCount; i++)
    {
        ParticleDragForce* currentForce = dragForces[i];

        String forceDataName = Format("forceName%d", i);
        PropertyLineYamlWriter::WritePropertyValueToYamlNode<String>(layerNode, forceDataName, currentForce->forceName);

        forceDataName = Format("forceType%d", i);
        PropertyLineYamlWriter::WritePropertyValueToYamlNode<String>(layerNode, forceDataName, TypeToString(currentForce->type, "drag", forceTypesMap));

        forceDataName = Format("forceIsActive%d", i);
        PropertyLineYamlWriter::WritePropertyValueToYamlNode<bool>(layerNode, forceDataName, currentForce->isActive);

        forceDataName = Format("dragForcePosition%d", i);
        PropertyLineYamlWriter::WritePropertyValueToYamlNode<Vector3>(layerNode, forceDataName, currentForce->position);

        forceDataName = Format("dragForceRotation%d", i);
        PropertyLineYamlWriter::WritePropertyValueToYamlNode<Vector3>(layerNode, forceDataName, currentForce->rotation);

        forceDataName = Format("dragForceInfinityRange%d", i);
        PropertyLineYamlWriter::WritePropertyValueToYamlNode<bool>(layerNode, forceDataName, currentForce->isInfinityRange);

        forceDataName = Format("dragForcePower%d", i);
        PropertyLineYamlWriter::WritePropertyValueToYamlNode<Vector3>(layerNode, forceDataName, currentForce->forcePower);

        forceDataName = Format("dragForceBoxSize%d", i);
        PropertyLineYamlWriter::WritePropertyValueToYamlNode<Vector3>(layerNode, forceDataName, currentForce->boxSize);

        forceDataName = Format("dragForceRadius%d", i);
        PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(layerNode, forceDataName, currentForce->radius);

        forceDataName = Format("dragForceShape%d", i);
        PropertyLineYamlWriter::WritePropertyValueToYamlNode<String>(layerNode, forceDataName, TypeToString(currentForce->shape, "box", shapeMap));

        forceDataName = Format("dragForceTimingType%d", i);
        PropertyLineYamlWriter::WritePropertyValueToYamlNode<String>(layerNode, forceDataName, TypeToString(currentForce->timingType, "const", timingTypesMap));

        forceDataName = Format("dragForceLine%d", i);
        PropertyLineYamlWriter::WritePropertyLineToYamlNode<Vector3>(layerNode, forceDataName, currentForce->forcePowerLine);

        forceDataName = Format("forceDirection%d", i);
        PropertyLineYamlWriter::WritePropertyValueToYamlNode<Vector3>(layerNode, forceDataName, currentForce->direction);

        forceDataName = Format("forceWindFreq%d", i);
        PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(layerNode, forceDataName, currentForce->windFrequency);

        forceDataName = Format("forceWindTurb%d", i);
        PropertyLineYamlWriter::WritePropertyValueToYamlNode<float32>(layerNode, forceDataName, currentForce->windTurbulence);
    }
}

void ParticleLayer::GetModifableLines(List<ModifiablePropertyLineBase*>& modifiables)
{
    PropertyLineHelper::AddIfModifiable(stripeSizeOverLife.Get(), modifiables);
    PropertyLineHelper::AddIfModifiable(stripeTextureTileOverLife.Get(), modifiables);
    PropertyLineHelper::AddIfModifiable(stripeNoiseUScrollSpeedOverLife.Get(), modifiables);
    PropertyLineHelper::AddIfModifiable(stripeNoiseVScrollSpeedOverLife.Get(), modifiables);
    PropertyLineHelper::AddIfModifiable(stripeColorOverLife.Get(), modifiables);

    PropertyLineHelper::AddIfModifiable(alphaRemapOverLife.Get(), modifiables);

    PropertyLineHelper::AddIfModifiable(flowSpeed.Get(), modifiables);
    PropertyLineHelper::AddIfModifiable(flowSpeedVariation.Get(), modifiables);
    PropertyLineHelper::AddIfModifiable(flowOffset.Get(), modifiables);
    PropertyLineHelper::AddIfModifiable(flowOffsetVariation.Get(), modifiables);
    PropertyLineHelper::AddIfModifiable(noiseScale.Get(), modifiables);
    PropertyLineHelper::AddIfModifiable(noiseScaleVariation.Get(), modifiables);
    PropertyLineHelper::AddIfModifiable(noiseScaleOverLife.Get(), modifiables);
    PropertyLineHelper::AddIfModifiable(noiseUScrollSpeed.Get(), modifiables);
    PropertyLineHelper::AddIfModifiable(noiseUScrollSpeedVariation.Get(), modifiables);
    PropertyLineHelper::AddIfModifiable(noiseUScrollSpeedOverLife.Get(), modifiables);
    PropertyLineHelper::AddIfModifiable(noiseVScrollSpeed.Get(), modifiables);
    PropertyLineHelper::AddIfModifiable(noiseVScrollSpeedVariation.Get(), modifiables);
    PropertyLineHelper::AddIfModifiable(noiseVScrollSpeedOverLife.Get(), modifiables);
    PropertyLineHelper::AddIfModifiable(life.Get(), modifiables);
    PropertyLineHelper::AddIfModifiable(lifeVariation.Get(), modifiables);
    PropertyLineHelper::AddIfModifiable(number.Get(), modifiables);
    PropertyLineHelper::AddIfModifiable(numberVariation.Get(), modifiables);
    PropertyLineHelper::AddIfModifiable(size.Get(), modifiables);
    PropertyLineHelper::AddIfModifiable(sizeVariation.Get(), modifiables);
    PropertyLineHelper::AddIfModifiable(sizeOverLifeXY.Get(), modifiables);
    PropertyLineHelper::AddIfModifiable(velocity.Get(), modifiables);
    PropertyLineHelper::AddIfModifiable(velocityVariation.Get(), modifiables);
    PropertyLineHelper::AddIfModifiable(velocityOverLife.Get(), modifiables);
    PropertyLineHelper::AddIfModifiable(spin.Get(), modifiables);
    PropertyLineHelper::AddIfModifiable(spinVariation.Get(), modifiables);
    PropertyLineHelper::AddIfModifiable(spinOverLife.Get(), modifiables);
    PropertyLineHelper::AddIfModifiable(colorRandom.Get(), modifiables);
    PropertyLineHelper::AddIfModifiable(alphaOverLife.Get(), modifiables);
    PropertyLineHelper::AddIfModifiable(colorOverLife.Get(), modifiables);
    PropertyLineHelper::AddIfModifiable(angle.Get(), modifiables);
    PropertyLineHelper::AddIfModifiable(angleVariation.Get(), modifiables);
    PropertyLineHelper::AddIfModifiable(animSpeedOverLife.Get(), modifiables);

    int32 forceCount = static_cast<int32>(this->forces.size());
    for (int32 i = 0; i < forceCount; i++)
    {
        forces[i]->GetModifableLines(modifiables);
    }

    size_t dragForcesCount = dragForces.size();
    for (size_t i = 0; i < dragForcesCount; ++i)
        dragForces[i]->GetModifableLines(modifiables);

    if ((type == TYPE_SUPEREMITTER_PARTICLES) && innerEmitter)
    {
        innerEmitter->GetModifableLines(modifiables);
    }
}

void ParticleLayer::AddForce(ParticleForce* force)
{
    SafeRetain(force);
    this->forces.push_back(force);
}

void ParticleLayer::RemoveForce(ParticleForce* force)
{
    Vector<ParticleForce*>::iterator iter = std::find(this->forces.begin(),
                                                      this->forces.end(),
                                                      force);
    if (iter != this->forces.end())
    {
        SafeRelease(*iter);
        this->forces.erase(iter);
    }
}

void ParticleLayer::RemoveForce(int32 forceIndex)
{
    if (forceIndex <= static_cast<int32>(this->forces.size()))
    {
        SafeRelease(this->forces[forceIndex]);
        this->forces.erase(this->forces.begin() + forceIndex);
    }
}

void ParticleLayer::CleanupForces()
{
    for (Vector<ParticleForce*>::iterator iter = this->forces.begin();
         iter != this->forces.end(); iter++)
    {
        SafeRelease(*iter);
    }

    this->forces.clear();
}

void ParticleLayer::AddDrag(ParticleDragForce* drag)
{
    SafeRetain(drag);
    dragForces.push_back(drag);
}

void ParticleLayer::RemoveDrag(ParticleDragForce* drag)
{
    auto iter = std::find(dragForces.begin(), dragForces.end(), drag);
    if (iter != dragForces.end())
    {
        SafeRelease(*iter);
        dragForces.erase(iter);
    }
}

void ParticleLayer::RemoveDrag(int32 dragIndex)
{
    if (dragIndex <= static_cast<int32>(dragForces.size()))
    {
        SafeRelease(dragForces[dragIndex]);
        dragForces.erase(dragForces.begin() + dragIndex);
    }
}

void ParticleLayer::CleanupDrag()
{
    for (auto& force : dragForces)
    {
        SafeRelease(force);
    }

    dragForces.clear();
}

void ParticleLayer::FillSizeOverlifeXY(RefPtr<PropertyLine<float32>> sizeOverLife)
{
    Vector<PropValue<float32>> wrappedPropertyValues = PropLineWrapper<float32>(sizeOverLife).GetProps();
    if (wrappedPropertyValues.empty())
    {
        this->sizeOverLifeXY = NULL;
        return;
    }
    else if (wrappedPropertyValues.size() == 1)
    {
        Vector2 singleValue(wrappedPropertyValues[0].v, wrappedPropertyValues[0].v);
        this->sizeOverLifeXY = RefPtr<PropertyLine<Vector2>>(new PropertyLineValue<Vector2>(singleValue));
        return;
    }

    RefPtr<PropertyLineKeyframes<Vector2>> sizeOverLifeXYKeyframes =
    RefPtr<PropertyLineKeyframes<Vector2>>(new PropertyLineKeyframes<Vector2>);
    size_t propsCount = wrappedPropertyValues.size();
    for (size_t i = 0; i < propsCount; i++)
    {
        Vector2 curValue(wrappedPropertyValues[i].v, wrappedPropertyValues[i].v);
        sizeOverLifeXYKeyframes->AddValue(wrappedPropertyValues[i].t, curValue);
    }

    this->sizeOverLifeXY = sizeOverLifeXYKeyframes;
}

ParticleLayer::eType ParticleLayer::StringToLayerType(const String& layerTypeName, eType defaultLayerType)
{
    int32 layerTypesCount = sizeof(layerTypeNamesInfoMap) / sizeof(*layerTypeNamesInfoMap);
    for (int32 i = 0; i < layerTypesCount; i++)
    {
        if (layerTypeNamesInfoMap[i].layerTypeName == layerTypeName)
        {
            return layerTypeNamesInfoMap[i].layerType;
        }
    }

    return defaultLayerType;
}

String ParticleLayer::LayerTypeToString(eType layerType, const String& defaultLayerTypeName)
{
    int32 layerTypesCount = sizeof(layerTypeNamesInfoMap) / sizeof(*layerTypeNamesInfoMap);
    for (int32 i = 0; i < layerTypesCount; i++)
    {
        if (layerTypeNamesInfoMap[i].layerType == layerType)
        {
            return layerTypeNamesInfoMap[i].layerTypeName;
        }
    }

    return defaultLayerTypeName;
}

void ParticleLayer::LoadForcesFromYaml(const YamlNode* node)
{
    using namespace ParticleLayerDetail;

    int32 dragForceCount = 0;
    const YamlNode* dragForceCountNode = node->Get("dragForceCount");
    if (dragForceCountNode)
        dragForceCount = dragForceCountNode->AsInt();

    for (int32 i = 0; i < dragForceCount; ++i)
    {
        ParticleDragForce* dragForce = new ParticleDragForce(this);

        String forceDataName = Format("forceName%d", i);
        const YamlNode* nameNode = node->Get(forceDataName);
        if (nameNode)
            dragForce->forceName = nameNode->AsString();

        forceDataName = Format("forceType%d", i);
        const YamlNode* typeNode = node->Get(forceDataName);
        if (typeNode)
        {
            String type = typeNode->AsString();
            dragForce->type = StringToType(type, ForceType::DRAG_FORCE, forceTypesMap);
        }

        forceDataName = Format("forceIsActive%d", i);
        const YamlNode* activeNode = node->Get(forceDataName);
        if (activeNode)
            dragForce->isActive = activeNode->AsBool();

        forceDataName = Format("dragForcePosition%d", i);
        const YamlNode* positionNode = node->Get(forceDataName);
        if (positionNode)
            dragForce->position = positionNode->AsVector3();

        forceDataName = Format("dragForceRotation%d", i);
        const YamlNode* rotationNode = node->Get(forceDataName);
        if (rotationNode)
            dragForce->rotation = rotationNode->AsVector3();

        forceDataName = Format("dragForceInfinityRange%d", i);
        const YamlNode* rangeNode = node->Get(forceDataName);
        if (rangeNode)
            dragForce->isInfinityRange = rangeNode->AsBool();

        forceDataName = Format("dragForcePower%d", i);
        const YamlNode* powerNode = node->Get(forceDataName);
        if (powerNode)
            dragForce->forcePower = powerNode->AsVector3();

        forceDataName = Format("dragForceBoxSize%d", i);
        const YamlNode* sizeNode = node->Get(forceDataName);
        if (sizeNode)
            dragForce->boxSize = sizeNode->AsVector3();

        forceDataName = Format("dragForceRadius%d", i);
        const YamlNode* radiusNode = node->Get(forceDataName);
        if (radiusNode)
            dragForce->radius = radiusNode->AsFloat();

        forceDataName = Format("dragForceShape%d", i);
        const YamlNode* shapeNode = node->Get(forceDataName);
        if (shapeNode)
        {
            String shapeName = shapeNode->AsString();
            dragForce->shape = StringToType(shapeName, ForceShape::BOX, shapeMap);
        }

        forceDataName = Format("dragForceTimingType%d", i);
        const YamlNode* timingNode = node->Get(forceDataName);
        if (timingNode)
        {
            String name = timingNode->AsString();
            dragForce->timingType = StringToType(name, ForceTimingType::CONSTANT, timingTypesMap);
        }
        
        forceDataName = Format("forceDirection%d", i);
        const YamlNode* directionNode = node->Get(forceDataName);
        if (directionNode)
            dragForce->direction = directionNode->AsVector3();

        forceDataName = Format("forceWindFreq%d", i);
        const YamlNode* windFreqNode = node->Get(forceDataName);
        if (windFreqNode)
            dragForce->windFrequency = windFreqNode->AsFloat();

        forceDataName = Format("forceWindTurb%d", i);
        const YamlNode* windTurbNode = node->Get(forceDataName);
        if (windTurbNode)
            dragForce->windTurbulence = windTurbNode->AsFloat();

        RefPtr<PropertyLine<Vector3>> forcePowerLine = PropertyLineYamlReader::CreatePropertyLine<Vector3>(node->Get(Format("dragForceLine%d", i)));
        dragForce->forcePowerLine = forcePowerLine;

        AddDrag(dragForce);
        dragForce->Release();
    }
}
};
