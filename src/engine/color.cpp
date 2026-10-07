#include "color.h"

Color::Color()
    : R(0.0f),
      G(0.0f),
      B(0.0f)
{
}

Color::Color(float red, float green, float blue)
    : R(red),
      G(green),
      B(blue)
{
}

Color Color::operator*(const Color& color) const
{
    return Color(R * color.R, G * color.G, B * color.B);
}

Color Color::operator*(float factor) const
{
    return Color(R * factor, G * factor, B * factor);
}

Color Color::operator+(const Color& color) const
{
    return Color(R + color.R, G + color.G, B + color.B);
}

Color& Color::operator+=(const Color& color)
{
    R += color.R;
    G += color.G;
    B += color.B;

    return *this;
}
