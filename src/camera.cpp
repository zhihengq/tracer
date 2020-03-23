#include "camera.h"

#include <atomic>
#include <glm/common.hpp>
#include <memory>
#include <boost/asio/executor.hpp>
#include <boost/asio/post.hpp>
#include <glm/glm.hpp>
#include "light.h"
#include "scene.h"
#include "ray.h"
#include "object.h"

namespace {

glm::vec3 shadow(const Scene &scene, const Ray &r,
				 const PointLight *source) noexcept {
	Intersection i;
	if (!scene.intersect(r, i))
		return {1, 1, 1};
	if (i.object == source)
		return {1, 1, 1};
	if (glm::length(i.object->transmissivity) < 1e-5)
		return {0, 0, 0};
	return i.object->transmissivity
			* shadow(scene, Ray{r.at(i.t) + 1e-5f*r.dir, r.dir}, source);
}

glm::vec3 shade(const Object &o, float shininess,
                const Scene &scene,
                glm::vec3 Q, glm::vec3 N, glm::vec3 V) noexcept {
    glm::vec3 I = o.emissive;                            // emissive
    I += scene.ambiant * o.color;                        // ambient
    for (const PointLight *pl : scene.point_lights) {
		glm::vec3 diff = pl->pos() - Q;
		glm::vec3 L = glm::normalize(diff);
		float dis = glm::length(diff);
		if (glm::dot(N, L) >= 0) {
			float atten = 1.0f / (dis * dis);
			glm::vec3 I_l = atten * pl->emissive
					* shadow(scene, Ray(Q + 1e-5f*L, L), pl);
			glm::vec3 H = glm::normalize(V + L);
			I += glm::dot(N, L) * I_l * o.color;          // diffuse
			I += glm::pow(glm::max(0.0f, glm::dot(H, N)),
						  shininess) * I_l * o.specular;  // specular
		}
	}
	for (const auto &dl : scene.dir_lights) {
		glm::vec3 L = -glm::normalize(dl->direction);
		if (glm::dot(N, L) >= 0) {
			glm::vec3 I_l = dl->color * shadow(scene, Ray(Q + 1e-5f*L, L), nullptr);
			glm::vec3 H = glm::normalize(V + L);
			I += glm::dot(N, L) * I_l * o.color;          // diffuse
			I += glm::pow(glm::max(0.0f, glm::dot(H, N)),
						  shininess) * I_l * o.specular;  // specular
		}
	}
    return I;
}

glm::vec3 trace(const Scene &scene,
				const Ray &r,
				int32_t depth) noexcept {
	if (depth == 0)
		return {0, 0, 0};
	Intersection i;
	if (scene.intersect(r, i)) {
		const Object &o = *i.object;
		const float shininess = i.object->shininess;
		const glm::vec3 Q = r.at(i.t);
		const glm::vec3 D = r.dir;
		const bool exit = glm::dot(D, i.normal) > 0.0;
		const glm::vec3 N = exit ? -i.normal : i.normal;

		// direct
		const glm::vec3 I_direct = shade(o, shininess, scene, Q, N, -D);

		const float d_dot_n = glm::dot(D, N);

		// reflection
		glm::vec3 I_reflected = {0, 0, 0};
		if (glm::length(o.specular) > 1e-5) {
			glm::vec3 R = D - 2.0f * d_dot_n * N;
			I_reflected = o.specular * trace(scene, Ray(Q, R), depth - 1);
		}

		// refraction
		glm::vec3 I_refracted = {0, 0, 0};
		if (glm::length(o.transmissivity) > 1e-5) {
			float ni, nt;
			if (exit) {
				// exiting object
				ni = o.ior;
				nt = 1.00029;
			} else {
				// entering object
				ni = 1.00029;
				nt = o.ior;
			}
			const auto sqr = [](float x) { return x * x; };
			float inside_sqrt = 1.0f -
					sqr(ni) / sqr(nt) * (1.0f - sqr(d_dot_n));
			if (inside_sqrt > 0.0f) {
				// not total internal reflection
				glm::vec3 T =
						(ni / nt) * (D - d_dot_n * N)
						- (glm::sqrt(inside_sqrt) * N);
				I_refracted = o.transmissivity
						* trace(scene, Ray(Q, T), depth - 1);
			}
		}

		return glm::clamp(I_direct + I_reflected + I_refracted, 0.0f, 1.0f);
	} else if (scene.envmap != nullptr) {
		return scene.envmap->get_color(r.dir);
	} else {
		return {0, 0, 0};
	}
}

template<typename It, typename F>
void parallelForEach(boost::asio::executor exec, It begin, It end, F f) {
	if (begin + 1 == end) {
		f(begin);
	} else {
		It mid = begin + (end - begin) / 2;
		boost::asio::post(
				exec,
				[=]() { parallelForEach(exec, begin, mid, f); });
		parallelForEach(exec, mid, end, f);
	}
}

}  // namespace

void Camera::trace_rays(
		const Scene &root,
		std::function<void(std::vector<glm::vec3>)> completion) const noexcept {
	auto results = std::make_shared<std::vector<glm::vec3>>(rays.size());
	auto finished = std::make_shared<std::atomic_int32_t>(0);
		auto task = [this, finished, results, &root, completion](int32_t i) {
			(*results)[i] = trace(
					root,
					Ray{inner_to_world({0, 0, 0, 1}), inner_to_world({rays[i], 0})},
					depth);
			if (finished->fetch_add(1, std::memory_order_relaxed) + 1
					== rays.size())
				completion(*results);
		};
	parallelForEach<int32_t>(exec_, 0, rays.size(), task);
}
