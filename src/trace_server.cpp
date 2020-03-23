#include <atomic>
#include <cstdlib>
#include <csignal>
#include <csetjmp>
#include <iostream>
#include <sstream>
#include <memory>
#include <system_error>
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "scene.h"
#include "camera.h"
#include "make_scene.h"

namespace asio = boost::asio;            // from <boost/asio.hpp>
namespace beast = boost::beast;          // from <boost/beast.hpp>
namespace http = beast::http;            // from <boost/beast/http.hpp>
namespace websocket = beast::websocket;  // from <boost/beast/websocket.hpp>
using tcp = asio::ip::tcp;               // from <boost/asio/ip/tcp.hpp>

inline bool check(const std::error_code &ec) {
	if (ec) {
		std::cerr << ec.category().name() << " "
				<< ec.value() << ": " << ec.message() << std::endl;
		return true;
	} else {
		return false;
	}
}

class Session : public std::enable_shared_from_this<Session> {
  private:
	websocket::stream<beast::tcp_stream> ws_;
	const Scene &scene_;
	Camera &cam_;
	std::atomic_bool tracing_ = false;

  public:
	explicit Session(tcp::socket socket, const Scene &scene, Camera &cam)
			: ws_(std::move(socket)), scene_(scene), cam_(cam) {
		ws_.set_option(websocket::stream_base::timeout::suggested(
						beast::role_type::server));
		ws_.set_option(websocket::stream_base::decorator(
				[](websocket::response_type &res) {
					res.set(http::field::server, "trace_server");
				}));
		ws_.text(false);
	}

	void accept() {
		ws_.async_accept([shared_this = shared_from_this()](std::error_code ec) {
			if (check(ec)) return;
			shared_this->run();
		});
	}

	void run() {
		auto input = std::make_unique<asio::streambuf>();
		ws_.async_read(
				*input,
				[shared_this = shared_from_this(), input = std::move(input)](
						beast::error_code ec,
						[[maybe_unused]] int32_t bytes_transferred) {
					if (ec == websocket::error::closed) return;
					if (check(ec)) return;
					shared_this->run();
					std::istream is(input.get());
					if (shared_this->ws_.got_text()) {
						std::string msg(bytes_transferred, '\0');
						is.read(msg.data(), bytes_transferred);
						std::cerr << msg << std::endl;
					} else {
						if (bytes_transferred == 28)
							shared_this->update_pose(is);
						else
							shared_this->set_rays(is);
					}
				});
	}

	void update_pose(std::istream &data) {
		if (tracing_.exchange(true, std::memory_order_relaxed)) {
			std::cerr << "Dropping pose update" << std::endl;
			return;
		}
		char buf[4];
		int32_t timestamp;
		float x, y, z;
		float yaw, pitch, roll;

		data.read(buf, 4); std::memcpy(&timestamp, buf, 4);
		data.read(buf, 4); std::memcpy(&x,         buf, 4);
		data.read(buf, 4); std::memcpy(&y,         buf, 4);
		data.read(buf, 4); std::memcpy(&z,         buf, 4);
		data.read(buf, 4); std::memcpy(&yaw,       buf, 4);
		data.read(buf, 4); std::memcpy(&pitch,     buf, 4);
		data.read(buf, 4); std::memcpy(&roll,      buf, 4);

		std::cerr << "Tracing ("
				<< yaw << ", " << pitch << ", " << roll
				<< ")" << std::endl;

		glm::mat4 transform(1.0);
		transform = glm::rotate(transform, yaw,   {0, 1, 0});
		transform = glm::rotate(transform, pitch, {1, 0, 0});
		transform = glm::rotate(transform, roll,  {0, 0, 1});
		transform = glm::translate(transform, {x, y, z});
		cam_.set_transform(transform);

		cam_.trace_rays(
				scene_,
				[shared_this = shared_from_this(), timestamp](
						std::vector<glm::vec3> colors) {
					auto output = std::make_unique<asio::streambuf>();
					std::ostream result(output.get());
					char buf[4];
					std::memcpy(buf, &timestamp, 4); result.write(buf, 4);
					for (const glm::vec3 color : colors) {
						result.put(static_cast<char>(color.r * 255.0 + 0.5));
						result.put(static_cast<char>(color.g * 255.0 + 0.5));
						result.put(static_cast<char>(color.b * 255.0 + 0.5));
					}
					shared_this->ws_.async_write(
							output->data(),
							[shared_this, bytes = 4 + 3 * colors.size()](
									beast::error_code ec,
									[[maybe_unused]] int32_t bytes_transferred) {
								if (ec == websocket::error::closed) return;
								if (check(ec)) return;
								assert(bytes_transferred == bytes);
								std::cerr << "Trace finished" << std::endl;
								shared_this->tracing_.store(false, std::memory_order_relaxed);
							});
				});
	}

	void set_rays(std::istream &data) {
		char buf[4];
		int32_t depth;
		int32_t num;

		data.read(buf, 4); std::memcpy(&depth, buf, 4);
		data.read(buf, 4); std::memcpy(&num,   buf, 4);

		tracing_.store(true, std::memory_order_relaxed);
		cam_.depth = depth;
		cam_.rays.clear();
		cam_.rays.reserve(num);
		for (int32_t i = 0; i < num; i++) {
			float x, y;
			data.read(buf, 4); std::memcpy(&x, buf, 4);
			data.read(buf, 4); std::memcpy(&y, buf, 4);
			cam_.rays.emplace_back(glm::normalize(glm::vec3{x, y, -1}));
		}
		tracing_.store(false, std::memory_order_relaxed);
		std::cerr << "Received " << num << " rays" << std::endl;
	}
};

class Listener : public std::enable_shared_from_this<Listener> {
  private:
	asio::executor exec_;
	tcp::acceptor acceptor_;
	const Scene &scene_;
	Camera &cam_;

  public:
	Listener(asio::executor exec, tcp::endpoint endpoint,
			 const Scene &scene, Camera &cam)
			: exec_(exec) , acceptor_(exec), scene_(scene), cam_(cam) {
		acceptor_.open(endpoint.protocol());
		acceptor_.set_option(asio::socket_base::reuse_address(true));
		acceptor_.bind(endpoint);
		acceptor_.listen(asio::socket_base::max_listen_connections);
	}

	void run() {
		acceptor_.async_accept(
				exec_,
				[shared_this = shared_from_this()](
						std::error_code ec,
						tcp::socket socket) {
					shared_this->run();
					if (check(ec)) return;
					std::make_shared<Session>(
							std::move(socket),
							shared_this->scene_,
							shared_this->cam_)->accept();
				});
	}
};

struct ExitGuard {
	~ExitGuard() {
		std::cerr << "exiting gracefully..." << std::endl;
	}
} _;


int main(int argc, char **argv) {
	if (argc != 2) {
		std::cerr << argv[0] << " <port>" << std::endl;
		return EXIT_FAILURE;
	}
	asio::thread_pool pool;

	Scene scene;
	Camera *cam = make_scene(scene, pool.get_executor());

	const uint16_t port = static_cast<uint16_t>(std::atoi(argv[1]));
	std::make_shared<Listener>(pool.get_executor(),
							   tcp::endpoint(tcp::v6(), port),
							   scene, *cam)->run();

	std::string command;
	static std::jmp_buf sigint;
	if (setjmp(sigint) == 0) {
		std::signal(SIGINT , [](int) { std::longjmp(sigint, 1); });
		std::signal(SIGTERM, [](int) { std::longjmp(sigint, 1); });
		while (std::getline(std::cin, command)) {
			if (command == "exit")
				break;
		}
	}

	// automatically stop and join the thread pool
	return EXIT_SUCCESS;
}
