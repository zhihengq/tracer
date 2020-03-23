#include "envmap.h"
#include <cassert>
#include <array>

constexpr std::array<const char *, 6> suffixes
	= {"ft", "bk", "up", "dn", "rt", "lf"};

Envmap::Envmap(const std::string &prefix, float multiplier) {
	for (int32_t i = 0; i < 6; i++)
		textures[i] = std::make_unique<Texture>(
				prefix + suffixes[i] + ".tga",
				multiplier);
}

glm::vec4 Envmap::get_color(glm::vec3 vec) const noexcept {
	auto x = vec.x;
	auto y = vec.y;
	auto z = vec.z;
	glm::vec3 abs = glm::abs(vec);

	float maxAxis;
	int8_t face;
	glm::vec2 uv;

	if (abs.x >= abs.y && abs.x >= abs.z) {
		maxAxis = abs.x;
		if (x > 0) {
			// POSITIVE X
			// u (0 to 1) goes from +z to -z
			// v (0 to 1) goes from -y to +y
			uv = glm::vec2(-z, y);
			face = 0;
		} else {
			// NEGATIVE X
			// u (0 to 1) goes from -z to +z
			// v (0 to 1) goes from -y to +y
			uv = glm::vec2(z, y);
			face = 1;
		}
	} else if (abs.y >= abs.x && abs.y >= abs.z) {
		maxAxis = abs.y;
		if (y > 0) {
			// POSITIVE Y
			// u (0 to 1) goes from -x to +x
			// v (0 to 1) goes from +z to -z
			uv = glm::vec2(x, -z);
			face = 2;
		} else {
			// NEGATIVE Y
			// u (0 to 1) goes from -x to +x
			// v (0 to 1) goes from -z to +z
			uv = glm::vec2(x, z);
			face = 3;
		}
	} else /*if (abs.z >= abs.x && abs.z >= abs.y)*/ {
		maxAxis = abs.z;
		if (z > 0) {
			// POSITIVE Z
			// u (0 to 1) goes from -x to +x
			// v (0 to 1) goes from -y to +y
			uv = glm::vec2(x, y);
			face = 4;
		} else {
			// NEGATIVE Z
			// u (0 to 1) goes from +x to -x
			// v (0 to 1) goes from -y to +y
			uv = glm::vec2(-x, y);
			face = 5;
		}
	}

	// Convert range from -1 to 1 to 0 to 1
	uv = (0.5f - 1e-3f) * (uv / maxAxis + glm::vec2(1, 1));
	uv.y = 1.0 - uv.y;    // flip v coord

	return textures[face]->get_color(uv);
}
