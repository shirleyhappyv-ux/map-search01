cmake_minimum_required(VERSION 3.10)
project(MapSearch)

set(CMAKE_CXX_STANDARD 17)

# 1. 查找依赖
find_package(nlohmann_json REQUIRED)
find_package(GDAL REQUIRED)

# --- 关键修改：添加这一行 ---
include_directories(${GDAL_INCLUDE_DIR})
# ---------------------------

# 2. 定义可执行文件
add_executable(TestSearch src/test_search.cpp src/MapSearch.cpp)

# 3. 链接依赖
target_link_libraries(TestSearch PRIVATE 
    nlohmann_json::nlohmann_json 
    ${GDAL_LIBRARY}
)