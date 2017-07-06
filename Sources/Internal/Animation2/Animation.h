#pragma once

class Skeleton;
class AnimationClip;
class AnimationPose;

class Animation
{
public:
    Animation();

    bool Bind(const Skeleton& target, const AnimationClip& anim);

    void Reset();
    void Advance(float dt_s);
    void GetPose(AnimationPose* pose) const;
    void GetPosePartial(AnimationPose* pose) const;

private:
};