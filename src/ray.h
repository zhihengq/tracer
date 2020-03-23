#ifndef _RAY_H_
#define _RAY_H_

#include "glm/glm.hpp"

class Object;

struct Ray {
	glm::vec3 pos;
	glm::vec3 dir;
	Ray(const glm::vec3 &p, const glm::vec3 &d) : pos(p), dir(d) {}
	glm::vec3 at(float t) const noexcept {
		return pos + t * dir;
	}
};

struct Intersection {
	const Object *object;
	glm::vec3 normal;
	glm::vec2 uv;
	double t;
};

#endif
