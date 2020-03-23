#ifndef _EXCEPTIONS_H_
#define _EXCEPTIONS_H_

#include <exception>
#include <string>

class FileException : public std::exception {
  private:
	std::string file_;

  public:
	explicit FileException(const std::string &file) : file_(file) {}

	const char *what() const noexcept override {
		return file_.c_str();
	}
};

#endif
