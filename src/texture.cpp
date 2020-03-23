#include "texture.h"
#include <SOIL/SOIL.h>
#include "exceptions.h"

Texture::Texture(const std::string &file, float multiplier)
		: multiplier_(multiplier) {
	int channel;
	image_ = SOIL_load_image(
			file.c_str(), &width_, &height_, &channel, SOIL_LOAD_RGBA);
	if (image_ == nullptr)
		throw FileException(file);
}

Texture::~Texture() noexcept {
	SOIL_free_image_data(image_);
}

glm::vec4 Texture::get_color(glm::vec2 uv) const noexcept {
	uv = glm::mod(uv, 1.0f);
	glm::vec2 puv = uv * glm::vec2(width_, height_);
	glm::uvec2 luv(puv);
	glm::vec2 blenduv = puv - glm::vec2(luv);
	glm::vec4 lox = glm::mix(get_pixel(luv.x  , luv.y),
							 get_pixel(luv.x+1, luv.y),
							 blenduv.x);
	glm::vec4 hix = glm::mix(get_pixel(luv.x  , luv.y+1),
							 get_pixel(luv.x+1, luv.y+1),
							 blenduv.x);
	return multiplier_ * glm::mix(lox, hix, blenduv.y);
}

glm::vec4 Texture::get_pixel(int32_t x, int32_t y) const noexcept {
	x = (x % width_ + width_) % width_;
	y = (y % height_ + height_) % height_;
	uint8_t* pixel = image_ + (y * width_ + x) * 4;
	return glm::vec4(pixel[0], pixel[1], pixel[2], pixel[3]) / 255.0f;
}
