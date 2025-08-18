find_path(DPP_INCLUDE_DIR NAMES dpp/dpp.h HINTS ${DPP_ROOT_DIR})
message(STATUS "Include dir: ${DPP_INCLUDE_DIR}")
# set(DPP_INCLUDE_DIR ${DPP_ROOT_DIR}/include)

find_library(DPP_LIBRARIES NAMES dpp "libdpp.a" HINTS ${DPP_ROOT_DIR})
message(STATUS "Libraries dir: ${DPP_LIBRARIES}")

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(DPP DEFAULT_MSG DPP_LIBRARIES DPP_INCLUDE_DIR)
message(STATUS "DPP: ${DPP}")
