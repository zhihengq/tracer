#include "make_scene.h"
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "scene.h"
#include "camera.h"
#include "sphere.h"
#include "light.h"

inline glm::mat4 transform(glm::vec3 tr, glm::vec3 rt, glm::vec3 sc) noexcept {
	glm::mat4 transform(1);
	transform = glm::translate(transform, tr);
	transform = glm::rotate(transform, rt[0], {0, 1, 0});
	transform = glm::rotate(transform, rt[1], {1, 0, 0});
	transform = glm::rotate(transform, rt[2], {0, 0, 1});
	transform = glm::scale(transform, sc);
	return transform;
}

Camera *make_scene(Scene &scene, boost::asio::executor exec) {
	scene.envmap = std::make_unique<Envmap>("env_", 1);
	scene.ambiant = {0.01, 0.01, 0.01};
	scene.set_transform(transform({0, 0, 0}, {-1, 0, 0}, {1, 1, 1}));

	scene.dir_lights.emplace_back(std::make_unique<DirLight>());
	scene.dir_lights[0]->color = {0.5, 0.5, 0.5};
	scene.dir_lights[0]->direction =
			scene.get_transform() * glm::vec4{-1, -1, 0, 0};

	auto pl = std::make_unique<PointLight>(glm::vec3{5, 5, 0.1});
	pl->set_transform(transform({-2, 2, -5}, {0, 0, 0}, {0.1, 0.1, 0.1}));
	scene.point_lights.emplace_back(pl.get());
	scene.add_child(std::move(pl));

	std::unique_ptr<Sphere> s;

	// translucent
	s = std::make_unique<Sphere>();
	s->color = {0, 0, 0};
	s->specular = {0.2, 0.2, 0.2};
	s->transmissivity = {0.7, 0.9, 0.7};
	s->ior = 1.3;
	s->shininess = 100;
	s->set_transform(transform({-1, 0, -4}, {0, 0, 0}, {1, 1, 1}));
	scene.add_child(std::move(s));

	// normal
	s = std::make_unique<Sphere>();
	s->color = {0.1, 0.3, 0.9};
	s->specular = {0.2, 0.2, 0.2};
	s->set_transform(transform({-1, 0, -6}, {0, 0.5, 1}, {2, 1, 2}));
	scene.add_child(std::move(s));

	// specular
	s = std::make_unique<Sphere>();
	s->color = {0.2, 0.2, 0.2};
	s->specular = {0.7, 0.7, 0.7};
	s->shininess = 50;
	s->set_transform(transform({4, 3, -13}, {0, 0, 0}, {5, 5, 5}));
	scene.add_child(std::move(s));

	// normal
	s = std::make_unique<Sphere>();
	s->color = {1, 1, 1};
	s->specular = {0, 0, 0};
	s->set_transform(transform({-2, -3, -8}, {0, 0, 0}, {10, 2, 10}));
	scene.add_child(std::move(s));

	auto camera = std::make_unique<Camera>(exec);
	Camera *cam = camera.get();
	scene.add_child(std::move(camera));
	return cam;
}
