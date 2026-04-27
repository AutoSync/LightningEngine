cmake_minimum_required(VERSION 3.20)
project(HelloPlugin CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ---------------------------------------------------------------------------
# Lightning Engine SDK location.
#
# Pass `-DLIGHTNING_ENGINE_SDK=<repo>/src/include` when building outside the
# engine repository. Inside the repo this defaults to ../../src/include.
# ---------------------------------------------------------------------------
set(LIGHTNING_ENGINE_SDK "${CMAKE_CURRENT_SOURCE_DIR}/../../../src/include"
    CACHE PATH "Path to Lightning Engine public headers")

if (NOT EXISTS "${LIGHTNING_ENGINE_SDK}/PluginContracts.h")
    message(FATAL_ERROR
        "LIGHTNING_ENGINE_SDK does not contain PluginContracts.h: "
        "${LIGHTNING_ENGINE_SDK}")
endif()

add_library(HelloPlugin SHARED src/HelloPlugin.cpp)

target_include_directories(HelloPlugin PRIVATE
    include
    "${LIGHTNING_ENGINE_SDK}")

target_compile_definitions(HelloPlugin PRIVATE LIGHTNING_PLUGIN_BUILD)

set_target_properties(HelloPlugin PROPERTIES
    OUTPUT_NAME "HelloPlugin"
    PREFIX ""                    # no "lib" prefix on Linux/macOS
)

# Copy plugin.json next to the binary on build for quick local install.
add_custom_command(TARGET HelloPlugin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/plugin.json"
            "$<TARGET_FILE_DIR:HelloPlugin>/plugin.json")
