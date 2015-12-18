#pragma once

#include "mycommon.h"

using nanogui::Vector3f;

struct TangentSpace {
	Vector3f dpdu;
	Vector3f dpdv;

	inline TangentSpace() { }
};