#define _USE_MATH_DEFINES
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_stdlib.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>
#include <vector>
#include <thread>

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "Shader.h"
#include "VAO.h"
#include "VBO.h"
#include "EBO.h"
#include "Camera.h"
#include "ControlledInputFloat.h"
#include "ControlledInputInt.h"

#include "cube.h"
#include "simulator.h"

const float near = 0.1f;
const float far = 100.0f;

Camera *camera;

glm::mat4 view;
glm::mat4 proj;

void window_size_callback(GLFWwindow *window, int width, int height);
void refreshParams();

int modelLoc, viewLoc, projLoc, colorLoc;

Cube* mainCube;
bool showWiremesh = true, showCps = true;
static ControlledInputFloat mass("Mass (per CP)", 1.f, 0.01f, 0.01f, 100.f);
static ControlledInputFloat c1("C1", 30.f, 0.01f, 0.01f, 100.f);
static ControlledInputFloat c2("C2", 35.f, 0.01f, 0.01f, 100.f);
static ControlledInputFloat k("Damping [k]", 5.f, 0.01f, 0.01f, 100.f);
bool c2off = false;
static ControlledInputFloat disturbance("Disturbance", 1.f, 0.01f, 0.f, 10.f);
static ControlledInputInt dt("dt (ms)", 10, 1, 1, 1000);

SymMemory* memory;
std::vector<glm::vec3> pos;
std::thread calcThread;

int main() { 
    // initial values
    int width = 1500;
    int height = 800;
    glm::vec3 cameraPosition = glm::vec3(3.0f, 3.0f, 3.0f);
    float fov = M_PI / 4.0f;
    int guiWidth = 300;

    #pragma region gl_boilerplate
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(width, height, "Dynamic deformations", NULL, NULL);
    if (window == NULL) {
      std::cout << "Failed to create GLFW window" << std::endl;
      glfwTerminate();
      return -1;
    }
    glfwMakeContextCurrent(window);

    gladLoadGL();
    glViewport(0, 0, width - guiWidth, height);
    glEnable(GL_DEPTH_TEST);

    GLFWimage icon;
    icon.pixels = stbi_load("icon.png", &icon.width, &icon.height, 0, 4);
    glfwSetWindowIcon(window, 1, &icon);
    stbi_image_free(icon.pixels);
    #pragma endregion

    // shaders and uniforms
    Shader shaderProgram("Shaders\\default.vert", "Shaders\\default.frag");
    //modelLoc = glGetUniformLocation(shaderProgram.ID, "model");
    viewLoc = glGetUniformLocation(shaderProgram.ID, "view");
    projLoc = glGetUniformLocation(shaderProgram.ID, "proj");
    colorLoc = glGetUniformLocation(shaderProgram.ID, "color");

    // callbacks
    glfwSetWindowSizeCallback(window, window_size_callback);

    camera = new Camera(width, height, cameraPosition, fov, near, far, guiWidth);
    camera->PrepareMatrices(view, proj);

    mainCube = new Cube();

    #pragma region imgui_boilerplate
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460");
    #pragma endregion

	// simulation
	memory = new SymMemory(dt.GetValue(), mass.GetValue(), c1.GetValue(), k.GetValue(), mainCube->GetControlPoints());
	pos = memory->data.positions;
	calcThread = std::thread(calculationThread, memory);

    while (!glfwWindowShouldClose(window)) 
    {
        #pragma region init
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::SetNextWindowSize(ImVec2(camera->guiWidth, camera->GetHeight()));
        ImGui::SetNextWindowPos(ImVec2(camera->GetWidth(), 0));
        #pragma endregion

		// camera inputs handling
        camera->HandleInputs(window);
        camera->PrepareMatrices(view, proj);

		// simulation
		memory->mutex.lock();
		pos = memory->data.positions;
		memory->mutex.unlock();
		mainCube->SetControlPoints(pos);
        
        // render objects
        shaderProgram.Activate();

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));

        if (showWiremesh)
		    mainCube->Render(colorLoc);
        if (showCps)
		    mainCube->RenderCps(colorLoc);

        // imgui rendering
        ImGui::Begin("Menu", 0,
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

        ImGui::Checkbox("Show springs", &showWiremesh);
        ImGui::Checkbox("Show control points", &showCps);

            ImGui::Separator();
        if (mass.Render()) refreshParams();
		    ImGui::Spacing();
		if (c1.Render()) refreshParams();
		if (c2.Render()) refreshParams();
		ImGui::Checkbox("Turn off c2", &c2off);
		    ImGui::Spacing();
	    if (k.Render()) refreshParams();
		    ImGui::Spacing();
        if (dt.Render()) refreshParams();

		    ImGui::SeparatorText("Disturbance");
		disturbance.Render();
        if (ImGui::Button("Disturb")) {
			memory->Distrupt(disturbance.GetValue());
        }

        ImGui::End();
        #pragma region rest
        ImGui::Render();
        //std::cout << ImGui::GetIO().Framerate << std::endl;
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
        #pragma endregion
    }
    #pragma region exit
	memory->stopThread = true;
	calcThread.join();
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    shaderProgram.Delete();
	mainCube->Delete();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
    #pragma endregion
}

// callbacks
void window_size_callback(GLFWwindow *window, int width, int height) {
  camera->SetWidth(width);
  camera->SetHeight(height);
  camera->PrepareMatrices(view, proj);
  glViewport(0, 0, width - camera->guiWidth, height);
}

void refreshParams()
{
	memory->mutex.lock();
	    memory->params.dt = dt.GetValue();
	    memory->params.mass = mass.GetValue();
	    memory->params.c1 = c1.GetValue();
	    memory->params.k = k.GetValue();
	memory->mutex.unlock();
}
