/*
 ==================================================================================
   TODO: DAY 06 - PHYSICS INTEGRATION & PLAYER MOVEMENT
   Goal: Attach the Camera to the Physics System (Stop Flying, Start Walking).
 ==================================================================================

 1. VOXEL INTERACTION (COMPLETED)
    [x] Raycast Visualizer: Debug line with offset (gun barrel view).
    [x] Destruction: Ctrl + Left Click -> Raycast -> Set Block 0 -> Rebuild Mesh.
    [x] Logic Fix: Moved 'updateHeightData' to constructor to prevent terrain reset on update.
    [x] Render Fix: Updated 'GenerateMesh' neighbor checks to render holes/caves correctly.

 2. PHYSICS BACKEND (READY)
    [x] AABB Struct: Min/Max bounds defined.
    [x] Collision Logic: 'PhysicsSystem::CheckCollision(A, B)' is implemented.
    [x] DDA Raycast: Implemented fast voxel traversal algorithm.
    [x] Gravity Pull: 'Velocity.y -= 9.8f * dt' logic is implemented in backend.

 3. PLAYER CONTROLLER (NEXT SESSION)
    [ ] Disable "Free Fly": Stop direct Camera pos updates in 'processInput'.
    [ ] Movement Vector: Map WASD to 'Player.Velocity.x' and 'Player.Velocity.z'.
    [ ] Apply Gravity: Call 'PhysicsSystem::Update' every frame to pull player down.
    [ ] Collision Resolution:
        - Use 'CheckCollision' to detect player vs world.
        - If colliding below: Set 'IsGrounded = true' and stop falling.
        - If colliding side: Slide or stop X/Z movement.
    [ ] Jump Mechanic:
        - Input: SPACE key.
        - Condition: Only if 'IsGrounded' is true.
        - Action: 'Velocity.y = JumpForce'.

 4. POLISH
    [ ] Toggle Mode (F1): Switch between "Walking" and "Flying" states.

 ==================================================================================
*/
#include <iostream>
#include <vector>
#include "physics/PhysicsSystem.h"
#include "physics/AABB.h"
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
#include <renderer/DebugRenderer.h>

#include <world/Chunk.h>

const unsigned int SCREEN_WIDTH = 1280;
const unsigned int SCREEN_HEIGHT = 720;
Core::Vec3 cameraPos = Core::Vec3(100.0f, 20.0f, 140.0f);
Core::Camera camera(cameraPos);
float lastX = SCREEN_WIDTH / 2.0f;
float lastY = SCREEN_HEIGHT / 2.0f;
bool bFirstMouse = true, bEnableFaceCulling = false, 
bPerspective = true;
float fOrthoSize = 10.0f;
float fDeltaTime = 0.0f;
float fLastFrame = 0.0f;

void framebuffer_size_callback([[maybe_unused]]GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void mouseCallBack(GLFWwindow* pWindow, double xPosIn, double yPosIn) {
	/*if (glfwGetKey(pWindow, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS || glfwGetKey(pWindow, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS)
		return;*/
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
		camera.SetCameraPosition(cameraPos);
		camera.SetCameraZoom(45.0f);
		camera.SetCameraFront(Core::Vec3(0.0f, 0.0f, -1.0f));
		camera.SetCameraYawPitch(-90.0f, -30.0f);
		camera.UpdateCameraVectors();
	}
}

void scroll_callback([[maybe_unused]]GLFWwindow* pWindow, [[maybe_unused]]double dXOffset, double dYOffset) {
	if(bPerspective)
	camera.ProcessMouseScroll(static_cast<float> (dYOffset));
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
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_DEPTH_BITS, 24);
	GLFWwindow* pWindow = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "HPC Voxel Engine", nullptr, nullptr);
	if (pWindow == nullptr)
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
	Renderer::DebugRenderer::Init();

	const GLubyte* vendor = glGetString(GL_VENDOR);
	const GLubyte* renderor = glGetString(GL_RENDERER);
	std::cout << "GPU Renderer: " << vendor << std::endl;
	std::cout << "Renderer: " << renderor << std::endl;

	Renderer::Shader shader("../assets/shaders/vertex.glsl", "../assets/shaders/fragment.glsl"); 
	std::vector<Chunk> chunks;
	int iRenderDistance = 8;
	int iTotalChunks = (iRenderDistance * 2) * (iRenderDistance * 2);
	chunks.reserve(iTotalChunks + 10);
	for (int iX = -iRenderDistance; iX < iRenderDistance; iX++)
	{
		for (int iZ = -iRenderDistance; iZ < iRenderDistance; iZ++)
		{
			chunks.emplace_back(iX, iZ);
		}
	}

	//Renderer::Texture texture("../assets/textures/container.jpg");
	Renderer::Texture texture("../assets/textures/texture_atlas.png");
	texture.Bind(0);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);
	static bool sbFPressedLastTime = false, sbPPressedLastTime = false, sbLMBClikedFirstTime = true;
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
			projection = Core::Mat4::perspective(camera.GetZoom(),
			fAspectRatio, 0.1f, 100.0f);
		}
		else
		{
			float fHeight = fOrthoSize;
			float fWidth = fOrthoSize * fAspectRatio;
			projection = Core::Mat4::orthographic(
				-fWidth, fWidth, -fHeight, fHeight, -100.0f, 100.0f);
		}
		Core::Mat4 view = camera.GetViewMatrix();
		Core::Mat4 viewProjection = projection * view;

		shader.setMat4("uViewProjection", viewProjection);
		for (auto& chunk : chunks)
		{
			chunk.Render();
		}
		glDisable(GL_DEPTH_TEST);
		//Draw GCS
		Renderer::DebugRenderer::DrawLine(Core::Vec3(0.0f, 0.0f, 0.0f),Core::Vec3(50.0f, 0.0f, 0.0f),
		Core::Vec3(1.0f, 0.0f, 0.0f),viewProjection);
		Renderer::DebugRenderer::DrawLine(Core::Vec3(0.0f, 0.0f, 0.0f),Core::Vec3(0.0f, 50.0f, 0.0f),
		Core::Vec3(0.0f, 1.0f, 0.0f),viewProjection);
		Renderer::DebugRenderer::DrawLine(Core::Vec3(0.0f, 0.0f, 0.0f),Core::Vec3(0.0f, 0.0f, 50.0f),
		Core::Vec3(0.0f, 0.0f, 1.0f),viewProjection);
		//Test Cube
		/*Renderer::DebugRenderer::DrawCube(
			Core::Vec3(0.0f, 0.0f, 0.0f),
			Core::Vec3(20.0f, 20.0f, 20.0f), 
			Core::Vec3(1.0f, 0.0f, 1.0f), viewProjection);*/

		glDisable(GL_DEPTH_TEST);
		if (glfwGetKey(pWindow, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS || glfwGetKey(pWindow, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS)
		{
			float fMaxDistance = 60.0f;
			Core::Ray objRay(camera.GetCameraPosition(), camera.GetFront());
			
			//Draw Ray
			Core::Vec3 objUp = camera.GetUp();
			Core::Vec3 objRight = camera.GetFront().cross(objUp).normalize();
			Core::Vec3 objRayStart = objRay.m_objPtOrigin - objUp * 0.1f + objRight * 0.2f;
			Core::Vec3 objRayEnd = objRay.at(fMaxDistance);
			Renderer::DebugRenderer::DrawLine(objRayStart, objRayEnd, Core::Vec3(1.0f, 1.0f, 0.0f), viewProjection);
			
			RayHit objRayHit = PhysicsSystem::RayCast(objRay, fMaxDistance, chunks);
			if(objRayHit.m_bHit)
			{
				Core::Vec3 objBlockPos(
					static_cast<float>(objRayHit.m_iBlocKX), 
					static_cast<float>(objRayHit.m_iBlocKY), 
					static_cast<float>(objRayHit.m_iBlocKZ));
	#if DEBUG
					std::cout << "Ray : " 
						<< "X: " << objRay.m_objPtOrigin.x << ", "
						<< "Y: " << objRay.m_objPtOrigin.y << ", "
						<< "Z: " << objRay.m_objPtOrigin.z << ", " 
						<< objRay.m_objDirection.x << ", " 
						<< objRay.m_objDirection.y << ", " 
						<< objRay.m_objDirection.z << std::endl;

						std::cout << "Ray Hit! Block: " 
						<< "X: " << objRayHit.m_objHitPoint.x << ", "
						<< "Y: " << objRayHit.m_objHitPoint.y << ", "
						<< "Z: " << objRayHit.m_objHitPoint.z << ", " 
						<< objRayHit.m_fDistance << ", " 
						<< objRayHit.m_iBlocKX << ", " 
						<< objRayHit.m_iBlocKY << ", " 
						<< objRayHit.m_iBlocKZ << std::endl;
	#endif
				Renderer::DebugRenderer::DrawCube(objBlockPos, Core::Vec3(1.005f, 1.005f, 1.005f), 
				Core::Vec3(1.0f, 0.0f, 1.0f), viewProjection);
			}
			if(glfwGetMouseButton(pWindow, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
			{
				if(sbLMBClikedFirstTime && objRayHit.m_bHit)
				{
					int iTargetBlockX = static_cast<int>(floor(static_cast<float>(objRayHit.m_iBlocKX) / CHUNK_SIZE));
					int iTargetBlockZ = static_cast<int>(floor(static_cast<float>(objRayHit.m_iBlocKZ) / CHUNK_SIZE));
					for(auto & chunk : chunks)
					{
						if (chunk.GetChunkX() == iTargetBlockX && chunk.GetChunkZ() == iTargetBlockZ)
						{
							int iLocalX = objRayHit.m_iBlocKX - (iTargetBlockX * CHUNK_SIZE);
							int iLocalZ = objRayHit.m_iBlocKZ - (iTargetBlockZ * CHUNK_SIZE);
							chunk.SetBlockAt(iLocalX, objRayHit.m_iBlocKY, iLocalZ, 0);
							chunk.GenerateMesh();
							chunk.Render();
						}
					}
					sbLMBClikedFirstTime = false;
				}
			}
			else
				sbLMBClikedFirstTime = true;
		}
		glEnable(GL_DEPTH_TEST);

		glfwSwapBuffers(pWindow);
		glfwPollEvents();
	}
	glfwTerminate();
	Renderer::DebugRenderer::Shutdown();
	return 0;
}
