#include "ParticleRenderObject.h"

#include "Render/DynamicBufferAllocator.h"
#include "Render/Renderer.h"
#include "Time/SystemTimer.h"

namespace DAVA
{
ParticleRenderObject::ParticleRenderObject(ParticleEffectData* effect)
    : effectData(effect)
    , sortingOffset(15)
{
    AddFlag(RenderObject::CUSTOM_PREPARE_TO_RENDER);

    layoutsData[1 << FRAME_BLEND] = { rhi::VS_TEXCOORD, 1, rhi::VDT_FLOAT, 3 };
    layoutsData[1 << FLOW] = { rhi::VS_TEXCOORD, 2, rhi::VDT_FLOAT, 4 }; // uv, speed, offset
    layoutsData[1 << NOISE] = { rhi::VS_TEXCOORD, 3, rhi::VDT_FLOAT, 3 }; // uv, scale
    layoutsData[1 << FRESNEL_TO_ALPHA_REMAP_PERP_MAPPING] = { rhi::VS_TEXCOORD, 5, rhi::VDT_FLOAT, 3 }; // Fresnel, alpha remap from chart, perspective mapping w.

    uint16 numBits = static_cast<uint16>(layoutsData.size());

    rhi::VertexLayout baseLayout; // We always have position, texcoord0 and color.
    GenerateBaseLayout(baseLayout);

    uint32 maxCount = (1 << numBits) - 1;
    for (uint32 i = 0; i <= maxCount; ++i)
    {
        rhi::VertexLayout layout = baseLayout;
        for (uint32 j = 0; j < numBits; j++)
        {
            if (i & (1 << j))
            {
                LayoutElement& element = layoutsData[1 << j];
                layout.AddElement(element.usage, element.usageIndex, element.type, element.dimension);
            }
        }
        layoutMap[i] = rhi::VertexLayout::UniqueId(layout);
    }

    type = RenderObject::TYPE_PARTICLE_EMITTER;
}

ParticleRenderObject::~ParticleRenderObject()
{
    for (size_t i = 0, sz = renderBatchCache.size(); i < sz; ++i)
    {
        SafeRelease(renderBatchCache[i]);
    }
}

void ParticleRenderObject::PrepareToRender(Camera* camera)
{
    if (!Renderer::GetOptions()->IsOptionEnabled(RenderOptions::PARTICLES_PREPARE_BUFFERS))
        return;

    PrepareRenderData(camera);

    if (!Renderer::GetOptions()->IsOptionEnabled(RenderOptions::PARTICLES_DRAW))
    {
        activeRenderBatchArray.clear();
        return;
    }
}

void ParticleRenderObject::SetSortingOffset(uint32 offset)
{
    sortingOffset = offset;
    for (size_t i = 0, sz = renderBatchCache.size(); i < sz; ++i)
        renderBatchCache[i]->SetSortingOffset(offset);
}

void ParticleRenderObject::PrepareRenderData(Camera* camera)
{
    //camera_facing, x_emitter, y_emitter, z_emitter, x_world, y_world, z_world
    static Vector3 basisVectors[7 * 2] = { Vector3(), Vector3(),
                                           Vector3(), Vector3(),
                                           Vector3(), Vector3(),
                                           Vector3(), Vector3(),
                                           Vector3(0, 1, 0), Vector3(0, 0, 1),
                                           Vector3(1, 0, 0), Vector3(0, 0, 1),
                                           Vector3(0, 1, 0), Vector3(1, 0, 0) };

    activeRenderBatchArray.clear();
    currRenderBatchId = 0;

    DVASSERT(worldTransform);

    Vector3 currCamDirection = camera->GetDirection();

    /*prepare effect basises*/
    const Matrix4& mv = camera->GetMatrix();
    basisVectors[0] = Vector3(mv._00, mv._10, mv._20);
    basisVectors[1] = Vector3(mv._01, mv._11, mv._21);
    basisVectors[0].Normalize();
    basisVectors[1].Normalize();
    Vector3 ex(worldTransform->_00, worldTransform->_01, worldTransform->_02);
    Vector3 ey(worldTransform->_10, worldTransform->_11, worldTransform->_12);
    Vector3 ez(worldTransform->_20, worldTransform->_21, worldTransform->_22);
    ex.Normalize();
    ey.Normalize();
    ez.Normalize();
    basisVectors[2] = ey;
    basisVectors[3] = ez;
    basisVectors[4] = ex;
    basisVectors[5] = ez;
    basisVectors[6] = ey;
    basisVectors[7] = ex;

    static Vector3 stripeBasisVectors[7] = { Vector3(),
        Vector3(), Vector3(),
        Vector3(), 
        Vector3(1, 0, 0),
        Vector3(0, 1, 0),
        Vector3(0, 0, 1) };
    stripeBasisVectors[0] = basisVectors[0];
    ex.Normalize();
    ey.Normalize();
    ez.Normalize();
    stripeBasisVectors[1] = ex;
    stripeBasisVectors[2] = ey;
    stripeBasisVectors[3] = ez;


    auto itGroupStart = effectData->groups.begin();
    uint32 particlesInGroup = 0;
    for (List<ParticleGroup>::iterator itGroupCurr = effectData->groups.begin(), e = effectData->groups.end(); itGroupCurr != e; ++itGroupCurr)
    {
        const ParticleGroup& group = (*itGroupCurr);
        //note - isDisabled just stop it from being rendered, still processing particles in ParticleEffectSystem
        if (!CheckGroup(group))
            continue; //if no material was set up, or empty group, or layer rendering is disabled or sprite is removed - don't draw anyway

        bool isLayerTypesNotTheSame = CheckIfSimpleParticle(itGroupStart->layer) != CheckIfSimpleParticle(itGroupCurr->layer);

        if (itGroupStart->material != itGroupCurr->material || isLayerTypesNotTheSame)
        {
            if (CheckIfSimpleParticle(itGroupStart->layer))
                AppendParticleGroup(itGroupStart, itGroupCurr, particlesInGroup, camera->GetDirection(), basisVectors);
            else
                AppendStripeParticle(itGroupStart, effectData->groups.end(), particlesInGroup, camera, stripeBasisVectors);
            itGroupStart = itGroupCurr;
            particlesInGroup = 0;
        }
        particlesInGroup += CalculateParticleCount(*itGroupCurr);
    }
    if (itGroupStart != effectData->groups.end())
    {
        if (CheckIfSimpleParticle(itGroupStart->layer))
            AppendParticleGroup(itGroupStart, effectData->groups.end(), particlesInGroup, camera->GetDirection(), basisVectors);
        else
            AppendStripeParticle(itGroupStart, effectData->groups.end(), particlesInGroup, camera, stripeBasisVectors);
    }
}

uint32 ParticleRenderObject::GetVertexStride(ParticleLayer* layer)
{
    uint32 vertexStride = (3 + 2 + 1) * sizeof(float); // vertex*3 + texcoord0*2 + color * 1;
    if (layer->enableFrameBlend && layer->type != ParticleLayer::TYPE_PARTICLE_STRIPE)
        vertexStride += (3) * sizeof(float); // texcoord1 * 3;
    if (layer->enableFlow)
        vertexStride += (2 + 2) * sizeof(float); // texcoord2.xy + speed and offset
    if (layer->enableNoise)
        vertexStride += (2 + 1) * sizeof(float); // texcoord.xy + noise scale
    if (layer->useFresnelToAlpha || layer->enableAlphaRemap || (layer->usePerspectiveMapping && layer->type == ParticleLayer::TYPE_PARTICLE_STRIPE))
        vertexStride += (3) * sizeof(float);
    return vertexStride;
}

void ParticleRenderObject::GenerateBaseLayout(rhi::VertexLayout& layout)
{
    layout = {};
    layout.AddElement(rhi::VS_POSITION, 0, rhi::VDT_FLOAT, 3);
    layout.AddElement(rhi::VS_TEXCOORD, 0, rhi::VDT_FLOAT, 2);
    layout.AddElement(rhi::VS_COLOR, 0, rhi::VDT_UINT8N, 4);
}

int32 ParticleRenderObject::CalculateParticleCount(const ParticleGroup& group)
{
    int32 basisCount = 0;
    if (group.layer->particleOrientation & ParticleLayer::PARTICLE_ORIENTATION_CAMERA_FACING)
        basisCount++;
    if (group.layer->particleOrientation & ParticleLayer::PARTICLE_ORIENTATION_X_FACING)
        basisCount++;
    if (group.layer->particleOrientation & ParticleLayer::PARTICLE_ORIENTATION_Y_FACING)
        basisCount++;
    if (group.layer->particleOrientation & ParticleLayer::PARTICLE_ORIENTATION_Z_FACING)
        basisCount++;

    return group.activeParticleCount * basisCount;
}

uint32 ParticleRenderObject::SelectLayout(const ParticleLayer& layer)
{
    uint32 isFramBlendEnabled = static_cast<uint32>(layer.enableFrameBlend && layer.type != ParticleLayer::TYPE_PARTICLE_STRIPE);
    uint32 key = isFramBlendEnabled << static_cast<uint32>(eParticlePropsOffsets::FRAME_BLEND);
    key |= static_cast<uint32>(layer.enableFlow) << static_cast<uint32>(eParticlePropsOffsets::FLOW);
    key |= static_cast<uint32>(layer.enableNoise) << static_cast<uint32>(eParticlePropsOffsets::NOISE);
    key |= static_cast<uint32>(layer.enableAlphaRemap || (layer.usePerspectiveMapping && layer.type == ParticleLayer::TYPE_PARTICLE_STRIPE) || layer.useFresnelToAlpha) << static_cast<uint32>(eParticlePropsOffsets::FRESNEL_TO_ALPHA_REMAP_PERP_MAPPING);
    return layoutMap[key];
}

void ParticleRenderObject::UpdateStripeVertex(float32*& dataPtr, Vector3& position, Vector3& uv, float32* color, ParticleLayer* layer, Particle* particle, float32 fresToAlpha)
{
    *dataPtr++ = position.x;
    *dataPtr++ = position.y;
    *dataPtr++ = position.z;

    *dataPtr++ = uv.x;
    *dataPtr++ = uv.y;

    *dataPtr++ = *color;

    if (layer->enableFlow && layer->flowmap.get() != nullptr)
    {
        *dataPtr++ = uv.x;
        *dataPtr++ = uv.y;
        *dataPtr++ = particle->currFlowSpeed;
        *dataPtr++ = particle->currFlowOffset;
    }
    if (layer->enableNoise && layer->noise.get() != nullptr)
    {
        *dataPtr++ = uv.x;
        if (layer->enableNoiseScroll)
            *(dataPtr - 1) += particle->currNoiseUOffset;

        *dataPtr++ = uv.y;
        if (layer->enableNoiseScroll)
            *(dataPtr - 1) += particle->currNoiseVOffset;

        *dataPtr++ = particle->currNoiseScale;
    }
    if (layer->enableAlphaRemap || layer->usePerspectiveMapping || layer->useFresnelToAlpha)
    {
        *dataPtr++ = fresToAlpha;
        *dataPtr++ = particle->alphaRemap;
        *dataPtr++ = uv.z;
    }
}

int32 ParticleRenderObject::PrepareBasisIndexes(const ParticleGroup& group, int32 (&basises)[4]) const
{
    int32 basisCount = 0;
    bool worldAlign = (group.layer->particleOrientation & ParticleLayer::PARTICLE_ORIENTATION_WORLD_ALIGN) != 0;
    if (group.layer->particleOrientation & ParticleLayer::PARTICLE_ORIENTATION_CAMERA_FACING)
        basises[basisCount++] = 0;
    if (group.layer->particleOrientation & ParticleLayer::PARTICLE_ORIENTATION_X_FACING)
        basises[basisCount++] = worldAlign ? 4 : 1;
    if (group.layer->particleOrientation & ParticleLayer::PARTICLE_ORIENTATION_Y_FACING)
        basises[basisCount++] = worldAlign ? 5 : 2;
    if (group.layer->particleOrientation & ParticleLayer::PARTICLE_ORIENTATION_Z_FACING)
        basises[basisCount++] = worldAlign ? 6 : 3;
    return basisCount;
}

void ParticleRenderObject::AppendRenderBatch(NMaterial* material, uint32 indexCount, uint32 vertexLayout, const DynamicBufferAllocator::AllocResultVB& vBuffer)
{
    AppendRenderBatch(material, indexCount, vertexLayout, vBuffer, DynamicBufferAllocator::AllocateQuadListIndexBuffer(indexCount), 0);
}

void ParticleRenderObject::AppendRenderBatch(NMaterial* material, uint32 indexCount, uint32 vertexLayout, const DynamicBufferAllocator::AllocResultVB& vBuffer, const rhi::HIndexBuffer iBuffer, uint32 startIndex)
{
    DVASSERT(indexCount);

    //now we need to create batch
    if (currRenderBatchId >= renderBatchCache.size())
    {
        RenderBatch* newBatch = new RenderBatch();
        newBatch->primitiveType = rhi::PRIMITIVE_TRIANGLELIST;
        newBatch->SetRenderObject(this);
        renderBatchCache.push_back(newBatch);
        DVASSERT(renderBatchCache.size() > currRenderBatchId); //O_o
    }

    RenderBatch* targetBatch = renderBatchCache[currRenderBatchId];
    targetBatch->SetMaterial(material);

    targetBatch->vertexBuffer = vBuffer.buffer;
    targetBatch->vertexCount = vBuffer.allocatedVertices;
    targetBatch->vertexBase = vBuffer.baseVertex;

    targetBatch->indexCount = indexCount;
    targetBatch->indexBuffer = iBuffer;
    targetBatch->startIndex = startIndex;
    targetBatch->vertexLayoutId = vertexLayout;
    activeRenderBatchArray.push_back(targetBatch);
    currRenderBatchId++;
}

void ParticleRenderObject::AppendParticleGroup(List<ParticleGroup>::iterator begin, List<ParticleGroup>::iterator end, uint32 particlesCount, const Vector3& cameraDirection, Vector3* basisVectors)
{
    if (!particlesCount)
        return; //hmmm?

    uint32 vertexStride = GetVertexStride(begin->layer); // If you change vertex layout, don't forget to change the stride.

    uint32 verteciesToAllocate = particlesCount * 4;
    DynamicBufferAllocator::AllocResultVB target = DynamicBufferAllocator::AllocateVertexBuffer(vertexStride, verteciesToAllocate);
    uint8* currpos = target.data;

    uint32 verteciesAppended = 0;
    uint32 particleStride = vertexStride * 4;
    for (auto it = begin; it != end; ++it)
    {
        const ParticleGroup& group = *it;
        if (!CheckGroup(group))
            continue; //if no material was set up, or empty group, or layer rendering is disabled or sprite is removed - don't draw anyway

        //prepare basis indexes
        int32 basisCount = 0;
        int32 basises[4]; //4 basises max per particle
        basisCount = PrepareBasisIndexes(group, basises);

        Particle* current = group.head;
        while (current)
        {
            float32* pT = group.layer->sprite->GetTextureVerts(current->frame);
            Color currColor = current->color;
            if (group.layer->colorOverLife)
                currColor = group.layer->colorOverLife->GetValue(current->life / current->lifeTime);
            if (group.layer->alphaOverLife)
                currColor.a = group.layer->alphaOverLife->GetValue(current->life / current->lifeTime);
            uint32 color = rhi::NativeColorRGBA(currColor.r, currColor.g, currColor.b, Min(currColor.a, 1.0f));
            float32 sin_angle;
            float32 cos_angle;
            SinCosFast(-current->angle, sin_angle, cos_angle); //- is because artists consider positive rotation to be clockwise

            for (int32 i = 0; i < basisCount; i++)
            {
                if (verteciesAppended + 4 > target.allocatedVertices)
                {
                    uint32 vLayout = SelectLayout(*group.layer);
                    AppendRenderBatch(group.material, verteciesAppended / 4 * 6, vLayout, target);
                    verteciesToAllocate -= verteciesAppended;
                    verteciesAppended = 0;

                    target = DynamicBufferAllocator::AllocateVertexBuffer(vertexStride, verteciesToAllocate);
                    currpos = target.data;
                }

                float32* verts[4];
                verts[0] = reinterpret_cast<float32*>(currpos);
                verts[1] = reinterpret_cast<float32*>(currpos + vertexStride);
                verts[2] = reinterpret_cast<float32*>(currpos + 2 * vertexStride);
                verts[3] = reinterpret_cast<float32*>(currpos + 3 * vertexStride);

                Vector3 ex = basisVectors[basises[i] * 2];
                Vector3 ey = basisVectors[basises[i] * 2 + 1];
                //TODO: rethink this code - it should be easier
                if (group.layer->isLong) //note that for now it's just a copy of long implementatio - later rethink it;
                {
                    ey = current->speed;
                    float32 vel = ey.Length();
                    ex = ey.CrossProduct(cameraDirection);
                    ex.Normalize();
                    ey *= (group.layer->scaleVelocityBase / vel + group.layer->scaleVelocityFactor); //optimized ex=(svBase+svFactor*vel)/vel
                }

                Vector3 left = ex * cos_angle + ey * sin_angle;
                Vector3 right = -left;
                Vector3 top = ey * (-cos_angle) + ex * sin_angle;
                Vector3 bot = -top;

                float32 fresnelToAlpha = 0.0f;
                if (begin->layer->useFresnelToAlpha)
                {
                    Vector3 viewNormal;
                    float32 dot = 0.0f;
                    viewNormal = left.CrossProduct(top);
                    dot = cameraDirection.DotProduct(viewNormal);
                    dot = 1.0f - Abs(dot);
                    fresnelToAlpha = FresnelShlick(dot, current->fresnelToAlphaBias, current->fresnelToAlphaPower);
                }

                left *= 0.5f * current->currSize.x * (1 + group.layer->layerPivotPoint.x);
                right *= 0.5f * current->currSize.x * (1 - group.layer->layerPivotPoint.x);
                top *= 0.5f * current->currSize.y * (1 + group.layer->layerPivotPoint.y);
                bot *= 0.5f * current->currSize.y * (1 - group.layer->layerPivotPoint.y);

                Vector3 particlePosition = current->position;
                if (group.layer->inheritPosition)
                    particlePosition += effectData->infoSources[group.positionSource].position;
                Array<Vector3, 4> quadPos = { particlePosition + left + bot, particlePosition + right + bot, particlePosition + left + top, particlePosition + right + top };
                uint32 ptrOffset = 0;

                for (int32 i = 0; i < 4; i++)
                {
                    verts[i][ptrOffset + 0] = quadPos[i].x; // Position xyz.
                    verts[i][ptrOffset + 1] = quadPos[i].y;
                    verts[i][ptrOffset + 2] = quadPos[i].z;

                    verts[i][ptrOffset + 3] = pT[i * 2]; // VS_TEXCOORD0 xy + color.
                    verts[i][ptrOffset + 4] = pT[i * 2 + 1];
                    uint32* cp = reinterpret_cast<uint32*>(verts[i]) + (ptrOffset + 5);
                    *cp = color;
                }
                ptrOffset += 6;

                if (begin->layer->enableFrameBlend)
                {
                    int32 nextFrame = current->frame + 1;
                    if (nextFrame >= group.layer->sprite->GetFrameCount())
                    {
                        if (group.layer->loopSpriteAnimation)
                            nextFrame = 0;
                        else
                            nextFrame = group.layer->sprite->GetFrameCount() - 1;
                    }
                    float32* pT = group.layer->sprite->GetTextureVerts(nextFrame);

                    for (int32 i = 0; i < 4; i++) // VS_TEXCOORD1 xy + time.
                    {
                        verts[i][ptrOffset] = *(pT++);
                        verts[i][ptrOffset + 1] = *(pT++);
                        verts[i][ptrOffset + 2] = current->animTime;
                    }
                    ptrOffset += 3;
                }
                if (begin->layer->enableFlow && begin->layer->flowmap.get() != nullptr)
                {
                    // todo:: mmmm frameblend?
                    float32* flowUV = group.layer->flowmap->GetTextureVerts(current->frame);
                    for (int32 i = 0; i < 4; i++) // VS_TEXCOORD2.xy, z - speed, w - offset.
                    {
                        verts[i][ptrOffset + 0] = flowUV[i * 2];
                        verts[i][ptrOffset + 1] = flowUV[i * 2 + 1];
                        verts[i][ptrOffset + 2] = current->currFlowSpeed;
                        verts[i][ptrOffset + 3] = current->currFlowOffset;
                    }
                    ptrOffset += 4;
                }
                if (begin->layer->enableNoise && begin->layer->noise.get() != nullptr)
                {
                    float32* noiseUV = group.layer->noise->GetTextureVerts(current->frame);
                    for (int32 i = 0; i < 4; ++i)
                    {
                        verts[i][ptrOffset + 0] = noiseUV[i * 2]; // VS_TEXCOORD0 xy + color.
                        verts[i][ptrOffset + 1] = noiseUV[i * 2 + 1];
                        verts[i][ptrOffset + 2] = current->currNoiseScale;
                        if (begin->layer->enableNoiseScroll)
                        {
                            verts[i][ptrOffset + 0] += current->currNoiseUOffset;
                            verts[i][ptrOffset + 1] += current->currNoiseVOffset;
                        }
                    }
                    ptrOffset += 3;
                }
                if (begin->layer->enableAlphaRemap || begin->layer->useFresnelToAlpha)
                {
                    for (int32 i = 0; i < 4; ++i)
                    {
                        verts[i][ptrOffset + 0] = fresnelToAlpha;
                        verts[i][ptrOffset + 1] = current->alphaRemap;
                        verts[i][ptrOffset + 2] = 0.0f;
                    }
                    ptrOffset += 3;
                }
                currpos += particleStride;
                verteciesAppended += 4;
            }
            current = current->next;
        }
    }

    if (verteciesAppended)
    {
        AppendRenderBatch(begin->material, verteciesAppended / 4 * 6, SelectLayout(*begin->layer), target);
    }
}

void ParticleRenderObject::AppendStripeParticle(List<ParticleGroup>::iterator begin, List<ParticleGroup>::iterator end, uint32 particlesCount, Camera* camera, Vector3* basisVectors)
{
    if (!particlesCount)
        return;

    uint32 vertexStride = GetVertexStride(begin->layer); // If you change vertex layout, don't forget to change the stride.

    for (auto it = begin; it != end; ++it)
    {
        const ParticleGroup& group = *it;
        if (!CheckGroup(group))
            continue; //if no material was set up, or empty group, or layer rendering is disabled or sprite is removed - don't draw anyway

        int32 basisCount = 0;
        int32 basises[4]; //4 basises max per particle

        basisCount = PrepareBasisIndexes(group, basises);

        if (basisCount == 0)
            continue;

        Particle* currentParticle = group.head;
        while (currentParticle)
        {
            StripeData& data = currentParticle->stripe;
            if (!data.isActive)
            {
                currentParticle = currentParticle->next;
                continue;
            }

            float32* pT = group.layer->sprite->GetTextureVerts(currentParticle->frame);
            Color currColor = currentParticle->color;
            if (group.layer->colorOverLife)
                currColor = group.layer->colorOverLife->GetValue(currentParticle->life / currentParticle->lifeTime);
            if (group.layer->alphaOverLife)
                currColor.a = group.layer->alphaOverLife->GetValue(currentParticle->life / currentParticle->lifeTime);

            StripeNode& base = data.baseNode;
            List<StripeNode>& nodes = data.strpeNodes;
            
            int32 vCountInBasis = static_cast<int32>((nodes.size() + 1) * 2);
            int32 vCount = vCountInBasis * basisCount;
            uint32 iCount = static_cast<int32>(nodes.size()) * 6 * basisCount;
            uint32 baseVertex = 0;

            DynamicBufferAllocator::AllocResultVB vb = DynamicBufferAllocator::AllocateVertexBuffer(vertexStride, vCount);
            DynamicBufferAllocator::AllocResultIB ib = DynamicBufferAllocator::AllocateIndexBuffer(iCount);

            uint16* indexBufferData = ib.data;
            float* vertexBufferData = reinterpret_cast<float*>(vb.data);

            for (int32 i = 0; i < basisCount; i++)
            {
                if (nodes.size() == 0)
                    return;
                Vector3 basisVector = basisVectors[basises[i]];
                if(i == 0 && group.layer->particleOrientation & ParticleLayer::PARTICLE_ORIENTATION_CAMERA_FACING)
                {
                    basisVector = (base.position - camera->GetPosition()).CrossProduct(base.speed);
                    basisVector = (camera->GetDirection()).CrossProduct(base.speed);
                    
                    basisVector.Normalize();
                }

                float32 fresnelToAlpha = 0.0f;
                if (begin->layer->useFresnelToAlpha)
                {
                    Vector3 viewNormal;
                    float32 dot = 0.0f;
                    viewNormal = Vector3(basisVector.y, -basisVector.x, 0.0f); // basisVector.CrossProduct(Vector3(0.0f, 0.0f, 1.0f));
                    viewNormal.Normalize();
                    dot = camera->GetDirection().DotProduct(viewNormal);
                    fresnelToAlpha = FresnelShlick(1.0f - Abs(dot), currentParticle->fresnelToAlphaBias, currentParticle->fresnelToAlphaPower);
                }

                float32 size = group.layer->stripeStartSize * 0.5f;
                if (group.layer->stripeSizeOverLifeProp)
                    size *= group.layer->stripeSizeOverLifeProp->GetValue(0.0f);
                Vector3 scaledBasis = basisVector * size;
                float32 fullEdgeSize = size + size;
                Vector3 left = base.position + data.inheritPositionOffset + scaledBasis;
                Vector3 right = base.position + data.inheritPositionOffset - scaledBasis;

                float32 startU = currentParticle->life * group.layer->stripeUScrollSpeed;
                float32 startV = currentParticle->life * group.layer->stripeVScrollSpeed;

                Vector3 uv1 = Vector3(startU, startV, 0.0f);
                Vector3 uv2 = Vector3(startU + 1.0f, startV, 0.0f);
                if (group.layer->usePerspectiveMapping)
                {
                    uv1.x *= fullEdgeSize;
                    uv1.y *= fullEdgeSize;
                    uv1.z = fullEdgeSize;

                    uv2.x *= fullEdgeSize;
                    uv2.y *= fullEdgeSize;
                    uv2.z = fullEdgeSize;
                }

                Color colOverLife = Color::White;
                if (group.layer->stripeColorOverLife)
                    colOverLife = group.layer->stripeColorOverLife->GetValue(0.0f);

                uint32 col = rhi::NativeColorRGBA(Saturate(currColor.r * colOverLife.r), Saturate(currColor.g * colOverLife.g), Saturate(currColor.b * colOverLife.b), Saturate(currColor.a * colOverLife.a));
                float32* color = reinterpret_cast<float32*>(&col);
                UpdateStripeVertex(vertexBufferData, left, uv1, color, group.layer, currentParticle, fresnelToAlpha);
                UpdateStripeVertex(vertexBufferData, right, uv2, color, group.layer, currentParticle, fresnelToAlpha);

                StripeNode* prevNode = &base;
                float32 distance = 0.0f;
                for (auto& node : nodes)
                {
                    if (i == 0 && group.layer->particleOrientation & ParticleLayer::PARTICLE_ORIENTATION_CAMERA_FACING)
                    {
                        basisVector = (node.position - camera->GetPosition()).CrossProduct(node.speed);
                        basisVector = (camera->GetDirection()).CrossProduct(node.speed);
                        basisVector.Normalize();
                    }

                    float32 overLifeTime = node.lifeime / group.layer->stripeLifetime;
                    size = group.layer->stripeStartSize * 0.5f;
                    if (group.layer->stripeSizeOverLifeProp)
                        size *= group.layer->stripeSizeOverLifeProp->GetValue(overLifeTime);
                    fullEdgeSize = size + size;
                    scaledBasis = basisVector * size;
                    left = node.position + data.inheritPositionOffset + scaledBasis;
                    right = node.position + data.inheritPositionOffset - scaledBasis;

                    colOverLife = Color::White;
                    if (group.layer->stripeColorOverLife)
                        colOverLife = group.layer->stripeColorOverLife->GetValue(overLifeTime);

                    col = rhi::NativeColorRGBA(Saturate(currColor.r * colOverLife.r), Saturate(currColor.g * colOverLife.g), Saturate(currColor.b * colOverLife.b), Saturate(currColor.a * colOverLife.a));

                    distance += (prevNode->position - node.position).Length();
                    float32 tile = 1.0f;
                    if (group.layer->stripeTextureTile)
                        tile = group.layer->stripeTextureTile->GetValue(overLifeTime);
                    float32 v = (distance * tile + currentParticle->life * group.layer->stripeVScrollSpeed);
                    if (group.layer->usePerspectiveMapping)
                    {
                        uv1.x = startU * fullEdgeSize;
                        v *= fullEdgeSize;
                        uv1.z = fullEdgeSize;
                        uv2.x = (startU + 1.0f) * fullEdgeSize;
                        uv2.z = fullEdgeSize;
                    }
                    uv1.y = v;
                    uv2.y = v;

                    UpdateStripeVertex(vertexBufferData, left, uv1, color, group.layer, currentParticle, fresnelToAlpha);
                    UpdateStripeVertex(vertexBufferData, right, uv2, color, group.layer, currentParticle, fresnelToAlpha);

                    prevNode = &node;
                }
                for (uint32 i = 0; i < static_cast<uint32>(nodes.size()); ++i)
                {
                    uint32 twoI = i * 2;
                    *(indexBufferData++) = twoI + 0 + baseVertex;
                    *(indexBufferData++) = twoI + 3 + baseVertex;
                    *(indexBufferData++) = twoI + 1 + baseVertex;

                    *(indexBufferData++) = twoI + 0 + baseVertex;
                    *(indexBufferData++) = twoI + 2 + baseVertex;
                    *(indexBufferData++) = twoI + 3 + baseVertex;
                }
                baseVertex += vCountInBasis;
            }
            AppendRenderBatch(begin->material, iCount, SelectLayout(*begin->layer), vb, ib.buffer, ib.baseIndex);
            currentParticle = currentParticle->next;
        }
    }
}

void ParticleRenderObject::BindDynamicParameters(Camera* camera)
{
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_WORLD, &Matrix4::IDENTITY, reinterpret_cast<pointer_size>(&Matrix4::IDENTITY));
}

} //namespace
