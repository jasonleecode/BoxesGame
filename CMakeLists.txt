cmake_minimum_required(VERSION 3.14)
project(BoxStackingGame)

set(CMAKE_CXX_STANDARD 17)

# 1. 帮助 CMake 找到 Homebrew 安装的库
if(APPLE)
    list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew")
endif()

# 2. 查找 SFML
# 注意：Homebrew 目前默认安装的是 SFML 2.6.1。
# 如果你没有手动编译安装 SFML 3.0，请去掉 "3.0" 版本号限制，否则配置会失败。
find_package(SFML COMPONENTS Graphics Window System REQUIRED)

# 3. 查找 Box2D
find_package(box2d REQUIRED)

add_executable(BoxGame main.cpp)

# 4. 链接库 - 关键修复！
# 使用 SFML::Graphics 而不是 sfml-graphics
target_link_libraries(BoxGame PRIVATE SFML::Graphics SFML::Window SFML::System)

# Box2D 链接
if(TARGET box2d::box2d)
    target_link_libraries(BoxGame PRIVATE box2d::box2d)
elseif(TARGET box2d)
    target_link_libraries(BoxGame PRIVATE box2d)
else()
    # 最后的手段，如果找不到 Target，才尝试手动指定库名
    target_link_libraries(BoxGame PRIVATE box2d)
endif()