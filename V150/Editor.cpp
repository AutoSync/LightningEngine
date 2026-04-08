// Lightning Engine — Editor Entry Point
// 2018-2026 (C) AutoSync Lightning Engine - By Erick Andrade. All Rights Reserved.
#include "src/editor/EditorApp.h"
#include "src/tests/ImagePipelineTests.h"
#include "src/tests/UITests.h"
#include <cstring>

int main(int argc, char* argv[])
{
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--run-tests") == 0) {
            int imageRc = LightningTests::RunImagePipelineTests();
            int uiRc    = LightningTests::RunUITests();
            return (imageRc == 0 && uiRc == 0) ? 0 : 1;
        }
    }

    EditorApp editor;
    editor.Run("Lightning Engine Editor V0.15.0", 1280, 720);
    return 0;
}
