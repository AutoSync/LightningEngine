#version 450

// ── Inputs ────────────────────────────────────────────────────────────────────
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;

// ── Vertex uniforms (SDL_GPU set=3) ───────────────────────────────────────────
layout(set = 3, binding = 0) uniform Transforms {
    mat4 mvp;    // projection * view * model
    mat4 model;  // model matrix (for world-space normal transform)
};

// ── Outputs ───────────────────────────────────────────────────────────────────
layout(location = 0) out vec3 vNormal;
layout(location = 1) out vec2 vUV;

void main()
{
    // Transform normal to world space (handles non-uniform scale)
    mat3 normalMat = mat3(transpose(inverse(model)));
    vNormal = normalize(normalMat * aNormal);
    vUV     = aUV;
    gl_Position = mvp * vec4(aPos, 1.0);
}
