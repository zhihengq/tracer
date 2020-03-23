#ifndef _OBJECT_H_
#define _OBJECT_H_

#include "glm/glm.hpp"
#include "entity.h"

class Object : public Entity {
  public:
	glm::vec3 color;
	glm::vec3 emissive = {0, 0, 0};
	glm::vec3 specular = {1, 1, 1};
	glm::vec3 transmissivity = {0, 0, 0};
	float shininess = 10;
	float ior = 1.00029;
};

#endif
