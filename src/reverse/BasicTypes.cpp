#include "BasicTypes.h"

#include "RED4ext/REDreverse/CName.hpp"

std::string Quaternion::ToString() const noexcept
{
    return "Quaternion{ x: " + std::to_string(x) + " y: " + std::to_string(y) + " z: " + std::to_string(z) + " w: " + std::to_string(w) + " }";
}

std::string CName::ToString() const noexcept
{
    return "CName{ hash: " + std::to_string(hash) + " - '" + RED4ext::REDreverse::CName::ToString(hash) + "' }";
}
