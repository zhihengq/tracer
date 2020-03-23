#include "entity.h"

glm::vec4 Entity::inner_to_outer(const glm::vec4 &inner) const noexcept {
	return transform_ * inner;
}

glm::vec4 Entity::inner_to_world(const glm::vec4 &inner) const noexcept {
	glm::vec4 v = inner_to_outer(inner);
	for (const Entity *e = get_parent(); e != nullptr; e = e->get_parent())
		v = e->inner_to_outer(v);
	return v;
}

glm::vec4 Entity::outer_to_inner(const glm::vec4 &outer) const noexcept {
	return inv_transform_ * outer;
}

Entity &Entity::set_transform(const glm::mat4 &transform) noexcept {
	transform_ = transform;
	inv_transform_ = glm::inverse(transform_);
	return *this;
}

Entity &Entity::add_child(std::unique_ptr<Entity> child) noexcept {
	assert(child != nullptr);
	child->parent_ = this;
	children_.emplace_back(std::move(child));
	return *this;
}

bool Entity::intersect(const Ray &r, Intersection &i) const noexcept {
	Ray local_ray = {
		outer_to_inner({r.pos, 1}),
		outer_to_inner({r.dir, 0}),
	};
	bool intersected = intersect_local(local_ray, i);
	for (const std::unique_ptr<Entity> &c : children_) {
		Intersection cur;
		if (c->intersect(local_ray, cur))
			if (!intersected || cur.t < i.t) {
				i = cur;
				intersected = true;
			}
	}
	if (intersected)
		i.normal = glm::normalize(
				glm::transpose(inv_transform_)
				* glm::vec4{i.normal, 0});
	return intersected;
}
