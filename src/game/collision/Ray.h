#pragma once

#include "../../engine/vector.h"

// Ein einfacher Weltstrahl für Interaktions- und Kollisionstests
struct Ray
{
    Vector origin;
    Vector direction;
};
