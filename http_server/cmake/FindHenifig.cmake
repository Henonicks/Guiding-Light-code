find_path(HENIFIG_INCLUDE_DIR NAMES henifig/henifig.hpp HINTS "../Henifig/include")

find_library(HENIFIG_LIBRARIES NAMES henifig "libhenifig" HINTS "../build/Henifig")

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(Henifig DEFAULT_MSG HENIFIG_INCLUDE_DIR HENIFIG_LIBRARIES)
