#ifndef _ENVMAP_H_
#define _ENVMAP_H_

#include <string>
#include <memory>
#include "texture.h"

class Envmap {
	Envmap(const Envmap &) = delete;
	Envmap &operator=(const Envmap &) = delete;

  public:
	explicit Envmap(const std::string &prefix, float multiplier);
	glm::vec4 get_color(glm::vec3 vec) const noexcept;

  private:
	std::unique_ptr<Texture> textures[6];
};

#endif
