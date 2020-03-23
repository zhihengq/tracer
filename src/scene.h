#ifndef _SCENE_H_
#define _SCENE_H_

#include <memory>
#include "entity.h"
#include "envmap.h"
#include "light.h"

class Scene : public Entity {
  public:
	std::unique_ptr<Envmap> envmap;
	glm::vec3 ambiant;
	std::vector<const PointLight *> point_lights;
	std::vector<std::unique_ptr<DirLight>> dir_lights;
};

#endif
