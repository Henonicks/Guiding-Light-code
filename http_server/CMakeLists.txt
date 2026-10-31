cmake_minimum_required(VERSION 3.15)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../cmake)

set(PROJECT_NAME "topgg_listener")
project(${PROJECT_NAME})

set(CMAKE_CXX_STANDARD 23)

find_package(Henifig MODULE)
find_package(FMT MODULE)
find_package(DPPTGG MODULE)
find_package(DPP MODULE)

aux_source_directory("src" coresrc)
add_executable(${PROJECT_NAME} ${coresrc}
        "../src/lib/guiding_light.cpp"
        "../src/lib/internal/logging.cpp"
        "../src/lib/database/database.cpp"
        "../src/lib/database/gl_on_rails.cpp"
        "../src/lib/internal/cfg.cpp"
        "../src/lib/internal/signal.cpp"
        "../src/lib/internal/launch_options.cpp"
        "../src/lib/resources/topgg_guild_choice.cpp"
        "../src/lib/resources/topgg_guild_vote_amount.cpp"
        "../src/lib/handlers/topgg.cpp"
        "../src/lib/resources/notification_channel.cpp"
        "../src/lib/resources/noguild_reminder_blocker.cpp"
)

if (USE_LOCAL_DB EQUAL 1)
    message(STATUS "USE_LOCAL_DB is ON. Switching to an sqlite3 database inside the project root.")
    target_compile_definitions(${PROJECT_NAME} PUBLIC GL_USE_LOCAL_DB)
    target_link_libraries(${PROJECT_NAME} sqlite3)
    target_include_directories(${PROJECT_NAME} PUBLIC ../sqlite_modern_cpp/hdr)
else()
    message(STATUS "USE_LOCAL_DB is OFF. Using the GoR API as the database backend.")
endif()

target_compile_definitions(${PROJECT_NAME} PUBLIC FMT_HEADER_ONLY)

target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${HENIFIG_INCLUDE_DIR}
    ${DPPTGG_INCLUDE_DIR}
    ../include
    ../colorize
)

target_link_libraries(${PROJECT_NAME}
    ${HENIFIG_LIBRARIES}
    ${DPPTGG_LIBRARIES}
    ${DPP_LIBRARIES}
)
