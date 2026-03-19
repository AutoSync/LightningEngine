@echo off
setlocal

:: Requires glslc from the Vulkan SDK: https://vulkan.lunarg.com/
where glslc >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: glslc not found in PATH.
    echo Install the Vulkan SDK and make sure its Bin folder is in PATH.
    echo   https://vulkan.lunarg.com/
    exit /b 1
)

set OUT=..\..\assets\shaders
mkdir %OUT% 2>nul

echo Compiling rect2d.vert.glsl ...
glslc rect2d.vert -o %OUT%\rect2d_vert.spv
if %errorlevel% neq 0 ( echo FAILED & exit /b 1 )

echo Compiling rect2d.frag ...
glslc rect2d.frag -o %OUT%\rect2d_frag.spv
if %errorlevel% neq 0 ( echo FAILED & exit /b 1 )

echo Compiling rect2d_tex.vert ...
glslc rect2d_tex.vert -o %OUT%\rect2d_tex_vert.spv
if %errorlevel% neq 0 ( echo FAILED & exit /b 1 )

echo Compiling rect2d_tex.frag ...
glslc rect2d_tex.frag -o %OUT%\rect2d_tex_frag.spv
if %errorlevel% neq 0 ( echo FAILED & exit /b 1 )

echo Compiling blur.frag ...
glslc blur.frag -o %OUT%\blur_frag.spv
if %errorlevel% neq 0 ( echo FAILED & exit /b 1 )

echo Compiling mesh3d.vert ...
glslc mesh3d.vert -o %OUT%\mesh3d_vert.spv
if %errorlevel% neq 0 ( echo FAILED & exit /b 1 )

echo Compiling mesh3d.frag ...
glslc mesh3d.frag -o %OUT%\mesh3d_frag.spv
if %errorlevel% neq 0 ( echo FAILED & exit /b 1 )

echo Done. Shaders written to %OUT%

:: ── Spark shaders ──────────────────────────────────────────────────────────
:: .spark files are compiled at runtime via SparkCompiler::Compile().
:: You can also pre-compile them here using the pattern below:
::
::   SparkCompiler::Compile("src/shaders/my.spark", "assets/shaders/my");
::
:: Or manually using glslc (after running SparkCompiler::GenerateGLSL to get .glsl):
::   glslc my_vert.glsl -o assets/shaders/my_vert.spv
::   glslc my_frag.glsl -o assets/shaders/my_frag.spv
::
:: Included .spark examples:
::   spark_default.spark — pass-through tinted blit
::   spark_wave.spark    — sine-wave distortion (requires @inject Time)
