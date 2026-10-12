#pragma once

#include "geometry/MeshData.hpp"

// The single shape used for every body part: a 1x1x1 box centred on the
// origin whose edges and corners are rounded with `radius` (0 < radius <= 0.5).
// `segments` is the number of subdivisions of each rounded edge.
MeshData makeRoundedBox(float radius, int segments);
