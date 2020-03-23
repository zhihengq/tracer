#ifndef _ENTITY_H_
#define _ENTITY_H_

#include <cassert>
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include "ray.h"

class Entity {
  private:
	glm::mat4 transform_{1.0f};
	glm::mat4 inv_transform_{1.0f};
	Entity *parent_ = nullptr;
	std::vector<std::unique_ptr<Entity>> children_;

  public:
	virtual ~Entity() noexcept {}

	Entity *get_parent() const noexcept {
		return parent_;
	}

	const std::vector<std::unique_ptr<Entity>> &get_children() const noexcept {
		return children_;
	}

	glm::mat4 get_transform() const noexcept {
		return transform_;
	}

	Entity &set_transform(const glm::mat4 &transform) noexcept;
	glm::vec4 inner_to_outer(const glm::vec4 &inner) const noexcept;
	glm::vec4 inner_to_world(const glm::vec4 &inner) const noexcept;
	glm::vec4 outer_to_inner(const glm::vec4 &outer) const noexcept;

	Entity &add_child(std::unique_ptr<Entity> child) noexcept;
	bool intersect(const Ray &r, Intersection &i) const noexcept;
	virtual bool intersect_local(const Ray &, Intersection &) const noexcept {
		return false;
	}
};

#endif
