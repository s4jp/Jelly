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
#include "controlCube.h"
#include "restrainingCube.h"
#include "model.h"

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
ControlCube* controlCube;
bool showControlCube = true, showLinks = true;
RestrainingCube* restrainingCube;
bool showRestrainingCube = true;
static ControlledInputFloat mu("mu", 1.f, 0.01f, 0.f, 1.f);
static int reflectionMode = 0;
float lightPos[3] = { 10.f, 10.f, 10.f };
Model* model;
static int displayMode = 2;
bool interpolateNormals = false;

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

    Shader tessShaderProgram(
        "Shaders\\tessellation.vert", "Shaders\\phong.frag",
        "Shaders\\tessellation.tesc", "Shaders\\tessellation.tese");
    int tessViewLoc = glGetUniformLocation(tessShaderProgram.ID, "view");
    int tessProjLoc = glGetUniformLocation(tessShaderProgram.ID, "proj");
    int tessColorLoc = glGetUniformLocation(tessShaderProgram.ID, "color");
    int tessViewPosLoc = glGetUniformLocation(tessShaderProgram.ID, "viewPos");
	int tessLightPosLoc = glGetUniformLocation(tessShaderProgram.ID, "lightPos");

    Shader phongShader("Shaders\\phong.vert", "Shaders\\phong.frag");
    int phongViewLoc = glGetUniformLocation(phongShader.ID, "view");
    int phongProjLoc = glGetUniformLocation(phongShader.ID, "proj");
    int phongColorLoc = glGetUniformLocation(phongShader.ID, "color");
    int phongViewPosLoc = glGetUniformLocation(tessShaderProgram.ID, "viewPos");
    int phongLightPosLoc = glGetUniformLocation(tessShaderProgram.ID, "lightPos");
	int phongControlPointsLoc = glGetUniformLocation(phongShader.ID, "controlPoints");
    int phongInterpolateNormalsLoc = glGetUniformLocation(phongShader.ID, "interpolateNormals");
	int phongMinMaxLoc = glGetUniformLocation(phongShader.ID, "minMaxValues");

    // callbacks
    glfwSetWindowSizeCallback(window, window_size_callback);

    camera = new Camera(width, height, cameraPosition, fov, near, far, guiWidth);
    camera->PrepareMatrices(view, proj);

    mainCube = new Cube();
    controlCube = new ControlCube(mainCube->GetCorners());
	restrainingCube = new RestrainingCube();
	model = new Model("monkey.obj");

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
	memory = new SymMemory(dt.GetValue(), mass.GetValue(), c1.GetValue(), c2.GetValue(), k.GetValue(), controlCube->CalculateControlPoints(), mu.GetValue(), restrainingCube->GetControlPoints(), static_cast<bool>(reflectionMode), mainCube->GetControlPoints());
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
		controlCube->UpdateControlPoints(mainCube->GetCorners());
        
        // render objects
        shaderProgram.Activate();

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(proj));

        if (showWiremesh)
		    mainCube->RenderWireframe(colorLoc);
        if (showCps)
		    mainCube->RenderCps(colorLoc);
		if (showControlCube)
			controlCube->Render(colorLoc);
        if (showLinks)
			controlCube->RenderLinks(colorLoc);
		if (showRestrainingCube)
		    restrainingCube->Render(colorLoc);

        // render tess objects
		tessShaderProgram.Activate();

        glUniformMatrix4fv(tessViewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(tessProjLoc, 1, GL_FALSE, glm::value_ptr(proj));
        glUniform3fv(tessViewPosLoc, 1, glm::value_ptr(camera->Position));
		glUniform3fv(tessLightPosLoc, 1, lightPos);

		if (displayMode == 1) mainCube->Render(tessColorLoc);

		// render phong objects
		phongShader.Activate();

		glUniformMatrix4fv(phongViewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(phongProjLoc, 1, GL_FALSE, glm::value_ptr(proj));
        glUniform3fv(phongViewPosLoc, 1, glm::value_ptr(camera->Position));
		glUniform3fv(phongLightPosLoc, 1, lightPos);
		std::vector<glm::vec3> controlPoints = mainCube->GetControlPoints();
		glUniform3fv(phongControlPointsLoc, 64, &controlPoints[0].x);
		glUniform1i(phongInterpolateNormalsLoc, interpolateNormals);
		glUniform1fv(phongMinMaxLoc, 6, &model->minMaxValues[0]);

		if (displayMode == 2) model->Render(phongColorLoc);

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
		if (ImGui::Checkbox("Turn off c2", &c2off)) refreshParams();
		    ImGui::Spacing();
	    if (k.Render()) refreshParams();
		    ImGui::Spacing();
        if (dt.Render()) refreshParams();

		    ImGui::SeparatorText("Disturbance");
		disturbance.Render();
        if (ImGui::Button("Disturb")) memory->Distrupt(disturbance.GetValue());

		    ImGui::SeparatorText("Control cube");
		ImGui::Checkbox("Show control cube", &showControlCube);
		ImGui::Checkbox("Show links", &showLinks);

        if (ImGui::DragFloat3("position", controlCube->transation, 0.01f)) refreshParams();
		if (ImGui::DragFloat3("rotation", controlCube->rotation, 0.1f, -360.f, 360.f)) refreshParams();
		// no need to update control cube, it's updated in the main loop

		    ImGui::SeparatorText("Restraining cube");
		ImGui::Checkbox("Show restraining cube", &showRestrainingCube);
		if (mu.Render()) refreshParams();
		ImGui::Text("Reflection mode:");
		if (ImGui::RadioButton("One component", &reflectionMode, 0)) refreshParams();
            ImGui::SameLine();
		if (ImGui::RadioButton("Whole vector", &reflectionMode, 1)) refreshParams();

        ImGui::SeparatorText("Display");

        //ImGui::DragFloat3("light pos", lightPos, 0.01f);

        //ImGui::Text("Display mode:");
        ImGui::RadioButton("None", &displayMode, 0); ImGui::SameLine();
        ImGui::RadioButton("Patches", &displayMode, 1); ImGui::SameLine();
        ImGui::RadioButton("Model", &displayMode, 2);

        ImGui::Spacing();
        if (displayMode != 0)
            if (ImGui::DragFloat3("model color", model->color, 0.01f, 0.f, 1.f)) {
				mainCube->bezierColor[0] = model->color[0];
				mainCube->bezierColor[1] = model->color[1];
				mainCube->bezierColor[2] = model->color[2];
            }
        if (displayMode == 2)  
            ImGui::Checkbox("Interpolate normals", &interpolateNormals);

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
		memory->params.c2 = c2off ? 0.f : c2.GetValue();
	    memory->params.k = k.GetValue();
		memory->params.controlCube = controlCube->CalculateControlPoints();
		memory->params.mu = mu.GetValue();
		memory->params.wholeVectorReflection = static_cast<bool>(reflectionMode);
	memory->mutex.unlock();
}
