#pragma once

class Color
{
public:
    float R;
    float G;
    float B;

    Color();

    Color(float red, float green, float blue);

    Color operator*(const Color& color) const;

    Color operator*(float factor) const;

    Color operator+(const Color& color) const;

    Color& operator+=(const Color& color);
};
