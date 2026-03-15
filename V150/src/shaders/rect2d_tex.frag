#version 450

layout(location = 0) in vec2 inUV;

// Fragment sampler slot 0 → SDL_BindGPUFragmentSamplers(pass, 0, ...)
// SPIRV: set=2, binding=0
layout(set = 2, binding = 0) uniform sampler2D texSampler;

// Fragment uniform slot 0 → SDL_PushGPUFragmentUniformData(cmd, 0, ...)
// SPIRV: set=3, binding=0
layout(set = 3, binding = 0) uniform TintData {
    vec4 color;
} tint;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(texSampler, inUV) * tint.color;
}
