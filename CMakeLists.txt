cmake_minimum_required (VERSION 3.15)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(BOT_NAME "guidingLight")

project(${BOT_NAME})

if (CMAKE_PREFIX_PATH)
    link_directories(${CMAKE_PREFIX_PATH}/lib)
endif()

aux_source_directory("src" coresrc)
aux_source_directory("src/lib" codParts)
aux_source_directory("src/lib/helpers" helpers)
add_executable(${BOT_NAME} ${coresrc} ${codParts} ${helpers})

target_compile_definitions(${BOT_NAME} PUBLIC FMT_HEADER_ONLY)

string(ASCII 27 Esc)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set_target_properties(${BOT_NAME} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

set(THREADS_PREFER_PTHREAD_FLAG TRUE)
set(DPP_LIBRARIES dpp)
find_package(Threads REQUIRED)
find_package(DPP)
if(APPLE)
	if(CMAKE_APPLE_SILICON_PROCESSOR)
		set(OPENSSL_ROOT_DIR "/opt/homebrew/opt/openssl")
	else()
		set(OPENSSL_ROOT_DIR "/usr/local/opt/openssl")
	endif()
	find_package(OpenSSL REQUIRED)
else()
	find_package(OpenSSL REQUIRED)
endif()

target_include_directories(${BOT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${OPENSSL_INCLUDE_DIR}
)

find_package(FMT)
add_subdirectory(colorize)
add_subdirectory(cpp-linenoise)
add_subdirectory(sqlite_modern_cpp)
add_subdirectory(Henifig)

target_compile_options(${BOT_NAME} PRIVATE -Wall -Wextra -Wpedantic -g)

target_link_libraries(${BOT_NAME}
    dl
    ${CMAKE_THREAD_LIBS_INIT}
    ${OPENSSL_CRYPTO_LIBRARY}
    ${OPENSSL_SSL_LIBRARY}
    sqlite_modern_cpp
    linenoise
    colorize
    henifig
)

target_compile_definitions(${BOT_NAME} PUBLIC DPP_CORO)
target_compile_features(${BOT_NAME} PUBLIC cxx_std_20)

if (DPP_FOUND)
    target_link_libraries(${BOT_NAME} ${DPP_LIBRARIES})
    target_include_directories(${BOT_NAME} PUBLIC ${DPP_INCLUDE_DIR})
else()
    message(WARNING "Could not find DPP install. Building from source instead.")
    option(DPP_BUILD_TEST "" OFF)
    include(FetchContent)

    FetchContent_Declare(
            libdpp
            GIT_REPOSITORY https://github.com/brainboxdotcc/DPP.git
            GIT_TAG master)

    FetchContent_GetProperties(libdpp)
    if(NOT libdpp_POPULATED)
        FetchContent_MakeAvailable(libdpp)
    endif()
    target_link_libraries(${BOT_NAME} dpp)
endif()

if (NOT FMT_FOUND)
    message(WARNING "Could not find libfmt install. Installing in the build directory.")
    include(FetchContent)

    FetchContent_Declare(
            libfmt
            GIT_REPOSITORY https://github.com/fmtlib/fmt.git
            GIT_TAG master)

    FetchContent_GetProperties(libfmt)
    if(NOT libfmt_POPULATED)
        FetchContent_MakeAvailable(libfmt)
    endif()
    target_include_directories(${BOT_NAME} PUBLIC
        ${libfmt_SOURCE_DIR}/include
    )
endif()
