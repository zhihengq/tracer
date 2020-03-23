#include <cstdlib>
#include <csetjmp>
#include <csignal>
#include <iostream>
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <SOIL.h>
#include <boost/asio/thread_pool.hpp>
#include "scene.h"
#include "camera.h"
#include "envmap.h"
#include "make_scene.h"
#include <chrono>

struct ExitGuard {
	~ExitGuard() {
		std::cerr << "exiting gracefully..." << std::endl;
	}
} _;

std::jmp_buf sigint;

constexpr int32_t width = 1920;
constexpr int32_t height = 1080;
constexpr float fov = 140.0;

using std::chrono::high_resolution_clock;
using std::chrono::duration;
using std::chrono::duration_cast;

int main(int argc, char **argv) {
	if (argc < 2) {
		std::cerr << argv[0] << " < <env_prefix>" << std::endl;
		return EXIT_FAILURE;
	}
	boost::asio::thread_pool pool(8);
	Scene scene;
	Camera *cam = make_scene(scene, pool.get_executor());
	cam->depth = 5;
	const float px = glm::tan(fov / 2.0 * 3.14159265359 / 180.0) / width;
	for (int32_t y = -height/2; y < height/2; y++)
		for (int32_t x = -width/2; x < width/2; x++)
			cam->rays.emplace_back(glm::normalize(glm::vec3{x*px, -y*px, -1}));
	if (setjmp(sigint) == 0) {
		std::signal(SIGINT,  [](int) { std::longjmp(sigint, 1); });
		std::signal(SIGTERM, [](int) { std::longjmp(sigint, 1); });
		while (true) {
			float yaw, pitch, roll;
			std::cin >> yaw >> pitch >> roll;
			if (std::cin.fail()) {
				return EXIT_SUCCESS;
			}
			glm::mat4 transform = cam->get_transform();
			transform = glm::rotate(transform, glm::radians(yaw),   {0, 1, 0});
			transform = glm::rotate(transform, glm::radians(pitch), {1, 0, 0});
			transform = glm::rotate(transform, glm::radians(roll),  {0, 0, 1});
			cam->set_transform(transform);
			high_resolution_clock::time_point t1 = high_resolution_clock::now();
			cam->trace_rays(
					scene,
					[t1](std::vector<glm::vec3> results) {
						high_resolution_clock::time_point t2 = high_resolution_clock::now();
						duration<double> time_span = duration_cast<duration<double>>(t2 - t1);
						std::cerr << "time: " << time_span.count() << " seconds" << std::endl;
						uint8_t buffer[width*height*4];
						for (int32_t i = 0; i < static_cast<int32_t>(results.size()); i++) {
							buffer[4*i+0] = results[i].x * 255.0 + 0.5;
							buffer[4*i+1] = results[i].y * 255.0 + 0.5;
							buffer[4*i+2] = results[i].z * 255.0 + 0.5;
							buffer[4*i+3] = 255;
						}
						SOIL_save_image("output.tga", SOIL_SAVE_TYPE_TGA, width, height, 4, buffer);
					});
		}
	}
	return EXIT_FAILURE;
}
