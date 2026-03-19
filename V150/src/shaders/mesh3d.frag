#version 450

// ── Inputs ────────────────────────────────────────────────────────────────────
layout(location = 0) in vec3 vNormal;
layout(location = 1) in vec2 vUV;

// ── Fragment sampler (SDL_GPU set=4) ──────────────────────────────────────────
layout(set = 4, binding = 0) uniform sampler2D uAlbedo;

// ── Fragment uniforms (SDL_GPU set=7) ─────────────────────────────────────────
layout(set = 7, binding = 0) uniform LightParams {
    vec4 matColor;    // base material colour tint (rgba)
    vec4 lightDir;    // world-space light direction (unnormalised, xyz used)
    vec4 lightColor;  // rgb=colour, a=intensity
    vec4 ambient;     // additive ambient colour (rgb used)
};

// ── Output ────────────────────────────────────────────────────────────────────
layout(location = 0) out vec4 fragColor;

void main()
{
    vec4 albedo = texture(uAlbedo, vUV) * matColor;

    // Lambert diffuse
    float diff  = max(dot(normalize(vNormal), normalize(-lightDir.xyz)), 0.0);
    vec3  light = ambient.rgb + diff * lightColor.rgb * lightColor.a;

    fragColor = vec4(albedo.rgb * light, albedo.a);
}
