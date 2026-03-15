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

echo Done. Shaders written to %OUT%
