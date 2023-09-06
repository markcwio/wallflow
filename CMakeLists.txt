cmake_minimum_required(VERSION 3.19)
set(PROJECT_NAME wallflow)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(${PROJECT_NAME})

file(GLOB
    SOURCES
    "src/*.cpp"
)

add_executable(
    ${PROJECT_NAME}
    ${SOURCES}
    wallflow.rc
)

message("CMBT:" ${CMAKE_BUILD_TYPE})

if(NOT DEFINED APP_DATA_DIR)
    set(APP_DATA_DIR "WallFlow" CACHE STRING "Directory to use in AppData")
endif()

target_compile_definitions(
    ${PROJECT_NAME}
    PRIVATE
    UNICODE
    STB_IMAGE_STATIC
    STB_IMAGE_WRITE_STATIC
    STB_IMAGE_IMPLEMENTATION
    STB_IMAGE_WRITE_IMPLEMENTATION
    APP_DATA_DIR="${APP_DATA_DIR}"
    PROJECT_ROOT="${PROJECT_SOURCE_DIR}"
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(${PROJECT_NAME} PRIVATE ENABLE_LOGGING)
endif()

target_include_directories(
    ${PROJECT_NAME}
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)

find_package(imageinfo CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE imageinfo::imageinfo)

file(COPY "assets/icon.ico" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}")