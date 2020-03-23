#include "sphere.h"
#include "glm/glm.hpp"

constexpr float RAY_EPSILON = 0.001;

inline float sqr(float x) {
	return x * x;
}

bool Sphere::intersect_local(const Ray &r, Intersection &i) const noexcept {
    const glm::vec3 &pos = r.pos;
    const glm::vec3 &dir = r.dir;

    // Quadratic equation is At^2 + Bt + C = 0 where,
    // A = d_x^2 + d_y^2 + d_z^2
    float A = sqr(dir.x) + sqr(dir.y) + sqr(dir.z);
    // B = 2(p_x*d_x + p_y*d_y + p_z*d_z)
    float B = 2 * (pos.x * dir.x + pos.y * dir.y + pos.z * dir.z);
    // C = p_x^2 + p_y^2 + p_z^2 - r^2
    float C = sqr(pos.x) + sqr(pos.y) + sqr(pos.z) - sqr(0.5);
    // Now, the only unknown is t. Solve using the quadratic equation.
    // [-B +- sqrt(B^2 - 4AC)]/[2A]
    // but first do a preliminary check of the discriminant.
    float discrim = sqr(B) - 4 * A * C;

    float t, t1, t2;
    if (discrim < 0) {
        // Only complex solutions
        return false;
    } else if (discrim == 0) {
        // One real solution (a grazing of the sphere)
        t1 = (-B + glm::sqrt(discrim)) / (2 * A);
        t2 = -1000;
    } else {
        // Two real solutions (in and out of the sphere)
        t1 = (-B + glm::sqrt(discrim)) / (2 * A);
        t2 = (-B - glm::sqrt(discrim)) / (2 * A);
    }

    // Check for which t (if either) is best
    if (t1 < RAY_EPSILON && t2 < RAY_EPSILON)
		return false;
    else if (t1 <= RAY_EPSILON)
		t = t2;
    else if (t2 <= RAY_EPSILON)
		t = t1;
    else
		t = t1 < t2 ? t1 : t2;

	i.object = this;

    // Calculate and set normal vector
    glm::vec3 normal = glm::normalize(r.at(t));
    i.normal = normal;
    i.t = t;

    // Calculate UV coords
    float u = 0.5 + atan2(normal.z, normal.x) / (2.0 * M_PI);
    float v = 0.5 - asin(normal.y) / M_PI;
    i.uv = {u, v};

    return true;
}
