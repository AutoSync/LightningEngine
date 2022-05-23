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

void Lightning::SceneComponent::SetTransform(Transform T)
{
    transform = T;
}

Lightning::Transform Lightning::SceneComponent::GetTransform()
{
    return transform;
}

void Lightning::SceneComponent::SetPosition(V3 P)
{
    this->transform.Position = P;
}

Lightning::V3 Lightning::SceneComponent::GetPosition()
{
    return transform.Position;
}

void Lightning::SceneComponent::SetRotation(V3 R)
{
    this->transform.Rotation = R;
}

Lightning::V3 Lightning::SceneComponent::GetRotation()
{
    return transform.Rotation;
}

void Lightning::SceneComponent::SetScale(V3 S)
{
    this->transform.Scale = S;
}

Lightning::V3 Lightning::SceneComponent::GetScale()
{
    return transform.Scale;
}
