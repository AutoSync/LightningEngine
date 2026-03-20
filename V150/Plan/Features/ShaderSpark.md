# Shader / Spark

Baseado no roadmap v0.15.0, com status unificado por contexto.

- [x] `SparkShader` runtime post-process (`Load`, `Process`)
- [x] Setters: `SetTint`, `SetTime`, `SetLight`, `SetFog`
- [x] `SparkCompiler` (.spark -> GLSL -> SPIR-V)
- [x] Formato `.spark` com `@vertex`, `@fragment`, `@inject`
- [x] Injeções: Time, Light, Fog, Transform
- [x] `spark_default.spark`
- [x] `spark_wave.spark`
- [ ] `@inject UVCoords`
- [ ] Sistema de luzes completo (Directional/Point/Spot/Rect/Probes)
  - [ ] Shadow Mapping
  - [ ] Deferred Shading
  - [ ] Forward+
- [ ] Variantes de fog (Distance/Height/Volumetric)
- [ ] Geometry/Tessellation/Compute shaders
