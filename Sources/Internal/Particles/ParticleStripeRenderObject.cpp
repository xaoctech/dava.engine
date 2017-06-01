#include "Particles/ParticleStripeRenderObject.h"

#include "Math/MathHelpers.h"
#include "Time/SystemTimer.h"

namespace DAVA
{

ParticleStripeRenderObject::ParticleStripeRenderObject(ParticleEffectData* effectData_)
    : effectData(effectData_)
{
    AddFlag(RenderObject::ALWAYS_CLIPPING_VISIBLE);
    AddFlag(RenderObject::CUSTOM_PREPARE_TO_RENDER);

    type = RenderObject::TYPE_PARTICLE_EMITTER;

    rhi::VertexLayout l;
    l.AddElement(rhi::VS_POSITION, 0, rhi::VDT_FLOAT, 3);
    l.AddElement(rhi::VS_TEXCOORD, 0, rhi::VDT_FLOAT, 2);
    l.AddElement(rhi::VS_COLOR, 0, rhi::VDT_UINT8N, 4);
    layout = rhi::VertexLayout::UniqueId(l);
    stride = (3 + 2 + 1) * sizeof(float);

    batch = new RenderBatch();
    mat = new NMaterial();
    mat->SetFXName(NMaterialName::PARTICLES);
    mat->AddFlag(NMaterialFlagName::FLAG_BLENDING, 1);
}

ParticleStripeRenderObject::~ParticleStripeRenderObject()
{
    SafeRelease(batch);
    SafeRelease(mat);
}

struct ParticleVertex
{
    Vector3 pos;
    Vector2 uv;
    uint32 color;
};


void ParticleStripeRenderObject::PrepareToRender(Camera* camera)
{
    static Vector3 basisVector;
    activeRenderBatchArray.clear();

    const Matrix4& mv = camera->GetMatrix();
    basisVector = Vector3(0.0f, 0.0f, 1.0f).CrossProduct(camera->GetDirection());
    basisVector.Normalize();

    if (effectData->groups.size() == 0)
        return;
    DAVA::ParticleGroup group = *(effectData->groups.begin());

    DAVA::Particle* particle = group.head;
    if (group.head == nullptr)
        return;

    float32 dt = SystemTimer::GetFrameDelta();
    if (dt < 0 || dt > 1)
        dt = 0;
    baseNode.lifeime = 0;
    baseNode.position = particle->position;

    spawnTimer += dt;
    gameTimer += dt;
    float32 spawnTime = 1.0f / group.layer->stripeRate;
    if (spawnTimer > spawnTime)
    {
        static int i = 0;
        i++;
        spawnTimer -= spawnTime;
        StripeNode newNode = baseNode;
        stripeNodes.emplace_front(0.0f, baseNode.position, baseNode.speed);
    }
    auto nodeIter = stripeNodes.begin();
    while (nodeIter != stripeNodes.end())
    {
        nodeIter->lifeime += dt;
        nodeIter->position += Vector3(0.0f, 0.0f, 1.0f) * group.layer->stripeSpeed * dt;

        if (nodeIter->lifeime >= group.layer->stripeLifetime)
        {
            stripeNodes.erase(nodeIter++);
        }
        else
            ++nodeIter;
    }
    if (stripeNodes.size() == 0)
        return;

    int32 vCount = static_cast<int32>((stripeNodes.size() + 1) * 2);
    DynamicBufferAllocator::AllocResultVB vb = DynamicBufferAllocator::AllocateVertexBuffer(stride, vCount);
    uint32 iCount = (vCount - 2) * 3;
    DynamicBufferAllocator::AllocResultIB ib = DynamicBufferAllocator::AllocateIndexBuffer(iCount);
    Vector3 left = baseNode.position + basisVector * group.layer->stripeStartSize * 0.5f;
    Vector3 right = baseNode.position - basisVector * group.layer->stripeStartSize * 0.5f;
    Vector2 uv1(gameTimer*group.layer->stripeUScrollSpeed, gameTimer*group.layer->stripeVScrollSpeed);
    Vector2 uv2(gameTimer*group.layer->stripeUScrollSpeed + 1.0f, gameTimer*group.layer->stripeVScrollSpeed);
    float* current = reinterpret_cast<float*>(vb.data);
    uint32 col = rhi::NativeColorRGBA(particle->color.r, particle->color.g, particle->color.b, particle->color.a);
    float32* color = reinterpret_cast<float32*>(&col);
    *(current++) = left.x;
    *(current++) = left.y;
    *(current++) = left.z;
    *(current++) = uv1.x;
    *(current++) = uv1.y;
    *(current++) = *color;

    *(current++) = right.x;
    *(current++) = right.y;
    *(current++) = right.z;
    *(current++) = uv2.x;
    *(current++) = uv2.y;
    *(current++) = *color;

    StripeNode& prevNode = baseNode;
    float32 distance = 0.0f;
    for (auto& node : stripeNodes)
    {
        float32 lv = node.lifeime / group.layer->stripeLifetime;
        float32 size = Lerp(group.layer->stripeStartSize, group.layer->stripeSizeOverLife, lv);
        left = node.position + basisVector * size * 0.5f;
        right = node.position - basisVector * size * 0.5f;

        float32 alpha = Lerp(1.0f, group.layer->stripeAlphaOverLife, lv);
        col = rhi::NativeColorRGBA(particle->color.r, particle->color.g, particle->color.b, particle->color.a * alpha);


        distance += (prevNode.position - node.position).Length();
        float32 v = distance * group.layer->stripeTextureTile + gameTimer*group.layer->stripeVScrollSpeed;
        uv1.y = v;
        uv2.y = v;
        *(current++) = left.x;
        *(current++) = left.y;
        *(current++) = left.z;
        *(current++) = uv1.x;
        *(current++) = uv1.y;
        *(current++) = *color;

        *(current++) = right.x;
        *(current++) = right.y;
        *(current++) = right.z;
        *(current++) = uv2.x;
        *(current++) = uv2.y;
        *(current++) = *color;

        prevNode = node;
    }
    uint16* currentI = ib.data;
    for (uint32 i = 0; i < static_cast<uint32>(stripeNodes.size()); ++i)
    {
        uint32 twoI = i * 2;
        *(currentI++) = twoI + 0;
        *(currentI++) = twoI + 3;
        *(currentI++) = twoI + 1;

        *(currentI++) = twoI + 0;
        *(currentI++) = twoI + 2;
        *(currentI++) = twoI + 3;
    }

    batch->primitiveType = rhi::PRIMITIVE_TRIANGLELIST;
    batch->SetRenderObject(this);
    batch->SetMaterial(group.material);

    batch->vertexBuffer = vb.buffer;
    batch->vertexCount = vCount;
    batch->vertexBase = 0;

    batch->indexCount = iCount;
    batch->indexBuffer = ib.buffer;
    batch->startIndex = 0;
    batch->vertexLayoutId = layout;

    activeRenderBatchArray.push_back(batch);
}

void ParticleStripeRenderObject::BindDynamicParameters(Camera* camera)
{
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_WORLD, &Matrix4::IDENTITY, reinterpret_cast<pointer_size>(&Matrix4::IDENTITY));
}

void ParticleStripeRenderObject::SetSortingOffset(uint32 offset)
{

}

}