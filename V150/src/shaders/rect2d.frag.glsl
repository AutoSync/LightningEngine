#version 450

// Fragment uniform slot 0 (SDL_PushGPUFragmentUniformData slot 0)
// SPIRV binding: set=3, binding=0
layout(set = 3, binding = 0) uniform ColorData {
    vec4 color;
} ubo;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = ubo.color;
}
