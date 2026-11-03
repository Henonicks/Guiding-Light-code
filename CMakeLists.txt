cmake_minimum_required (VERSION 3.15)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(BOT_NAME "guidingLight")

project(${BOT_NAME})

if (CMAKE_PREFIX_PATH)
    link_directories(${CMAKE_PREFIX_PATH}/lib)
endif()

aux_source_directory("src" coresrc)
aux_source_directory("src/lib" codParts)
aux_source_directory("src/lib/resources" resourcecode)
aux_source_directory("src/lib/database" resourcecode)
aux_source_directory("src/lib/handlers" handlercode)
aux_source_directory("src/lib/internal" internalcode)
aux_source_directory("src/lib/json" jsoncode)
aux_source_directory("src/lib/generic" genericcode)
add_executable(${BOT_NAME} ${coresrc} ${codParts} ${resourcecode} ${handlercode} ${internalcode} ${database} ${jsoncode} ${genericcode})

target_compile_definitions(${BOT_NAME} PRIVATE GL_BOT_ENVIRONMENT)
target_compile_definitions(${BOT_NAME} PRIVATE GL_EXE_NAME="${BOT_NAME}")

string(ASCII 27 Esc)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set_target_properties(${BOT_NAME} PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
)

set(THREADS_PREFER_PTHREAD_FLAG TRUE)
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
    "http_server/include"
)

add_subdirectory(colorize)
add_subdirectory(cpp-linenoise)
add_subdirectory(sqlite_modern_cpp)
add_subdirectory(Henifig)
add_subdirectory(DPPTGG)

#set(SANMAKE_ASAN OFF)
#add_subdirectory(sanmake)

#target_compile_options(${BOT_NAME} PRIVATE -Wall -Wextra -Wpedantic -g)

if (USE_LOCAL_DB EQUAL 1)
    message(STATUS "USE_LOCAL_DB is ON. Switching to an sqlite3 database inside the project root.")
    target_compile_definitions(${BOT_NAME} PUBLIC GL_USE_LOCAL_DB)
    target_link_libraries(${BOT_NAME} sqlite_modern_cpp)
else()
    message(STATUS "USE_LOCAL_DB is OFF. Using the GoR API as the database backend.")
endif()

target_link_libraries(${BOT_NAME}
#    tsan
    dl
    ${CMAKE_THREAD_LIBS_INIT}
    ${OPENSSL_CRYPTO_LIBRARY}
    ${OPENSSL_SSL_LIBRARY}
    linenoise
    colorize
    henifig
    dpptgg
)

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
