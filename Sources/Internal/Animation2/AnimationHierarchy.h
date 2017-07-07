#pragma once

namespace DAVA
{
struct Matrix4;
class Quaternion;
class Vector3;
}
class AnimationPose;

class Skeleton
{
public:
    class Pose;

    Skeleton();

    bool Load(const char* fileName);

    unsigned NodeCount() const;
    const char* NodeId(unsigned node_i) const;
    const DAVA::Matrix4& NodeBindMatrix(unsigned node_i) const;
    const DAVA::Matrix4& NodeInvBindMatrix(unsigned node_i) const;

    void ApplyPose(const Pose& pose);
    void ComputeBoneWeights();

private:
    unsigned nodeCount;
    DAVA::Matrix4* bindMatrix;
    DAVA::Matrix4* invBindMatrix;
    const char** nodeId;
};

class Skeleton::Pose
{
public:
    Pose();

    void SetNodeTransform(unsigned node_i, DAVA::Quaternion q, DAVA::Vector3 t);

    const DAVA::Matrix4* NodeTransforms() const;

    static void Blend(Pose* dst, const Pose& src1, const Pose& src2, float ratio);
    static void Add(Pose* dst, const Pose& src);

private:
    unsigned nodeCount;
    DAVA::Matrix4* transform;
    DAVA::Quaternion* rotation;
    DAVA::Vector3* translation;
};