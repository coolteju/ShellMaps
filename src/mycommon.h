#pragma once

#include "nanogui\common.h"

#include <iostream>
#include <Eigen/Geometry>

using nanogui::Vector3f;

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

inline void coordinate_system(const Vector3f &a, Vector3f &b, Vector3f &c) {
	if (std::abs(a.x()) > std::abs(a.y())) {
		float invLen = 1.0f / std::sqrt(a.x() * a.x() + a.z() * a.z());
		c = Vector3f(a.z() * invLen, 0.0f, -a.x() * invLen);
	}
	else {
		float invLen = 1.0f / std::sqrt(a.y() * a.y() + a.z() * a.z());
		c = Vector3f(0.0f, a.z() * invLen, -a.y() * invLen);
	}
	b = c.cross(a);
}
