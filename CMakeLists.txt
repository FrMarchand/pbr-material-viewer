cmake_minimum_required (VERSION 3.0)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

#--------------------------------------------------------------------
# Resources
#--------------------------------------------------------------------

file(GLOB_RECURSE SHADER_SOURCE_FILES 
	${CMAKE_SOURCE_DIR}/resources/shaders/*.vs
	${CMAKE_SOURCE_DIR}/resources/shaders/*.fs
)

include (cmake/CMakeRC.cmake)
cmrc_add_resource_library(
	resource-files
	NAMESPACE resources
	WHENCE resources
	${SHADER_SOURCE_FILES}
)

cmrc_add_resources(
	resource-files
	WHENCE resources
	resources/textures/albedo.png
	resources/textures/ao.png
	resources/textures/height.png
	resources/textures/metal.png
	resources/textures/normal.png
	resources/textures/rough.png
	resources/textures/default_env.hdr
)

project(pbr_demo)

file(GLOB_RECURSE SOURCE_FILES 
	${CMAKE_SOURCE_DIR}/src/*.c
	${CMAKE_SOURCE_DIR}/src/*.cpp
)

file(GLOB_RECURSE HEADER_FILES 
	${CMAKE_SOURCE_DIR}/include/*.h
	${CMAKE_SOURCE_DIR}/include/*.hpp
)

add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES} ${SHADER_SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} PRIVATE resource-files)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/include)

#--------------------------------------------------------------------
# OpenGL
#--------------------------------------------------------------------
find_package(OpenGL REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE opengl32)

#--------------------------------------------------------------------
# GLM (header-only)
#--------------------------------------------------------------------
find_package(GLM REQUIRED)
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/thirdparty/glm/include")

#--------------------------------------------------------------------
# GLFW (pre-compiled)
#--------------------------------------------------------------------
find_package(GLFW3 REQUIRED)
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/thirdparty/glfw/include")
target_link_libraries(${PROJECT_NAME} PRIVATE ${GLFW3_LIBRARY})

#--------------------------------------------------------------------
# STB_IMAGE
#--------------------------------------------------------------------
add_library(stb_image "thirdparty/stb/src/stb_image.cpp")
target_include_directories(stb_image PUBLIC "${CMAKE_SOURCE_DIR}/thirdparty/stb/include")
target_link_libraries(${PROJECT_NAME} PRIVATE stb_image)

#--------------------------------------------------------------------
# GLAD
#--------------------------------------------------------------------
add_library(glad "thirdparty/glad/src/glad.c")
target_include_directories(glad PUBLIC "${CMAKE_SOURCE_DIR}/thirdparty/glad/include")
target_link_libraries(${PROJECT_NAME} PRIVATE glad)

#--------------------------------------------------------------------
# IMGUI
#--------------------------------------------------------------------
add_library(imgui 
	"thirdparty/imgui/src/imgui.cpp"
	"thirdparty/imgui/src/imgui_demo.cpp"
	"thirdparty/imgui/src/imgui_draw.cpp"
	"thirdparty/imgui/src/imgui_widgets.cpp"
)
target_include_directories(imgui PUBLIC "${CMAKE_SOURCE_DIR}/thirdparty/imgui/include")
target_link_libraries(${PROJECT_NAME} PRIVATE imgui)

#--------------------------------------------------------------------
# Hide the console window in visual studio projects
#--------------------------------------------------------------------
#if(MSVC)
#	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
#endif()

#--------------------------------------------------------------------
# Hide the console window in visual studio projects - Release
#--------------------------------------------------------------------
if(MSVC)
	set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS} /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")
endif()
