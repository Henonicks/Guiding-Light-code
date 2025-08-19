#include "guiding_light/exception.hpp"

[[nodiscard]] const char* success_exception::what() const noexcept {
    return "success";
}
