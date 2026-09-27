#pragma once

#include <cstdint>
namespace config
{

    enum class Mode : std::uint8_t
    {
        Default,
        Extended
    };

    struct Params
    {
        Mode mode{ Mode::Default };
    };
} // namespace config