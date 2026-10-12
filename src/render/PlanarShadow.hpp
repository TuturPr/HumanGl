#pragma once

#include "math/Mat4.hpp"

// Projects any point onto the horizontal plane y = height along the light
// direction `lightDirection` (pointing from the light toward the scene,
// lightDirection.y < 0). Used to flatten the body into its shadow.
Mat4 planarShadowMatrix(const Vec3& lightDirection, float height);
