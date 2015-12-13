#pragma once

#include "nanogui\common.h"

#include <iostream>
#include <Eigen/Geometry>

typedef Eigen::Matrix<uint32_t, 1, 1> Vector3u;

inline float fast_acos(float x) {
	float negate = float(x < 0.0f);
	x = std::abs(x);
	float ret = -0.0187293f;
	ret *= x; ret = ret + 0.0742610f;
	ret *= x; ret = ret - 0.2121144f;
	ret *= x; ret = ret + 1.5707288f;
	ret = ret * std::sqrt(1.0f - x);
	ret = ret - 2.0f * negate * ret;
	return negate * (float)M_PI + ret;
}