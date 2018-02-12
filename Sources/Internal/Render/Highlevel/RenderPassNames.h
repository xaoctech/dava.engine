#ifndef __DAVAENGINE_RENDER_FASTNAMES_H__
#define __DAVAENGINE_RENDER_FASTNAMES_H__

#include "Base/BaseTypes.h"
#include "Base/FastName.h"

namespace DAVA
{
// GLOBAL PASSES
static const FastName PASS_PICKING("PickingPass");
static const FastName PASS_FORWARD("ForwardPass");
static const FastName PASS_FORWARD_COMBINE("ForwardPassWithCombine");
static const FastName PASS_REFLECTION_REFRACTION("ReflectionRefractionPass");
static const FastName PASS_NO_FOG("NoFogPass");
static const FastName PASS_SHADOW("ShadowPass");
static const FastName PASS_STATIC_OCCLUSION("StaticOcclusionPass");
static const FastName PASS_GBUFFER("gBufferPass");
static const FastName PASS_GBUFFER_FETCH("gBufferFetchPass");
static const FastName PASS_GBUFFER_RESOLVE("gBufferResolvePass");
static const FastName PASS_POSTEFFECT("PostEffectPass");
static const FastName PASS_IB_REFLECTION("IBReflectionPass");
static const FastName PASS_IB_REFLECTION_LDR("LDRIBReflectionPass");
static const FastName PASS_VT("VTPass");
static const FastName PASS_VT_FETCH("VTFetchPass");
static const FastName PASS_VT_DECORATION("VTDecorationPass");
static const FastName PASS_RESCALE("RescalePass"); //later it would be converted to some AA part
static const FastName PASS_FORWARD_LDR("LDRForwardPass");

} // ns

#endif /* __DAVAENGINE_RENDER_FASTNAMES_H__ */
