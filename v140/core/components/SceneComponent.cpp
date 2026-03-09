#include "../../SceneComponent.h"

LightningEngine::SceneComponent::SceneComponent()
{
    InitializeSceneComponent();
}

void LightningEngine::SceneComponent::InitializeSceneComponent()
{

}

void LightningEngine::SceneComponent::UpdateByNodes()
{
    if (Parent != NULL)
        this->transform += Parent->transform;
}

void LightningEngine::SceneComponent::SetRelativeTransform(Transform T)
{
    transform = T;
}

LightningEngine::Transform LightningEngine::SceneComponent::GetRelativeTransform()
{
    return transform;
}

void LightningEngine::SceneComponent::SetRelativePosition(V3 P)
{
    this->transform.Position = P;
}

LightningEngine::V3 LightningEngine::SceneComponent::GetRelativePosition()
{
    return transform.Position;
}

void LightningEngine::SceneComponent::SetRelativeRotation(V3 R)
{
    this->transform.Rotation = R;
}

LightningEngine::V3 LightningEngine::SceneComponent::GetRelativeRotation()
{
    return transform.Rotation;
}

void LightningEngine::SceneComponent::SetRelativeScale(V3 S)
{
    this->transform.Scale = S;
}

LightningEngine::V3 LightningEngine::SceneComponent::GetRelativeScale()
{
    return transform.Scale;
}
