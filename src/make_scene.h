#ifndef _MAKE_SCENE_H_
#define _MAKE_SCENE_H_

#include <boost/asio/executor.hpp>
#include "scene.h"
#include "camera.h"

Camera *make_scene(Scene &scene, boost::asio::executor exec);

#endif
