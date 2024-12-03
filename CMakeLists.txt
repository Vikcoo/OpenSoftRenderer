# CMakeList.txt: 顶层 CMake 项目文件，在此处执行全局配置
# 并包含子项目。
#
cmake_minimum_required (VERSION 3.8)

# 如果支持，请为 MSVC 编译器启用热重载。
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project (OpenSoftRenderer)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)

# 添加 include 文件夹到包含目录
include_directories("${CMAKE_SOURCE_DIR}/include")

# 添加 3rdparty SFML 到包含目录
include_directories("${CMAKE_SOURCE_DIR}/3rdparty/eigen-3.4.0")
# 添加 3rdparty EIGEN 到包含目录
include_directories("${CMAKE_SOURCE_DIR}/3rdparty/SFML-2.6.2/include")
# 添加 3rdparty tinyobjloader 到包含目录
include_directories("${CMAKE_SOURCE_DIR}/3rdparty/tinyobjloader-release")
# 添加 3rdparty glm 到包含目录
include_directories("${CMAKE_SOURCE_DIR}/3rdparty/glm")

# 添加 3rdparty SFML 到链接目录
link_directories("${CMAKE_SOURCE_DIR}/3rdparty/SFML-2.6.2/lib")

# 创建可执行文件
add_executable(OpenSoftRenderer "main.cpp" "src/Application.cpp" "include/Application.h" "include/Loader.h" "src/Loader.cpp" "include/Model.h" "src/Model.cpp" "include/World.h" "src/World.cpp" "include/Renderer.h" "src/Renderer.cpp" "include/Shader.h" "src/Shader.cpp" "include/Geometry.h" "src/Geometry.cpp" "include/Uniform.h" "src/Uniform.cpp" "include/Camera.h" "src/Camera.cpp"  )

# 根据构建模式选择链接 SFML 库
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    # Debug 模式下链接调试版本的库
    target_link_libraries(OpenSoftRenderer
        sfml-graphics-d
        sfml-window-d
        sfml-system-d
    )
else()
    # Release 模式下链接发布版本的库
    target_link_libraries(OpenSoftRenderer
        sfml-graphics
        sfml-window
        sfml-system
    )
endif()

# 将所有 SFML DLL 文件复制到构建目录
set(SFML_DLL_DIR "${CMAKE_SOURCE_DIR}/3rdparty/SFML-2.6.2/bin")
file(GLOB SFML_DLLS "${SFML_DLL_DIR}/*.dll")
foreach(DLL ${SFML_DLLS})
    add_custom_command(TARGET OpenSoftRenderer POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${DLL} $<TARGET_FILE_DIR:OpenSoftRenderer>
    )
endforeach()

# 模型文件路径作为一个定义传递给代码：
add_compile_definitions(OBJ_PATH="${CMAKE_SOURCE_DIR}/obj/")

# 纹理文件路径作为一个定义传递给代码：
add_compile_definitions(TEX_PATH="${CMAKE_SOURCE_DIR}/textures/")
