#pragma once

namespace data {
    // @brief Represents a 3-component normalised RGB colour
    struct ColourRGB {
        float r = 0.0f;
        float g = 0.0f;
        float b = 0.0f;
    };

    // @brief Represents a 4-component normalised RGBA colour
    struct ColourRGBA {
        float r = 0.0f;
        float g = 0.0f;
        float b = 0.0f;
        float a = 0.0f;
    };

    // @brief Represents a 3-component normalised HSV colour
    struct ColourHSV {
        float h = 0.0f;
        float s = 0.0f;
        float v = 0.0f;
    };

    // @brief Represents a 4-component normalised HSVA colour
    struct ColourHSVA {
        float h = 0.0f;
        float s = 0.0f;
        float v = 0.0f;
        float a = 0.0f;
    };
}