#ifndef _POINT_LIGHT_H_
#define _POINT_LIGHT_H_

#include "glm/glm.hpp"
#include "sphere.h"
#include "ray.h"

class PointLight : public Sphere {
  public:
	explicit PointLight(const glm::vec3 &color) noexcept {
		this->color = {0, 0, 0};
		this->emissive = color;
		this->specular = {0, 0, 0};
		this->transmissivity = {1, 1, 1};
	}

	glm::vec3 pos() const noexcept {
		return inner_to_world({0, 0, 0, 1});
	}
};

class DirLight {
  public:
	glm::vec3 color;
	glm::vec3 direction;
};

#endif
