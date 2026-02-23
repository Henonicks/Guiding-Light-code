find_path(FMT_INCLUDE_DIR NAMES fmt/format.h)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(FMT DEFAULT_MSG FMT_INCLUDE_DIR)
