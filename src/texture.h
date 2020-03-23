#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#include <cstdint>
#include <string>
#include <glm/glm.hpp>

class Texture {
	Texture(const Texture &) = delete;
	Texture &operator=(const Texture &) = delete;

  public:
	explicit Texture(const std::string &file, float multiplier_);
	~Texture() noexcept;
	glm::vec4 get_color(glm::vec2 uv) const noexcept;

  private:
	int32_t width_;
	int32_t height_;
	float multiplier_;
	uint8_t *image_;

	glm::vec4 get_pixel(int32_t x, int32_t y) const noexcept;
};

#endif
