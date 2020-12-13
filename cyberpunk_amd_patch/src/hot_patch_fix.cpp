#include <windows.h>

#include "Image.h"
#include <utility>
#include <cstring>
#include <spdlog/spdlog.h>

void HotPatchFix_1_4(Image* apImage);

void HotPatchFix(Image* apImage)
{
    if (apImage->version == Image::MakeVersion(1, 4))
        HotPatchFix_1_4(apImage);
    else
    {
        spdlog::warn("\tHot patch removal: failed, unknown version");
        return;
    }

    spdlog::info("\tHot patch removal: success");
}
