/*
 ==================================================================================
   TODO: DAY 05 - PHYSICS INTEGRATION & INTERACTION SYSTEM
   Goal: Connect the Physics Backend to the Rendering Frontend + Add Gameplay.
 ==================================================================================

 1. PHYSICS INTEGRATION (The "Body" & "Eyes" Merge)
	[ ] Remove "Spectator Mode" WASD logic (direct position modification).
	[ ] Hook Input: Map WASD to 'Player.m_ObjVelocity.x/z'.
	[ ] Hook Camera: In the render loop, set 'Camera.Position = Player.m_ObjPos'.
	[ ] Enable Gravity: Ensure 'PhysicsSystem::Update' is called every frame.
	[ ] Implement Jumping: Map SPACE to Velocity.y (Check 'IsGrounded' flag).

 2. RAYCASTING (Block Selection)
	[ ] Implement DDA Algorithm (Digital Differential Analyzer) for fast traversal.
	[ ] Create 'Raycast(origin, direction, range)' function.
	[ ] Visual Debug: Draw a wireframe cube around the "Targeted Voxel".

 3. VOXEL INTERACTION (The "Minecraft" Mechanics)
	[ ] Left Click: Get Target Voxel -> Set ID = 0 (Air) -> Trigger Mesh Rebuild.
	[ ] Right Click: Get "Previous" Voxel (Face Normal) -> Set ID = 1 -> Trigger Mesh Rebuild.
	[ ] Optimization: Only rebuild the specific Chunk that was modified.

 4. POLISH & MODES
	[ ] Add "Toggle Mode" (F1): Switch between "Physics Walk" and "Free Cam Fly".
	[ ] Verify "Sliding": Ensure player slides against walls (X/Z separation check).

 ==================================================================================
*/
#include <iostream>
#include <vector>
#include "PhysicsSystem.h"
#include "core/AABB.h"
#define BENCHMARK 0

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <core/MathUtils.h>
#include <core/Matrix.h>
#include <core/camera.h>

#include <renderer/Shader.h>
#include <renderer/Buffer.h>
#include <renderer/VertexArray.h>
#include <renderer/Texture.h>

#include <world/Chunk.h>

const unsigned int SCREEN_WIDTH = 1280;
const unsigned int SCREEN_HEIGHT = 720;
Core::Vec3 cameraPos = Core::Vec3(100.0f, 40.0f, 140.0f);
Core::Camera camera(cameraPos);
float lastX = SCREEN_WIDTH / 2.0f;
float lastY = SCREEN_HEIGHT / 2.0f;
bool bFirstMouse = true, bEnableFaceCulling = false, 
bPerspective = true;
float fOrthoSize = 10.0f;
float fDeltaTime = 0.0f;
float fLastFrame = 0.0f;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void mouseCallBack(GLFWwindow* pWindow, double xPosIn, double yPosIn) {
	bool bIsLeftDown = glfwGetMouseButton(pWindow, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
	bool bIsRightDown = glfwGetMouseButton(pWindow, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
	if (!bIsLeftDown && !bIsRightDown)
	{
		bFirstMouse = true;
		return;
	}
	float xPos = static_cast<float>(xPosIn);
	float yPos = static_cast<float>(yPosIn);

	if (bFirstMouse)
	{
		lastX = xPos;
		lastY = yPos;
		bFirstMouse = false;
	}

	float xOffset = xPos - lastX;
	float yOffset = lastY - yPos; //Inverse Direction of rotate bottom to top

	lastX = xPos;
	lastY = yPos;
	if(bIsLeftDown)
		camera.processMouseMovement(xOffset, yOffset);
	else if(bIsRightDown)
		camera.processMousePan(xOffset, yOffset);
}

void processInput(GLFWwindow* pWindow) {
	if (glfwGetKey(pWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(pWindow, true);
	if (bPerspective) {
		if (glfwGetKey(pWindow, GLFW_KEY_W) == GLFW_PRESS)
			camera.processKeyboard(0, fDeltaTime);
		if (glfwGetKey(pWindow, GLFW_KEY_S) == GLFW_PRESS)
			camera.processKeyboard(1, fDeltaTime);
		if (glfwGetKey(pWindow, GLFW_KEY_SPACE) == GLFW_PRESS)
			camera.processKeyboard(4, fDeltaTime);
		if (glfwGetKey(pWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
			camera.processKeyboard(5, fDeltaTime);
	}
	else {
		if (glfwGetKey(pWindow, GLFW_KEY_W) == GLFW_PRESS)
			camera.processKeyboard(4, fDeltaTime);
		if (glfwGetKey(pWindow, GLFW_KEY_S) == GLFW_PRESS)
			camera.processKeyboard(5, fDeltaTime);
		float fZoomSpeed = 10.0f * fDeltaTime;
		if (glfwGetKey(pWindow, GLFW_KEY_SPACE) == GLFW_PRESS) {
			fOrthoSize -= fZoomSpeed; //Zoom In
			if(fOrthoSize < 1.0f)
				fOrthoSize = 1.0f;
		}
		if (glfwGetKey(pWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
			fOrthoSize += fZoomSpeed; //Zoom Out
			if (fOrthoSize > 40.0f)
				fOrthoSize = 40.0f;
		}
	}
	
	if (glfwGetKey(pWindow, GLFW_KEY_A) == GLFW_PRESS)
		camera.processKeyboard(2, fDeltaTime);
	if (glfwGetKey(pWindow, GLFW_KEY_D) == GLFW_PRESS)
		camera.processKeyboard(3, fDeltaTime);
	if (glfwGetKey(pWindow, GLFW_KEY_R) == GLFW_PRESS) //Resets cameta position
	{
		camera.position = cameraPos;
		camera.yaw = -90.0f;
		camera.pitch = -30.0f;
		camera.zoom = 45.0f;
		camera.front = Core::Vec3(0.0f, 0.0f, -1.0f);
		camera.updateCameraVectors();
	}
}

void scroll_callback(GLFWwindow* pWindow, double dXOffset, double dYOffset) {
	if(bPerspective)
	camera.processMouseScroll(static_cast<float> (dYOffset));
	else
	{
		fOrthoSize -= static_cast<float>(dYOffset);
		if (fOrthoSize < 1.0f)
			fOrthoSize = 1.0f;
		if (fOrthoSize > 40.0f)
			fOrthoSize = 40.0f;
	}
}

void updateTitleInfo(GLFWwindow *pWindow, int iFrameCount, float fTimer) {
	if (!pWindow)
		return;
	int iFPS = static_cast<int>(iFrameCount / fTimer);
	std::string strTitle = "HPC Voxel Engine FPS:" + std::to_string(iFPS);
	if (bEnableFaceCulling)
		strTitle += "\tFace Culling Enabled (Press 'F' key to toogle)";
	else
		strTitle += "\tFace Culling Disabled (Press 'F' key to toogle)";
		
	if (bPerspective)
		strTitle += "\tPerspective Projection (Press 'P' key to toggle)";
	else
		strTitle += "\tOrthographic Projection (Press 'P' key to toggle)";
	glfwSetWindowTitle(pWindow, strTitle.c_str());
	}

int main() {
#if DEBUG
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_DEPTH_BITS, 24);
	GLFWwindow* pWindow = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "HPC Voxel Engine", nullptr, nullptr);
	if (pWindow == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(pWindow);
#if BENCHMARK
	glfwSwapInterval(0); // 0 = Unlock FPS (VSync OFF), 1 = Lock to 60 (VSync ON)
#endif
	glfwSetFramebufferSizeCallback(pWindow, framebuffer_size_callback);
	glfwSetCursorPosCallback(pWindow, mouseCallBack);
	glfwSetScrollCallback(pWindow, scroll_callback);

	//glfwSetInputMode(pWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetInputMode(pWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	const GLubyte* vendor = glGetString(GL_VENDOR);
	const GLubyte* renderor = glGetString(GL_RENDERER);
	std::cout << "GPU Renderer: " << vendor << std::endl;
	std::cout << "Renderer: " << renderor << std::endl;

	Renderer::Shader shader("../assets/shaders/vertex.glsl", "../assets/shaders/fragment.glsl"); 
	std::vector<Chunk> chunks;
	int iRenderDistance = 8;
	for (int iX = -iRenderDistance; iX < iRenderDistance; iX++)
	{
		for (int iZ = -iRenderDistance; iZ < iRenderDistance; iZ++)
		{
			chunks.emplace_back(iX, iZ);
		}
	}

	//Renderer::Texture texture("../assets/textures/container.jpg");
	Renderer::Texture texture("../assets/textures/texture_atlas.png");
	texture.bind(0);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	static bool sbFPressedLastTime = false, sbPPressedLastTime = false;
	static int iFrameCount = 0;
	static float fTimer = 0.0f;
	std::string strFaceCullMsg = "";
	fLastFrame = static_cast<float>(glfwGetTime());
	while (!glfwWindowShouldClose(pWindow))
	{
		glClearColor(0.2f, 0.3f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		float fCurrentFrame = static_cast<float>(glfwGetTime());
		fDeltaTime = fCurrentFrame - fLastFrame;
		fLastFrame = fCurrentFrame;

		//For FPS Counter
		iFrameCount++;
		fTimer += fDeltaTime;

		if (glfwGetKey(pWindow, GLFW_KEY_F) == GLFW_PRESS)
		{
			if(!sbFPressedLastTime)
			{
				bEnableFaceCulling = !bEnableFaceCulling;
				sbFPressedLastTime = true;
				updateTitleInfo(pWindow, iFrameCount, fTimer);
			}
		}
		else
			sbFPressedLastTime = false;

		if (glfwGetKey(pWindow, GLFW_KEY_P) == GLFW_PRESS)
		{
			if(!sbPPressedLastTime)
			{
				bPerspective = !bPerspective;
				sbPPressedLastTime = true;
				updateTitleInfo(pWindow, iFrameCount, fTimer);
			}
		}
		else
			sbPPressedLastTime = false;
		if (fTimer >= 1.0f)
		{
			updateTitleInfo(pWindow, iFrameCount, fTimer);
			iFrameCount = 0;
			fTimer = 0.0f;
		}

		processInput(pWindow);

		shader.use();

		Core::Mat4 projection;
		float fAspectRatio = static_cast<float>(SCREEN_WIDTH) / static_cast<float>(SCREEN_HEIGHT);
		if(bPerspective)
		{
			projection = Core::Mat4::perspective(camera.getZoom(),
			fAspectRatio, 0.1f, 100.0f);
		}
		else
		{
			float fHeight = fOrthoSize;
			float fWidth = fOrthoSize * fAspectRatio;
			projection = Core::Mat4::orthographic(
				-fWidth, fWidth, -fHeight, fHeight, -100.0f, 100.0f);
		}
		Core::Mat4 view = camera.getViewMatrix();
		Core::Mat4 viewProjection = projection * view;

		shader.setMat4("uViewProjection", viewProjection);
		for (auto& chunk : chunks)
		{
			chunk.render();
		}
		glfwSwapBuffers(pWindow);
		glfwPollEvents();
	}
	glfwTerminate();
#endif
	PhysicsSystem physicsSystem;
	std::vector<AABB> objStaticWorld;
	objStaticWorld.emplace_back(
		Core::Vec3(-10.0f, -2.0f, -10.0f),
		Core::Vec3(10.0f, -1.0f, 10.0f));

	RigidBody playerBody;
	playerBody.m_ObjPos = Core::Vec3(0.0f, 5.0f, 0.0f);
	playerBody.m_ObjVelocity = Core::Vec3(0.0f, 0.0f, 0.0f);
	playerBody.m_ObjSize = Core::Vec3(0.5f, 0.5f, 0.5f);

	std::cout << "--- STARTING PHYSICS SIMULATION ---\n";
	std::cout << "Target: Player should land at Y = -0.5 (Center of 1.0 unit box)\n\n";

	float fDelTime = 1.0f / 60.0f;
	for (size_t iFrame = 0; iFrame < 100.0f; iFrame++)
	{
		physicsSystem.Update(playerBody, fDelTime, objStaticWorld);
		if (iFrame % 5)
		{
			std::cout << "Frame" << iFrame << " | " << "Pos Y" << playerBody.m_ObjPos.y;
			std::cout << " | Vel Y" << playerBody.m_ObjVelocity.y << "\n";
		}
	}

	std::cout << "\n--- END SIMULATION ---\n";
	if (playerBody.m_ObjPos.y >= -0.51f && playerBody.m_ObjPos.y <= -0.49f) {
		std::cout << "✅ SUCCESS: Player landed correctly on the voxel surface.\n";
	}
	else {
		std::cout << "❌ FAILURE: Player ignored collision or stopped wrong.\n";
	}

	return 0;
}
