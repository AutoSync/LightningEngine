#include "SceneComponent.h"

Lightning::SceneComponent::SceneComponent()
{
    InitializeSceneComponent();
}

void Lightning::SceneComponent::InitializeSceneComponent()
{
}

void Lightning::SceneComponent::UpdateByNodes()
{
    if (Parent != NULL)
        this->transform += Parent->transform;
}

void Lightning::SceneComponent::SetRelativeTransform(Transform T)
{
    transform = T;
}

Lightning::Transform Lightning::SceneComponent::GetRelativeTransform()
{
    return transform;
}

void Lightning::SceneComponent::SetRelativePosition(V3 P)
{
    this->transform.Position = P;
}

Lightning::V3 Lightning::SceneComponent::GetRelativePosition()
{
    return transform.Position;
}

void Lightning::SceneComponent::SetRelativeRotation(V3 R)
{
    this->transform.Rotation = R;
}

Lightning::V3 Lightning::SceneComponent::GetRelativeRotation()
{
    return transform.Rotation;
}

void Lightning::SceneComponent::SetRelativeScale(V3 S)
{
    this->transform.Scale = S;
}

Lightning::V3 Lightning::SceneComponent::GetRelativeScale()
{
    return transform.Scale;
}
