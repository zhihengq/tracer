#ifndef _CAMERA_H_
#define _CAMERA_H_

#include <functional>
#include <vector>
#include <boost/asio/executor.hpp>
#include "entity.h"
#include "scene.h"

class Camera : public Entity {
  private:
	boost::asio::executor exec_;

  public:
	std::vector<glm::vec3> rays;
	int32_t depth = 0;

	explicit Camera(boost::asio::executor exec) : exec_(exec) {}

	void trace_rays(
			const Scene &root,
			std::function<void(std::vector<glm::vec3>)> completion) const noexcept;
};

#endif
