#include "Image.h"
#include <spdlog/spdlog.h>

void SpectrePatch_1_4(Image* apImage);

void SpectrePatch(Image* apImage)
{
    if (apImage->version == Image::MakeVersion(1, 4))
        SpectrePatch_1_4(apImage);
    else
    {
        spdlog::warn("\tSpectre patch: failed, unknown version");
        return;
    }

    spdlog::info("\tSpectre patch: success");
}
