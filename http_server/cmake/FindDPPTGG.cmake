find_path(DPPTGG_INCLUDE_DIR NAMES dpptgg/dpptgg.hpp HINTS "../DPPTGG/include")

find_library(DPPTGG_LIBRARIES NAMES dpptgg "libdpptgg" HINTS "../build/DPPTGG")

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(DPPTGG DEFAULT_MSG DPPTGG_INCLUDE_DIR DPPTGG_LIBRARIES)
