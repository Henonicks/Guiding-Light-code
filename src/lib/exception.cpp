#include "exception.h"

[[nodiscard]] const char* success_exception::what() const noexcept {
    return "success";
}
