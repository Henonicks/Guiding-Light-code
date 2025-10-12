#ifndef EXCEPTION_H
#define EXCEPTION_H

#include <exception>

/**
 * @brief The exception to indicate success. I throw it whenever
 * I want to exit a function that's called multiple times,
 * once and for all.
 */
class success_exception final : public std::exception {
public:
	/**
	 * @brief On success, returns "success" to indicate success in the
	 * oh so successful function that I hope is going to successfully
	 * complete with complete success to deal with any unsuccessness.
	 * I hope I successfully explained what the function always returning
	 * "success" in the success exception class designed for success indication
	 * successfully does. If it does what it's designed for unsuccessfully, I
	 * wish you success in reporting the unsuccess, so I can successfully
	 * fix the unsuccessness of this currently seemed-to-be successful code.
	 * @return Always returns "success"
	 */
	[[nodiscard]] const char* what() const noexcept override;
};

#endif
