#version 450

layout(location = 0) in vec2 inUV;

// Fragment sampler slot 0
layout(set = 2, binding = 0) uniform sampler2D texSampler;

// Fragment uniform slot 0
// params: x = blur radius (pixels), y = unused, z = texture width, w = texture height
layout(set = 3, binding = 0) uniform BlurParams {
    vec4 params;
} blur;

layout(location = 0) out vec4 outColor;

void main() {
    float radius   = max(1.0, blur.params.x);
    vec2  texSize  = blur.params.zw;
    vec2  texelSize = 1.0 / texSize;

    vec4  result = vec4(0.0);
    float total  = 0.0;
    float sigma  = radius * 0.5;

    int r = int(ceil(radius));
    for (int x = -r; x <= r; x++) {
        for (int y = -r; y <= r; y++) {
            float dist = float(x * x + y * y);
            float w = exp(-dist / (2.0 * sigma * sigma));
            result += texture(texSampler, inUV + vec2(x, y) * texelSize) * w;
            total  += w;
        }
    }

    outColor = result / total;
}
