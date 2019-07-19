#define GLFW_EXPOSE_NATIVE_WIN32

#include <iostream>
#include <memory>

#include <windows.h>
#include <glad\glad.h>
#include <GLFW\glfw3.h>
#include <GLFW\glfw3native.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "droptarget.h"
#include "shader.h"
#include "camera.h"
#include "texture.h"
#include "cubemap.h"
#include "skybox.h"
#include "sphere.h"
#include "quad.h"
#include "cubemapgenerator.h"

namespace MaterialMapPreview {
	enum Type { ALBEDO, NORMAL, METALLIC, ROUGHNESS, AO, DISPLACEMENT, NONE };
}

void glfwErrorCallback(int error, const char* description);
void processKeyboardInput(GLFWwindow *window, bool keyboardCaptured);
void processMouseInput(GLFWwindow *window, bool mouseCaptured);
void mouseCallback(GLFWwindow* window, double xpos, double ypos);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void fileDropCallback(const char* path);
void createFbo(int width, int height);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

// frameBuffer
unsigned int hdrFBO, colorBuffer, renderBuffer;

// Skybox
std::unique_ptr<Skybox> skybox = nullptr;
std::shared_ptr<CubeMap> environmentMap = nullptr;
std::shared_ptr<CubeMap> irradianceMap = nullptr;
std::shared_ptr<CubeMap> preFilterMap = nullptr;
std::shared_ptr<Texture> brdfLUT = nullptr;

// Geometry
std::unique_ptr<Sphere> sphere = nullptr;
std::unique_ptr<Sphere> light = nullptr;
std::unique_ptr<Quad> quad = nullptr;

// Material
std::shared_ptr<Texture> albedoMap = nullptr;
std::shared_ptr<Texture> normalMap = nullptr;
std::shared_ptr<Texture> metallicMap = nullptr;
std::shared_ptr<Texture> roughnessMap = nullptr;
std::shared_ptr<Texture> aoMap = nullptr;
std::shared_ptr<Texture> displacementMap = nullptr;

// Dear ImGui
int skyboxComboItem = 0;
int textureScale[2] = { 1, 1 };
float lightPos[3] = { 2.0, 0.0, 2.0 };
bool showAppControls = true;
bool showAppMaterial = true;
bool showAppStats = true;
bool vsyncEnabled = false;
bool rotationEnabled = false;
bool wireframeEnabled = false;
bool lightEnabled = false;
float displacementAmount = 0.05f;
MaterialMapPreview::Type hoveredPreviewItem = MaterialMapPreview::NONE;

int main()
{
	const char* glslVersion = "#version 330 core";

	// glfw: initialize and configure
	glfwSetErrorCallback(glfwErrorCallback);
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// glfw window creation
	OleInitialize(NULL);
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Demo", NULL, NULL);
	HWND hwnd = glfwGetWin32Window(window);
	DropTarget dropTarget(fileDropCallback);
	DragAcceptFiles(hwnd, FALSE); // Use custom dropTarget instead
	RegisterDragDrop(hwnd, &dropTarget);

	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
	glfwSetCursorPosCallback(window, mouseCallback);
	glfwSetScrollCallback(window, scrollCallback);

	// glad: load all OpenGL function pointers
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.IniFilename = NULL; // Disable ini file

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer bindings
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glslVersion);

	Shader shaderSingleColor("shaders/shadersinglecolor.vs", "shaders/shadersinglecolor.fs");
	Shader shaderWireframe("shaders/shaderwireframe.vs", "shaders/shaderwireframe.fs");
	Shader shaderScreen("shaders/shaderscreen.vs", "shaders/shaderscreen.fs");
	Shader shaderSkybox("shaders/shaderskybox.vs", "shaders/shaderskybox.fs");
	Shader shaderPBR("shaders/shaderpbr.vs", "shaders/shaderpbr.fs");


	// Initialize geometry
	sphere = std::make_unique<Sphere>();
	light = std::make_unique<Sphere>();
	quad = std::make_unique<Quad>();

	// load and create textures 
	// -------------------------

	albedoMap = std::make_shared<Texture>("textures/albedo.png", true);
	normalMap = std::make_shared<Texture>("textures/normal.png");
	metallicMap = std::make_shared<Texture>("textures/metal.png");
	roughnessMap = std::make_shared<Texture>("textures/rough.png");
	aoMap = std::make_shared<Texture>("textures/ao.png");
	displacementMap = std::make_shared<Texture>("textures/height.png");

	{
		CubeMapGenerator cubeMapGenerator;
		environmentMap = cubeMapGenerator.generateEnvironmentMap("textures/default_env.hdr");
		irradianceMap = cubeMapGenerator.generateIrradianceMap(environmentMap);
		preFilterMap = cubeMapGenerator.generatePreFilterMap(environmentMap);
		brdfLUT = cubeMapGenerator.generateBrdfLUT();
	}

	sphere->setAlbedoMap(albedoMap);
	sphere->setNormalMap(normalMap);
	sphere->setMetallicMap(metallicMap);
	sphere->setRoughnessMap(roughnessMap);
	sphere->setAoMap(aoMap);
	sphere->setDisplacementMap(displacementMap);
	sphere->setTextureScale(1, 1);

	sphere->setIrradianceMap(irradianceMap);
	sphere->setPreFilterMap(preFilterMap);
	sphere->setBrdfLUT(brdfLUT);

	skybox = std::make_unique<Skybox>();
	skybox->setEnvironmentMap(environmentMap);

	float rotationAngle = 0;

	// Default shader values
	shaderScreen.use();
	shaderScreen.setInt("screenTexture", 0);

	createFbo(SCR_WIDTH, SCR_HEIGHT);

	glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	// Render loop

	while (!glfwWindowShouldClose(window))
	{
		// Poll events
		glfwPollEvents();

		// Per-frame time logic
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// Menu bar
		if (ImGui::BeginMainMenuBar()) {
			if (ImGui::BeginMenu("Menu")) {
				ImGui::MenuItem("Controls", NULL, &showAppControls);
				ImGui::MenuItem("Material", NULL, &showAppMaterial);
				ImGui::MenuItem("Stats", NULL, &showAppStats);

				if (ImGui::MenuItem("Quit", "Alt+F4")) {
					glfwSetWindowShouldClose(window, true);
				}

				ImGui::EndMenu();
			}

			ImGui::EndMainMenuBar();
		}

		hoveredPreviewItem = MaterialMapPreview::NONE;

		// Material window
		if (showAppMaterial) {
			ImGui::SetNextWindowSize(ImVec2(670, 158), ImGuiCond_FirstUseEver);
			ImGui::SetNextWindowPos(ImVec2(10, 28), ImGuiCond_FirstUseEver);
			ImGui::Begin("Material", &showAppMaterial, ImGuiWindowFlags_NoResize);

			ImGui::BeginGroup();
			ImGui::Text("    Albedo");
			ImGui::Image((ImTextureID)albedoMap->getId(), ImVec2(100, 100), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1.0f, 1.0f, 1.0f, 1.0f), ImVec4(1.0f, 1.0f, 1.0f, 0.5f));

			if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem | ImGuiHoveredFlags_AllowWhenOverlapped) && dropTarget.AcceptFormat()) {
				ImRect r(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
				r.Expand(3.5f);
				ImGui::GetWindowDrawList()->AddRect(r.Min, r.Max, IM_COL32(255, 255, 0, 255), 0.0f, ~0, 2.0f);
				hoveredPreviewItem = MaterialMapPreview::ALBEDO;
			}

			ImGui::EndGroup();

			ImGui::SameLine();
			ImGui::BeginGroup();
			ImGui::Text("    Normal");
			ImGui::Image((ImTextureID)normalMap->getId(), ImVec2(100, 100), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1.0f, 1.0f, 1.0f, 1.0f), ImVec4(1.0f, 1.0f, 1.0f, 0.5f));

			if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem | ImGuiHoveredFlags_AllowWhenOverlapped) && dropTarget.AcceptFormat()) {
				ImRect r(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
				r.Expand(3.5f);
				ImGui::GetWindowDrawList()->AddRect(r.Min, r.Max, IM_COL32(255, 255, 0, 255), 0.0f, ~0, 2.0f);
				hoveredPreviewItem = MaterialMapPreview::NORMAL;
			}

			ImGui::EndGroup();

			ImGui::SameLine();
			ImGui::BeginGroup();
			ImGui::Text("   Metallic");
			ImGui::Image((ImTextureID)metallicMap->getId(), ImVec2(100, 100), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1.0f, 1.0f, 1.0f, 1.0f), ImVec4(1.0f, 1.0f, 1.0f, 0.5f));

			if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem | ImGuiHoveredFlags_AllowWhenOverlapped) && dropTarget.AcceptFormat()) {
				ImRect r(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
				r.Expand(3.5f);
				ImGui::GetWindowDrawList()->AddRect(r.Min, r.Max, IM_COL32(255, 255, 0, 255), 0.0f, ~0, 2.0f);
				hoveredPreviewItem = MaterialMapPreview::METALLIC;
			}

			ImGui::EndGroup();

			ImGui::SameLine();
			ImGui::BeginGroup();
			ImGui::Text("   Roughness");
			ImGui::Image((ImTextureID)roughnessMap->getId(), ImVec2(100, 100), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1.0f, 1.0f, 1.0f, 1.0f), ImVec4(1.0f, 1.0f, 1.0f, 0.5f));

			if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem | ImGuiHoveredFlags_AllowWhenOverlapped) && dropTarget.AcceptFormat()) {
				ImRect r(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
				r.Expand(3.5f);
				ImGui::GetWindowDrawList()->AddRect(r.Min, r.Max, IM_COL32(255, 255, 0, 255), 0.0f, ~0, 2.0f);
				hoveredPreviewItem = MaterialMapPreview::ROUGHNESS;
			}

			ImGui::EndGroup();

			ImGui::SameLine();
			ImGui::BeginGroup();
			ImGui::Text("      AO");
			ImGui::Image((ImTextureID)aoMap->getId(), ImVec2(100, 100), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1.0f, 1.0f, 1.0f, 1.0f), ImVec4(1.0f, 1.0f, 1.0f, 0.5f));

			if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem | ImGuiHoveredFlags_AllowWhenOverlapped) && dropTarget.AcceptFormat()) {
				ImRect r(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
				r.Expand(3.5f);
				ImGui::GetWindowDrawList()->AddRect(r.Min, r.Max, IM_COL32(255, 255, 0, 255), 0.0f, ~0, 2.0f);
				hoveredPreviewItem = MaterialMapPreview::AO;
			}

			ImGui::EndGroup();

			ImGui::SameLine();
			ImGui::BeginGroup();
			ImGui::Text(" Displacement");
			ImGui::Image((ImTextureID)displacementMap->getId(), ImVec2(100, 100), ImVec2(0, 0), ImVec2(1, 1), ImVec4(1.0f, 1.0f, 1.0f, 1.0f), ImVec4(1.0f, 1.0f, 1.0f, 0.5f));

			if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem | ImGuiHoveredFlags_AllowWhenOverlapped) && dropTarget.AcceptFormat()) {
				ImRect r(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
				r.Expand(3.5f);
				ImGui::GetWindowDrawList()->AddRect(r.Min, r.Max, IM_COL32(255, 255, 0, 255), 0.0f, ~0, 2.0f);
				hoveredPreviewItem = MaterialMapPreview::DISPLACEMENT;
			}

			ImGui::EndGroup();

			ImGui::End();
		}

		// Controls window
		if (showAppControls) {
			ImGui::SetNextWindowSize(ImVec2(250, 240), ImGuiCond_FirstUseEver);
			ImGui::SetNextWindowPos(ImVec2(10, 195), ImGuiCond_FirstUseEver);
			ImGui::Begin("Controls", &showAppControls, ImGuiWindowFlags_NoResize);
			ImGui::SetNextItemWidth(120);

			if (ImGui::Checkbox("Vsync", &vsyncEnabled)) {
				glfwSwapInterval(vsyncEnabled);
			}

			if (ImGui::Combo("Skybox", &skyboxComboItem, "Environment\0Irradiance\0\0")) {
				if (skyboxComboItem == 0) {
					skybox->setEnvironmentMap(environmentMap);
				}
				else if (skyboxComboItem == 1) {
					skybox->setEnvironmentMap(irradianceMap);
				}
			}

			ImGui::SetNextItemWidth(120);
			ImGui::DragFloat("Displacement", (float*)&displacementAmount, 0.001f, 0.0f, 0.1f);
			ImGui::SetNextItemWidth(120);

			if (ImGui::SliderInt2("Texture scale", textureScale, 1, 5)) {
				sphere->setTextureScale(static_cast<float>(textureScale[0]), static_cast<float>(textureScale[1]));
			}

			if (ImGui::Button("Reset material to default")) {
				albedoMap = std::make_shared<Texture>("textures/albedo.png", true);
				normalMap = std::make_shared<Texture>("textures/normal.png");
				metallicMap = std::make_shared<Texture>("textures/metal.png");
				roughnessMap = std::make_shared<Texture>("textures/rough.png");
				aoMap = std::make_shared<Texture>("textures/ao.png");
				displacementMap = std::make_shared<Texture>("textures/height.png");

				sphere->setAlbedoMap(albedoMap);
				sphere->setNormalMap(normalMap);
				sphere->setMetallicMap(metallicMap);
				sphere->setRoughnessMap(roughnessMap);
				sphere->setAoMap(aoMap);
				sphere->setDisplacementMap(displacementMap);
			}

			ImGui::Checkbox("Rotation", &rotationEnabled);
			ImGui::Checkbox("Wireframe", &wireframeEnabled);
			ImGui::Checkbox("Light", &lightEnabled);
			ImGui::SetNextItemWidth(150);

			if (lightEnabled) {
				ImGui::DragFloat3("Light pos", lightPos, 0.05f, -3.0f, 3.0f);
			}

			ImGui::End();
		}

		// Stats overlay
		if (showAppStats) {
			ImVec2 window_pos = ImVec2(3.0f, io.DisplaySize.y - 3.0f);
			ImVec2 window_pos_pivot = ImVec2(0.0f, 1.0f);
			ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
			if (ImGui::Begin("Example: Simple overlay", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoInputs))
			{
				ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
				ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);

				if (ImGui::IsMousePosValid()) {
					ImGui::Text("Mouse Position: (%.1f,%.1f)", io.MousePos.x, io.MousePos.y);
				}
				else {
					ImGui::Text("Mouse Position: <invalid>");
				}
			}
			ImGui::End();
		}

		ImGui::Render();

		// Inputs
		processKeyboardInput(window, !io.WantCaptureKeyboard);
		processMouseInput(window, !io.WantCaptureMouse);

		// Render commands
		// bind to framebuffer and draw scene as we normally would to color texture 
		glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Render scene
		// ------------

		// camera/view transformation
		glm::mat4 view = camera.getViewMatrix();

		// projection matrix
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);
		glm::mat4 projection;

		if (width > 0 && height > 0) {
			projection = glm::perspective(glm::radians(camera.mZoom), (float)width / (float)height, 0.1f, 100.0f);
		}

		// Render light
		if (lightEnabled) {
			shaderSingleColor.use();
			shaderSingleColor.setMat4("view", view);
			shaderSingleColor.setMat4("projection", projection);
			shaderSingleColor.setVec3("color", 100.0, 100.0, 100.0);

			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, glm::vec3(lightPos[0], lightPos[1], lightPos[2]));
			model = glm::scale(model, glm::vec3(0.25f, 0.25f, 0.25f));
			shaderSingleColor.setMat4("model", model);
			light->draw(shaderSingleColor);
		}

		// Render object
		shaderPBR.use();
		shaderPBR.setMat4("view", view);
		shaderPBR.setMat4("projection", projection);
		shaderPBR.setVec3("eyePos", camera.mPosition);

		glm::mat4 model = glm::mat4(1.0f);

		if (rotationEnabled) {
			rotationAngle += (deltaTime * 0.5f);
		}

		model = glm::rotate(model, rotationAngle, glm::vec3(0.0, 1.0, 0.0));
		shaderPBR.setMat4("model", model);
		glm::mat3 normalMat = glm::mat3(glm::transpose(glm::inverse(model)));
		shaderPBR.setMat3("normalMat", normalMat);
		shaderPBR.setFloat("displacementAmount", displacementAmount);
		shaderPBR.setBool("lightEnabled", lightEnabled);
		shaderPBR.setVec3("lightPos", lightPos[0], lightPos[1], lightPos[2]);
		sphere->draw(shaderPBR);

		if (wireframeEnabled) {
			// Render Wireframe
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glEnable(GL_POLYGON_OFFSET_FILL);
			glPolygonOffset(-1, -1);
			shaderWireframe.use();
			shaderWireframe.setMat4("model", model);
			shaderWireframe.setMat4("view", view);
			shaderWireframe.setMat4("projection", projection);
			shaderWireframe.setVec3("color", 0.3f, 1.0f, 0.5f);
			shaderWireframe.setFloat("displacementAmount", displacementAmount);
			sphere->draw(shaderWireframe);
			glDisable(GL_POLYGON_OFFSET_FILL);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		// Render skybox
		// -------------
		shaderSkybox.use();
		view = glm::mat4(glm::mat3(camera.getViewMatrix()));
		shaderSkybox.setMat4("view", view);
		shaderSkybox.setMat4("projection", projection);
		skybox->draw(shaderSkybox);

		// Render quad with scene's visuals as its texture image
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glDisable(GL_DEPTH_TEST);

		// Draw Screen quad
		shaderScreen.use();
		shaderScreen.setVec2("screenSize", glm::vec2(width, height));
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, colorBuffer);
		quad->draw(shaderScreen);
		//glDrawArrays(GL_TRIANGLES, 0, 6);
		glEnable(GL_DEPTH_TEST);

		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Swap buffers
		glfwSwapBuffers(window);
	}

	// Cleanup
	RevokeDragDrop(hwnd);
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}

void glfwErrorCallback(int error, const char* description)
{
	fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

void processKeyboardInput(GLFWwindow *window, bool keyboardCaptured)
{
	// ...
}

void processMouseInput(GLFWwindow *window, bool mouseCaptured)
{
	bool button_left_pressed = (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) && mouseCaptured;
	bool button_right_pressed = (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) && mouseCaptured;

	camera.processMousePress(button_left_pressed, button_right_pressed);
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = static_cast<float>(xpos);
		lastY = static_cast<float>(ypos);
		firstMouse = false;
	}

	float xoffset = static_cast<float>(xpos) - lastX;
	float yoffset = lastY - static_cast<float>(ypos); // reversed since y-coordinates go from bottom to top

	lastX = static_cast<float>(xpos);
	lastY = static_cast<float>(ypos);

	camera.processMouseMovement(xoffset, yoffset);
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	if (!io.WantCaptureMouse) {
		camera.processMouseScroll(static_cast<float>(yoffset));
	}
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	createFbo(width, height);
}

void fileDropCallback(const char* path)
{
	switch (hoveredPreviewItem) {
	case MaterialMapPreview::ALBEDO:
		albedoMap = std::make_shared<Texture>(path, true);
		sphere->setAlbedoMap(albedoMap);
		break;
	case MaterialMapPreview::NORMAL:
		normalMap = std::make_shared<Texture>(path);
		sphere->setNormalMap(normalMap);
		break;
	case MaterialMapPreview::METALLIC:
		metallicMap = std::make_shared<Texture>(path);
		sphere->setMetallicMap(metallicMap);
		break;
	case MaterialMapPreview::ROUGHNESS:
		roughnessMap = std::make_shared<Texture>(path);
		sphere->setRoughnessMap(roughnessMap);
		break;
	case MaterialMapPreview::AO:
		aoMap = std::make_shared<Texture>(path);
		sphere->setAoMap(aoMap);
		break;
	case MaterialMapPreview::DISPLACEMENT:
		displacementMap = std::make_shared<Texture>(path);
		sphere->setDisplacementMap(displacementMap);
		break;
	default:
		// Load new cube maps
		CubeMapGenerator generator;
		environmentMap = generator.generateEnvironmentMap(path);
		irradianceMap = generator.generateIrradianceMap(environmentMap);
		preFilterMap = generator.generatePreFilterMap(environmentMap);
		
		if (skyboxComboItem == 0) {
			skybox->setEnvironmentMap(environmentMap);
		}
		else if (skyboxComboItem == 1) {
			skybox->setEnvironmentMap(irradianceMap);
		}
		
		sphere->setIrradianceMap(irradianceMap);
		sphere->setPreFilterMap(preFilterMap);
	}
}

void createFbo(int width, int height)
{
	glDeleteTextures(1, &colorBuffer);
	glDeleteRenderbuffers(1, &renderBuffer);

	glDeleteFramebuffers(1, &hdrFBO);

	// HDR framebuffer configuration
	// --------------------------
	glGenFramebuffers(1, &hdrFBO);
	// create a color attachment texture
	glGenTextures(1, &colorBuffer);
	glBindTexture(GL_TEXTURE_2D, colorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	// create a renderbuffer object for depth and stencil attachment
	glGenRenderbuffers(1, &renderBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height); // use a single renderbuffer object for both a depth AND stencil buffer.
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	// Attach buffers
	glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBuffer, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderBuffer); // now actually attach it
	// now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}