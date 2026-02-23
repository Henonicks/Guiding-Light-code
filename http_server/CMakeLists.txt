cmake_minimum_required(VERSION 4.0)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set(PROJECT_NAME "topgg_listener")
project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 20)

find_package(Henifig MODULE)
find_package(FMT MODULE)
add_subdirectory(DPPTGG)

aux_source_directory("src" coresrc)
add_executable(${PROJECT_NAME} ${coresrc})

target_compile_definitions(${PROJECT_NAME} PUBLIC FMT_HEADER_ONLY)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${HENIFIG_INCLUDE_DIR}
    ${libfmt_SOURCE_DIR}/include
)
target_link_libraries(${PROJECT_NAME}
    ${HENIFIG_LIBRARIES}
    dpptgg
)
