#ifndef _SPHERE_H_
#define _SPHERE_H_

#include "ray.h"
#include "object.h"

class Sphere : public Object {
  public:
	bool intersect_local(const Ray &r, Intersection &i) const noexcept override;
};

#endif
