#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "vk.h" 	//vk.h

//vulkan related header files
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>


//glm related macros and header files
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"


//vulkan related libraries
#pragma comment (lib, "vulkan-1.lib")

//macro definitions
#define WIN_WIDTH 800
#define WIN_HIGHT 600

//-------------------------------------------------------------------------------------------------------//
//-------------------------------------------------------------------------------------------------------//
//vulkan related global variables
//instance extension related
uint32_t enabledInstanceExtensionCount = 0;
//VK_KHR_SURFACE_EXTENSION_NAME
//VK_KHR_WIN32_SURFACE_EXTENSION_NAME
//VK_EXT_DEBUG_REPORT_EXTENSION_NAME
const char *enabledInstanceExtensionNames_Array[3];

VkInstance vkInstance = VK_NULL_HANDLE;
VkSurfaceKHR vkSurfaceKHR = VK_NULL_HANDLE;

//Vulkan Physical device related global variables
VkPhysicalDevice vkPhysicalDevice_Selected = VK_NULL_HANDLE;
uint32_t graphicsQueueFamilyIndex_Selected = UINT32_MAX;
VkPhysicalDeviceMemoryProperties vkPhysicalDeviceMemoryProperties;

uint32_t physicalDeviceCount = 0;
VkPhysicalDevice *vkPhysicalDevice_array = NULL;

//---vulkan related global variables---
//device extension related
uint32_t enabledDeviceExtensionCount = 0;
//VK_KHR_SWAPCHAIN_EXTENSION_NAME
//VK_KHR_WIN32_SURFACE_EXTENSION_NAME
const char *enabledDeviceExtensionNames_Array[1];

//vulkan device
VkDevice vkDevice = VK_NULL_HANDLE;

//Device queue
VkQueue vkQueue = VK_NULL_HANDLE;

//Color Format And Color Space
VkFormat vkFormat_color = VK_FORMAT_UNDEFINED;
VkColorSpaceKHR vkColorSpaceKHR = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

//presentation mode
VkPresentModeKHR vkPresentModeKHR = VK_PRESENT_MODE_FIFO_KHR;

//swapchain
int winWidth = WIN_WIDTH;
int winHeight = WIN_HIGHT;
VkSwapchainKHR vkSwapchainKHR;
VkExtent2D vkExtend2D_Swapchain;

//Swapchain Images and Iamge Views data
uint32_t swapchainImageCount = UINT32_MAX;
VkImage *swapchainImage_array = NULL;
VkImageView *swapchainImageView_array = NULL;

//for depth image
VkFormat vkFormat_depth = VK_FORMAT_UNDEFINED;
VkImage vkImage_depth = VK_NULL_HANDLE;
VkDeviceMemory vkDeviceMemory_depth = VK_NULL_HANDLE;
VkImageView vkImageView_depth = VK_NULL_HANDLE;


//command pool
VkCommandPool vkCommandPool = VK_NULL_HANDLE;

//command buffers
VkCommandBuffer *vkCommandBuffer_array = VK_NULL_HANDLE;

//RenderPass
VkRenderPass vkRenderPass = VK_NULL_HANDLE;

//frame Buffer
VkFramebuffer *vkFramebuffer_array = VK_NULL_HANDLE;

//semaphore 
VkSemaphore vkSemaphore_backBuffer = VK_NULL_HANDLE;
VkSemaphore	vkSemaphore_renderComplete = VK_NULL_HANDLE;

//fence
VkFence *vkFence_array = NULL;

//Clear Color Values
VkClearColorValue vkClearColorValue;
VkClearDepthStencilValue vkClearDepthStencilValue; //not used, but can be used for depth stencil clear values


//Render
BOOL bInitialized = FALSE;
uint32_t currentImageIndex = UINT32_MAX;

//validations
BOOL bValidationLayerSupport = TRUE;				//TRUE: for validation layer support
uint32_t enabledValidationLayerCount = 0;			//count of validation layers
const char *enabledValidationLayerNames_Array[1]; 	//for VK_LAYER_KHRONOS_validation
VkDebugReportCallbackEXT vkDebugReportCallbackEXT = VK_NULL_HANDLE;
PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT_fnptr = NULL;

//vertex buffer
typedef struct
{
	/* data */
	VkBuffer vkBuffer;
	VkDeviceMemory vkDeviceMemory;
} VertexData;

//position
VertexData vertexData_position;

//texture coordinates
VertexData vertexData_texCoords;

//uniform related declarations
struct MyUniformData				//MyMatrixUniformData, MyColorUniformData, etc.
{
	glm::mat4 modelMatrix;			//model matrix
	glm::mat4 viewMatrix;			//view matrix
	glm::mat4 projectionMatrix;		//projection matrix
};

struct UniformData					//similar as struct VertexData
{
	VkBuffer vkBuffer;				//uniform buffer
	VkDeviceMemory vkDeviceMemory;	//device memory for uniform buffer
};


UniformData uniformData_cube;

//shaders 
VkShaderModule vkShaderModule_vertex_shader = VK_NULL_HANDLE;
VkShaderModule vkShaderModule_fragment_shader = VK_NULL_HANDLE;

//descriptor set layout
VkDescriptorSetLayout vkDescriptorSetLayout = VK_NULL_HANDLE;
//VkDescriptorSet vkDescriptorSet = VK_NULL_HANDLE;

//pipeline layout
VkPipelineLayout vkPipelineLayout = VK_NULL_HANDLE;

//Descriptor Pool
VkDescriptorPool vkDescriptorPool = VK_NULL_HANDLE;

//Descriptor Set
VkDescriptorSet vkDescriptorSet_cube = VK_NULL_HANDLE;

//Pipeline
VkViewport vkViewport;
VkRect2D vkRect2D_scissor;
VkPipeline vkPipeline = VK_NULL_HANDLE;
//texture related global data
VkImage vkImage_texture = VK_NULL_HANDLE;
VkDeviceMemory vkDeviceMemory_texture = VK_NULL_HANDLE;
VkImageView vkImageView_texture = VK_NULL_HANDLE;

//sampler
VkSampler vkSampler_texture = VK_NULL_HANDLE;

/*------------------------------------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////
//WINDOWING RELATED FUNCTIONS
/////////////////////////////////////////////////////////////////////
------------------------------------------------------------------------------------------------------*/
//window procedure
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

//global variable declarations
HWND ghwnd = NULL;

DWORD dwStyle;
//WINDOWPLACEMENT wpPrev = { };
WINDOWPLACEMENT wpPrev = {sizeof(WINDOWPLACEMENT)};

BOOL gbFullscreen = FALSE;
BOOL gbWindowMinimized = FALSE;
BOOL gbActiveWindow = FALSE;
FILE *gpFile = NULL;

//application name
const char *gpszAppName = "VulkanApp";

//for rotation
float angle = 0.0f;	//angle of rotation

//main
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	//function declations
	VkResult initialize(void);
	VkResult display(void);
	void update(void);
	void uninitialize(void);
	
	VkResult vkResult = VK_SUCCESS;

	//variable declarations
	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;
	TCHAR szAppName[255];
	RECT rect;
	BOOL bDone = FALSE;
	
	UINT VerPos;
	UINT HorPos;

	//code
	//opening a file
	gpFile = fopen("DebugLog.txt", "w");
	if(gpFile == NULL)
	{
		MessageBox(NULL, TEXT("Can Not Open Desired File..."),TEXT("Error"), MB_OK);
		exit(0);
	}
	else
	{
		fprintf(gpFile, "\nDEBUG:Log File Created Successfully");
	}

	wsprintf(szAppName, TEXT("%s"), gpszAppName);

	//initializing winndow class
	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.cbWndExtra = 0;
	wndclass.cbClsExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.hIcon = LoadIcon(hInstance , MAKEINTRESOURCE(MYICON));
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hIconSm = LoadIcon(hInstance , MAKEINTRESOURCE(MYICON));
	wndclass.lpfnWndProc = WndProc;
	wndclass.lpszClassName = szAppName;
	wndclass.lpszMenuName = NULL;

	//Registering above class
	RegisterClassEx(&wndclass);
	fprintf(gpFile, "\nDEBUG:Class Registered Successfully\n");
	//SystemParametersInfo
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);

	HorPos = (rect.right - WIN_WIDTH) / 2;
	VerPos = (rect.bottom - WIN_HIGHT) / 2;

	//creating window
	hwnd = CreateWindowEx(	WS_EX_APPWINDOW,
							szAppName,
							TEXT("Vulkan: Texture on Cube"),
							WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
							HorPos,
							VerPos,
							WIN_WIDTH,
							WIN_HIGHT,
							NULL,
							NULL,
							hInstance,
							NULL);

	fprintf(gpFile, "\nDEBUG:Window Created\n");

	ghwnd = hwnd;
	vkResult = initialize();
	if(vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nDEBUG:[WINMAIN][ERROR] Failed initialize()");
		DestroyWindow(hwnd);
	}
	else
		fprintf(gpFile, "\nDEBUG:[WINMAIN][SUCCESS] initialize()");
		fflush(gpFile);

		
	ShowWindow(hwnd, iCmdShow);
	fprintf(gpFile, "\nDEBUG:[WINMAIN][SUCCESS] ShowWindow()");
	SetForegroundWindow(hwnd);
	fprintf(gpFile, "\nDEBUG:[WINMAIN][SUCCESS] SetForegroundWindow()");
	SetFocus(hwnd);
	fprintf(gpFile, "\nDEBUG:[WINMAIN][SUCCESS] SetFocus()");
	fflush(gpFile);

	fprintf(gpFile, "\nDEBUG:[WINMAIN][SUCCESS] display() lopp started");
	//MessageLoop
	while(bDone == FALSE)
	{
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if(msg.message == WM_QUIT)
				bDone = TRUE;
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			if(gbActiveWindow == TRUE)
			{
				//HERE YOU SHOULD CALL UPDATE FUNCTION FOR OPENGL RENDERING
				//HERE YOU SHOULD CALL DISPLAY FUNCTION FOR OPENGL RENDERING
				if(gbWindowMinimized == FALSE)
				{
					update();
					vkResult = display();
					
				}

				if((vkResult != VK_FALSE) && (vkResult != VK_SUCCESS) && (vkResult != VK_SUBOPTIMAL_KHR) && (vkResult != VK_ERROR_OUT_OF_DATE_KHR))
				{
					fprintf(gpFile, "\nDEBUG:[WINMAIN][ERROR] display() = %d", vkResult);
					bDone = TRUE;
				}
				else
				{
					//fprintf(gpFile, "\nDEBUG:[WINMAIN][SUCCESS] display() = %d", vkResult);
				}
			}
		}
	}

	fprintf(gpFile, "\nDEBUG:[WINMAIN][SUCCESS] display() lopp ended");

	return((int)msg.wParam);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	//function declarations
	void ToggleFullscreen(void);
	void uninitialize(void);
	VkResult resize(int, int);

	//code
	switch(iMsg)
	{
		case WM_CREATE:
			fprintf(gpFile, "\nDEBUG:WM_CREATE Recieved");
			break;

		case WM_SETFOCUS:
			gbActiveWindow = TRUE;
			break;

		case WM_KILLFOCUS:
			gbActiveWindow = FALSE;
			break;

		case WM_ERASEBKGND:
			return(0);

		case WM_SIZE:
			if(wParam == SIZE_MINIMIZED)
			{
				gbWindowMinimized = TRUE;
			}
			else
			{
				resize(LOWORD(lParam), HIWORD(lParam));
				gbWindowMinimized = FALSE;
			}			
			break;

		case WM_KEYDOWN:
			switch(wParam)
			{
				case 0x46:
				case 0x66:
					ToggleFullscreen();
					break;

				case VK_ESCAPE:
					DestroyWindow(hwnd);
					break;

				default:
					break;
			}
			break;

		case WM_CLOSE:
			DestroyWindow(hwnd);
			break;

		case WM_QUIT:
			fprintf(gpFile, "\nDEBUG:WM_QUIT Recieved");
			fflush(gpFile);
			//uninitialize();
			PostQuitMessage(0);
			break;

		case WM_DESTROY:
			fprintf(gpFile, "\nDEBUG:WM_DESTROY Recieved");
			fflush(gpFile);
			uninitialize();
			PostQuitMessage(0);
			break;
	}

	return(DefWindowProc(hwnd, iMsg, wParam, lParam));
}

void ToggleFullscreen(void)
{
	//variable declarations
	MONITORINFO mInfo = {sizeof(MONITORINFO)};

	//code
	if(gbFullscreen == FALSE)
	{
		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);
		if(dwStyle && WS_OVERLAPPEDWINDOW)
		{
			if(GetWindowPlacement(ghwnd, &wpPrev) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mInfo))
			{
				SetWindowLong(ghwnd, GWL_STYLE, (dwStyle & ~(WS_OVERLAPPEDWINDOW)));
				SetWindowPos(	ghwnd,
								HWND_TOP,
								mInfo.rcMonitor.left,
								mInfo.rcMonitor.top,
								mInfo.rcMonitor.right - mInfo.rcMonitor.left,
								mInfo.rcMonitor.bottom - mInfo.rcMonitor.top,
								SWP_NOZORDER | SWP_FRAMECHANGED);
			}
		}
		ShowCursor(FALSE);
		gbFullscreen = TRUE;
	}

	else
	{
		SetWindowLong(ghwnd, GWL_STYLE, (dwStyle | WS_OVERLAPPEDWINDOW));
		SetWindowPlacement(ghwnd, &wpPrev);
		SetWindowPos(ghwnd,
					HWND_TOP,
					0,
					0,
					0,
					0,
					SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);

				ShowCursor(TRUE);
				gbFullscreen = FALSE;
	}
}

VkResult initialize(void)
{	
	//function declarations
	VkResult createVulkanInstance(void);
	VkResult getSupportedSurface(void);
	VkResult getPhysicalDevice(void);
	VkResult printVkInfo(void);
	VkResult createVulkanDevice(void);
	void getDeviceQueue(void);
	VkResult createSwapchain(VkBool32);
	VkResult createImagesAndImageViews(void);
	VkResult createCommandPool(void);
	VkResult createCommandBuffers(void);
	VkResult createVertexBuffers(void);
	VkResult CreateTexture(const char *);
	VkResult createUniformBuffers(void);
	VkResult createShaders(void);
	VkResult createDescriptorSetLayout(void);
	VkResult createPipelineLayout(void);				//
	VkResult createDescriptorPool(void);				//
	VkResult createDescriptorSet(void);
	VkResult createRenderPass(void);
	VkResult createPipeline(void);
	VkResult createFrameBuffers(void);
	VkResult createSemaphores(void);
	VkResult createFences(void);
	VkResult buildCommandBuffers(void);

	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	//code
	fprintf(gpFile, "\n\nDEBUG:------------------------------------Inside[initialize()]------------------------------------");

	vkResult = createVulkanInstance();
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[initialize][ERROR] Failed createVulkanInstance()");
		return vkResult;
	}
	else
		fprintf(gpFile, "\nDEBUG:[initialize][SUCCESS] createVulkanInstance() Completed");

	//Presetation Surface Steps
	vkResult = getSupportedSurface();
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[initialize][ERROR] Failed getSupportedSurface()");
		return vkResult;
	}
	else
		fprintf(gpFile, "\nDEBUG:[initialize][SUCCESS] getSupportedSurface() Completed");

	//Get/select required physical device and its queue family index
	vkResult = getPhysicalDevice();
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[initialize][ERROR] Failed getPhysicalDevice()");
		return vkResult;
	}
	else
		fprintf(gpFile, "\nDEBUG:[initialize][SUCCESS] getPhysicalDevice() Completed");

	//print vulkan info
	vkResult = printVkInfo();
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[initialize][ERROR] Failed printVkInfo()");
		return vkResult;
	}
	else
		fprintf(gpFile, "\nDEBUG:[initialize][SUCCESS] printVkInfo() Completed");
	
	//create Vulkan device
	vkResult = createVulkanDevice();
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[initialize][ERROR] Failed createVulkanDevice()");
		return vkResult;
	}
	else
		fprintf(gpFile, "\nDEBUG:[initialize][SUCCESS] createVulkanDevice() Completed");

	//device queue
	getDeviceQueue();

	//swapchain
	vkResult = createSwapchain(VK_FALSE);
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[initialize][ERROR] Failed createSwapchain()");
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	else
		fprintf(gpFile, "\nDEBUG:[initialize][SUCCESS] createSwapchain() Completed");

	//create images and image views
	vkResult = createImagesAndImageViews();
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[initialize][ERROR] Failed createImagesAndImageViews()");
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	else
		fprintf(gpFile, "\nDEBUG:[initialize][SUCCESS] createImagesAndImageViews() Completed");
	
	//create command pool
	vkResult = createCommandPool();
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[initialize][ERROR] Failed createCommandPool()");
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	else
		fprintf(gpFile, "\nDEBUG:[initialize][SUCCESS] createCommandPool() Completed");

	//create command Buffers
	vkResult = createCommandBuffers();
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[initialize][ERROR] Failed createCommandBuffers()");
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	else
		fprintf(gpFile, "\nDEBUG:[initialize][SUCCESS] createCommandBuffers() Completed");

	//create vertex buffers
	vkResult = createVertexBuffers();
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[initialize][ERROR] Failed createVertexBuffers()");
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	else
		fprintf(gpFile, "\nDEBUG:[initialize][SUCCESS] createVertexBuffers() Completed");
	
	//create texture
	vkResult = CreateTexture("kundali.png");
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[initialize][ERROR] Failed CreateTexture()");
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	else
		fprintf(gpFile, "\nDEBUG:[initialize][SUCCESS] CreateTexture() Completed");

	//create uniform buffers
	vkResult = createUniformBuffers();
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[initialize][ERROR] Failed createUniformBuffers()");
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	else
		fprintf(gpFile, "\nDEBUG:[initialize][SUCCESS] createUniformBuffers() Completed");

	//create shaders
	vkResult = createShaders();
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[initialize][ERROR] Failed createShaders()");
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	else
		fprintf(gpFile, "\nDEBUG:[initialize][SUCCESS] createShaders() Completed");
	
	//create descriptor set layout
	vkResult = createDescriptorSetLayout();
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[initialize][ERROR] Failed createDescriptorSetLayout()");
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	else
		fprintf(gpFile, "\nDEBUG:[initialize][SUCCESS] createDescriptorSetLayout() Completed");	
	
	//create pipeline layout
	vkResult = createPipelineLayout();
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[initialize][ERROR] Failed createPipelineLayout()");
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	else
		fprintf(gpFile, "\nDEBUG:[initialize][SUCCESS] createPipelineLayout() Completed");

	//create descriptor pool
	vkResult = createDescriptorPool();
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[initialize][ERROR] Failed createDescriptorPool()");
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	else
		fprintf(gpFile, "\nDEBUG:[initialize][SUCCESS] createDescriptorPool() Completed");
	
	//create descriptor set
	vkResult = createDescriptorSet();
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[initialize][ERROR] Failed createDescriptorSet()");
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	else
		fprintf(gpFile, "\nDEBUG:[initialize][SUCCESS] createDescriptorSet() Completed");	
	
	//create RenderPass
	vkResult = createRenderPass();
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[initialize][ERROR] Failed createRenderPass()");
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	else
		fprintf(gpFile, "\nDEBUG:[initialize][SUCCESS] createRenderPass() Completed");

	//create pipeline
	vkResult = createPipeline();
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[initialize][ERROR] Failed createPipeline()");
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	else
		fprintf(gpFile, "\nDEBUG:[initialize][SUCCESS] createPipeline() Completed");
	
	//create FrameBuffer
	vkResult = createFrameBuffers();
	if(vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nDEBUG:[initialize][ERROR] Failed createFrameBuffers()");
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	else
		fprintf(gpFile, "\nDEBUG:[initialize][SUCCESS] createFrameBuffers() Completed");

	//create semaphores
	vkResult = createSemaphores();
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[initialize][ERROR] Failed createSemaphores()");
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	else
		fprintf(gpFile, "\nDEBUG:[initialize][SUCCESS] createSemaphores() Completed");
		fflush(gpFile);
	
	//create fences
	vkResult = createFences();
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[initialize][ERROR] Failed createFences()");
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	else
		fprintf(gpFile, "\nDEBUG:[initialize][SUCCESS] createFences() Completed");
		fflush(gpFile);

	//initialize clearColorValues.
	memset((void *)&vkClearColorValue, 0, sizeof(VkClearColorValue));

	memset((void *)&vkClearDepthStencilValue, 0, sizeof(VkClearDepthStencilValue));

	vkClearDepthStencilValue.depth = 1.0f;
	vkClearDepthStencilValue.stencil = 0;
	
	vkClearColorValue.float32[0] = 0.0f;	//red
	vkClearColorValue.float32[1] = 0.0f;	//green	
	vkClearColorValue.float32[2] = 0.0f; 	//blue
	vkClearColorValue.float32[3] = 1.0f;	//alpha  //analogous to glClearColor()
	
	fprintf(gpFile, "\nDEBUG:[initialize][SUCCESS] vkClearColorValue initialized successfully");
	fflush(gpFile);

	//build command buffers
	vkResult = buildCommandBuffers();
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[initialize][ERROR] Failed buildCommandBuffers()");
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	else
		fprintf(gpFile, "\nDEBUG:[initialize][SUCCESS] buildCommandBuffers() Completed");
		fflush(gpFile);
	
	//initialization complete
	bInitialized = TRUE;
	fprintf(gpFile, "\nDEBUG:[initialize][SUCCESS] bInitialized = TRUE");
	

	fprintf(gpFile, "\nDEBUG:------------------------------------Done [initialize()]------------------------------------\n\n");
	fflush(gpFile);
	return vkResult;
}

VkResult resize(int width, int height)
{
	//variable declarations
	VkResult vkResult = VK_SUCCESS;
	
	//function declarations
	VkResult createSwapchain(VkBool32);
	VkResult createImagesAndImageViews(void);
	VkResult createCommandBuffers(void);
	VkResult createPipelineLayout(void);
	VkResult createPipeline(void);
	VkResult createRenderPass(void);
	VkResult createCommandBuffers(void);
	VkResult buildCommandBuffers(void);
	VkResult createFrameBuffers(void);

	//code
	fprintf(gpFile, "\n\nDEBUG:------------------------------------Inside[resize()]------------------------------------");
	
	if(height <= 0)
	{
		height = 1; //to avoid division by zero
		
	}
	
	//if control comes here before initialization completed, then return false.
	if(bInitialized == FALSE)
	{
		fprintf(gpFile, "\nDEBUG:[resize][ERROR] bInitialized not completed/Failed = FALSE");
		return VK_ERROR_INITIALIZATION_FAILED;
	}

	//as recreation of swapchsain is required, we are going to require many steps of initialization again.
	//hence initialize bInitialized to FALSE.
	bInitialized = FALSE;
	fprintf(gpFile, "\nDEBUG:[resize][SUCCESS] bInitialized = FALSE");

	//set global winWidth and winHeight
	winWidth = width;
	winHeight = height;

	fprintf(gpFile, "\nDEBUG:[resize][SUCCESS] winWidth = %d, winHeight = %d", winWidth, winHeight);

	//wait for device for in hand tasks
	vkDeviceWaitIdle(vkDevice);

	//check presence of swapchain
	if(vkSwapchainKHR == VK_NULL_HANDLE)
	{
		fprintf(gpFile, "\nDEBUG:[resize][ERROR] vkSwapchainKHR is NULL, can not proceed");
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
	}


	//destroy framebuffers
	if(vkFramebuffer_array)
	{
		for(uint32_t i = 0; i < swapchainImageCount; i++)
		{
			vkDestroyFramebuffer(vkDevice, vkFramebuffer_array[i], NULL);
			fprintf(gpFile, "\nDEBUG:[resize()][SUCCESS] vkDestroyFramebuffer() = %d", i);
		}
		if(vkFramebuffer_array)
		{
			free(vkFramebuffer_array);
			vkFramebuffer_array = NULL;
		}
		fprintf(gpFile, "\nDEBUG:[resize()][SUCCESS] vkFramebuffer_array freed");
		fflush(gpFile);
	}

	//destroy command buffers
	if(vkCommandBuffer_array)
	{
		//for(uint32_t i = 0; i < swapchainImageCount; i++)
		//{
			vkFreeCommandBuffers(vkDevice, vkCommandPool, swapchainImageCount, vkCommandBuffer_array);
			fprintf(gpFile, "\nDEBUG:[resize()][SUCCESS] vkFreeCommandBuffers()");
		//}
		if(vkCommandBuffer_array)
		{
			free(vkCommandBuffer_array);
			vkCommandBuffer_array = NULL;
		}
		fflush(gpFile);
	}

	//destroy pipeline
	if(vkPipeline != VK_NULL_HANDLE)
	{
		vkDestroyPipeline(vkDevice, vkPipeline, NULL);
		vkPipeline = VK_NULL_HANDLE;
		fprintf(gpFile, "\nDEBUG:[resize()][SUCCESS] vkPipeline destroyed successfully");
		fflush(gpFile);
	}

	//destroy pipeline layout
	if(vkPipelineLayout != VK_NULL_HANDLE)
	{
		vkDestroyPipelineLayout(vkDevice, vkPipelineLayout, NULL);
		vkPipelineLayout = VK_NULL_HANDLE;
		fprintf(gpFile, "\nDEBUG:[resize()][SUCCESS] vkPipelineLayout destroyed successfully");
		fflush(gpFile);
	}

	//destroy render pass
	if(vkRenderPass != VK_NULL_HANDLE)
	{
		vkDestroyRenderPass(vkDevice, vkRenderPass, NULL);
		vkRenderPass = VK_NULL_HANDLE;
		fprintf(gpFile, "\nDEBUG:[resize()][SUCCESS] vkRenderPass destroyed successfully");
		fflush(gpFile);
	}

	//destroy depth attachment
	if(vkImageView_depth)
	{
		vkDestroyImageView(vkDevice, vkImageView_depth, NULL);
		vkImageView_depth = VK_NULL_HANDLE;
		//fprintf(gpFile, "\nDEBUG:[resize()][SUCCESS] vkDestroyImageView() = vkImageView_depth");
	}

	//destroy device memory for depth attachment
	if(vkDeviceMemory_depth)
	{
		vkFreeMemory(vkDevice, vkDeviceMemory_depth, NULL);
		vkDeviceMemory_depth = VK_NULL_HANDLE;
		//fprintf(gpFile, "\nDEBUG:[resize()][SUCCESS] vkFreeMemory() = vkDeviceMemory_depth");
	}

	//destroy depth image
	if(vkImage_depth)
	{
		vkDestroyImage(vkDevice, vkImage_depth, NULL);
		vkImage_depth = VK_NULL_HANDLE;
		//fprintf(gpFile, "\nDEBUG:[resize()][SUCCESS] vkDestroyImage() = vkImage_depth");
	}
	
	//destroy swapchain
	if(vkSwapchainKHR)
	{
		vkDestroySwapchainKHR(vkDevice, vkSwapchainKHR, NULL);
		vkSwapchainKHR = VK_NULL_HANDLE;
		//fprintf(gpFile, "\nDEBUG:[resize()][SUCCESS] vkDestroySwapchainKHR()");
	}

	
	//Recreate for resize
	//create swapchain
	//swapchain
	vkResult = createSwapchain(VK_FALSE);
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[resize][ERROR] Failed createSwapchain()");
		return VK_ERROR_INITIALIZATION_FAILED;
	}

	//create images and image views
	vkResult = createImagesAndImageViews();
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[resize][ERROR] Failed createImagesAndImageViews()");
		return VK_ERROR_INITIALIZATION_FAILED;
	}

	
	//create pipeline layout
	vkResult = createPipelineLayout();
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[resize][ERROR] Failed createPipelineLayout()");
		return VK_ERROR_INITIALIZATION_FAILED;
	}

	//create RenderPass
	vkResult = createRenderPass();
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[resize][ERROR] Failed createRenderPass()");
		return VK_ERROR_INITIALIZATION_FAILED;
	}

	//create pipeline
	vkResult = createPipeline();
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[resize][ERROR] Failed createPipeline()");
		return VK_ERROR_INITIALIZATION_FAILED;
	}

	//create command Buffers
	vkResult = createCommandBuffers();
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[resize][ERROR] Failed createCommandBuffers()");
		return VK_ERROR_INITIALIZATION_FAILED;
	}

	//create FrameBuffer
	vkResult = createFrameBuffers();
	if(vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nDEBUG:[resize][ERROR] Failed createFrameBuffers()");
		return VK_ERROR_INITIALIZATION_FAILED;
	}

	vkResult = buildCommandBuffers();
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[resize][ERROR] Failed buildCommandBuffers()");
		return VK_ERROR_INITIALIZATION_FAILED;
	}

	bInitialized = TRUE;
	fprintf(gpFile, "\nDEBUG:[resize][SUCCESS] bInitialized = TRUE");
	return vkResult;
}

VkResult display(void)
{
	//function declarations
	VkResult resize(int, int);
	VkResult updateUniformBuffer(void);

	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	//code
	//fprintf(gpFile, "\n\nDEBUG:------------------------------------Inside[display()]------------------------------------");
	//if control comes here before initialization completed, then return false.
	if(bInitialized == FALSE)
	{
		fprintf(gpFile, "\nDEBUG:[display][ERROR] bInitialized not completed = FALSE");
		return (VkResult)VK_FALSE;
	}
	
	//Acquire next image from swapchain
	//here we are waiting for UINT64_MAX nano seconds for swapchain for next image.
	//here we are using semaphore to synchronize between image acquisition and rendering.
	vkResult = vkAcquireNextImageKHR(vkDevice, vkSwapchainKHR, UINT64_MAX, vkSemaphore_backBuffer, VK_NULL_HANDLE, &currentImageIndex);
	if(vkResult != VK_SUCCESS)
	{
		if(vkResult == VK_ERROR_OUT_OF_DATE_KHR || vkResult == VK_SUBOPTIMAL_KHR)
		{
			//if swapchain is out of date, then we need to resize the swapchain.
			//resize() will recreate swapchain and other related resources.
			fprintf(gpFile, "\nDEBUG:[display][ERROR] Swapchain is out of date, resizing required");
			resize(winWidth, winHeight);
		}
		else
		{
			fprintf(gpFile, "\nDEBUG:[display][ERROR] Failed vkAcquireNextImageKHR() = %d", vkResult);
			return vkResult;
		}
		
	}
	
	//use fence to allow to host to wait to completion of exexcution of previous command buffer
	//ie: to synchronize between rendering and presentation.
	vkResult = vkWaitForFences(vkDevice, 1, &vkFence_array[currentImageIndex], VK_TRUE, UINT64_MAX);
	if(vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nDEBUG:[display][ERROR] Failed vkWaitForFences() = %d", vkResult);
		return vkResult;
	}

	//make ready the fences for execution of next command buffers
	vkResult = vkResetFences(vkDevice, 1, &vkFence_array[currentImageIndex]); 
	if(vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nDEBUG:[display][ERROR] Failed vkResetFences() = %d", vkResult);
		return vkResult;
	}

	const VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	//declare memset and initialize VkSubmitInfo structure.
	VkSubmitInfo vkSubmitInfo;
	memset((void *)&vkSubmitInfo, 0, sizeof(VkSubmitInfo));

	vkSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	vkSubmitInfo.pNext = NULL;
	vkSubmitInfo.pWaitDstStageMask = &waitDstStageMask; //VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	vkSubmitInfo.waitSemaphoreCount = 1;
	vkSubmitInfo.pWaitSemaphores = &vkSemaphore_backBuffer;
	vkSubmitInfo.commandBufferCount = 1;
	vkSubmitInfo.pCommandBuffers = &vkCommandBuffer_array[currentImageIndex];
	vkSubmitInfo.signalSemaphoreCount = 1;
	vkSubmitInfo.pSignalSemaphores = &vkSemaphore_renderComplete;

	//submit command buffer to queue for execution.
	vkResult = vkQueueSubmit(vkQueue, 1, &vkSubmitInfo, vkFence_array[currentImageIndex]);
	if(vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nDEBUG:[display][ERROR] Failed vkQueueSubmit() = %d", vkResult);
		return vkResult;
		//VK_ERROR_DEVICE_LOST; 
	}

	//present the image to the screen.
	VkPresentInfoKHR vkPresentInfoKHR;
	memset((void *)&vkPresentInfoKHR, 0, sizeof(VkPresentInfoKHR));

	vkPresentInfoKHR.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	vkPresentInfoKHR.pNext = NULL;
	vkPresentInfoKHR.swapchainCount		= 1;//swapchainImageCount;
	vkPresentInfoKHR.pSwapchains = &vkSwapchainKHR;
	vkPresentInfoKHR.pImageIndices = &currentImageIndex;
	vkPresentInfoKHR.waitSemaphoreCount = 1;
	vkPresentInfoKHR.pWaitSemaphores = &vkSemaphore_renderComplete;
	//vkPresentInfoKHR.pResults = NULL; //NULL : not required
	
	vkResult = vkQueuePresentKHR(vkQueue, &vkPresentInfoKHR);
	if(vkResult != VK_SUCCESS)
	{
		if(vkResult == VK_ERROR_OUT_OF_DATE_KHR || vkResult == VK_SUBOPTIMAL_KHR)
		{
			//if swapchain is out of date, then we need to resize the swapchain.
			//resize() will recreate swapchain and other related resources.
			fprintf(gpFile, "\nDEBUG:[display][ERROR] Swapchain is out of date, resizing required");
			resize(winWidth, winHeight);
		}
		else
		{
			fprintf(gpFile, "\nDEBUG:[display][ERROR] Failed vkQueuePresentKHR() = %d", vkResult);
			return vkResult;
		}
	}

	vkResult = updateUniformBuffer();
	if(vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nDEBUG:[display][ERROR] Failed updateUniformBuffer() = %d", vkResult);
		return vkResult;
	}

	vkResult = vkQueueWaitIdle(vkQueue);
	return vkResult;
}

void update()
{
	angle = angle + 2.0f; // Increment the rotation angle
	if(angle >= 360.0f)
	{
		angle = angle - 360.0f; // Reset the angle to avoid overflow
	}
}

void uninitialize(void)
{
	fprintf(gpFile, "\n\nDEBUG:------------------------------------Inside[uninitialize()]------------------------------------");
	//closing file
	if(gbFullscreen == TRUE)
	{
		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);
		SetWindowLong(ghwnd, GWL_STYLE, (dwStyle | WS_OVERLAPPEDWINDOW));
		SetWindowPos(ghwnd,
					HWND_TOP,
					0,
					0,
					0,
					0,
					SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);
		ShowCursor(TRUE);
	}
	

	//no need to destroy/uninitialise device queue
	//destroy vulkan device
	if(vkDevice)
	{
		vkDeviceWaitIdle(vkDevice);
		fprintf(gpFile, "\nDEBUG:[uninitialize()][SUCCESS] vkDeviceWaitIdle()");
		
		//destroy fences
		if(vkFence_array)
		{
			for(uint32_t i = 0; i < swapchainImageCount; i++)
			{
				vkDestroyFence(vkDevice, vkFence_array[i], NULL);
				fprintf(gpFile, "\nDEBUG:[uninitialize()][SUCCESS] vkDestroyFence() = %d", i);
			}
			if(vkFence_array)
			{
				free(vkFence_array);
				vkFence_array = NULL;
			}
			fprintf(gpFile, "\nDEBUG:[uninitialize()][SUCCESS] vkFence_array freed");
			fflush(gpFile);
		}

		//destroy semaphores
		if(vkSemaphore_renderComplete)
		{
			vkDestroySemaphore(vkDevice, vkSemaphore_renderComplete, NULL);
			vkSemaphore_renderComplete = VK_NULL_HANDLE;
			fprintf(gpFile, "\nDEBUG:[uninitialize()][SUCCESS] vkDestroySemaphore() = vkSemaphore_renderComplete");
		}
		if(vkSemaphore_backBuffer)
		{
			vkDestroySemaphore(vkDevice, vkSemaphore_backBuffer, NULL);
			vkSemaphore_backBuffer = VK_NULL_HANDLE;
			fprintf(gpFile, "\nDEBUG:[uninitialize()][SUCCESS] vkDestroySemaphore() = vkSemaphore_backBuffer");
		}

		//destroy pipeline
		if(vkPipeline)
		{
			vkDestroyPipeline(vkDevice, vkPipeline, NULL);
			vkPipeline = VK_NULL_HANDLE;
			fprintf(gpFile, "\nDEBUG:[uninitialize()][SUCCESS] vkDestroyPipeline()");
		}
		//destroy frame buffers
		if(vkFramebuffer_array)
		{
			for(uint32_t i = 0; i < swapchainImageCount; i++)
			{
				vkDestroyFramebuffer(vkDevice, vkFramebuffer_array[i], NULL);
				fprintf(gpFile, "\nDEBUG:[uninitialize()][SUCCESS] vkDestroyFramebuffer() = %d", i);
			}
			if(vkFramebuffer_array)
			{
				free(vkFramebuffer_array);
				vkFramebuffer_array = NULL;
			}
			fprintf(gpFile, "\nDEBUG:[uninitialize()][SUCCESS] vkFramebuffer_array freed");
			fflush(gpFile);
		}

		//destroy shaders
		if(vkShaderModule_fragment_shader)
		{
			vkDestroyShaderModule(vkDevice, vkShaderModule_fragment_shader, NULL);
			vkShaderModule_fragment_shader = VK_NULL_HANDLE;
			fprintf(gpFile, "\nDEBUG:[uninitialize()][SUCCESS] vkDestroyShaderModule() = vkShaderModule_fragment_shader");
		}

		if(vkShaderModule_vertex_shader)
		{
			vkDestroyShaderModule(vkDevice, vkShaderModule_vertex_shader, NULL);
			vkShaderModule_vertex_shader = VK_NULL_HANDLE;
			fprintf(gpFile, "\nDEBUG:[uninitialize()][SUCCESS] vkDestroyShaderModule() = vkShaderModule_vertex_shader");
		}

		
		//destroy uniform buffers
		if(uniformData_cube.vkBuffer)
		{
			vkDestroyBuffer(vkDevice, uniformData_cube.vkBuffer, NULL);
			uniformData_cube.vkBuffer = VK_NULL_HANDLE;
			fprintf(gpFile, "\nDEBUG:[uninitialize()][SUCCESS] vkDestroyBuffer() = uniform.vkBuffer");
		}

		//destroy uniform memory
		if(uniformData_cube.vkDeviceMemory)
		{
			vkFreeMemory(vkDevice, uniformData_cube.vkDeviceMemory, NULL);
			uniformData_cube.vkDeviceMemory = VK_NULL_HANDLE;
			fprintf(gpFile, "\nDEBUG:[uninitialize()][SUCCESS] vkFreeMemory() = uniform.vkDeviceMemory");
		}

		//destroy pipeline layout
		if(vkPipelineLayout)
		{
			vkDestroyPipelineLayout(vkDevice, vkPipelineLayout, NULL);
			vkPipelineLayout = VK_NULL_HANDLE;
			fprintf(gpFile, "\nDEBUG:[uninitialize()][SUCCESS] vkDestroyPipelineLayout()");
		}
		
		//texture buffer
		if(vertexData_texCoords.vkBuffer)
		{
			vkDestroyBuffer(vkDevice, vertexData_texCoords.vkBuffer, NULL);
			vertexData_texCoords.vkBuffer = VK_NULL_HANDLE;
			fprintf(gpFile, "\nDEBUG:[uninitialize()][SUCCESS] vkDestroyBuffer() = vertexData_texCoords.vkBuffer");
		}

		if(vertexData_texCoords.vkDeviceMemory)
		{
			vkFreeMemory(vkDevice, vertexData_texCoords.vkDeviceMemory, NULL);
			vertexData_texCoords.vkDeviceMemory = VK_NULL_HANDLE;
			fprintf(gpFile, "\nDEBUG:[uninitialize()][SUCCESS] vkFreeMemory() = vertexData_texCoords.vkDeviceMemory");
		}


		if(vkSampler_texture)
		{
			vkDestroySampler(vkDevice, vkSampler_texture, NULL);
			vkSampler_texture = VK_NULL_HANDLE;
			fprintf(gpFile, "\nDEBUG:[uninitialize()][SUCCESS] vkDestroySampler() = vkSampler_texture");
		}

		if(vkImageView_texture)
		{
			vkDestroyImageView(vkDevice, vkImageView_texture, NULL);
			vkImageView_texture = VK_NULL_HANDLE;
			fprintf(gpFile, "\nDEBUG:[uninitialize()][SUCCESS] vkDestroyImageView() = vkImageView_texture");
		}

		if(vkDeviceMemory_texture)
		{
			vkFreeMemory(vkDevice, vkDeviceMemory_texture, NULL);
			vkDeviceMemory_texture = VK_NULL_HANDLE;
			fprintf(gpFile, "\nDEBUG:[uninitialize()][SUCCESS] vkFreeMemory() = vkDeviceMemory_texture");
		}

		if(vkImage_texture)
		{
			vkDestroyImage(vkDevice, vkImage_texture, NULL);
			vkImage_texture = VK_NULL_HANDLE;
			fprintf(gpFile, "\nDEBUG:[uninitialize()][SUCCESS] vkDestroyImage() = vkImage_texture");
		}
		//Vertex buffers
		if(vertexData_position.vkBuffer)
		{
			vkDestroyBuffer(vkDevice, vertexData_position.vkBuffer, NULL);
			vertexData_position.vkBuffer = VK_NULL_HANDLE;
			fprintf(gpFile, "\nDEBUG:[uninitialize()][SUCCESS] vkDestroyBuffer() = vertexData_position.vkBuffer");
		}

		if(vertexData_position.vkDeviceMemory)
		{
			vkFreeMemory(vkDevice, vertexData_position.vkDeviceMemory, NULL);
			vertexData_position.vkDeviceMemory = VK_NULL_HANDLE;
			fprintf(gpFile, "\nDEBUG:[uninitialize()][SUCCESS] vkFreeMemory() = vertexData_position.vkDeviceMemory");
		}

		//descriptor set layout
		if(vkDescriptorSetLayout)
		{
			vkDestroyDescriptorSetLayout(vkDevice, vkDescriptorSetLayout, NULL);
			vkDescriptorSetLayout = VK_NULL_HANDLE;
			fprintf(gpFile, "\nDEBUG:[uninitialize()][SUCCESS] vkDestroyDescriptorSetLayout()");
		}

		
		

		//destroy render pass
		if(vkRenderPass)
		{
			vkDestroyRenderPass(vkDevice, vkRenderPass, NULL);
			vkRenderPass = VK_NULL_HANDLE;
			fprintf(gpFile, "\nDEBUG:[uninitialize()][SUCCESS] vkDestroyRenderPass()");
		}

		//descriptor pool
		if(vkDescriptorPool)
		{
			vkDestroyDescriptorPool(vkDevice, vkDescriptorPool, NULL);
			vkDescriptorPool = VK_NULL_HANDLE;
			fprintf(gpFile, "\nDEBUG:[uninitialize()][SUCCESS] vkDestroyDescriptorPool()");
		}

		// if(vkDescriptorSet)
		// {
		// 	//free descriptor set
		// 	vkFreeDescriptorSets(vkDevice, NULL, 1, &vkDescriptorSet);
		// 	//vkFreeDescriptorSet(vkDevice, vkDescriptorSetLayout, 1, &vkDescriptorSet);
		// 	vkDescriptorSet = VK_NULL_HANDLE;
		// 	fprintf(gpFile, "\nDEBUG:[uninitialize()][SUCCESS] vkFreeDescriptorSets()");
		// }


		//commad bufffers
		if(vkCommandBuffer_array)
		{
			//for (uint32_t i = 0; i < 1; i++)
			//{
				vkFreeCommandBuffers(vkDevice, vkCommandPool, swapchainImageCount, vkCommandBuffer_array);
				fprintf(gpFile, "\nDEBUG:[uninitialize()][SUCCESS] vkFreeCommandBuffers()");

			//}
			if (vkCommandBuffer_array)
			{
				free(vkCommandBuffer_array);
				vkCommandBuffer_array = VK_NULL_HANDLE;
			}
			fflush(gpFile);
		}
		
		//command pool
		if(vkCommandPool)
		{
			vkDestroyCommandPool(vkDevice, vkCommandPool, NULL);
			vkCommandPool = VK_NULL_HANDLE;
			fprintf(gpFile, "\nDEBUG:[uninitialize()][SUCCESS] vkDestroyCommandPool()");
		}

		//destroy depth attachment
		if(vkImageView_depth)
		{
			vkDestroyImageView(vkDevice, vkImageView_depth, NULL);
			vkImageView_depth = VK_NULL_HANDLE;
			fprintf(gpFile, "\nDEBUG:[uninitialize()][SUCCESS] vkDestroyImageView() = vkImageView_depth");
		}

		//destroy device memory for depth attachment
		if(vkDeviceMemory_depth)
		{
			vkFreeMemory(vkDevice, vkDeviceMemory_depth, NULL);
			vkDeviceMemory_depth = VK_NULL_HANDLE;
			fprintf(gpFile, "\nDEBUG:[uninitialize()][SUCCESS] vkFreeMemory() = vkDeviceMemory_depth");
		}

		//destroy depth image
		if(vkImage_depth)
		{
			vkDestroyImage(vkDevice, vkImage_depth, NULL);
			vkImage_depth = VK_NULL_HANDLE;
			fprintf(gpFile, "\nDEBUG:[uninitialize()][SUCCESS] vkDestroyImage() = vkImage_depth");
		}

		//destroy image views
		for(uint32_t i = 0; i < swapchainImageCount; i++)
		{
			vkDestroyImageView(vkDevice, swapchainImageView_array[i], NULL);
			fprintf(gpFile, "\nDEBUG:[uninitialize()][SUCCESS] vkDestroyImageView() = %d", i);
		}
		
		// if(swapchainImageView_array)
		// {
		// 	free(swapchainImageView_array);
		// 	swapchainImageView_array = NULL;
		// 	fprintf(gpFile, "\nDEBUG:[uninitialize()][SUCCESS] swapchainImageView_array freed");
		// }

		// if(swapchainImageCount > 0)
		// {
		// 	for(uint32_t i = 0; i < swapchainImageCount; i++)
		// 	{
		// 		vkDestroyImage(vkDevice, swapchainImage_array[i], NULL);
		// 		fprintf(gpFile, "\nDEBUG:[uninitialize()][SUCCESS] vkDestroyImage() = %d", i);
				
		// 	}
		// 	//fclose(gpFile);
		// 	//gpFile = NULL;
		// 	fflush(gpFile);
		// }

		//destroy swapchain images
		if(swapchainImage_array)
		{
			free(swapchainImage_array);
			swapchainImage_array = NULL;
			fprintf(gpFile, "\nDEBUG:[uninitialize()][SUCCESS] swapchainImage_array freed");
		}
		
		//destroy swapchain
		if(vkSwapchainKHR)
		{
			vkDestroySwapchainKHR(vkDevice, vkSwapchainKHR, NULL);
			vkSwapchainKHR = VK_NULL_HANDLE;
			fprintf(gpFile, "\nDEBUG:[uninitialize()][SUCCESS] vkDestroySwapchainKHR()");
		}

		fflush(gpFile);
		
		//destroy vkSurfaceKHR
		if(vkSurfaceKHR)
		{
			vkDestroySurfaceKHR(vkInstance, vkSurfaceKHR, NULL);
			vkSurfaceKHR = VK_NULL_HANDLE;
			fprintf(gpFile, "\nDEBUG:[uninitialize()][SUCCESS] vkDestroySurfaceKHR()");
		}

		if(vkDebugReportCallbackEXT && vkDestroyDebugReportCallbackEXT_fnptr)
		{
			vkDestroyDebugReportCallbackEXT_fnptr(vkInstance, vkDebugReportCallbackEXT, NULL);
			vkDebugReportCallbackEXT = VK_NULL_HANDLE;
			fprintf(gpFile, "\nDEBUG:[uninitialize()][SUCCESS] vkDestroyDebugReportCallbackEXT()");
		}

		fflush(gpFile);
	
		//vkDevice
		vkDestroyDevice(vkDevice, NULL);
		vkDevice = VK_NULL_HANDLE;
		fprintf(gpFile, "\nDEBUG:[uninitialize()][SUCCESS] vkDestroyDevice()");

		fflush(gpFile);
	}

	//destroy vkinstance
	if(vkInstance)
	{
		vkDestroyInstance(vkInstance, NULL);
		vkInstance = VK_NULL_HANDLE;
		fprintf(gpFile, "\nDEBUG:[uninitialize()][SUCCESS] vkDestroyInstance()");
	}
	
	if(gpFile)
	{
		fprintf(gpFile, "\nDEBUG:------------------------------------Done [uninitialize()]------------------------------------\n");
		fprintf(gpFile, "\nDEBUG:[uninitialize()]File Closed\n");
		fclose(gpFile);
		gpFile = NULL;
	}

}


/*------------------------------------------------------------------------------------------------------
//////////////////////////////////////////////////////////////////////
//VULKAN RELATED FUNCTIONS
/////////////////////////////////////////////////////////////////////
------------------------------------------------------------------------------------------------------*/

/*------------------------------------------------------------------------------------------------------
3> STEPS: Instance Creation
--------------------------------------------------------------------------------------------------------
in initailise()
{
	1> As explained before, fill and initailise required extension names and count in global variable.
	2> initialize struct VkApplicationInfo.
	3> initialise struct VkInstanceCreateInfo by using information from step1 and step2
	4> call vkCreateInstance() to get vkInstance in global variable and do error checking.
}

5> destroy vkInstance in uninitialize().
------------------------------------------------------------------------------------------------------*/
VkResult createVulkanInstance(void)
{
	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	//function declaration
	VkResult fillInstanceExtensionNames(void);
	VkResult fillValidationLayerNames(void);
	VkResult createValidationCallbackFunction(void);
	
	//code
	fprintf(gpFile, "\n\nDEBUG:------------------------------------Inside[createVulkanInstance()]------------------------------------");
	//STEP 1:
	//fill and initailise required extension names and count in global
	vkResult = fillInstanceExtensionNames();
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[createVulkanInstance][ERROR] Failed fillInstanceExtensionNames()");
		return vkResult;
	}
	else
		fprintf(gpFile, "\nDEBUG:[createVulkanInstance][SUCCESS] fillInstanceExtensionNames()");

	
	
	//fill and initailise required validation layer names and count in global
	if(bValidationLayerSupport)
	{
		vkResult = fillValidationLayerNames();
		if(vkResult != VK_SUCCESS)
		{	
			fprintf(gpFile, "\nDEBUG:[createVulkanInstance][ERROR] Failed fillValidationLayerNames()");
			return vkResult;
		}
		else
		fprintf(gpFile, "\nDEBUG:[createVulkanInstance][SUCCESS] fillValidationLayerNames()");
	}

	
	
	//STEP 2
	//initialize struct VkApplicationInfo.
	VkApplicationInfo vkApplicationInfo;
	memset((void *)&vkApplicationInfo, 0, sizeof(vkApplicationInfo));

	vkApplicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	vkApplicationInfo.pNext = NULL;
	vkApplicationInfo.pApplicationName = gpszAppName;
	vkApplicationInfo.applicationVersion = 1;
	vkApplicationInfo.pEngineName = gpszAppName;
	vkApplicationInfo.engineVersion = 1;
	vkApplicationInfo.apiVersion = VK_API_VERSION_1_3;


	//STEP 3
	//3> initialise struct VkInstanceCreateInfo by using information from step1 and step2

	VkInstanceCreateInfo vkInstanceCreateInfo;
	memset((void *)&vkInstanceCreateInfo, 0, sizeof(vkInstanceCreateInfo));

	vkInstanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	vkInstanceCreateInfo.pNext = NULL;
	vkInstanceCreateInfo.pApplicationInfo = &vkApplicationInfo;
	vkInstanceCreateInfo.enabledExtensionCount = enabledInstanceExtensionCount;
	vkInstanceCreateInfo.ppEnabledExtensionNames = enabledInstanceExtensionNames_Array;
	
	if(bValidationLayerSupport)
	{
		vkInstanceCreateInfo.enabledLayerCount = enabledValidationLayerCount;
		vkInstanceCreateInfo.ppEnabledLayerNames = enabledValidationLayerNames_Array;
	}
	

	//STEP 4
	//call vkCreateInstance() to get vkInstance in global variable and do error checking.

	vkResult = vkCreateInstance(&vkInstanceCreateInfo, NULL, &vkInstance);
	if(vkResult == VK_ERROR_INCOMPATIBLE_DRIVER)
	{
		fprintf(gpFile, "\nDEBUG:[createVulkanInstance][ERROR] Failed VK_ERROR_UNCOMPATIBLE_DRIVER = %d", vkResult);
		return vkResult;
	}
	else if(vkResult == VK_ERROR_EXTENSION_NOT_PRESENT)
	{
		fprintf(gpFile, "\nDEBUG:[createVulkanInstance][ERROR] Failed VK_ERROR_EXTENSION_NOT_PRESENT = %d", vkResult);
		return vkResult;
	}
	else if(vkResult == VK_ERROR_EXTENSION_NOT_PRESENT)
	{
		fprintf(gpFile, "\nDEBUG:[createVulkanInstance][ERROR] Failed VK_ERROR_EXTENSION_NOT_PRESENT = %d", vkResult);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nDEBUG:[createVulkanInstance][SUCCESS]");
	}

	// do for validation callbacks
	if(bValidationLayerSupport == TRUE)
	{
		vkResult = createValidationCallbackFunction();
		if(vkResult != VK_SUCCESS)
		{	
			fprintf(gpFile, "\nDEBUG:[createVulkanInstance][ERROR] Failed createValidationCallbackFunction()");
			return vkResult;
		}
		else
		fprintf(gpFile, "\nDEBUG:[createVulkanInstance][SUCCESS] createValidationCallbackFunction()");
	}
	
	fprintf(gpFile, "\nDEBUG:------------------------------------Done [createVulkanInstance()]------------------------------------\n");
	return vkResult;
}
/*----------------------------------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------------------------------
2> STEPS: Instance Extensions
--------------------------------------------------------------------------------------------------------

STEPS: for instance extensions
1> Find how many instance extences are supported by vulkan driver of this version. 
   and keep the count in local variable.
2> Allocate and Fill struct VkExtensionProperties array corresponding to above count.
3> Fill and Display a local string array of extension names obtained from the VkExtensionProperties.
4> As not required onwords, here free VkExtensionProperties.
5> Find whether above extension names contains our 2 required extension
	1> VK_KHR_SURFACE_EXTENSION_NAME
	2> VK_KHR_WIN32_SURFACE_EXTENSION_NAME
	Accordingly set two global variables 1-> reqired extension count 2-> reqired extensions array

6> as not needed hencefore, free local string array.
7> print whether our vulkan driver supports the required extensions or not.
8> print only supported extension names. 

Steps for instance creation
in initailise()
{
	1> As explained before, fill and initailise required extension names and count in global variable.
	2> initialize struct VkApplicationInfo.
	3> initialise struct VkInstanceCreateInfo by using information from step1 and step2
	4> call vkCreateInstance() to get vkInstance in global variable and do error checking.
}

5> destroy vkInstance in uninitialize().
------------------------------------------------------------------------------------------------------*/
VkResult fillInstanceExtensionNames(void)
{
	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	//code
	fprintf(gpFile, "\n\nDEBUG:------------------------------------Inside[fillInstanceExtensionNames()]------------------------------------");
	//STEP 1:
	//Find how many instance extences are supported 
	//by vulkan driver of this version. and keep the count in local variable.
	
	uint32_t instanceExtensionCount = 0;
	vkResult = vkEnumerateInstanceExtensionProperties(NULL, &instanceExtensionCount, NULL);
	//NULL : layers/ we need all extensions by driver not by specific layer
	//instanceExtensionCount : fill/out varaible
	//NULL : Array to fill
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[fillInstanceExtensionNames][ERROR] Failed vkEnumerateInstanceExtensionProperties() first call");
		return vkResult;
	}
	else
		fprintf(gpFile, "\nDEBUG:[fillInstanceExtensionNames][SUCCESS] vkEnumerateInstanceExtensionProperties() first call");

	VkExtensionProperties *vkExtensionProperties_array = NULL;

	//STEP 2
	//2> Allocate and Fill struct VkExtensionProperties array corresponding to above count.
	vkExtensionProperties_array = (VkExtensionProperties *)malloc(sizeof(VkExtensionProperties) * instanceExtensionCount);
	//add error checking for malloc

	vkResult = vkEnumerateInstanceExtensionProperties(NULL, &instanceExtensionCount, vkExtensionProperties_array);
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[fillInstanceExtensionNames][ERROR] Failed vkEnumerateInstanceExtensionProperties() Second call");
		return vkResult;
	}
	else
		fprintf(gpFile, "\nDEBUG:[fillInstanceExtensionNames][SUCCESS] vkEnumerateInstanceExtensionProperties() Second call");
	
	//STEP 3
	//3> Fill and Display a local string array of extension names obtained from the VkExtensionProperties.
	char** instanceExtensionNames_array = NULL;

	instanceExtensionNames_array = (char **) malloc (sizeof(char *) * instanceExtensionCount);
	//add error checking for malloc()
	
	for(uint32_t i = 0; i < instanceExtensionCount; i++)
	{
		//allocate memory for each string and copy the string from vkExtensionProperties_array to instanceExtensionNames_array
		instanceExtensionNames_array[i] = (char *)malloc(sizeof(char) * strlen(vkExtensionProperties_array[i].extensionName) + 1);
		//add error checking for malloc()

		//copy the string from vkExtensionProperties_array to instanceExtensionNames_array
		memcpy(instanceExtensionNames_array[i], vkExtensionProperties_array[i].extensionName, strlen(vkExtensionProperties_array[i].extensionName) + 1);

		//print the extension name
		fprintf(gpFile, "\nDEBUG:[fillInstanceExtensionNames][INFO] Vulkan instance Extension Name[%d] = %s", i, instanceExtensionNames_array[i]);
	}

	//STEP 4
	//4> As not required onwords, here free VkExtensionProperties.
	free(vkExtensionProperties_array);
	vkExtensionProperties_array = NULL;

	//STEP 5
	//5> Find whether above extension names contains our 2 required extension
	//1> VK_KHR_SURFACE_EXTENSION_NAME
	//2> VK_KHR_WIN32_SURFACE_EXTENSION_NAME
	//Accordingly set two global variables 1-> reqired extension count 2-> reqired extensions array

	VkBool32 vulkanSurfaceExtensionFound = VK_FALSE;
	VkBool32 vulkanWin32SurfaceExtensionFound = VK_FALSE;
	VkBool32 debugReportExtensionFound = VK_FALSE;

	for(uint32_t i = 0; i < instanceExtensionCount; i++)
	{
		if(strcmp(instanceExtensionNames_array[i], VK_KHR_SURFACE_EXTENSION_NAME) == 0)
		{
			vulkanSurfaceExtensionFound = VK_TRUE;
			enabledInstanceExtensionNames_Array[enabledInstanceExtensionCount++] = VK_KHR_SURFACE_EXTENSION_NAME;
		}

		if(strcmp(instanceExtensionNames_array[i], VK_KHR_WIN32_SURFACE_EXTENSION_NAME) == 0)
		{
			vulkanWin32SurfaceExtensionFound = VK_TRUE;
			enabledInstanceExtensionNames_Array[enabledInstanceExtensionCount++] = VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
		}

		if(strcmp(instanceExtensionNames_array[i], VK_EXT_DEBUG_REPORT_EXTENSION_NAME) == 0) //VK_EXT_debug_report
		{
			fprintf(gpFile, "\nDEBUG:[fillInstanceExtensionNames][INFO] VK_EXT_DEBUG_REPORT_EXTENSION_NAME found");
			if(bValidationLayerSupport == VK_TRUE)
			{
				
				debugReportExtensionFound = VK_TRUE;
				fprintf(gpFile, "\nDEBUG:[fillInstanceExtensionNames][INFO] VK_EXT_DEBUG_REPORT_EXTENSION_NAME found");
				enabledInstanceExtensionNames_Array[enabledInstanceExtensionCount++] = VK_EXT_DEBUG_REPORT_EXTENSION_NAME;
			}
			else
			{
				//array will not have entry of VK_EXT_DEBUG_REPORT_EXTENSION_NAME
				fprintf(gpFile, "\nDEBUG:[fillInstanceExtensionNames][ERROR] VK_EXT_DEBUG_REPORT_EXTENSION_NAME not found");
			}
		}
	}

	
	//STEP 6
	//6> as not needed hencefore, free local string array.
	for(uint32_t i = 0; i < instanceExtensionCount; i++)
	{
		free((void *)instanceExtensionNames_array[i]);
	}
	free(instanceExtensionNames_array);
	//enabledInstanceExtensionNames_Array = NULL;

	//STEP 7
	//7> print whether our vulkan driver supports the required extensions or not.
	if(vulkanSurfaceExtensionFound == VK_FALSE)
	{
		vkResult = VK_ERROR_INITIALIZATION_FAILED; //return hardcoded failure
		fprintf(gpFile, "\nDEBUG:[fillInstanceExtensionNames][ERROR] VK_KHR_SURFACE_EXTENSION_NAME not found");
		return vkResult;
	}
	else
		fprintf(gpFile, "\nDEBUG:[fillInstanceExtensionNames][SUCCESS] VK_KHR_SURFACE_EXTENSION_NAME found");
	
	if(vulkanWin32SurfaceExtensionFound == VK_FALSE)
	{
		vkResult = VK_ERROR_INITIALIZATION_FAILED; //return hardcoded failure
		fprintf(gpFile, "\nDEBUG:[fillInstanceExtensionNames][ERROR] VK_KHR_WIN32_SURFACE_EXTENSION_NAME not found");
		return vkResult;
	}
	else
		fprintf(gpFile, "\nDEBUG:[fillInstanceExtensionNames][SUCCESS] VK_KHR_WIN32_SURFACE_EXTENSION_NAME found");

	//check for debug report extension only if validation layer is supported.
	//if validation layer is not supported, then no need to check for debug report extension.

	if(debugReportExtensionFound == VK_FALSE)
	{
		if(bValidationLayerSupport == VK_TRUE)
		{
			vkResult = VK_ERROR_INITIALIZATION_FAILED; //return hardcoded failure
			fprintf(gpFile, "\nDEBUG:[fillInstanceExtensionNames][ERROR] Validation is On but VK_EXT_DEBUG_REPORT_EXTENSION_NAME not Supported");
			return vkResult;
		}
		else
		{
			fprintf(gpFile, "\nDEBUG:[fillInstanceExtensionNames][INFO]  Validation is Off, VK_EXT_DEBUG_REPORT_EXTENSION_NAME not Supported");
		}	
	}
	else
	{
		if(bValidationLayerSupport == VK_TRUE)
			fprintf(gpFile, "\nDEBUG:[fillInstanceExtensionNames][INFO] Validation is On and VK_EXT_DEBUG_REPORT_EXTENSION_NAME Supported");
		else
			fprintf(gpFile, "\nDEBUG:[fillInstanceExtensionNames][ERROR] VK_EXT_DEBUG_REPORT_EXTENSION_NAME is Supported but Validation is Off");
	}
		
	
	//STEP 8
	//8> print only supported/enabled extension names.
	for(uint32_t i = 0; i < enabledInstanceExtensionCount; i++)
	{
		fprintf(gpFile, "\nDEBUG:[fillInstanceExtensionNames][INFO] Enabled Extension Name [%d]= %s", i, enabledInstanceExtensionNames_Array[i]);
	}

	fprintf(gpFile, "\nDEBUG:------------------------------------Done [fillInstanceExtensionNames()]------------------------------------\n");
	return vkResult;
}
/*----------------------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------------------*/
VkResult fillValidationLayerNames(void)
{
	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	//code
	fprintf(gpFile, "\n\nDEBUG:------------------------------------Inside[fillValidationLayerNames()]------------------------------------");

	uint32_t validationLayerCount = 0;
	vkResult = vkEnumerateInstanceLayerProperties(&validationLayerCount, NULL);
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[fillValidationLayerNames][ERROR] Failed vkEnumerateInstanceLayerProperties() first call");
		return vkResult;
	}
	else
		fprintf(gpFile, "\nDEBUG:[fillValidationLayerNames][SUCCESS] vkEnumerateInstanceLayerProperties() first call");

	VkLayerProperties *vkLayerProperties_array = NULL;
	//2> Allocate and Fill struct VkLayerProperties array corresponding to above count.
	vkLayerProperties_array = (VkLayerProperties *)malloc(sizeof(VkLayerProperties) * validationLayerCount);
	//add error checking for malloc

	vkResult = vkEnumerateInstanceLayerProperties(&validationLayerCount, vkLayerProperties_array);
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[fillValidationLayerNames][ERROR] Failed vkEnumerateInstanceLayerProperties() Second call");
		return vkResult;
	}
	else
		fprintf(gpFile, "\nDEBUG:[fillValidationLayerNames][SUCCESS] vkEnumerateInstanceLayerProperties() Second call");
	
	char **validationLayerNames_array = NULL;

	validationLayerNames_array = (char **) malloc (sizeof(char *) * validationLayerCount);
	//add error checking for malloc()
	
	for(uint32_t i = 0; i < validationLayerCount; i++)
	{
		//get name of validation layer
		validationLayerNames_array[i] = (char *)malloc(sizeof(char) * strlen(vkLayerProperties_array[i].layerName) + 1);
		//add error checking for malloc()
		
		//copy name of validation layer to local string array
		memcpy(validationLayerNames_array[i], vkLayerProperties_array[i].layerName, strlen(vkLayerProperties_array[i].layerName) + 1);
		
		//print validation layer name
		fprintf(gpFile, "\nDEBUG:[fillValidationLayerNames][INFO] Vulkan Validation Layer Name[%d] = %s", i, validationLayerNames_array[i]);
	}

	free(vkLayerProperties_array);
	vkLayerProperties_array = NULL;

	VkBool32 validationLayerFound = VK_FALSE;

	for(uint32_t i = 0; i < validationLayerCount; i++)
	{
		if(strcmp(validationLayerNames_array[i], "VK_LAYER_KHRONOS_validation") == 0)
		{
			validationLayerFound = VK_TRUE;
			enabledValidationLayerNames_Array[enabledValidationLayerCount++] = "VK_LAYER_KHRONOS_validation";
		}
	}


	for(uint32_t i = 0; i < validationLayerCount; i++)
	{
		free((void *)validationLayerNames_array[i]);
	}
	free(validationLayerNames_array);
	validationLayerNames_array = NULL;

	if(validationLayerFound == VK_FALSE)
	{
		vkResult = VK_ERROR_INITIALIZATION_FAILED; //return hardcoded failure
		fprintf(gpFile, "\nDEBUG:[fillValidationLayerNames][ERROR] VK_LAYER_KHRONOS_validation not supported");
		return vkResult;
	}
	else
		fprintf(gpFile, "\nDEBUG:[fillValidationLayerNames][SUCCESS] VK_LAYER_KHRONOS_validation supported");

	
	//3> Display a string array of extension names obtained from the VkLayerProperties.
	for(uint32_t i = 0; i < enabledValidationLayerCount; i++)
	{
		fprintf(gpFile, "\nDEBUG:[fillValidationLayerNames][INFO] Enabled Validation Layer Name [%d]= %s", i, enabledValidationLayerNames_Array[i]);
	}
	
	fprintf(gpFile, "\nDEBUG:------------------------------------Done [fillValidationLayerNames()]------------------------------------\n");
	return vkResult;
}
/*----------------------------------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------------------------------*/
VkResult createValidationCallbackFunction(void)
{
	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	//function declaration
	VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallback(
		
		VkDebugReportFlagsEXT ,				////flags for error, warning, etc
		VkDebugReportObjectTypeEXT ,		//object type of the object that generated the message
		uint64_t ,							//object that generated the message
		size_t ,							//location of the message in the object
		int32_t ,							//message code of the message
		const char *,						//prefix of the layer that generated the message
		const char *,						//message string
		void *								//user data passed to the callback function
	);

	PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT_fnptr = NULL;

	//code
	fprintf(gpFile, "\n\nDEBUG:------------------------------------Inside[createValidationCallbackFunction()]------------------------------------");

	//get the required function pointers vkCreateDebugReportCallbackEXT_fnptr
	vkCreateDebugReportCallbackEXT_fnptr = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(vkInstance, "vkCreateDebugReportCallbackEXT");

	if(vkCreateDebugReportCallbackEXT_fnptr == NULL)
	{
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		fprintf(gpFile, "\nDEBUG:[createValidationCallbackFunction][ERROR] vkGetInstanceProcAddr() failed to get vkCreateDebugReportCallbackEXT function pointer");
		return vkResult;
	}
	else
		fprintf(gpFile, "\nDEBUG:[createValidationCallbackFunction][SUCCESS] vkGetInstanceProcAddr() = vkCreateDebugReportCallbackEXT function pointer");


	//get the required function pointers vkDestroyDebugReportCallbackEXT_fnptr
	vkDestroyDebugReportCallbackEXT_fnptr = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(vkInstance, "vkDestroyDebugReportCallbackEXT");
	if(vkDestroyDebugReportCallbackEXT_fnptr == NULL)
	{
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		fprintf(gpFile, "\nDEBUG:[createValidationCallbackFunction][ERROR] vkGetInstanceProcAddr() failed to get vkDestroyDebugReportCallbackEXT function pointer");
		return vkResult;
	}
	else
		fprintf(gpFile, "\nDEBUG:[createValidationCallbackFunction][SUCCESS] vkGetInstanceProcAddr() = vkDestroyDebugReportCallbackEXT function pointer");
	
	//get the vulkan debug report callback object
	VkDebugReportCallbackCreateInfoEXT vkDebugReportCallbackCreateInfoEXT;
	memset((void *)&vkDebugReportCallbackCreateInfoEXT, 0, sizeof(VkDebugReportCallbackCreateInfoEXT));

	vkDebugReportCallbackCreateInfoEXT.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	vkDebugReportCallbackCreateInfoEXT.pNext = NULL;
	vkDebugReportCallbackCreateInfoEXT.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	vkDebugReportCallbackCreateInfoEXT.pfnCallback = debugReportCallback;
	vkDebugReportCallbackCreateInfoEXT.pUserData = NULL; //user data passed to the callback function

	vkResult = vkCreateDebugReportCallbackEXT_fnptr(vkInstance, &vkDebugReportCallbackCreateInfoEXT, NULL, &vkDebugReportCallbackEXT);
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[createValidationCallbackFunction][ERROR] Failed vkCreateDebugReportCallbackEXT_fnptr()");
		return vkResult;
	}
	else
		fprintf(gpFile, "\nDEBUG:[createValidationCallbackFunction][SUCCESS] vkCreateDebugReportCallbackEXT_fnptr()");
	

	fprintf(gpFile, "\nDEBUG:------------------------------------Done [createValidationCallbackFunction()]------------------------------------\n");
	return vkResult;
}
/*----------------------------------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------------------------------
Presetation Surface Steps
--------------------------------------------------------------------------------------------------------
STEPS>
1> Declare a global varible to hold presentation surface object.
2> Declare and memset platform specific windows, Linux, Android, etc surfaceCreateInfoStructure.
3> Initialize it perticularly its hInstance and hwnd members
4> Now Call vkCreateWin32SurfaceKHR() to create presentation surface object.
5> Chages in uninitialize()
------------------------------------------------------------------------------------------------------*/
VkResult getSupportedSurface(void)
{
	//variable declaration
	VkResult vkResult;

	//code
	fprintf(gpFile, "\n\nDEBUG:------------------------------------Inside[getSupportedSurface()]------------------------------------");
	
	//2> Declare and memset platform specific windows, Linux, Android, etc surfaceCreateInfoStructure.
	VkWin32SurfaceCreateInfoKHR vkWin32SurfaceCreateInfoKHR;

	memset((void*)&vkWin32SurfaceCreateInfoKHR, 0, sizeof(VkWin32SurfaceCreateInfoKHR));

	//VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR
	vkWin32SurfaceCreateInfoKHR.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	vkWin32SurfaceCreateInfoKHR.pNext = NULL;
	vkWin32SurfaceCreateInfoKHR.flags = 0;
	vkWin32SurfaceCreateInfoKHR.hinstance = (HINSTANCE)GetWindowLongPtr(ghwnd, GWLP_HINSTANCE);
	vkWin32SurfaceCreateInfoKHR.hwnd = ghwnd;
	
	vkResult = vkCreateWin32SurfaceKHR(vkInstance, &vkWin32SurfaceCreateInfoKHR, NULL, &vkSurfaceKHR);
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[getSupportedSurface][ERROR] Failed vkCreateWin32SurfaceKHR()");
		return vkResult;
	}
	else
		fprintf(gpFile, "\nDEBUG:[getSupportedSurface][SUCCESS] vkCreateWin32SurfaceKHR()");
	
	
	//3> Initialize it perticularly its hInstance and hwnd members
	//4> Now Call vkCreateWin32SurfaceKHR() to create presentation surface object.

	fprintf(gpFile, "\nDEBUG:------------------------------------Done [getSupportedSurface()]------------------------------------\n");
	return vkResult;
}
/*----------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------------
5> STEPS: Get Physical Device
--------------------------------------------------------------------------------------------------------
1> Declare 3 global variables.
	i>   For selected physical device.
	ii>  For selected Queue Family Index
	iii> For Physical Device's Properties(required in later steps)
2> Call vkEnumeratePhysicalDevices() to get physical device count.
3> Allocate VkPhysicalDeviceArray array according physical device count 
4> Call vkEnumeratePhysicalDevices() again to fill above array (physical device names).
5> Start Loop using physical device count and physical device array.
	NOTE: Declare a boolean bFound variable before this loop which will decide whether we found desired physical device or not.
	      Inside this Loop
		  a> Declare a local varible to loop count.
		  b> Call vkGetPhysicalDeviceQueueFamilyProperties() To initialize above queue count variable.
		  c> Declare and allocate VkQueueFamilyPropertiesArray array according to above queue count variable.
		  d> Call vkGetPhysicalDeviceQueueFamilyProperties() again to fill above array.
		  e> Declare VkBool32 type array and allocate it using above queue count variable.
		  f> start nested loop and fill VkBool32 type array By calling vkGetPhysicalDeviceSurfaceSupportKHR().
		  g> start another nested(inside main)loop and check whether physical device in its array with its Queue family "has" Graphics Bit or not.
		     if yes then this is selected physical device. Assign it global variable.
			 Similarly this index is selected queue family index, assign it to global variable also/too.
			 and set bFound = TRUE and break from from second nested(inside main) loop. 
		  h> Now we are back in main loop. So free the VkQueueFamilyPropertiesArray and VkBool32.
		  i> still being in main loop according to bFound variable break out of main loop.
		  j> Free physical device array.
6> Do error checking according to bFound.
7> memset the global physical device memory proprty structure.
8> Initialize above structure with vkGetPhysicalDeviceMemoryProperties.
9> Declare a local structure variable VkPhysicalDeviceFeatures. Memset it and initialize it by calling vkGetPhysicalDeviceFeatures().
10> By using tissilation Shader member of above structure check selected device's tissilation shader property/support.
11> By using geometry Shader member of above structure check selected device's geometry shader property/support.
12> There is no need destroy/free/uninitialize selected physical device because, later we will create vulkan logical device 
	which needs to be destroyed, which will destroy selected physical device.   
--------------------------------------------------------------------------------------------------------*/
VkResult getPhysicalDevice(void)
{
	//variable declaration
	VkResult vkResult = VK_SUCCESS;

	//code
	fprintf(gpFile, "\n\nDEBUG:------------------------------------Inside[getPhysicalDevice()]------------------------------------");	

	vkResult = vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, NULL);
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[getPhysicalDevice][ERROR] Failed vkEnumeratePhysicalDevices() First Call");
		return vkResult;
	}
	else if(physicalDeviceCount == 0)
	{	
		fprintf(gpFile, "\nDEBUG:[getPhysicalDevice][ERROR] Failed vkEnumeratePhysicalDevices() physicalDeviceCount = %d", physicalDeviceCount);
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}
	else
		fprintf(gpFile, "\nDEBUG:[getPhysicalDevice][SUCCESS] vkEnumeratePhysicalDevices() First Call");

	
	vkPhysicalDevice_array = (VkPhysicalDevice *)malloc(physicalDeviceCount * sizeof(VkPhysicalDevice));
	//handle malloc error checking

	vkResult = vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, vkPhysicalDevice_array);
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[getPhysicalDevice][ERROR] Failed vkEnumeratePhysicalDevices() Second Call");
		return vkResult;
	}
	else
		fprintf(gpFile, "\nDEBUG:[getPhysicalDevice][SUCCESS] vkEnumeratePhysicalDevices() Second Call");

	
	VkBool32 bFound = VK_FALSE;
	for(uint32_t i = 0; i < physicalDeviceCount; i++)
	{
		uint32_t queueCount = UINT32_MAX;

		//returns void.
		vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice_array[i], &queueCount, NULL);
		
		VkQueueFamilyProperties *vkQueueFamilyProperties_array = NULL;
		vkQueueFamilyProperties_array = (VkQueueFamilyProperties *)malloc(queueCount * sizeof(VkQueueFamilyProperties));
		//handle malloc error checking.

		vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice_array[i], &queueCount, vkQueueFamilyProperties_array);

		VkBool32 *isQueueSurfaceSupported_array = NULL;
		isQueueSurfaceSupported_array = (VkBool32 *)malloc(queueCount * sizeof(VkBool32));

		for(uint32_t j = 0; j < queueCount; j++)
		{
			vkGetPhysicalDeviceSurfaceSupportKHR(vkPhysicalDevice_array[i], j, vkSurfaceKHR, &isQueueSurfaceSupported_array[j]);
		}

		for(uint32_t j = 0; j < queueCount; j++)
		{
			if(vkQueueFamilyProperties_array[j].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				if(isQueueSurfaceSupported_array[j] == VK_TRUE)
				{
					vkPhysicalDevice_Selected = vkPhysicalDevice_array[i];
					graphicsQueueFamilyIndex_Selected = j;
					bFound = VK_TRUE;
					fprintf(gpFile, "\nDEBUG:[getPhysicalDevice][SUCCESS] vkPhysicalDevice_Selected = %d", i);
					fprintf(gpFile, "\nDEBUG:[getPhysicalDevice][SUCCESS] graphicsQueueFamilyIndex_Selected = %d", j);
					//break;
				}
			}
		}

		if(isQueueSurfaceSupported_array)
		{
			free(isQueueSurfaceSupported_array);
			isQueueSurfaceSupported_array = NULL;
			fprintf(gpFile, "\nDEBUG:[getPhysicalDevice][SUCCESS] Free isQueueSurfaceSupported_array");
		}

		if(vkQueueFamilyProperties_array)
		{
			free(vkQueueFamilyProperties_array);
			vkQueueFamilyProperties_array = NULL;
			fprintf(gpFile, "\nDEBUG:[getPhysicalDevice][SUCCESS] Free vkQueueFamilyProperties_array");
		}

		if(bFound == VK_TRUE)
			break;
	}

	if(bFound == VK_TRUE)
	{
		fprintf(gpFile, "\nDEBUG:[getPhysicalDevice][SUCCESS] with graphics enabled");
		
	}
	else
	{
		free(vkPhysicalDevice_array);
		vkPhysicalDevice_array = NULL;
		fprintf(gpFile, "\nDEBUG:[getPhysicalDevice][Failed] with graphics enabled");
		return VK_ERROR_INITIALIZATION_FAILED;
	}

	memset((void *)&vkPhysicalDeviceMemoryProperties, 0, sizeof(VkPhysicalDeviceMemoryProperties));

	vkGetPhysicalDeviceMemoryProperties(vkPhysicalDevice_Selected, &vkPhysicalDeviceMemoryProperties);


	VkPhysicalDeviceFeatures vkPhysicalDeviceFeatures;

	memset((void *)&vkPhysicalDeviceFeatures, 0, sizeof(VkPhysicalDeviceFeatures));

	vkGetPhysicalDeviceFeatures(vkPhysicalDevice_Selected, &vkPhysicalDeviceFeatures);

	if(vkPhysicalDeviceFeatures.tessellationShader)
	{
		fprintf(gpFile, "\nDEBUG:[getPhysicalDevice][INFO] Selected Physical Device Support Tissilation Shader");
	}
	else
	{
		fprintf(gpFile, "\nDEBUG:[getPhysicalDevice][INFO] Selected Physical Device Not Support Tissilation Shader");
	}

	if(vkPhysicalDeviceFeatures.geometryShader)
	{
		fprintf(gpFile, "\nDEBUG:[getPhysicalDevice][INFO] Selected Physical Device Support Geometry Shader");
	}
	else
	{
		fprintf(gpFile, "\nDEBUG:[getPhysicalDevice][INFO] Selected Physical Device Not Support Geometry Shader");
	}

	fprintf(gpFile, "\nDEBUG:------------------------------------Done [getPhysicalDevice()]------------------------------------\n");
	return vkResult;
}
/*----------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------------
6> STEPS: Print VK_INFO
--------------------------------------------------------------------------------------------------------
1> Remove local declaration of physical device count and physical device array and declare it globally, from getPhysicalDevice().
2> Accordingly remove physical device array freeing code block from if(bFound == VK_TRUE) block, and we will later write this freeing code block in print vk info function.
3> Write printVKInfo() user defined function with following steps:
	a> Start a loop using global physical device count and inside it declare and memset VK_PHYSICAL_DEVICE_PROPERTIES structure variable.
	b> Initialize this structure variable by calling vkGetPhysicalDeviceProperties vulkan API.
	c> Print vulkan API version using "apiVersion" member of the structure. this requires 3 vulkan macros.
	d> Print device name by using "deviceName" member of structure.
	e> Use "deviceType" member of structure in a switch case block and accordingly print device type.
	f> Print hexadecimal vendorID of device "vedorID" member of structure.
	g> Print hexadecimal deviceID using "deviceID" member of structure.
		NOTE: for sake of the completeness we can repeat step 5 (steps a tp h) from getPhysicalDevice() function.
			  but now instead of assingning selected queue and selected device print wheher this device support Graphics bit compute bit transfer bit using if-else-if blocks/ladder.
			  similarly we also can repeat device fetures from get physical device function and can print all around 50+ device features including tissilation and geometric shader support.
	h> Free physical device array here which we removed from if(bFound == VK_TRUE) block of getPhysicalDevice().
--------------------------------------------------------------------------------------------------------*/
VkResult printVkInfo(void)
{
	//variable declaration
	VkResult vkResult = VK_SUCCESS;

	//code
	fprintf(gpFile, "\n\nDEBUG:------------------------------------Inside[printVkInfo()]------------------------------------");
	for(uint32_t i = 0; i < physicalDeviceCount; i++)
	{	
		fprintf(gpFile, "\nDEBUG:[printVkInfo][INFO] ******************************DEVICE %d ******************************", i);
		VkPhysicalDeviceProperties  vkPhysicalDeviceProperties;
		memset((void *)&vkPhysicalDeviceProperties, 0, sizeof(VkPhysicalDeviceProperties));	

		vkGetPhysicalDeviceProperties(vkPhysicalDevice_array[i], &vkPhysicalDeviceProperties);

		//API Version
		uint32_t majorVersion = VK_API_VERSION_MAJOR(vkPhysicalDeviceProperties.apiVersion);
		uint32_t minorVersion = VK_API_VERSION_MINOR(vkPhysicalDeviceProperties.apiVersion);
		uint32_t patchVersion = VK_API_VERSION_PATCH(vkPhysicalDeviceProperties.apiVersion);
		
		fprintf(gpFile, "\nDEBUG:[printVkInfo][INFO] API Version = %d %d %d", majorVersion, minorVersion, patchVersion);

		//Device Name
		fprintf(gpFile, "\nDEBUG:[printVkInfo][INFO] Device Name = %s", vkPhysicalDeviceProperties.deviceName);

		//device type
		switch(vkPhysicalDeviceProperties.deviceType)
		{
			case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
				fprintf(gpFile, "\nDEBUG:[printVkInfo][INFO] Device Type = Integrated GPU (iGPU)");
				break;
			
			case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
				fprintf(gpFile, "\nDEBUG:[printVkInfo][INFO] Device Type = Descete GPU (dGPU)");
				break;
			
			case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
				fprintf(gpFile, "\nDEBUG:[printVkInfo][INFO] Device Type = Virtual GPU (vGPU)");
				break;

			case VK_PHYSICAL_DEVICE_TYPE_CPU:
				fprintf(gpFile, "\nDEBUG:[printVkInfo][INFO] Device Type = CPU");
				break;
			
			case VK_PHYSICAL_DEVICE_TYPE_OTHER:
				fprintf(gpFile, "\nDEBUG:[printVkInfo][INFO] Device Type = Other");
				break;
			
			default:
				fprintf(gpFile, "\nDEBUG:[printVkInfo][INFO] Device Type = UNKNOWN");
				break;
		}

		//vendor ID
		fprintf(gpFile, "\nDEBUG:[printVkInfo][INFO] Device Vendor ID = 0x%04x", vkPhysicalDeviceProperties.vendorID);

		//Device ID
		fprintf(gpFile, "\nDEBUG:[printVkInfo][INFO] Device ID = 0x%04x", vkPhysicalDeviceProperties.deviceID);
		fprintf(gpFile, "\nDEBUG:[printVkInfo][INFO] ********************************************************************");
	}

	//free physical device array
	if(vkPhysicalDevice_array)
	{
		free(vkPhysicalDevice_array);
		vkPhysicalDevice_array = NULL;
		fprintf(gpFile, "\nDEBUG:[printVkInfo][SUCCESS] Free vkPhysicalDevice_array");
	}
	
	fprintf(gpFile, "\nDEBUG:------------------------------------Done [printVkInfo()]------------------------------------\n");	
	return vkResult;

}
/*----------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------------
7> STEPS: Device Extensions
--------------------------------------------------------------------------------------------------------
Use the same logic/steps of getting instance extensions, we are going to retrive device extensions.
--------------------------------------------------------------------------------------------------------*/
VkResult fillDeviceExtensionNames(void)
{
	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	//code
	fprintf(gpFile, "\n\nDEBUG:------------------------------------Inside[fillDeviceExtensionNames()]------------------------------------");
	//STEP 1:
	//Find how many device extences are supported 
	//by vulkan driver of this version. and keep the count in local variable.
	
	uint32_t deviceExtensionCount = 0;
	vkResult = vkEnumerateDeviceExtensionProperties(vkPhysicalDevice_Selected, NULL, &deviceExtensionCount, NULL);
	//NULL : layers/ we need all extensions by driver not by specific layer
	//instanceExtensionCount : fill/out varaible
	//NULL : Array to fill
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[fillDeviceExtensionNames][ERROR] Failed vkEnumerateDeviceExtensionProperties() first call");
		return vkResult;
	}
	else
		fprintf(gpFile, "\nDEBUG:[fillDeviceExtensionNames][SUCCESS] vkEnumerateDeviceExtensionProperties() first call");

	VkExtensionProperties *vkExtensionProperties_array = NULL;

	//STEP 2
	//2> Allocate and Fill struct VkExtensionProperties array corresponding to above count.
	vkExtensionProperties_array = (VkExtensionProperties *)malloc(sizeof(VkExtensionProperties) * deviceExtensionCount);
	//add error checking for malloc

	vkResult = vkEnumerateDeviceExtensionProperties(vkPhysicalDevice_Selected, NULL, &deviceExtensionCount, vkExtensionProperties_array);
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[fillDeviceExtensionNames][ERROR] Failed vkEnumerateDeviceExtensionProperties() Second call");
		return vkResult;
	}
	else
		fprintf(gpFile, "\nDEBUG:[fillDeviceExtensionNames][SUCCESS] vkEnumerateDeviceExtensionProperties() Second call");
	
	//STEP 3
	//3> Fill and Display a local string array of extension names obtained from the VkExtensionProperties.
	char** deviceExtensionNames_array = NULL;

	deviceExtensionNames_array = (char **) malloc (sizeof(char *) * deviceExtensionCount);
	//add error checking for malloc()
	for(uint32_t i = 0; i < deviceExtensionCount; i++)
	{
		deviceExtensionNames_array[i] = (char *)malloc(sizeof(char) * strlen(vkExtensionProperties_array[i].extensionName) + 1);
		memcpy(deviceExtensionNames_array[i], vkExtensionProperties_array[i].extensionName, strlen(vkExtensionProperties_array[i].extensionName) + 1);
		fprintf(gpFile, "\nDEBUG:[fillDeviceExtensionNames][INFO] Vulkan Device Extension Name[%d] = %s", i, deviceExtensionNames_array[i]);
	}

	//STEP 4
	//4> As not required onwords, here free VkExtensionProperties.
	free(vkExtensionProperties_array);
	vkExtensionProperties_array = NULL;

	//STEP 5
	//5> Find whether above extension names contains our 2 required extension
	//1> VK_KHR_SURFACE_EXTENSION_NAME
	//2> VK_KHR_WIN32_SURFACE_EXTENSION_NAME
	//Accordingly set two global variables 1-> reqired extension count 2-> reqired extensions array

	VkBool32 vulkanSwapchainExtensionFound = VK_FALSE;

	for(uint32_t i = 0; i < deviceExtensionCount; i++)
	{
		if(strcmp(deviceExtensionNames_array[i], VK_KHR_SWAPCHAIN_EXTENSION_NAME) == 0)
		{
			vulkanSwapchainExtensionFound = VK_TRUE;
			enabledDeviceExtensionNames_Array[enabledDeviceExtensionCount++] = VK_KHR_SWAPCHAIN_EXTENSION_NAME;
		}
	}

	
	//STEP 6
	//6> as not needed hencefore, free local string array.
	for(uint32_t i = 0; i < deviceExtensionCount; i++)
	{
		free((void *)deviceExtensionNames_array[i]);
	}
	free(deviceExtensionNames_array);
	//enabledInstanceExtensionNames_Array = NULL;

	//STEP 7
	//7> print whether our vulkan driver supports the required extensions or not.
	if(vulkanSwapchainExtensionFound == VK_FALSE)
	{
		vkResult = VK_ERROR_INITIALIZATION_FAILED; //return hardcoded failure
		fprintf(gpFile, "\nDEBUG:[fillDeviceExtensionNames][ERROR] VK_KHR_SWAPCHAIN_EXTENSION_NAME not found");
		return vkResult;
	}
	else
		fprintf(gpFile, "\nDEBUG:[fillDeviceExtensionNames][SUCCESS] VK_KHR_SWAPCHAIN_EXTENSION_NAME found");

	//STEP 8
	//8> print only supported/enabled extension names.
	for(uint32_t i = 0; i < enabledDeviceExtensionCount; i++)
	{
		fprintf(gpFile, "\nDEBUG:[fillDeviceExtensionNames][INFO] Enabled Device Extension Name [%d]= %s", i, enabledDeviceExtensionNames_Array[i]);
	}

	fprintf(gpFile, "\nDEBUG:------------------------------------Done [fillDeviceExtensionNames()]------------------------------------\n");
	return vkResult;
}
/*----------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------------
8> STEPS: Vulkan Device
--------------------------------------------------------------------------------------------------------
1> Create userdefined function createVulkanDevice()
2> Call previviously created fillDeviceExtensionNames() in it.
3> Declare and initialize VkDeviceCreateInfoStructure.
4> Use previously obtained device extension count and device extension array to initialize this structure.
5> Now Call vkCreateDevice() vulkan API to create actual device and do error checking.
6> Destroy this device when done. 
	IMP NOTE: Before destroying the device ensure all operations on that device are finished. 
			  Till that wait on that device.
--------------------------------------------------------------------------------------------------------*/
VkResult createVulkanDevice(void)
{
	//function declaration
	VkResult fillDeviceExtensionNames(void);

	//variable declaration
	VkResult vkResult = VK_SUCCESS;

	//code
	fprintf(gpFile, "\n\nDEBUG:------------------------------------Inside[createVulkanDevice()]------------------------------------");

	//device extensions
	vkResult = fillDeviceExtensionNames();
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[cretaeVulkanDevice][ERROR] Failed fillDeviceExtensionNames()");
		return vkResult;
	}
	else
		fprintf(gpFile, "\nDEBUG:[cretaeVulkanDevice][SUCCESS] fillDeviceExtensionNames()");


	VkQueue presentQueue;
	float presentqueuePriority[1] = {1.0f};

	VkDeviceQueueCreateInfo vkQueueCreateInfo;
	memset((void *)&vkQueueCreateInfo, 0, sizeof(VkDeviceQueueCreateInfo));
	vkQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	vkQueueCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex_Selected;
	vkQueueCreateInfo.queueCount = 1;
	vkQueueCreateInfo.pQueuePriorities = presentqueuePriority;

	VkDeviceCreateInfo vkDeviceCreateInfo;
	memset((void*)&vkDeviceCreateInfo, 0, sizeof(VkDeviceCreateInfo));


	vkDeviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	vkDeviceCreateInfo.pNext = NULL;
	vkDeviceCreateInfo.flags = 0;
	vkDeviceCreateInfo.enabledExtensionCount = enabledDeviceExtensionCount;
	vkDeviceCreateInfo.ppEnabledExtensionNames = enabledDeviceExtensionNames_Array;
	vkDeviceCreateInfo.enabledLayerCount = 0;
	vkDeviceCreateInfo.ppEnabledLayerNames = NULL;
	vkDeviceCreateInfo.pEnabledFeatures = NULL;
	vkDeviceCreateInfo.queueCreateInfoCount = 1;
	vkDeviceCreateInfo.pQueueCreateInfos = &vkQueueCreateInfo;


	vkResult = vkCreateDevice(vkPhysicalDevice_Selected, &vkDeviceCreateInfo, NULL, &vkDevice);
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[cretaeVulkanDevice][ERROR] Failed vkCreateDevice()");
		return vkResult;
	}
	else
		fprintf(gpFile, "\nDEBUG:[cretaeVulkanDevice][SUCCESS] vkCreateDevice()");

		fprintf(gpFile, "\nDEBUG:------------------------------------Done [createVulkanDevice()]------------------------------------\n");
	return vkResult;
}
/*----------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------------
9> STEPS: Device Queue
--------------------------------------------------------------------------------------------------------
1> Call vkGetDeviceQueue() using newly created vkDevice, selected family index, 0th queue in that queue 
   family.
   NOTE: when we create vkdevice, vkqueue are created/constructed.
		 if so we destroy vkdevice results release vkqueues.
--------------------------------------------------------------------------------------------------------*/
void getDeviceQueue(void)
{
	//variable Declaration
	VkDeviceQueueCreateInfo vkQueueCreateInfo;

	//code
	fprintf(gpFile, "\n\nDEBUG:------------------------------------Inside[getDeviceQueue()]------------------------------------");
	vkGetDeviceQueue(vkDevice, graphicsQueueFamilyIndex_Selected, 0, &vkQueue);
	if(vkQueue == VK_NULL_HANDLE)
	{
		fprintf(gpFile, "\nDEBUG:[getDeviceQueue][ERROR] Failed vkGetDeviceQueue()");
		return;
	}
	else
		fprintf(gpFile, "\nDEBUG:[getDeviceQueue][SUCCESS] vkGetDeviceQueue()");

	fprintf(gpFile, "\nDEBUG:------------------------------------Done [getDeviceQueue()]------------------------------------\n");
}
/*----------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------------
10> Surface Format And Color Space 
--------------------------------------------------------------------------------------------------------
1> Call vkGetPhysicalDeviceSurfaceFormatsKHR() to retrieve the count of supported color formats.
2> Declare and allocate array of VkGetPhysicalDeviceSurfaceFormat
	VK_COLOR_SPACE_SRGB_NONLINEAR_KHR
	VK_FORMAT_B8G8R8A8_SRGB
3> Call again vkGetPhysicalDeviceSurfaceFormatsKHR() to fill array.
4> According to contents above filled array decide the surface color format and surface color space.
5> free the allocated array.
--------------------------------------------------------------------------------------------------------*/
VkResult getPhysicalDeviceSurfaceFormatAndColorSpace(void)
{
	//variable declaration
	VkResult vkResult = VK_SUCCESS;

	//code
	fprintf(gpFile, "\n\nDEBUG:------------------------------------Inside[getPhysicalDeviceSurfaceFormatAndColorSpace()]------------------------------------");

	//get the count of supported surface color formats
	uint32_t formatCount = 0;

	vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice_Selected, vkSurfaceKHR, &formatCount, NULL);
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[getPhysicalDeviceSurfaceFormatAndColorSpace][ERROR] Failed vkGetPhysicalDeviceSurfaceFormatsKHR() first count");
		return vkResult;
	}
	else if(formatCount == 0)
	{
		fprintf(gpFile, "\nDEBUG:[getPhysicalDeviceSurfaceFormatAndColorSpace][ERROR] Failed vkGetPhysicalDeviceSurfaceFormatsKHR() formatCount == 0");
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	else
		fprintf(gpFile, "\nDEBUG:[getPhysicalDeviceSurfaceFormatAndColorSpace][SUCCESS] vkGetPhysicalDeviceSurfaceFormatsKHR() first call");

	VkSurfaceFormatKHR *vkSurfaceFormatKHR_array = (VkSurfaceFormatKHR *)malloc(formatCount * sizeof(VkSurfaceFormatKHR));

	//filling the array
	vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice_Selected, vkSurfaceKHR, &formatCount, vkSurfaceFormatKHR_array);
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[getPhysicalDeviceSurfaceFormatAndColorSpace][ERROR] Failed vkGetPhysicalDeviceSurfaceFormatsKHR() second call");
		return vkResult;
	}
	else
		fprintf(gpFile, "\nDEBUG:[getPhysicalDeviceSurfaceFormatAndColorSpace][SUCCESS] vkGetPhysicalDeviceSurfaceFormatsKHR() second call");

	
	// for(uint32_t i = 0; i < formatCount; i++)
	// {
	// 	if(vkSurfaceFormatKHR_array[i].format == VK_FORMAT_UNDEFINED)
	// 	{
	// 		//vkPresentModeKHR = VK_PRESENT_MODE_MAILBOX_KHR;
	// 		fprintf(gpFile, "\nDEBUG:[getPhysicalDevicePresentModeKHR][INFO] vkGetPhysicalDeviceSurfacePresentModesKHR() vkPresentModeKHR = VK_PRESENT_MODE_MAILBOX_KHR");
	// 		break;
	// 	}
	// }

	//decide the surface color format first.
	if(formatCount == 1 && vkSurfaceFormatKHR_array[0].format == VK_FORMAT_UNDEFINED)
	{
		vkFormat_color = VK_FORMAT_B8G8R8A8_UNORM;
		fprintf(gpFile, "\nDEBUG:[getPhysicalDeviceSurfaceFormatAndColorSpace][SUCCESS]  vkFormat_color = VK_FORMAT_B8G8R8A8_UNORM");
	}
	else
	{
		vkFormat_color = vkSurfaceFormatKHR_array[0].format;
		fprintf(gpFile, "\nDEBUG:[getPhysicalDeviceSurfaceFormatAndColorSpace][SUCCESS] free vkSurfaceFormatKHR_array[]");
	}

	//decide the color space
	vkColorSpaceKHR = vkSurfaceFormatKHR_array[0].colorSpace;

	if(vkSurfaceFormatKHR_array)
	{
		free(vkSurfaceFormatKHR_array);
		vkSurfaceFormatKHR_array = NULL;
		fprintf(gpFile, "\nDEBUG:[getPhysicalDeviceSurfaceFormatAndColorSpace][SUCCESS] free vkSurfaceFormatKHR_array[]");
	}
	
	fprintf(gpFile, "\nDEBUG:------------------------------------Done [getPhysicalDeviceSurfaceFormatAndColorSpace()]------------------------------------\n");
	return vkResult;
}
/*----------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------------
11>Presetation Mode
--------------------------------------------------------------------------------------------------------
1> Call vkGetPhysicalDeviceSurfacePresentModesKHR() to retrieve the count of supported presentation modes.
2> Declare VkPresentModeKHR structure array varaible.
3> Call again vkGetPhysicalDeviceSurfacePresentModesKHR() to fill VkPresentModeKHR array.
4> According to contents above filled array decide the presentation mode.
5> free the allocated array.
--------------------------------------------------------------------------------------------------------*/
VkResult getPhysicalDevicePresentModeKHR(void)
{
	//variable declaration
	VkResult vkResult = VK_SUCCESS;

	//code
	fprintf(gpFile, "\n\nDEBUG:------------------------------------Inside[getPhysicalDevicePresentModeKHR()]------------------------------------");

	uint32_t presentModeCount = 0;
	vkResult = vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice_Selected, vkSurfaceKHR, &presentModeCount, NULL);
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[getPhysicalDevicePresentModeKHR][ERROR] Failed vkGetPhysicalDeviceSurfacePresentModesKHR() first count");
		return vkResult;
	}
	else if(presentModeCount == 0)
	{
		fprintf(gpFile, "\nDEBUG:[getPhysicalDevicePresentModeKHR][ERROR] Failed vkGetPhysicalDeviceSurfacePresentModesKHR() presentModeCount == 0");
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	else
		fprintf(gpFile, "\nDEBUG:[getPhysicalDevicePresentModeKHR][SUCCESS] vkGetPhysicalDeviceSurfacePresentModesKHR() first call");


	VkPresentModeKHR *vkPresentModeKHR_array = (VkPresentModeKHR *)malloc(presentModeCount * sizeof(VkPresentModeKHR));
	vkResult = vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice_Selected, vkSurfaceKHR, &presentModeCount, vkPresentModeKHR_array);
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[getPhysicalDevicePresentModeKHR][ERROR] Failed vkGetPhysicalDeviceSurfacePresentModesKHR() second call");
		return vkResult;
	}
	else
		fprintf(gpFile, "\nDEBUG:[getPhysicalDevicePresentModeKHR][SUCCESS] vkGetPhysicalDeviceSurfacePresentModesKHR() second call");

	
	//get decide presetation mode
	for(uint32_t i = 0; i < presentModeCount; i++)
	{
		if(vkPresentModeKHR_array[i] == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			vkPresentModeKHR = VK_PRESENT_MODE_MAILBOX_KHR;
			fprintf(gpFile, "\nDEBUG:[getPhysicalDevicePresentModeKHR][INFO] vkGetPhysicalDeviceSurfacePresentModesKHR() vkPresentModeKHR = VK_PRESENT_MODE_MAILBOX_KHR");
			break;
		}
	}

	if(vkPresentModeKHR != VK_PRESENT_MODE_MAILBOX_KHR)
	{
		vkPresentModeKHR = VK_PRESENT_MODE_MAILBOX_KHR;
		fprintf(gpFile, "\nDEBUG:[getPhysicalDevicePresentModeKHR][INFO] vkGetPhysicalDeviceSurfacePresentModesKHR() vkPresentModeKHR = VK_PRESENT_MODE_MAILBOX_KHR");
	}

	if(vkPresentModeKHR_array)
	{
		free(vkPresentModeKHR_array);
		vkPresentModeKHR_array = NULL;
		fprintf(gpFile, "\nDEBUG:[getPhysicalDevicePresentModeKHR][SUCCESS] free vkPresentModeKHR_array[]");
	}

	fprintf(gpFile, "\nDEBUG:------------------------------------Done [getPhysicalDevicePresentModeKHR()]------------------------------------\n");
	return vkResult;
}
/*----------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------------
12>Swapchain
--------------------------------------------------------------------------------------------------------
1> Get physical device Surface Format And Color Space using STEP-10.
2> Get physical device surface capabilities by using vulkan API vkGetPhysicalDeviceSurfaceCapabilitiesKHR()
   and accordingly initailise VkSurfaceCapabilitiesKHR structure.
3> By using meanImageCount and maxImageCount members of above structure decide desired imageCount for Swapchain.
   Note:
4> By using currentExtent.width and currentExtent.height members of above structure and comparing them with
   current width and height of window, and decide image width and image height of Swapchain
5> Decide how we are going to use the swapchain images means whether we are going to store image data and 
   use it later(Deffered Rendering) OR we are using it immediately as color attachment.
6> swapchain is capable of storing transformed image before presentation which is called as pretransform.
   while creating swapchain we can decide whether to pretransform or not(pretransform also include flipping).
7> Get presentation mode for swapchain images using STEP-11.
8> According above data declare, memset and initialize VkSwapchainCreateInfoKHR structure.
9> At the end call vkCreateSwapchainKHR() to create the swapchain.
10> when done destroy it uninitialize() by using vkDestroySwapchainKHR() API.
--------------------------------------------------------------------------------------------------------*/
VkResult createSwapchain(VkBool32 vsync)
{
	//variable declaration
	VkResult vkResult = VK_SUCCESS;

	//Function declaration
	VkResult getPhysicalDeviceSurfaceFormatAndColorSpace(void);
	VkResult getPhysicalDevicePresentModeKHR(void);

	//code
	fprintf(gpFile, "\n\nDEBUG:------------------------------------Inside[createSwapchain()]------------------------------------");

	//surface format and color space 
	vkResult = getPhysicalDeviceSurfaceFormatAndColorSpace();
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[createSwapchain][ERROR] Failed getPhysicalDeviceSurfaceFormatAndColorSpace()");
		return vkResult;
	}
	else
		fprintf(gpFile, "\nDEBUG:[createSwapchain][SUCCESS] getPhysicalDeviceSurfaceFormatAndColorSpace()");

	//2> Get physical device surface capabilities
	VkSurfaceCapabilitiesKHR vkSurfaceCapabilitiesKHR;
	memset((void *)&vkSurfaceCapabilitiesKHR, 0, sizeof(VkSurfaceCapabilitiesKHR));

	vkResult = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkPhysicalDevice_Selected, vkSurfaceKHR, &vkSurfaceCapabilitiesKHR);
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[createSwapchain][ERROR] Failed vkGetPhysicalDeviceSurfaceCapabilitiesKHR()");
		return vkResult;
	}
	else
		fprintf(gpFile, "\nDEBUG:[createSwapchain][SUCCESS] vkGetPhysicalDeviceSurfaceCapabilitiesKHR()");

	//3> By using meanImageCount and maxImageCount members of above structure decide desired imageCount for Swapchain.
	uint32_t testingNumberOfSwapchainImages = vkSurfaceCapabilitiesKHR.minImageCount + 1;
	uint32_t desiredNumberOfSwapchainImages = 0;

	if(vkSurfaceCapabilitiesKHR.maxImageCount > 0 && vkSurfaceCapabilitiesKHR.maxImageCount < testingNumberOfSwapchainImages)
	{
		desiredNumberOfSwapchainImages = vkSurfaceCapabilitiesKHR.maxImageCount;
	}
	else
	{
		desiredNumberOfSwapchainImages = vkSurfaceCapabilitiesKHR.minImageCount;
	}

 	//4 decide image width and image height of Swapchain
	memset((void*)&vkExtend2D_Swapchain, 0, sizeof(VkExtent2D));
	if(vkSurfaceCapabilitiesKHR.currentExtent.width != UINT32_MAX)
	{
		vkExtend2D_Swapchain.width = vkSurfaceCapabilitiesKHR.currentExtent.width;
		vkExtend2D_Swapchain.height = vkSurfaceCapabilitiesKHR.currentExtent.height;
		fprintf(gpFile, "\nDEBUG:[createSwapchain][SUCCESS] if vkExtend2D_Swapchain.width  = %d", vkExtend2D_Swapchain.width);
		fprintf(gpFile, "\nDEBUG:[createSwapchain][SUCCESS] if vkExtend2D_Swapchain.height = %d", vkExtend2D_Swapchain.height);
	}
	else
	{
		//if surface size already defined then swapchain image size must match with it.
		VkExtent2D vkExtend2D;
		memset((void *)&vkExtend2D, 0, sizeof(VkExtent2D));
		vkExtend2D.width = (uint32_t)winWidth;
		vkExtend2D.height = (uint32_t)winHeight;

		vkExtend2D_Swapchain.width = glm::max(vkSurfaceCapabilitiesKHR.minImageExtent.width, glm::min(vkSurfaceCapabilitiesKHR.maxImageExtent.width, vkExtend2D.width));
		vkExtend2D_Swapchain.height = glm::max(vkSurfaceCapabilitiesKHR.minImageExtent.height, glm::min(vkSurfaceCapabilitiesKHR.maxImageExtent.height, vkExtend2D.height));

		fprintf(gpFile, "\nDEBUG:[createSwapchain][SUCCESS] else vkExtend2D_Swapchain.width  = %d", vkExtend2D_Swapchain.width);
		fprintf(gpFile, "\nDEBUG:[createSwapchain][SUCCESS] else vkExtend2D_Swapchain.height = %d", vkExtend2D_Swapchain.height);
	}

	//set swapchain usage flag
	//VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT => MUST
	//VK_IMAGE_USAGE_TRANSFER_SRC_BIT => OPTIONAL(may useful for render to texture, FBO, Compute shader)
	VkImageUsageFlags vkImageUsageFlags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

	//whether to consider pretransform/flipping or not
	VkSurfaceTransformFlagBitsKHR vkSurfaceTransformFlagBitsKHR; //enum

	if(vkSurfaceCapabilitiesKHR.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
	{
		vkSurfaceTransformFlagBitsKHR = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}
	else
	{
		vkSurfaceTransformFlagBitsKHR = vkSurfaceCapabilitiesKHR.currentTransform;
	}

	//presentation mode
	vkResult = getPhysicalDevicePresentModeKHR();
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[createSwapchain][ERROR] Failed getPhysicalDevicePresentModeKHR()");
		return vkResult;
	}
	else
		fprintf(gpFile, "\nDEBUG:[createSwapchain][SUCCESS] getPhysicalDevicePresentModeKHR()");

	
	//initialize VkSwapchainCreateInfoKHR structure
	VkSwapchainCreateInfoKHR vkSwapchainCreateInfoKHR;
	memset((void*)&vkSwapchainCreateInfoKHR, 0, sizeof(VkSwapchainCreateInfoKHR));

	vkSwapchainCreateInfoKHR.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	vkSwapchainCreateInfoKHR.pNext = NULL;
	vkSwapchainCreateInfoKHR.flags = 0;

	vkSwapchainCreateInfoKHR.surface = vkSurfaceKHR;
	vkSwapchainCreateInfoKHR.minImageCount = desiredNumberOfSwapchainImages;
	vkSwapchainCreateInfoKHR.imageFormat = vkFormat_color;
	vkSwapchainCreateInfoKHR.imageColorSpace = vkColorSpaceKHR;
	vkSwapchainCreateInfoKHR.imageExtent.width = vkExtend2D_Swapchain.width;
	vkSwapchainCreateInfoKHR.imageExtent.height = vkExtend2D_Swapchain.height;
	vkSwapchainCreateInfoKHR.imageUsage = vkImageUsageFlags;
	vkSwapchainCreateInfoKHR.preTransform = vkSurfaceTransformFlagBitsKHR;
	
	vkSwapchainCreateInfoKHR.imageArrayLayers = 1;
	vkSwapchainCreateInfoKHR.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	vkSwapchainCreateInfoKHR.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	vkSwapchainCreateInfoKHR.presentMode = vkPresentModeKHR;
	vkSwapchainCreateInfoKHR.clipped = VK_TRUE;

	vkResult = vkCreateSwapchainKHR(vkDevice, &vkSwapchainCreateInfoKHR, NULL, &vkSwapchainKHR);
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[createSwapchain][ERROR] Failed vkCreateSwapchainKHR()");
		return vkResult;
	}
	else
		fprintf(gpFile, "\nDEBUG:[createSwapchain][SUCCESS] vkCreateSwapchainKHR()");


	fprintf(gpFile, "\nDEBUG:------------------------------------Done [createSwapchain()]------------------------------------\n");
	return vkResult;
}
/*----------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------------
13>Swapchain Images and Iamge Views
--------------------------------------------------------------------------------------------------------
1> Get Swapchain image count in global variable using vkGetSwapchainImagesKHR().
2> Declare a variable VkImage data type array and allocate it to the swapchain image count using malloc.
3> Now call the same function agian in step 1 and fill this array. 
4> Declare another global array of type VkImageView and allocate it to the size of swapchain image count.
5> Declare and initialize VkImageViewCreateInfo structure except its ".image" member.
6> Now start a loop for swapchain Image count. and inside this loop initialize above ".image" member to the
   swapchain image array 'i'th index obtained above. and then call vkCreateImageView() to fill above image 
   view arrray.
7> uninitialize-1: In uninitialize() keeping the destructor logic aside for a while first destroy swapchain 
   images from the swapimages array in a loop using vkDestroySwapchainImage().
8> In ---------- using free();
9> uninitialize-2: In uninitialize() destroy imageviews in imageviewsarray in loop vkDestroyImageViews().
10> now actually free imageviewsarray using free().
--------------------------------------------------------------------------------------------------------*/
VkResult createImagesAndImageViews(void)
{
	//function declaration
	VkResult getSupportedDepthFormat(void);

	//variable declaration
	VkResult vkResult = VK_SUCCESS;

	//code
	fprintf(gpFile, "\n\nDEBUG:------------------------------------Inside[createImagesAndImageViews()]------------------------------------");

	//1> Get Swapchain image count in global variable using vkGetSwapchainImagesKHR().
	vkResult = vkGetSwapchainImagesKHR(vkDevice, vkSwapchainKHR, &swapchainImageCount, NULL);
	 if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[createImagesAndImageViews][ERROR] Failed vkGetSwapchainImagesKHR() first call");
		return vkResult;
	}
	else if(swapchainImageCount == 0)
	{
		fprintf(gpFile, "\nDEBUG:[createImagesAndImageViews][ERROR] Failed vkGetSwapchainImagesKHR() swapchainImageCount == 0 first call");
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nDEBUG:[createImagesAndImageViews][SUCCESS] vkGetSwapchainImagesKHR() first call");
		fprintf(gpFile, "\nDEBUG:[createImagesAndImageViews][SUCCESS] vkGetSwapchainImagesKHR() first call swapchainImageCount = %d", swapchainImageCount);
	}
	
	//2> Declare a variable VkImage data type array and allocate it to the swapchain image count using malloc.
	swapchainImage_array = (VkImage *)malloc(swapchainImageCount * sizeof(VkImage));
	//error checking for malloc

	//3> Now call the same function agian in step 1 and fill this array. 
	vkResult = vkGetSwapchainImagesKHR(vkDevice, vkSwapchainKHR, &swapchainImageCount, swapchainImage_array);
	 if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[createImagesAndImageViews][ERROR] Failed vkGetSwapchainImagesKHR() second call");
		return vkResult;
	}
	else if(swapchainImageCount == 0)
	{
		fprintf(gpFile, "\nDEBUG:[createImagesAndImageViews][ERROR] Failed vkGetSwapchainImagesKHR() second call swapchainImageCount == 0");
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nDEBUG:[createImagesAndImageViews][SUCCESS] vkGetSwapchainImagesKHR() second call");
		fprintf(gpFile, "\nDEBUG:[createImagesAndImageViews][SUCCESS] vkGetSwapchainImagesKHR() second call swapchainImageCount = %d", swapchainImageCount);
	}

	//4> Declare another global array of type VkImageView and allocate it to the size of swapchain image count.
	swapchainImageView_array = (VkImageView *)malloc(swapchainImageCount * sizeof(VkImageView));
	//error checking malloc

	//5> Declare and initialize VkImageViewCreateInfo structure except its ".image" member.
	VkImageViewCreateInfo vkImageViewCreateInfo;
	memset((void*)&vkImageViewCreateInfo, 0, sizeof(VkImageViewCreateInfo));

	vkImageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	vkImageViewCreateInfo.pNext = NULL;
	vkImageViewCreateInfo.flags = 0;
	vkImageViewCreateInfo.format = vkFormat_color;
	
	//VkComponentMapping components
	//VkComponentSwizzle r g b a
	vkImageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_R;
	vkImageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_G;
	vkImageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_B;
	vkImageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_A;

	//VkImageSubresourceRange subresourceRange
	//
	vkImageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	vkImageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	vkImageViewCreateInfo.subresourceRange.levelCount = 1;
	vkImageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	vkImageViewCreateInfo.subresourceRange.layerCount = 1;

	vkImageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	
	//6> Now start a loop for swapchain Image count. and inside this loop initialize above ".image" member to the
	//   swapchain image array 'i'th index obtained above. and then call vkCreateImageView() to fill above image 
	//   view arrray.

	for(uint32_t i = 0; i < swapchainImageCount; i++)
	{
		vkImageViewCreateInfo.image = swapchainImage_array[i];
		//vkCreateImageView();
		vkResult = vkCreateImageView(vkDevice, &vkImageViewCreateInfo, NULL, &swapchainImageView_array[i]);
		if(vkResult != VK_SUCCESS)
		{	
			fprintf(gpFile, "\nDEBUG:[createImagesAndImageViews][ERROR] Failed vkCreateImageView() i = %d", i);
			return vkResult;
		}
		else
		{
			fprintf(gpFile, "\nDEBUG:[createImagesAndImageViews][SUCCESS] vkCreateImageView() i = %d", i);
		}
	}

	//for depth image
	vkResult = getSupportedDepthFormat();
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[createImagesAndImageViews][ERROR] Failed getSupportedDepthFormat()");
		return vkResult;
	}
	else
		fprintf(gpFile, "\nDEBUG:[createImagesAndImageViews][SUCCESS] getSupportedDepthFormat()");

	//for dpth image initialize vkimage_depth
	VkImageCreateInfo vkImageCreateInfo;
	memset((void*)&vkImageCreateInfo, 0, sizeof(VkImageCreateInfo));

	vkImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	vkImageCreateInfo.pNext = NULL;
	vkImageCreateInfo.flags = 0;
	vkImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	vkImageCreateInfo.format = vkFormat_depth;
	vkImageCreateInfo.extent.width = winWidth;
	vkImageCreateInfo.extent.height = winHeight;
	vkImageCreateInfo.extent.depth = 1;
	vkImageCreateInfo.mipLevels = 1;
	vkImageCreateInfo.arrayLayers = 1;
	vkImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	vkImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	vkImageCreateInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	
	vkResult = vkCreateImage(vkDevice, &vkImageCreateInfo, NULL, &vkImage_depth);
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[createImagesAndImageViews][ERROR] Failed vkCreateImage() for depth image");
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nDEBUG:[createImagesAndImageViews][SUCCESS] vkCreateImage() for depth image");
	}



	//get memory requirements for depth image
	VkMemoryRequirements vkMemoryRequirements;
	memset((void *)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));

	vkGetImageMemoryRequirements(vkDevice, vkImage_depth, &vkMemoryRequirements);

	VkMemoryAllocateInfo vkMemoryAllocateInfo;
	memset((void *)&vkMemoryAllocateInfo, 0, sizeof(VkMemoryAllocateInfo));

	vkMemoryAllocateInfo.sType 				= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	vkMemoryAllocateInfo.pNext 				= NULL;
	vkMemoryAllocateInfo.allocationSize 	= vkMemoryRequirements.size; //size of the buffer in bytes
	vkMemoryAllocateInfo.memoryTypeIndex 	= 0; //will be set in next step
	
	for(uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
	{
		if((vkMemoryRequirements.memoryTypeBits & 1) == 1)
		{
			if((vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
			{
				vkMemoryAllocateInfo.memoryTypeIndex = i;
				fprintf(gpFile, "\nDEBUG:[createImagesAndImageViews][SUCCESS] vkMemoryAllocateInfo.memoryTypeIndex = %d", vkMemoryAllocateInfo.memoryTypeIndex);
				break;
			}
		}
		else
		{
			vkMemoryRequirements.memoryTypeBits = vkMemoryRequirements.memoryTypeBits >> 1;
			fprintf(gpFile, "\nDEBUG:[createImagesAndImageViews][INFO] vkMemoryRequirements.memoryTypeBits = %d", vkMemoryRequirements.memoryTypeBits);
		}
	}

	//11> Now call vkAllocateMemory() and get the required vulkan memory objects handle into the ".vkDeviceMemory" member of global variable.
	vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vkDeviceMemory_depth);
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[createImagesAndImageViews][ERROR] Failed vkAllocateMemory()");
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nDEBUG:[createImagesAndImageViews][SUCCESS] vkAllocateMemory()");
	}
	
	//12> Now we have our reqired memory as well as vkBuffer handle. binnd this device memory handle to the vulkan buffer handle by using
	//    vkBindBufferMemory().
	vkResult = vkBindImageMemory(vkDevice, vkImage_depth, vkDeviceMemory_depth, 0);
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[createImagesAndImageViews][ERROR] Failed vkBindImageMemory()");
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nDEBUG:[createImagesAndImageViews][SUCCESS] vkBindImageMemory()");
	}

	//create image view for depth image
	VkImageViewCreateInfo vkImageViewCreateInfo_depth;
	memset((void*)&vkImageViewCreateInfo_depth, 0, sizeof(VkImageViewCreateInfo));

	vkImageViewCreateInfo_depth.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	vkImageViewCreateInfo_depth.pNext = NULL;
	vkImageViewCreateInfo_depth.flags = 0;
	vkImageViewCreateInfo_depth.image = vkImage_depth;
	vkImageViewCreateInfo_depth.viewType = VK_IMAGE_VIEW_TYPE_2D;
	vkImageViewCreateInfo_depth.format = vkFormat_depth;
	vkImageViewCreateInfo_depth.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	vkImageViewCreateInfo_depth.subresourceRange.baseMipLevel = 0;
	vkImageViewCreateInfo_depth.subresourceRange.levelCount = 1;
	vkImageViewCreateInfo_depth.subresourceRange.baseArrayLayer = 0;
	vkImageViewCreateInfo_depth.subresourceRange.layerCount = 1;

	vkResult = vkCreateImageView(vkDevice, &vkImageViewCreateInfo_depth, NULL, &vkImageView_depth);
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[createImagesAndImageViews][ERROR] Failed vkCreateImageView() for depth image");
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nDEBUG:[createImagesAndImageViews][SUCCESS] vkCreateImageView() for depth image");
	}

	fprintf(gpFile, "\nDEBUG:------------------------------------Done [createImagesAndImageViews()]------------------------------------\n");
	return vkResult;
}
/*--------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------*/
VkResult getSupportedDepthFormat(void)
{
	//variable declaration
	VkResult vkResult = VK_SUCCESS;
	//code
	fprintf(gpFile, "\n\nDEBUG:------------------------------------Inside[getSupportedDepthFormat()]------------------------------------");
	
	VkFormat *vkDepth_array = NULL;
	vkDepth_array = (VkFormat *)malloc(5 * sizeof(VkFormat));
	//error checking malloc
	vkDepth_array[0] = VK_FORMAT_D32_SFLOAT_S8_UINT; //VK_FORMAT_D16_UNORM_S8_UINT;
	vkDepth_array[1] = VK_FORMAT_D24_UNORM_S8_UINT;
	vkDepth_array[2] = VK_FORMAT_D32_SFLOAT; //VK_FORMAT_D32_SFLOAT_S8_UINT;
	vkDepth_array[3] = VK_FORMAT_D16_UNORM; //VK_FORMAT_D32_SFLOAT_S8_UINT;
	vkDepth_array[4] = VK_FORMAT_D16_UNORM_S8_UINT;
	//vkDepth_array[5] = VK_FORMAT_D32_SFLOAT_S8_UINT;	


	//VkFormat vkFormat_depth = VK_FORMAT_D32_SFLOAT_S8_UINT; //default value 
	//VkFormat vkFormat_depth_stencil = VK_FORMAT_D32_SFLOAT_S8_UINT;

	for(uint32_t i = 0; i < 5; i++)
	{
		VkFormatProperties vkFormatProperties;
		memset((void *)&vkFormatProperties, 0, sizeof(VkFormatProperties));


		vkGetPhysicalDeviceFormatProperties(vkPhysicalDevice_Selected, vkDepth_array[i], &vkFormatProperties);
		

		if((vkFormatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) == VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
		{
			vkFormat_depth = vkDepth_array[i];
			vkResult = VK_SUCCESS;
			fprintf(gpFile, "\nDEBUG:[getSupportedDepthFormat][SUCCESS] vkFormat_depth = %d", vkFormat_depth);
			break;
		}
	}
	//
	return vkResult;
}
/*--------------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------------
14> Command Pool
--------------------------------------------------------------------------------------------------------
1> Declare and initialise VkCommandPool structure.
2> call vkCretaeCommandPool() to create command Pool
3> destroy command pool using vkDestroyCommandPool().
--------------------------------------------------------------------------------------------------------*/
VkResult createCommandPool(void)
{

	//variable declaration
	VkResult vkResult = VK_SUCCESS;

	//code
	fprintf(gpFile, "\n\nDEBUG:------------------------------------Inside[createCommandPool()]------------------------------------");
	VkCommandPoolCreateInfo vkCommondPoolInfo;
	memset((void *)&vkCommondPoolInfo, 0, sizeof(VkCommandPoolCreateInfo));

	vkCommondPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	vkCommondPoolInfo.pNext = NULL;
	vkCommondPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	vkCommondPoolInfo.queueFamilyIndex = graphicsQueueFamilyIndex_Selected;

	vkResult = vkCreateCommandPool(vkDevice, &vkCommondPoolInfo, NULL, &vkCommandPool);
		if(vkResult != VK_SUCCESS)
		{	
			fprintf(gpFile, "\nDEBUG:[createCommandPool][ERROR] Failed vkCreateCommandPool()");
			return vkResult;
		}
		else
		{
			fprintf(gpFile, "\nDEBUG:[createCommandPool][SUCCESS] vkCreateCommandPool()");
		}



	fprintf(gpFile, "\nDEBUG:------------------------------------Done [createCommandPool()]------------------------------------\n");
	return vkResult;
}
/*--------------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------------
15> Command Buffer
--------------------------------------------------------------------------------------------------------
1> Declare and Initialize struct VkCommandBuffersAllocateInfo.
   NOTE:The number of command buffer are conventionally is equal to number of swapcian images.
2> Declare a command buffer array globally and allocate it to the size of swapchain image count.
3> In a loop which is equal to swapchain image count, allocate each command buffer in array by using
   vkAllocateCommandBuffers() API. Remember at the time of allocation all the buffers are empty, later we
   fill graphics or compute commands in it.
4> In uninitialize free each command buffer vkFreeCommandBuffers() in a loop of swapchain image count size.
5> free the actual command buffer array.
--------------------------------------------------------------------------------------------------------*/
VkResult createCommandBuffers(void)
{
	//variable declaration
	VkResult vkResult = VK_SUCCESS;

	//code
	fprintf(gpFile, "\n\nDEBUG:------------------------------------Inside[createCommandPool()]------------------------------------");
	//1> Declare and Initialize struct VkCommandBuffersAllocateInfo.
	//NOTE:The number of command buffer are conventionally is equal to number of swapcian images.
	VkCommandBufferAllocateInfo vkCommandBufferAllocateInfo;
	memset((void *)&vkCommandBufferAllocateInfo, 0, sizeof(VkCommandBufferAllocateInfo));

	vkCommandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	vkCommandBufferAllocateInfo.pNext = NULL;
	vkCommandBufferAllocateInfo.commandPool = vkCommandPool;
	vkCommandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; 
	vkCommandBufferAllocateInfo.commandBufferCount = 1;

	vkCommandBuffer_array = (VkCommandBuffer*)malloc(swapchainImageCount * sizeof(VkCommandBuffer));
	//error checking malloc

	//allocate 
	for(uint32_t i = 0; i < swapchainImageCount; i++)
	{
		vkResult = vkAllocateCommandBuffers(vkDevice, &vkCommandBufferAllocateInfo, &vkCommandBuffer_array[i]);
		if(vkResult != VK_SUCCESS)
		{	
			fprintf(gpFile, "\nDEBUG:[createCommandBuffers][ERROR] Failed vkAllocateCommandBuffers() i = %d", i);
			return vkResult;
		}
		else
		{
			fprintf(gpFile, "\nDEBUG:[createCommandBuffers][SUCCESS] vkAllocateCommandBuffers() i = %d", i);
		}
	}

	fprintf(gpFile, "\nDEBUG:------------------------------------Done [createCommandPool()]------------------------------------\n");
	return vkResult;
}
/*--------------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------------
22-Vertex Buffer
--------------------------------------------------------------------------------------------------------
1> Declare a global structure ,we will call it as struct VertexData and delcare global variable if this structure
   named as vertexData_position.
2> Declare VERTEX_BUFFER write its prototype below createCommandBuffer and above createRenderpass and call it
   initialise()
3> Implement this user defined function. 
   Inside, declare our triangle postion array.
4> memset our global vertexData_position. delcare and memset struct VkBufferCreateInfo()
   it has 8 members, we will use 5. out of 2 are very important.
   1> Usage
   2> Size
5> Call vkCreateBuffer() vulkan API in the ".vkBuffer" member of our global struct.

7> Declare and memset struct VkMemoryRequirements and then call vkGetBufferMemoryRequirements() API
   to get the memory requirements.
8> To acually allocate the required memomry, we need to call vkAllocateMemory() API. But before that declare and 
   memset VkMemoryAllocateInfo structure.
9> important members of this structure are ".memoryTypeIndex" and ".allocationSize". For allocation size use the size 
   obtained from memory requirements.
10> For memoryTypeIndex: 
		a> Start a loop with counter as vkPhysiacalDeviceMemoryProperties.memoryTypeCount.
		b> Inside the loop check vkmemoryrequirements memoryTypebits contains '1' or not. if yes
		c> check vkPhysiacalDeviceMemoryProperties.memoryTypes[i].propertyFlags member coNtains  
		   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT 
		d> then ith index will be our memoryType index. if found break out of the loop.
		e> if not found, continue the loop by right shifting vkmemoryrequirements.memoryType bits by 1 over each iteration.
11> Now call vkAllocateMemory() and get the required vulkan memory objects handle into the ".vkDeviceMemory" member of global variable.
12> Now we have our reqired memory as well as vkBuffer handle. binnd this device memory handle to the vulkan buffer handle by using
    vkBindBufferMemory().
13> Declare a local void* buffer as "data" and call vkMapMemory() to map to our device memory object handle to this void * data.
14> This will allow us to do mwmory mapped IO means we write on void *data it will get automatically get written/copied onto the 
    device memory represented by device memory object handle.
15> Now to do actually to do memorymapped IO call memcpy()
16> To complete this memory mapped IO finally call vkUnmapMemory() API.
17> In uninitialize() first free the ".vkDeviceMemory" member of our global structure using vkFreeMemory()
    and then destroy ".vkBuffer" member of our global structure by using vkDestroyBuffer() API.
--------------------------------------------------------------------------------------------------------*/	 
VkResult createVertexBuffers(void)
{
	//variable declaration
	VkResult vkResult = VK_SUCCESS;

	//code
	fprintf(gpFile, "\n\nDEBUG:------------------------------------Inside[createVertexBuffers()]------------------------------------");
	//1> Declare a global structure ,we will call it as struct VertexData and delcare global variable if this structure
	//2> named as vertexData_position.
	//struct VertexData vertexData_position;
	//3> Declare VERTEX_BUFFER write its prototype below createCommandBuffer and above createRenderpass and call it
	//   initialise()
	//4> Implement this user defined function.
	//   Inside, declare our triangle postion array.
// position
float cubeVertices[] =
{
	// front   =1
	// triangle one
	 1.0f,  1.0f,  1.0f, // top-right of front
	-1.0f,  1.0f,  1.0f, // top-left of front
  	 1.0f, -1.0f,  1.0f, // bottom-right of front
	
	// triangle two
	 1.0f, -1.0f,  1.0f, // bottom-right of front
	-1.0f,  1.0f,  1.0f, // top-left of front
	-1.0f, -1.0f,  1.0f, // bottom-left of front

	// right 	=2
	// triangle one
	 1.0f,  1.0f, -1.0f, // top-right of right
	 1.0f,  1.0f,  1.0f, // top-left of right
	 1.0f, -1.0f, -1.0f, // bottom-right of right
	 
	// triangle two
	 1.0f, -1.0f, -1.0f, // bottom-right of right
	 1.0f,  1.0f,  1.0f, // top-left of right
	 1.0f, -1.0f,  1.0f, // bottom-left of right

	// back 	=3
	// triangle one
	 1.0f,  1.0f, -1.0f, // top-right of back
	-1.0f,  1.0f, -1.0f, // top-left of back
	 1.0f, -1.0f, -1.0f, // bottom-right of back
	
	// triangle two
	 1.0f, -1.0f, -1.0f, // bottom-right of back
	-1.0f,  1.0f, -1.0f, // top-left of back
	-1.0f, -1.0f, -1.0f, // bottom-left of back

	// left 	=4
	// triangle one
	-1.0f,  1.0f,  1.0f, // top-right of left
	-1.0f,  1.0f, -1.0f, // top-left of left
	-1.0f, -1.0f,  1.0f, // bottom-right of left
	
	// triangle two
	-1.0f, -1.0f,  1.0f, // bottom-right of left
	-1.0f,  1.0f, -1.0f, // top-left of left
	-1.0f, -1.0f, -1.0f, // bottom-left of left

	// top 		=5	
	// triangle one
	 1.0f,  1.0f, -1.0f, // top-right of top
	-1.0f,  1.0f, -1.0f, // top-left of top
	 1.0f,  1.0f,  1.0f, // bottom-right of top

	// triangle two
	 1.0f,  1.0f,  1.0f, // bottom-right of top
	-1.0f,  1.0f, -1.0f, // top-left of top
	-1.0f,  1.0f,  1.0f, // bottom-left of top

	// bottom	=6
	// triangle one
	 1.0f, -1.0f,  1.0f, // top-right of bottom
	-1.0f, -1.0f,  1.0f, // top-left of bottom
	 1.0f, -1.0f, -1.0f, // bottom-right of bottom
	
	// triangle two
	 1.0f, -1.0f, -1.0f, // bottom-right of bottom
	-1.0f, -1.0f,  1.0f, // top-left of bottom
	-1.0f, -1.0f, -1.0f, // bottom-left of bottom
};
	
	float cubeTexcoords[] =
	{
		// front   =1
		// triangle two
		0.0f, 0.0f, // bottom-left of front
		0.0f, 1.0f, // top-left of front
		1.0f, 0.0f, // bottom-right of front

		// triangle one
		1.0f, 0.0f, // bottom-right of front
		0.0f, 1.0f, // top-left of front
		1.0f, 1.0f, // top-right of front


		// right	=2	
		// triangle two
		0.0f, 0.0f, // bottom-left of right
		0.0f, 1.0f, // top-left of right
		1.0f, 0.0f, // bottom-right of right
		// triangle one
		1.0f, 0.0f, // bottom-right of right
		0.0f, 1.0f, // top-left of right
		1.0f, 1.0f, // top-right of right

	
		// back		=3
		// triangle two
		0.0f, 0.0f, // bottom-left of back
		0.0f, 1.0f, // top-left of back
		1.0f, 0.0f, // bottom-right of back

		// triangle one
		1.0f, 0.0f, // bottom-right of back
		0.0f, 1.0f, // top-left of back
		1.0f, 1.0f, // top-right of back

		// left		=4
	
		// triangle two
		0.0f, 0.0f, // bottom-left of left
		0.0f, 1.0f, // top-left of left
		1.0f, 0.0f, // bottom-right of left

		// triangle one
		1.0f, 0.0f, // bottom-right of left
		0.0f, 1.0f, // top-left of left
		1.0f, 1.0f, // top-right of left

	
		// top		=5
		// triangle two
		0.0f, 0.0f, // bottom-left of top
		0.0f, 1.0f, // top-left of top
		1.0f, 0.0f, // bottom-right of top
		// triangle one
		1.0f, 0.0f, // bottom-right of top
		0.0f, 1.0f, // top-left of top
		1.0f, 1.0f, // top-right of top

		// bottom	=6
		// triangle two
		0.0f, 0.0f, // bottom-left of bottom
		0.0f, 1.0f, // top-left of bottom
		1.0f, 0.0f, // bottom-right of bottom

		// triangle one
		1.0f, 0.0f, // bottom-right of bottom
		0.0f, 1.0f, // top-left of bottom
		1.0f, 1.0f, // top-right of bottom
		
	};



	//vertex position buffer
	memset((void *)&vertexData_position, 0, sizeof(VertexData));
	//5> memset our global vertexData_position. delcare and memset struct VkBufferCreateInfo()
	VkBufferCreateInfo vkBufferCreateInfo;
	memset((void *)&vkBufferCreateInfo, 0, sizeof(VkBufferCreateInfo));

	//   it has 8 members, we will use 5. out of 2 are very important.
	vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vkBufferCreateInfo.pNext = NULL;
	vkBufferCreateInfo.flags = 0; //valid flags are used in scatterd buffers/sparse buffers
	vkBufferCreateInfo.size = sizeof(cubeVertices); //size of the buffer in bytes
	vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT; //VK_BUFFER_USAGE_TRANSFER_SRC_BIT;


	//6> Call vkCreateBuffer() vulkan API in the ".vkBuffer" member of our global struct.
	vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo, NULL, &vertexData_position.vkBuffer);
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[createVertexBuffers][ERROR] Failed vkCreateBuffer()");
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nDEBUG:[createVertexBuffers][SUCCESS] vkCreateBuffer()");
	}

	//7> Declare and memset struct VkMemoryRequirements and then call vkGetBufferMemoryRequirements() API
	VkMemoryRequirements vkMemoryRequirements;
	memset((void *)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));

	vkGetBufferMemoryRequirements(vkDevice, vertexData_position.vkBuffer, &vkMemoryRequirements);

	//8> To acually allocate the required memomry, we need to call vkAllocateMemory() API. But before that declare and
	//   memset VkMemoryAllocateInfo structure.
	VkMemoryAllocateInfo vkMemoryAllocateInfo;
	memset((void *)&vkMemoryAllocateInfo, 0, sizeof(VkMemoryAllocateInfo));

	vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	vkMemoryAllocateInfo.pNext = NULL;
	vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size; //size of the buffer in bytes
	vkMemoryAllocateInfo.memoryTypeIndex = 0; //will be set in next step
											  // initial value before loop

	//loop to find memoryTypeIndex
	//9> important members of this structure are ".memoryTypeIndex" and ".allocationSize". For allocation size use the size
	//   obtained from memory requirements.
	//10> For memoryTypeIndex:
	//		a> Start a loop with counter as vkPhysiacalDeviceMemoryProperties.memoryTypeCount.
	//		b> Inside the loop check vkmemoryrequirements memoryTypebits contains '1' or not. if yes
	//		c> check vkPhysiacalDeviceMemoryProperties.memoryTypes[i].propertyFlags member coNtains
	//		   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
	//		d> then ith index will be our memoryType index. if found break out of the loop.
	//		e> if not found, continue the loop by right shifting vkmemoryrequirements.memoryType bits by 1 over each iteration.
	for(uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
	{
		if((vkMemoryRequirements.memoryTypeBits & 1) == 1)
		{
			if((vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT))
			{
				vkMemoryAllocateInfo.memoryTypeIndex = i;
				fprintf(gpFile, "\nDEBUG:[createVertexBuffers][SUCCESS] vkMemoryAllocateInfo.memoryTypeIndex = %d", vkMemoryAllocateInfo.memoryTypeIndex);
				break;
			}
		}
		else
		{
			vkMemoryRequirements.memoryTypeBits = vkMemoryRequirements.memoryTypeBits >> 1;
			fprintf(gpFile, "\nDEBUG:[createVertexBuffers][INFO] vkMemoryRequirements.memoryTypeBits = %d", vkMemoryRequirements.memoryTypeBits);
		}
	}

	//11> Now call vkAllocateMemory() and get the required vulkan memory objects handle into the ".vkDeviceMemory" member of global variable.
	vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_position.vkDeviceMemory);
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[createVertexBuffers][ERROR] Failed vkAllocateMemory()");
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nDEBUG:[createVertexBuffers][SUCCESS] vkAllocateMemory()");
	}
	
	//12> Now we have our reqired memory as well as vkBuffer handle. binnd this device memory handle to the vulkan buffer handle by using
	//    vkBindBufferMemory().
	vkResult = vkBindBufferMemory(vkDevice, vertexData_position.vkBuffer, vertexData_position.vkDeviceMemory, 0);
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[createVertexBuffers][ERROR] Failed vkBindBufferMemory()");
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nDEBUG:[createVertexBuffers][SUCCESS] vkBindBufferMemory()");
	}
	
	//13> Declare a local void* buffer as "data" and call vkMapMemory() to map to our device memory object handle to this void * data.
	void *data = NULL;
	
	vkResult = vkMapMemory(vkDevice, vertexData_position.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[createVertexBuffers][ERROR] Failed vkMapMemory()");
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nDEBUG:[createVertexBuffers][SUCCESS] vkMapMemory()");
	}

	//14> This will allow us to do mwmory mapped IO means we write on void *data it will get automatically get written/copied onto the
	//    device memory represented by device memory object handle.
	//15> Now to do actually to do memorymapped IO call memcpy()
	memcpy(data, cubeVertices, sizeof(cubeVertices));
	fprintf(gpFile, "\nDEBUG:[createVertexBuffers][SUCCESS] memcpy()");

	//16> To complete this memory mapped IO finally call vkUnmapMemory() API.
	vkUnmapMemory(vkDevice, vertexData_position.vkDeviceMemory);



	//texture buffer
	memset((void *)&vertexData_texCoords, 0, sizeof(VertexData));
	//5> memset our global vertexData_color. delcare and memset struct VkBufferCreateInfo()
	//VkBufferCreateInfo vkBufferCreateInfo;
	memset((void *)&vkBufferCreateInfo, 0, sizeof(VkBufferCreateInfo));

	//   it has 8 members, we will use 5. out of 2 are very important.
	vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vkBufferCreateInfo.pNext = NULL;
	vkBufferCreateInfo.flags = 0; //valid flags are used in scatterd buffers/sparse buffers
	vkBufferCreateInfo.size = sizeof(cubeTexcoords); //size of the buffer in bytes
	vkBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT; //VK_BUFFER_USAGE_TRANSFER_SRC_BIT;


	//6> Call vkCreateBuffer() vulkan API in the ".vkBuffer" member of our global struct.
	vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo, NULL, &vertexData_texCoords.vkBuffer);
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[createVertexBuffers][ERROR] Failed vkCreateBuffer() to vertexData_texCoords.vkBuffer");
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nDEBUG:[createVertexBuffers][SUCCESS] vkCreateBuffer() to vertexData_texCoords.vkBuffer");
	}

	//7> Declare and memset struct VkMemoryRequirements and then call vkGetBufferMemoryRequirements() API
	//VkMemoryRequirements vkMemoryRequirements;
	memset((void *)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));

	vkGetBufferMemoryRequirements(vkDevice, vertexData_texCoords.vkBuffer, &vkMemoryRequirements);

	//8> To acually allocate the required memomry, we need to call vkAllocateMemory() API. But before that declare and
	//   memset VkMemoryAllocateInfo structure.
	//VkMemoryAllocateInfo vkMemoryAllocateInfo;
	memset((void *)&vkMemoryAllocateInfo, 0, sizeof(VkMemoryAllocateInfo));

	vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	vkMemoryAllocateInfo.pNext = NULL;
	vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements.size; //size of the buffer in bytes
	vkMemoryAllocateInfo.memoryTypeIndex = 0; //will be set in next step
											  // initial value before loop

	//loop to find memoryTypeIndex
	//9> important members of this structure are ".memoryTypeIndex" and ".allocationSize". For allocation size use the size
	//   obtained from memory requirements.
	//10> For memoryTypeIndex:
	//		a> Start a loop with counter as vkPhysiacalDeviceMemoryProperties.memoryTypeCount.
	//		b> Inside the loop check vkmemoryrequirements memoryTypebits contains '1' or not. if yes
	//		c> check vkPhysiacalDeviceMemoryProperties.memoryTypes[i].propertyFlags member coNtains
	//		   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
	//		d> then ith index will be our memoryType index. if found break out of the loop.
	//		e> if not found, continue the loop by right shifting vkmemoryrequirements.memoryType bits by 1 over each iteration.
	for(uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
	{
		if((vkMemoryRequirements.memoryTypeBits & 1) == 1)
		{
			if((vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT))
			{
				vkMemoryAllocateInfo.memoryTypeIndex = i;
				fprintf(gpFile, "\nDEBUG:[createVertexBuffers][SUCCESS] vkMemoryAllocateInfo.memoryTypeIndex = %d", vkMemoryAllocateInfo.memoryTypeIndex);
				break;
			}
		}
		vkMemoryRequirements.memoryTypeBits = vkMemoryRequirements.memoryTypeBits >> 1;
		fprintf(gpFile, "\nDEBUG:[createVertexBuffers][INFO] vkMemoryRequirements.memoryTypeBits = %d", vkMemoryRequirements.memoryTypeBits);
	}		

	//11> Now call vkAllocateMemory() and get the required vulkan memory objects handle into the ".vkDeviceMemory" member of global variable.
	vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vertexData_texCoords.vkDeviceMemory);
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[createVertexBuffers][ERROR] Failed vkAllocateMemory() vertexData_texCoords.vkDeviceMemory");
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nDEBUG:[createVertexBuffers][SUCCESS] vkAllocateMemory() vertexData_texCoords.vkDeviceMemory");
	}
	
	//12> Now we have our reqired memory as well as vkBuffer handle. binnd this device memory handle to the vulkan buffer handle by using
	//    vkBindBufferMemory().
	vkResult = vkBindBufferMemory(vkDevice, vertexData_texCoords.vkBuffer, vertexData_texCoords.vkDeviceMemory, 0);
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[createVertexBuffers][ERROR] Failed vkBindBufferMemory() to vertexData_texCoords.vkDeviceMemory");
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nDEBUG:[createVertexBuffers][SUCCESS] vkBindBufferMemory() to vertexData_texCoords.vkDeviceMemory");
	}
	
	//13> Declare a local void* buffer as "data" and call vkMapMemory() to map to our device memory object handle to this void * data.
	//void *data = NULL;
	data = NULL;

	vkResult = vkMapMemory(vkDevice, vertexData_texCoords.vkDeviceMemory, 0, vkMemoryAllocateInfo.allocationSize, 0, &data);
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[createVertexBuffers][ERROR] Failed vkMapMemory() to vertexData_texCoords.vkDeviceMemory");
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nDEBUG:[createVertexBuffers][SUCCESS] vkMapMemory() to vertexData_texCoords.vkDeviceMemory");
	}

	//14> This will allow us to do mwmory mapped IO means we write on void *data it will get automatically get written/copied onto the
	//    device memory represented by device memory object handle.
	//15> Now to do actually to do memorymapped IO call memcpy()
	memcpy(data, cubeTexcoords, sizeof(cubeTexcoords));
	fprintf(gpFile, "\nDEBUG:[createVertexBuffers][SUCCESS] memcpy() pyramidTexcoords");

	//16> To complete this memory mapped IO finally call vkUnmapMemory() API.
	vkUnmapMemory(vkDevice, vertexData_texCoords.vkDeviceMemory);
	return vkResult;
}
/*--------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------*/
VkResult CreateTexture(const char *textureFileName)
{
	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	//code
	//1> Get image (info)data like width, height, number of channels and actual image data using functions
    //"stb_image.h"
	FILE *fp = NULL;
	fp = fopen(textureFileName, "rb");

	if(fp == NULL)
	{
		fprintf(gpFile, "\nDEBUG:[CreateTexture][ERROR] Failed to open texture file");
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	else
	{
		fprintf(gpFile, "\nDEBUG:[CreateTexture][SUCCESS] Opened texture file");
	}

	uint8_t *imageData = NULL;
	int texture_width, texture_height, texture_channels;

	imageData = stbi_load_from_file(fp, &texture_width, &texture_height, &texture_channels, STBI_rgb_alpha);
	if(imageData == NULL || texture_width <= 0 || texture_height <= 0 || texture_channels <= 0)
	{
		fprintf(gpFile, "\nDEBUG:[CreateTexture][ERROR] Failed to load texture file");
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	else
	{
		fprintf(gpFile, "\nDEBUG:[CreateTexture][SUCCESS] Loaded texture file");
	}


	VkDeviceSize image_size = texture_width * texture_height * 4; //4 bytes per pixel for RGBA

	//2> Put above image data into a staging buffer.
	VkBuffer vkBuffer_stagingBuffer = VK_NULL_HANDLE;
	VkDeviceMemory vkDeviceMemory_stagingBuffer = VK_NULL_HANDLE;

	// Create staging buffer
	VkBufferCreateInfo bufferCreateInfo_stagingBuffer;
	memset((void *)&bufferCreateInfo_stagingBuffer, 0, sizeof(VkBufferCreateInfo));
	
	bufferCreateInfo_stagingBuffer.sType 	= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo_stagingBuffer.pNext 	= NULL;
	bufferCreateInfo_stagingBuffer.size 	= image_size;
	bufferCreateInfo_stagingBuffer.usage 	= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	bufferCreateInfo_stagingBuffer.sharingMode = VK_SHARING_MODE_EXCLUSIVE; //private

	vkResult = vkCreateBuffer(vkDevice, &bufferCreateInfo_stagingBuffer, NULL, &vkBuffer_stagingBuffer);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nDEBUG:[CreateTexture][ERROR] Failed vkCreateBuffer() stagingBuffer");
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nDEBUG:[CreateTexture][SUCCESS] vkCreateBuffer() stagingBuffer");
	}

	// Allocate memory for staging buffer
	VkMemoryRequirements vkMemoryRequirements_stagingBuffer;
	vkGetBufferMemoryRequirements(vkDevice, vkBuffer_stagingBuffer, &vkMemoryRequirements_stagingBuffer);


	VkMemoryAllocateInfo vkMemoryAllocateInfo;
	memset((void *)&vkMemoryAllocateInfo, 0, sizeof(VkMemoryAllocateInfo));

	vkMemoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	vkMemoryAllocateInfo.pNext = NULL;
	vkMemoryAllocateInfo.allocationSize = vkMemoryRequirements_stagingBuffer.size;

	vkMemoryAllocateInfo.memoryTypeIndex = 0; // will be set in next step

	// Find suitable memory type
	for(uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
	{
		if((vkMemoryRequirements_stagingBuffer.memoryTypeBits & 1) == 1)
		{
			if((vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & (VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)))
			{
				vkMemoryAllocateInfo.memoryTypeIndex = i;
				fprintf(gpFile, "\nDEBUG:[CreateTexture][SUCCESS] vkMemoryAllocateInfo.memoryTypeIndex = %d", vkMemoryAllocateInfo.memoryTypeIndex);
				break;
			}
		}
		else
		{
			vkMemoryRequirements_stagingBuffer.memoryTypeBits >>= 1;
			fprintf(gpFile, "\nDEBUG:[CreateTexture][INFO] vkMemoryRequirements_stagingBuffer.memoryTypeBits = %d", vkMemoryRequirements_stagingBuffer.memoryTypeBits);
		}
	}

	vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &vkDeviceMemory_stagingBuffer);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nDEBUG:[CreateTexture][ERROR] Failed vkAllocateMemory() vkDeviceMemory_stagingBuffer");
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nDEBUG:[CreateTexture][SUCCESS] vkAllocateMemory() vkDeviceMemory_stagingBuffer");
	}

	// Bind memory to staging buffer
	vkResult = vkBindBufferMemory(vkDevice, vkBuffer_stagingBuffer, vkDeviceMemory_stagingBuffer, 0);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nDEBUG:[CreateTexture][ERROR] Failed vkBindBufferMemory() vkDeviceMemory_stagingBuffer");
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nDEBUG:[CreateTexture][SUCCESS] vkBindBufferMemory() vkDeviceMemory_stagingBuffer");
	}

	// Copy image data to staging buffer
	void *data = NULL;

	vkResult = vkMapMemory(vkDevice, vkDeviceMemory_stagingBuffer, 0, image_size, 0, &data);
	if(vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nDEBUG:[CreateTexture][ERROR] Failed vkMapMemory() vkDeviceMemory_stagingBuffer");
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nDEBUG:[CreateTexture][SUCCESS] vkMapMemory() vkDeviceMemory_stagingBuffer");
	}

	memcpy(data, imageData, image_size);
	vkUnmapMemory(vkDevice, vkDeviceMemory_stagingBuffer);

	//as copying of image data is done in staging buffer, we can free the image data by stb now 
	stbi_image_free(imageData);
	imageData = NULL;
	fprintf(gpFile, "\nDEBUG:[CreateTexture][SUCCESS] stbi_image_free()");

	//3> Create a vulkan image object to hold the texture data.
	VkImageCreateInfo vkCreateImageInfo;
	memset((void *)&vkCreateImageInfo, 0, sizeof(VkImageCreateInfo));

	vkCreateImageInfo.sType 		= VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	vkCreateImageInfo.pNext 		= NULL;
	vkCreateImageInfo.flags 		= 0;
	vkCreateImageInfo.imageType 	= VK_IMAGE_TYPE_2D;
	vkCreateImageInfo.format 		= VK_FORMAT_R8G8B8A8_UNORM;		
	vkCreateImageInfo.extent.width 	= texture_width;
	vkCreateImageInfo.extent.height = texture_height;
	vkCreateImageInfo.extent.depth 	= 1;
	vkCreateImageInfo.mipLevels 	= 1;
	vkCreateImageInfo.arrayLayers   = 1;
	vkCreateImageInfo.samples       = VK_SAMPLE_COUNT_1_BIT;
	vkCreateImageInfo.tiling        = VK_IMAGE_TILING_OPTIMAL;
	vkCreateImageInfo.usage         = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT; 		//VK_IMAGE_USAGE_SAMPLED_BIT;
	vkCreateImageInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
	vkCreateImageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	vkResult = vkCreateImage(vkDevice, &vkCreateImageInfo, NULL, &vkImage_texture);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nDEBUG:[CreateTexture][ERROR] Failed vkCreateImage()");
		fclose(fp);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nDEBUG:[CreateTexture][SUCCESS] vkCreateImage()");
	}

	VkMemoryRequirements vkMemoryRequirements_image;
	memset((void*)&vkMemoryRequirements_image, 0, sizeof(VkMemoryRequirements));

	vkGetImageMemoryRequirements(vkDevice, vkImage_texture, &vkMemoryRequirements_image);

	VkMemoryAllocateInfo vkMemoryAllocateInfo_image;
	memset((void*)&vkMemoryAllocateInfo_image, 0, sizeof(VkMemoryAllocateInfo));

	vkMemoryAllocateInfo_image.sType 			= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	vkMemoryAllocateInfo_image.pNext 			= NULL;
	vkMemoryAllocateInfo_image.allocationSize 	= vkMemoryRequirements_image.size;
	vkMemoryAllocateInfo_image.memoryTypeIndex 	= 0;

	// Find a suitable memory type
	for (uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
	{
		if ((vkMemoryRequirements_image.memoryTypeBits & 1 ) == 1)
		{
			if ((vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & (VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)))
			{
				vkMemoryAllocateInfo_image.memoryTypeIndex = i;
				fprintf(gpFile, "\nDEBUG:[CreateTexture][SUCCESS] vkMemoryAllocateInfo_image.memoryTypeIndex = %d", vkMemoryAllocateInfo_image.memoryTypeIndex);
				break;
			}
		}
		else
		{
			vkMemoryRequirements_image.memoryTypeBits >>= 1;
			fprintf(gpFile, "\nDEBUG:[CreateTexture][INFO] vkMemoryRequirements_image.memoryTypeBits = %d", vkMemoryRequirements_image.memoryTypeBits);
		}
	}

	//allocate image memory
	vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo_image, NULL, &vkDeviceMemory_texture);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nDEBUG:[CreateTexture][ERROR] Failed vkAllocateMemory() for vkDeviceMemory_texture");
		fclose(fp);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nDEBUG:[CreateTexture][SUCCESS] vkAllocateMemory() for vkDeviceMemory_texture");
	}

	//bind image memory
	vkResult = vkBindImageMemory(vkDevice, vkImage_texture, vkDeviceMemory_texture, 0);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nDEBUG:[CreateTexture][ERROR] Failed vkBindImageMemory() for vkDeviceMemory_texture");
		fclose(fp);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nDEBUG:[CreateTexture][SUCCESS] vkBindImageMemory() for vkDeviceMemory_texture");
	}

	//4> Send image transition layout to the vulkan engine/GPU before coppying the actual staging buffer 
    //from step 2 to the empty image(VkImage) copy of step 3 using pipeline barrier.
	
	VkCommandBufferAllocateInfo vkCommandBufferAllocateInfo_transition_layout;
	memset((void *)&vkCommandBufferAllocateInfo_transition_layout, 0, sizeof(VkCommandBufferAllocateInfo));

	vkCommandBufferAllocateInfo_transition_layout.sType 		= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	vkCommandBufferAllocateInfo_transition_layout.pNext 		= NULL;
	vkCommandBufferAllocateInfo_transition_layout.commandPool 	= vkCommandPool;
	vkCommandBufferAllocateInfo_transition_layout.commandBufferCount = 1;
	vkCommandBufferAllocateInfo_transition_layout.level 		= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	
	VkCommandBuffer vkCommandBuffer_transition_image_layout = VK_NULL_HANDLE;
	memset((void *)&vkCommandBuffer_transition_image_layout, 0, sizeof(VkCommandBuffer));

	vkResult = vkAllocateCommandBuffers(vkDevice, &vkCommandBufferAllocateInfo_transition_layout, &vkCommandBuffer_transition_image_layout);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nDEBUG:[CreateTexture][ERROR] Failed vkAllocateCommandBuffers() for vkCommandBuffer_transition_image_layout");
		fclose(fp);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nDEBUG:[CreateTexture][SUCCESS] vkAllocateCommandBuffers() for vkCommandBuffer_transition_image_layout");
	}

	VkCommandBufferBeginInfo vkCommandBufferBeginInfo_transition_image_layout;
	memset((void *)&vkCommandBufferBeginInfo_transition_image_layout, 0, sizeof(VkCommandBufferBeginInfo));

	vkCommandBufferBeginInfo_transition_image_layout.sType 			= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	vkCommandBufferBeginInfo_transition_image_layout.pNext 			= NULL;
	vkCommandBufferBeginInfo_transition_image_layout.flags 			= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; //VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	vkCommandBufferBeginInfo_transition_image_layout.pInheritanceInfo = NULL;


	vkResult = vkBeginCommandBuffer(vkCommandBuffer_transition_image_layout, &vkCommandBufferBeginInfo_transition_image_layout);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nDEBUG:[CreateTexture][ERROR] Failed vkBeginCommandBuffer() for vkCommandBuffer_transition_image_layout");
		fclose(fp);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nDEBUG:[CreateTexture][SUCCESS] vkBeginCommandBuffer() for vkCommandBuffer_transition_image_layout");
	}

	VkPipelineStageFlags vkPipelineStageFlags_source = 0;
	VkPipelineStageFlags vkPipelineStageFlags_destination = 0;

	VkImageMemoryBarrier vkImageMemoryBarrier;
	memset((void *)&vkImageMemoryBarrier, 0, sizeof(VkImageMemoryBarrier));

	vkImageMemoryBarrier.sType 				= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	vkImageMemoryBarrier.pNext 				= NULL;
	vkImageMemoryBarrier.srcAccessMask 		= vkPipelineStageFlags_source;
	vkImageMemoryBarrier.dstAccessMask 		= vkPipelineStageFlags_destination;
	vkImageMemoryBarrier.oldLayout 			= VK_IMAGE_LAYOUT_UNDEFINED;
	vkImageMemoryBarrier.newLayout 			= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	vkImageMemoryBarrier.srcQueueFamilyIndex 	= VK_QUEUE_FAMILY_IGNORED;
	vkImageMemoryBarrier.dstQueueFamilyIndex 	= VK_QUEUE_FAMILY_IGNORED;
	vkImageMemoryBarrier.image 				= vkImage_texture;
	vkImageMemoryBarrier.subresourceRange.aspectMask 	= VK_IMAGE_ASPECT_COLOR_BIT;
	vkImageMemoryBarrier.subresourceRange.baseArrayLayer = 0;
	vkImageMemoryBarrier.subresourceRange.baseMipLevel 	= 0;
	vkImageMemoryBarrier.subresourceRange.layerCount 	= 1;
	vkImageMemoryBarrier.subresourceRange.levelCount 	= 1;

	if((vkImageMemoryBarrier.oldLayout == VK_IMAGE_LAYOUT_UNDEFINED) && (vkImageMemoryBarrier.newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL))
	{
		vkImageMemoryBarrier.srcAccessMask 	= 0;
		vkImageMemoryBarrier.dstAccessMask 	= VK_ACCESS_TRANSFER_WRITE_BIT;
		vkPipelineStageFlags_source 		= VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		vkPipelineStageFlags_destination 	= VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if((vkImageMemoryBarrier.oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) && (vkImageMemoryBarrier.newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL))
	{
		vkImageMemoryBarrier.srcAccessMask 	= VK_ACCESS_TRANSFER_WRITE_BIT;
		vkImageMemoryBarrier.dstAccessMask 	= VK_ACCESS_SHADER_READ_BIT;
		vkPipelineStageFlags_source 		= VK_PIPELINE_STAGE_TRANSFER_BIT;
		vkPipelineStageFlags_destination 	= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else
	{
		fprintf(gpFile, "\nDEBUG:[CreateTexture][ERROR] Invalid Texture Layout Transition");
		fclose(fp);
		return VK_ERROR_INITIALIZATION_FAILED;
	}

	vkCmdPipelineBarrier(	vkCommandBuffer_transition_image_layout,
							vkPipelineStageFlags_source,
							vkPipelineStageFlags_destination,
							0,
							0,
							NULL,
							0,
							NULL,
							1,
							&vkImageMemoryBarrier );

	vkResult = vkEndCommandBuffer(vkCommandBuffer_transition_image_layout);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nDEBUG:[CreateTexture][ERROR] Failed vkEndCommandBuffer() for vkCommandBuffer_transition_image_layout");
		fclose(fp);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nDEBUG:[CreateTexture][SUCCESS] vkEndCommandBuffer() for vkCommandBuffer_transition_image_layout");
	}

	VkSubmitInfo vkSubmitInfo_transition_image_layout;
	memset((void *)&vkSubmitInfo_transition_image_layout, 0, sizeof(VkSubmitInfo));
	
	vkSubmitInfo_transition_image_layout.sType 				= VK_STRUCTURE_TYPE_SUBMIT_INFO;
	vkSubmitInfo_transition_image_layout.pNext 				= NULL;
	vkSubmitInfo_transition_image_layout.waitSemaphoreCount = 0;
	vkSubmitInfo_transition_image_layout.pWaitSemaphores 	= NULL;
	vkSubmitInfo_transition_image_layout.pWaitDstStageMask 	= NULL;
	vkSubmitInfo_transition_image_layout.commandBufferCount = 1;
	vkSubmitInfo_transition_image_layout.pCommandBuffers 	= &vkCommandBuffer_transition_image_layout;
	vkSubmitInfo_transition_image_layout.signalSemaphoreCount = 0;
	vkSubmitInfo_transition_image_layout.pSignalSemaphores 	= NULL;

	vkResult = vkQueueSubmit(vkQueue, 1, &vkSubmitInfo_transition_image_layout, VK_NULL_HANDLE);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nDEBUG:[CreateTexture][ERROR] Failed vkQueueSubmit() for vkCommandBuffer_transition_image_layout");
		fclose(fp);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nDEBUG:[CreateTexture][SUCCESS] vkQueueSubmit() for vkCommandBuffer_transition_image_layout");
	}

	vkResult = vkQueueWaitIdle(vkQueue);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nDEBUG:[CreateTexture][ERROR] Failed vkQueueWaitIdle() for vkCommandBuffer_transition_image_layout");
		fclose(fp);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nDEBUG:[CreateTexture][SUCCESS] vkQueueWaitIdle() for vkCommandBuffer_transition_image_layout");
	}

	vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &vkCommandBuffer_transition_image_layout);
	vkCommandBuffer_transition_image_layout = VK_NULL_HANDLE;
	fprintf(gpFile, "\nDEBUG:[CreateTexture][SUCCESS] vkFreeCommandBuffers() for vkCommandBuffer_transition_image_layout");

	//5> copy texture data to staging buffer
	VkCommandBufferAllocateInfo vkCommandBufferAllocateInfo_buffer_to_image_copy;
	memset((void *)&vkCommandBufferAllocateInfo_buffer_to_image_copy, 0, sizeof(VkCommandBufferAllocateInfo));

	vkCommandBufferAllocateInfo_buffer_to_image_copy.sType 				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	vkCommandBufferAllocateInfo_buffer_to_image_copy.pNext 				= NULL;
	vkCommandBufferAllocateInfo_buffer_to_image_copy.commandPool 		= vkCommandPool;
	vkCommandBufferAllocateInfo_buffer_to_image_copy.commandBufferCount = 1;
	vkCommandBufferAllocateInfo_buffer_to_image_copy.level 				= VK_COMMAND_BUFFER_LEVEL_PRIMARY;

	VkCommandBuffer vkCommandBuffer_buffer_to_image_copy = VK_NULL_HANDLE;
	memset((void *)&vkCommandBuffer_buffer_to_image_copy, 0, sizeof(VkCommandBuffer));

	vkResult = vkAllocateCommandBuffers(vkDevice, &vkCommandBufferAllocateInfo_buffer_to_image_copy, &vkCommandBuffer_buffer_to_image_copy);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nDEBUG:[CreateTexture][ERROR] Failed vkAllocateCommandBuffers() buffer_to_image_copy");
		fclose(fp);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nDEBUG:[CreateTexture][SUCCESS] vkAllocateCommandBuffers() buffer_to_image_copy");
	}

	VkCommandBufferBeginInfo vkCommandBufferBeginInfo_buffer_to_image_copy;
	memset((void *)&vkCommandBufferBeginInfo_buffer_to_image_copy, 0, sizeof(VkCommandBufferBeginInfo));

	vkCommandBufferBeginInfo_buffer_to_image_copy.sType 			= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	vkCommandBufferBeginInfo_buffer_to_image_copy.pNext 			= NULL;
	vkCommandBufferBeginInfo_buffer_to_image_copy.flags 			= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkCommandBufferBeginInfo_buffer_to_image_copy.pInheritanceInfo = NULL;

	vkResult = vkBeginCommandBuffer(vkCommandBuffer_buffer_to_image_copy, &vkCommandBufferBeginInfo_buffer_to_image_copy);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nDEBUG:[CreateTexture][ERROR] Failed vkBeginCommandBuffer() buffer_to_image_copy");
		fclose(fp);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nDEBUG:[CreateTexture][SUCCESS] vkBeginCommandBuffer() buffer_to_image_copy");
	}
	
	VkBufferImageCopy vkBufferImageCopy;
	memset((void *)&vkBufferImageCopy, 0, sizeof(VkBufferImageCopy));

	vkBufferImageCopy.bufferOffset 					= 0;
	vkBufferImageCopy.bufferRowLength 				= 0;
	vkBufferImageCopy.bufferImageHeight 			= 0;
	vkBufferImageCopy.imageSubresource.aspectMask 	= VK_IMAGE_ASPECT_COLOR_BIT;
	vkBufferImageCopy.imageSubresource.mipLevel 	= 0;
	vkBufferImageCopy.imageSubresource.baseArrayLayer = 0;
	vkBufferImageCopy.imageSubresource.layerCount 	= 1;

	vkBufferImageCopy.imageOffset.x 				= 0;
	vkBufferImageCopy.imageOffset.y 				= 0;
	vkBufferImageCopy.imageOffset.z 				= 0;

	vkBufferImageCopy.imageExtent.width 			= texture_width;
	vkBufferImageCopy.imageExtent.height 			= texture_height;
	vkBufferImageCopy.imageExtent.depth 			= 1;

	vkCmdCopyBufferToImage(vkCommandBuffer_buffer_to_image_copy, vkBuffer_stagingBuffer, vkImage_texture, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &vkBufferImageCopy);

	vkResult = vkEndCommandBuffer(vkCommandBuffer_buffer_to_image_copy);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nDEBUG:[CreateTexture][ERROR] Failed vkEndCommandBuffer() buffer_to_image_copy");
		fclose(fp);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nDEBUG:[CreateTexture][SUCCESS] vkEndCommandBuffer() buffer_to_image_copy");
	}

	VkSubmitInfo vkSubmitInfo_buffer_to_image_copy;
	memset((void *)&vkSubmitInfo_buffer_to_image_copy, 0, sizeof(VkSubmitInfo));

	vkSubmitInfo_buffer_to_image_copy.sType 				= VK_STRUCTURE_TYPE_SUBMIT_INFO;
	vkSubmitInfo_buffer_to_image_copy.pNext 				= NULL;
	vkSubmitInfo_buffer_to_image_copy.waitSemaphoreCount 	= 0;
	vkSubmitInfo_buffer_to_image_copy.pWaitSemaphores 		= NULL;
	vkSubmitInfo_buffer_to_image_copy.pWaitDstStageMask 	= NULL;
	vkSubmitInfo_buffer_to_image_copy.commandBufferCount 	= 1;
	vkSubmitInfo_buffer_to_image_copy.pCommandBuffers 		= &vkCommandBuffer_buffer_to_image_copy;
	vkSubmitInfo_buffer_to_image_copy.signalSemaphoreCount 	= 0;
	vkSubmitInfo_buffer_to_image_copy.pSignalSemaphores 	= NULL;

	vkResult = vkQueueSubmit(vkQueue, 1, &vkSubmitInfo_buffer_to_image_copy, VK_NULL_HANDLE);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nDEBUG:[CreateTexture][ERROR] Failed vkQueueSubmit() buffer_to_image_copy");
		fclose(fp);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nDEBUG:[CreateTexture][SUCCESS] vkQueueSubmit() buffer_to_image_copy");
	}

	vkResult = vkQueueWaitIdle(vkQueue);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nDEBUG:[CreateTexture][ERROR] Failed vkQueueWaitIdle() buffer_to_image_copy");
		fclose(fp);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nDEBUG:[CreateTexture][SUCCESS] vkQueueWaitIdle() buffer_to_image_copy");
	}

	vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &vkCommandBuffer_buffer_to_image_copy);
	vkCommandBuffer_buffer_to_image_copy = VK_NULL_HANDLE;
	fprintf(gpFile, "\nDEBUG:[CreateTexture][SUCCESS] vkFreeCommandBuffers() buffer_to_image_copy");





	//6> Create image view for the texture image.
	//VkCommandBufferAllocateInfo vkCommandBufferAllocateInfo_transition_layout;
	memset((void *)&vkCommandBufferAllocateInfo_transition_layout, 0, sizeof(VkCommandBufferAllocateInfo));

	vkCommandBufferAllocateInfo_transition_layout.sType 		= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	vkCommandBufferAllocateInfo_transition_layout.pNext 		= NULL;
	vkCommandBufferAllocateInfo_transition_layout.commandPool 	= vkCommandPool;
	vkCommandBufferAllocateInfo_transition_layout.commandBufferCount = 1;
	vkCommandBufferAllocateInfo_transition_layout.level 		= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	
	//VkCommandBuffer vkCommandBuffer_transition_image_layout = VK_NULL_HANDLE;
	vkCommandBuffer_transition_image_layout = VK_NULL_HANDLE;
	memset((void *)&vkCommandBuffer_transition_image_layout, 0, sizeof(VkCommandBuffer));

	vkResult = vkAllocateCommandBuffers(vkDevice, &vkCommandBufferAllocateInfo_transition_layout, &vkCommandBuffer_transition_image_layout);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nDEBUG:[CreateTexture][ERROR] Failed vkAllocateCommandBuffers() vkCommandBuffer_transition_image_layout");
		fclose(fp);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nDEBUG:[CreateTexture][SUCCESS] vkAllocateCommandBuffers() vkCommandBuffer_transition_image_layout");
	}

	//VkCommandBufferBeginInfo vkCommandBufferBeginInfo_transition_image_layout;
	memset((void *)&vkCommandBufferBeginInfo_transition_image_layout, 0, sizeof(VkCommandBufferBeginInfo));

	vkCommandBufferBeginInfo_transition_image_layout.sType 			= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	vkCommandBufferBeginInfo_transition_image_layout.pNext 			= NULL;
	vkCommandBufferBeginInfo_transition_image_layout.flags 			= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; //VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
	vkCommandBufferBeginInfo_transition_image_layout.pInheritanceInfo = NULL;


	vkResult = vkBeginCommandBuffer(vkCommandBuffer_transition_image_layout, &vkCommandBufferBeginInfo_transition_image_layout);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nDEBUG:[CreateTexture][ERROR] Failed vkBeginCommandBuffer() Transition 2 step 6");
		fclose(fp);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nDEBUG:[CreateTexture][SUCCESS] vkBeginCommandBuffer() Transition 2 step 6");
	}

	//VkPipelineStageFlags vkPipelineStageFlags_source = 0;
	vkPipelineStageFlags_source = 0;
	//VkPipelineStageFlags vkPipelineStageFlags_destination = 0;
	vkPipelineStageFlags_destination = 0;

	//VkImageMemoryBarrier vkImageMemoryBarrier;
	memset((void *)&vkImageMemoryBarrier, 0, sizeof(VkImageMemoryBarrier));

	vkImageMemoryBarrier.sType 								= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	vkImageMemoryBarrier.pNext 								= NULL;
	vkImageMemoryBarrier.srcAccessMask 						= vkPipelineStageFlags_source;
	vkImageMemoryBarrier.dstAccessMask 						= vkPipelineStageFlags_destination;
	vkImageMemoryBarrier.oldLayout 							= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	vkImageMemoryBarrier.newLayout 							= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	vkImageMemoryBarrier.srcQueueFamilyIndex 				= VK_QUEUE_FAMILY_IGNORED;
	vkImageMemoryBarrier.dstQueueFamilyIndex 				= VK_QUEUE_FAMILY_IGNORED;
	vkImageMemoryBarrier.image 								= vkImage_texture;
	vkImageMemoryBarrier.subresourceRange.aspectMask 		= VK_IMAGE_ASPECT_COLOR_BIT;
	vkImageMemoryBarrier.subresourceRange.baseArrayLayer 	= 0;
	vkImageMemoryBarrier.subresourceRange.baseMipLevel 		= 0;
	vkImageMemoryBarrier.subresourceRange.layerCount 		= 1;
	vkImageMemoryBarrier.subresourceRange.levelCount 		= 1;

	if((vkImageMemoryBarrier.oldLayout == VK_IMAGE_LAYOUT_UNDEFINED) && (vkImageMemoryBarrier.newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL))
	{
		vkImageMemoryBarrier.srcAccessMask 	= 0;
		vkImageMemoryBarrier.dstAccessMask 	= VK_ACCESS_TRANSFER_WRITE_BIT;
		vkPipelineStageFlags_source 		= VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		vkPipelineStageFlags_destination 	= VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if((vkImageMemoryBarrier.oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) && (vkImageMemoryBarrier.newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL))
	{
		vkImageMemoryBarrier.srcAccessMask 	= VK_ACCESS_TRANSFER_WRITE_BIT;
		vkImageMemoryBarrier.dstAccessMask 	= VK_ACCESS_SHADER_READ_BIT;
		vkPipelineStageFlags_source 		= VK_PIPELINE_STAGE_TRANSFER_BIT;
		vkPipelineStageFlags_destination 	= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else
	{
		fprintf(gpFile, "\nDEBUG:[CreateTexture][ERROR] Invalid Texture Layout Transition 2 step 6");
		fclose(fp);
		return VK_ERROR_INITIALIZATION_FAILED;
	}

	vkCmdPipelineBarrier(	vkCommandBuffer_transition_image_layout,
							vkPipelineStageFlags_source,
							vkPipelineStageFlags_destination,
							0,
							0,
							NULL,
							0,
							NULL,
							1,
							&vkImageMemoryBarrier );

	vkResult = vkEndCommandBuffer(vkCommandBuffer_transition_image_layout);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nDEBUG:[CreateTexture][ERROR] Failed vkEndCommandBuffer() Transition 2 step 6");
		fclose(fp);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nDEBUG:[CreateTexture][SUCCESS] vkEndCommandBuffer() Transition 2 step 6");
	}

	//VkSubmitInfo vkSubmitInfo_transition_image_layout;
	memset((void *)&vkSubmitInfo_transition_image_layout, 0, sizeof(VkSubmitInfo));
	
	vkSubmitInfo_transition_image_layout.sType 				= VK_STRUCTURE_TYPE_SUBMIT_INFO;
	vkSubmitInfo_transition_image_layout.pNext 				= NULL;
	vkSubmitInfo_transition_image_layout.waitSemaphoreCount = 0;
	vkSubmitInfo_transition_image_layout.pWaitSemaphores 	= NULL;
	vkSubmitInfo_transition_image_layout.pWaitDstStageMask 	= NULL;
	vkSubmitInfo_transition_image_layout.commandBufferCount = 1;
	vkSubmitInfo_transition_image_layout.pCommandBuffers 	= &vkCommandBuffer_transition_image_layout;
	vkSubmitInfo_transition_image_layout.signalSemaphoreCount = 0;
	vkSubmitInfo_transition_image_layout.pSignalSemaphores 	= NULL;

	vkResult = vkQueueSubmit(vkQueue, 1, &vkSubmitInfo_transition_image_layout, VK_NULL_HANDLE);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nDEBUG:[CreateTexture][ERROR] Failed vkQueueSubmit() Transition 2 step 6");
		fclose(fp);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nDEBUG:[CreateTexture][SUCCESS] vkQueueSubmit() Transition 2 step 6");
	}

	vkResult = vkQueueWaitIdle(vkQueue);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nDEBUG:[CreateTexture][ERROR] Failed vkQueueWaitIdle() Transition 2 step 6");
		fclose(fp);
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nDEBUG:[CreateTexture][SUCCESS] vkQueueWaitIdle() Transition 2 step 6");
	}

	vkFreeCommandBuffers(vkDevice, vkCommandPool, 1, &vkCommandBuffer_transition_image_layout);
	vkCommandBuffer_transition_image_layout = VK_NULL_HANDLE;
	fprintf(gpFile, "\nDEBUG:[CreateTexture][SUCCESS] vkFreeCommandBuffers() Transition 2 step 6");

	//7> Uninitialize
	//free buffer for staging buufer
	if(vkBuffer_stagingBuffer != VK_NULL_HANDLE)
	{
		vkDestroyBuffer(vkDevice, vkBuffer_stagingBuffer, NULL);
		vkBuffer_stagingBuffer = VK_NULL_HANDLE;
		fprintf(gpFile, "\nDEBUG:[CreateTexture][SUCCESS] vkDestroyBuffer() staging buffer");
	}

	//free device memory for staging buffer
	if(vkDeviceMemory_stagingBuffer != VK_NULL_HANDLE)
	{
		vkFreeMemory(vkDevice, vkDeviceMemory_stagingBuffer, NULL);
		vkDeviceMemory_stagingBuffer = VK_NULL_HANDLE;
		fprintf(gpFile, "\nDEBUG:[CreateTexture][SUCCESS] vkFreeMemory() staging buffer memory");
	}

	//create image view for texture image
	VkImageViewCreateInfo vkImageViewCreateInfo_texture;
	memset((void*)&vkImageViewCreateInfo_texture, 0, sizeof(VkImageViewCreateInfo));

	vkImageViewCreateInfo_texture.sType 							= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	vkImageViewCreateInfo_texture.pNext 							= NULL;
	vkImageViewCreateInfo_texture.flags 							= 0;
	vkImageViewCreateInfo_texture.image 							= vkImage_texture;
	vkImageViewCreateInfo_texture.viewType 							= VK_IMAGE_VIEW_TYPE_2D;
	vkImageViewCreateInfo_texture.format 							= VK_FORMAT_R8G8B8A8_UNORM; //VK_FORMAT_R8G8B8A8_SRGB;
	vkImageViewCreateInfo_texture.subresourceRange.aspectMask 		= VK_IMAGE_ASPECT_COLOR_BIT;
	vkImageViewCreateInfo_texture.subresourceRange.baseMipLevel 	= 0;
	vkImageViewCreateInfo_texture.subresourceRange.levelCount 		= 1;
	vkImageViewCreateInfo_texture.subresourceRange.baseArrayLayer 	= 0;
	vkImageViewCreateInfo_texture.subresourceRange.layerCount 		= 1;

	vkResult = vkCreateImageView(vkDevice, &vkImageViewCreateInfo_texture, NULL, &vkImageView_texture);
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[createImagesAndImageViews][ERROR] Failed vkCreateImageView() for texture image");
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nDEBUG:[createImagesAndImageViews][SUCCESS] vkCreateImageView() for texture image");
	}




	//9> create sampler
	VkSamplerCreateInfo vkSamplerCreateInfo;
	memset((void *)&vkSamplerCreateInfo, 0, sizeof(VkSamplerCreateInfo));

	vkSamplerCreateInfo.sType 						= VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	vkSamplerCreateInfo.pNext 						= NULL;
	vkSamplerCreateInfo.flags 						= 0;
	vkSamplerCreateInfo.magFilter 					= VK_FILTER_LINEAR;
	vkSamplerCreateInfo.minFilter 					= VK_FILTER_LINEAR;
	vkSamplerCreateInfo.mipmapMode 					= VK_SAMPLER_MIPMAP_MODE_LINEAR;

	vkSamplerCreateInfo.addressModeU 				= VK_SAMPLER_ADDRESS_MODE_REPEAT;
	vkSamplerCreateInfo.addressModeV 				= VK_SAMPLER_ADDRESS_MODE_REPEAT;
	vkSamplerCreateInfo.addressModeW 				= VK_SAMPLER_ADDRESS_MODE_REPEAT;
	
	vkSamplerCreateInfo.anisotropyEnable 			= VK_FALSE;
	vkSamplerCreateInfo.maxAnisotropy 				= 16;
	vkSamplerCreateInfo.borderColor 				= VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	vkSamplerCreateInfo.unnormalizedCoordinates 	= VK_FALSE;
	vkSamplerCreateInfo.compareEnable 				= VK_FALSE;
	vkSamplerCreateInfo.compareOp 					= VK_COMPARE_OP_ALWAYS;
	
	vkSamplerCreateInfo.minLod 						= 0;
	vkSamplerCreateInfo.maxLod 						= 0;

	vkResult = vkCreateSampler(vkDevice, &vkSamplerCreateInfo, NULL, &vkSampler_texture);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nDEBUG:[createImagesAndImageViews][ERROR] Failed vkCreateSampler() for texture");
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nDEBUG:[createImagesAndImageViews][SUCCESS] vkCreateSampler() for texture");
	}

	fclose(fp);
	return vkResult;
}
/*--------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------*/
VkResult createUniformBuffers(void)
{
	//function declarations
	VkResult updateUniformBuffer(void);
	
	//variable declarations
	VkResult vkResult = VK_SUCCESS;

	//code
	fprintf(gpFile, "\n\nDEBUG:------------------------------------Inside[createUniformBuffers()]------------------------------------");

	VkBufferCreateInfo vkBufferCreateInfo;
	memset((void *)&vkBufferCreateInfo, 0, sizeof(VkBufferCreateInfo));

	//   it has 8 members, we will use 5. out of 2 are very important.
	vkBufferCreateInfo.sType 	= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vkBufferCreateInfo.pNext 	= NULL;
	vkBufferCreateInfo.flags 	= 0; //valid flags are used in scatterd buffers/sparse buffers
	vkBufferCreateInfo.size 	= sizeof(struct MyUniformData); //size of the buffer in bytes
	vkBufferCreateInfo.usage 	= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT; //VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

	//memset((void *)&vertexData_position, 0, sizeof(VertexData));
	memset((void *)&uniformData_cube, 0, sizeof(struct UniformData));

	//6> Call vkCreateBuffer() vulkan API in the ".vkBuffer" member of our global struct.
	vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo, NULL, &uniformData_cube.vkBuffer);
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[createUniformBuffers][ERROR] Failed vkCreateBuffer()");
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nDEBUG:[createUniformBuffers][SUCCESS] vkCreateBuffer()");
	}

	//7> Declare and memset struct VkMemoryRequirements and then call vkGetBufferMemoryRequirements() API
	VkMemoryRequirements vkMemoryRequirements;
	memset((void *)&vkMemoryRequirements, 0, sizeof(VkMemoryRequirements));

	vkGetBufferMemoryRequirements(vkDevice, uniformData_cube.vkBuffer, &vkMemoryRequirements);

	//8> To acually allocate the required memomry, we need to call vkAllocateMemory() API. But before that declare and
	//   memset VkMemoryAllocateInfo structure.
	VkMemoryAllocateInfo vkMemoryAllocateInfo;
	memset((void *)&vkMemoryAllocateInfo, 0, sizeof(VkMemoryAllocateInfo));

	vkMemoryAllocateInfo.sType 				= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	vkMemoryAllocateInfo.pNext 				= NULL;
	vkMemoryAllocateInfo.allocationSize 	= vkMemoryRequirements.size; //size of the buffer in bytes
	vkMemoryAllocateInfo.memoryTypeIndex 	= 0; //will be set in next step
											  // initial value before loop

	//loop to find memoryTypeIndex
	//9> important members of this structure are ".memoryTypeIndex" and ".allocationSize". For allocation size use the size
	//   obtained from memory requirements.
	//10> For memoryTypeIndex:
	//		a> Start a loop with counter as vkPhysiacalDeviceMemoryProperties.memoryTypeCount.
	//		b> Inside the loop check vkmemoryrequirements memoryTypebits contains '1' or not. if yes
	//		c> check vkPhysiacalDeviceMemoryProperties.memoryTypes[i].propertyFlags member coNtains
	//		   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
	//		d> then ith index will be our memoryType index. if found break out of the loop.
	//		e> if not found, continue the loop by right shifting vkmemoryrequirements.memoryType bits by 1 over each iteration.
	for(uint32_t i = 0; i < vkPhysicalDeviceMemoryProperties.memoryTypeCount; i++)
	{
		if((vkMemoryRequirements.memoryTypeBits & 1) == 1)
		{
			if((vkPhysicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT))
			{
				vkMemoryAllocateInfo.memoryTypeIndex = i;
				fprintf(gpFile, "\nDEBUG:[createUniformBuffers][SUCCESS] vkMemoryAllocateInfo.memoryTypeIndex = %d", vkMemoryAllocateInfo.memoryTypeIndex);
				break;
			}
		}
		else
		{
			vkMemoryRequirements.memoryTypeBits = vkMemoryRequirements.memoryTypeBits >> 1;
			fprintf(gpFile, "\nDEBUG:[createUniformBuffers][INFO] vkMemoryRequirements.memoryTypeBits = %d", vkMemoryRequirements.memoryTypeBits);
		}
	}

	//11> Now call vkAllocateMemory() and get the required vulkan memory objects handle into the ".vkDeviceMemory" member of global variable.
	vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &uniformData_cube.vkDeviceMemory);
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[createUniformBuffers][ERROR] Failed vkAllocateMemory()");
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nDEBUG:[createUniformBuffers][SUCCESS] vkAllocateMemory()");
	}
	
	//12> Now we have our reqired memory as well as vkBuffer handle. binnd this device memory handle to the vulkan buffer handle by using
	//    vkBindBufferMemory().
	vkResult = vkBindBufferMemory(vkDevice, uniformData_cube.vkBuffer, uniformData_cube.vkDeviceMemory, 0);
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[createUniformBuffers][ERROR] Failed vkBindBufferMemory()");
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nDEBUG:[createUniformBuffers][SUCCESS] vkBindBufferMemory()");
	}





	//call updateUniformBuffer() to fill the uniform buffer with initial data
	vkResult = updateUniformBuffer();
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[createUniformBuffers][ERROR] Failed updateUniformBuffer()");
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nDEBUG:[createUniformBuffers][SUCCESS] updateUniformBuffer()");
	}

	//end
	fprintf(gpFile, "\nDEBUG:------------------------------------Done [createUniformBuffers()]------------------------------------\n");
	return vkResult;
}
/*--------------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------*/
VkResult updateUniformBuffer(void)
{
	//variable declarations
	VkResult vkResult = VK_SUCCESS;
	MyUniformData myUniformData;

	//code
	//fprintf(gpFile, "\n\nDEBUG:------------------------------------Inside[updateUniformBuffer()]------------------------------------");

	memset((void *)&myUniformData, 0, sizeof(struct MyUniformData));
	
	//fill the uniform data
	//update matrices
	glm::mat4 modelMatrix 					= glm::mat4(1.0f);		//identity matrix
	glm::mat4 viewMatrix 					= glm::mat4(1.0f);		//identity matrix
	glm::mat4 translationMatrix 			= glm::mat4(1.0f);		//identity matrix
	glm::mat4 rotationMatrix_x 				= glm::mat4(1.0f);		//identity matrix
	glm::mat4 rotationMatrix_y 				= glm::mat4(1.0f);		//identity matrix
	glm::mat4 rotationMatrix_z 				= glm::mat4(1.0f);		//identity matrix
	glm::mat4 rotationMatrix 				= glm::mat4(1.0f);		//identity matrix
	glm::mat4 scaleMatrix 					= glm::mat4(1.0f);		//identity matrix
	glm::mat4 projectionMatrix 				= glm::mat4(1.0f);		//identity matrix
	glm::mat4 perspectiveProjectionMatrix 	= glm::mat4(1.0f);		//identity matrix

	//matrices operations
	translationMatrix 				= glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -5.0f)); 					//translate to -Z axis
	rotationMatrix_x 				= glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(1.0f, 0.0f, 0.0f)); 	//rotate around Y axis
	rotationMatrix_y 				= glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f)); 	//rotate around Y axis
	rotationMatrix_z 				= glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 0.0f, 1.0f)); 	//rotate around Y axis
	rotationMatrix 					= rotationMatrix_x * rotationMatrix_y * rotationMatrix_z;
	scaleMatrix 					= glm::scale(glm::mat4(1.0f), glm::vec3(0.75f, 0.75f, 0.75f)); //scale matrix
	modelMatrix 					= translationMatrix * scaleMatrix * rotationMatrix; //model matrix = translation * rotation
	projectionMatrix 				= modelMatrix * viewMatrix; 														//model * view

	//create perspective projection matrix
	perspectiveProjectionMatrix = glm::perspective(glm::radians(45.0f), (float)winWidth / (float)winHeight, 0.1f, 100.0f);
	//invert Y axis for Vulkan
	perspectiveProjectionMatrix[1][1] = perspectiveProjectionMatrix[1][1] * (-1.0f); //invert Y axis for Vulkan

	//fill the uniform data
	myUniformData.modelMatrix 		= glm::mat4(1.0f);		//identity matrix
	myUniformData.viewMatrix 		= glm::mat4(1.0f);		//identity matrix
	myUniformData.projectionMatrix  = glm::mat4(1.0f);		//identity matrix

	myUniformData.modelMatrix 		= modelMatrix;
	myUniformData.viewMatrix 		= viewMatrix;
	myUniformData.projectionMatrix  = perspectiveProjectionMatrix;


	//map the uniform buffer memory
	//fprintf(gpFile, "\nDEBUG:------------------------------------Mapping Uniform Buffer Memory------------------------------------");
	void *data = NULL;
	
	vkResult = vkMapMemory(vkDevice, uniformData_cube.vkDeviceMemory, 0, sizeof(struct UniformData), 0, &data);
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[updateUniformBuffer][ERROR] Failed vkMapMemory()");
		return vkResult;
	}

	//copy data to the mapped memory
	memcpy(data, &myUniformData, sizeof(MyUniformData));
	//fprintf(gpFile, "\nDEBUG:[updateUniformBuffer][SUCCESS] memcpy()");

	//unmap the memory
	vkUnmapMemory(vkDevice, uniformData_cube.vkDeviceMemory);

	//fprintf(gpFile, "\nDEBUG:------------------------------------Done [updateUniformBuffer()]------------------------------------\n");
	return vkResult;
}
/*--------------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------------
--------------------------------------------------------------------------------------------------------
23-Shaders
--------------------------------------------------------------------------------------------------------
1> Write shaders and compile them to spir-v using shader compilation tools that we received Vulkan SDK.
2> Globally declare shader module object variables of VkShaderModuleType to hold Vulkan compatible vertex shader and
   fragment shader object respectively. 
3> Declare prototype of UDF say cretaeShader() following convention i.e. after createVertex() and before createRenderpass().
4> Using above same convetions call createShaders() between the calls of above two.
5> maintainng the same above convetions for defining createShaders() between the definitions of above two.
6> inside our function first open the shader file, set the file pointer at the end of the file EOF, find the byte size of shader file data,
   reset the file poiter at the begining of the file, allocate a character buffer of file size and read shader file data into it.
   and finally close the file. do all this using using convetional File IO.
7> Declare and memset struct VkShaderModuleCreateInfo and specify above file size and buffer while initializing it.
8> Call vkShaderModuleCreate() pass above struct parameter and obtain shader module object that we declared globally in step 2.
9> Free the shader code buffer in step 6.
10> Assuming we did above four steps 6 to 9 for vertex shader, repeat them all for fragment shader too.
11> In uninitialize() destoy both global shader objects using vkDestroyShaderModule() Vulkan API.
---------------------------------------------------------------------------------------------------------*/
VkResult createShaders(void)
{
	//variable declaration
	VkResult vkResult = VK_SUCCESS;

	//code
	fprintf(gpFile, "\n\nDEBUG:------------------------------------Inside[createShaders()]------------------------------------");

	const char *szFileName = "shader.vert.spv";
	size_t fileSize = 0;
	FILE *fp = fopen(szFileName, "rb");
	if (fp == NULL)
	{
		fprintf(gpFile, "\nDEBUG:[createShaders][ERROR] Failed fopen() shader.vert.spv");
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	else
		fprintf(gpFile, "\nDEBUG:[createShaders][SUCCESS] fopen() shader.vert.spv");

	fseek(fp, 0L, SEEK_END);
	fileSize = ftell(fp);
	if (fileSize == 0)
	{
		fprintf(gpFile, "\nDEBUG:[createShaders][ERROR] Failed ftell() shader.vert.spv fileSize = %zd", fileSize);
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	else
		fprintf(gpFile, "\nDEBUG:[createShaders][SUCCESS] ftell() shader.vert.spv fileSize = %zd", fileSize);

	fseek(fp, 0L, SEEK_SET);

	char *shaderData = (char *)malloc(sizeof(char) * fileSize);
	if (shaderData == NULL)
	{
		fprintf(gpFile, "\nDEBUG:[createShaders][ERROR] Failed malloc() shaderData");
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	else
		fprintf(gpFile, "\nDEBUG:[createShaders][SUCCESS] malloc() shaderData");

	size_t retVal = fread(shaderData, sizeof(char), fileSize, fp);
	if (retVal != fileSize)
	{
		fprintf(gpFile, "\nDEBUG:[createShaders][ERROR] Failed fread() shaderData = %zd", retVal);
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	else
		fprintf(gpFile, "\nDEBUG:[createShaders][SUCCESS] fread() shaderData");
	
	fclose(fp);
	fp = NULL;

	VkShaderModuleCreateInfo vkShaderModuleCreateInfo;
	memset((void *)&vkShaderModuleCreateInfo, 0, sizeof(VkShaderModuleCreateInfo));

	vkShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	vkShaderModuleCreateInfo.pNext = NULL;
	vkShaderModuleCreateInfo.flags = 0;		//VK_SHADER_MODULE_CREATE_DEVICE_ADDRESS_BIT_EXT; //valid only for device address extension
	vkShaderModuleCreateInfo.codeSize = fileSize;
	vkShaderModuleCreateInfo.pCode = (const uint32_t *)shaderData;


	vkResult = vkCreateShaderModule(vkDevice, &vkShaderModuleCreateInfo, NULL, &vkShaderModule_vertex_shader);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nDEBUG:[createShaders][ERROR] Failed vkCreateShaderModule() vertex shader");
		return vkResult;
	}
	else
		fprintf(gpFile, "\nDEBUG:[createShaders][SUCCESS] vkCreateShaderModule() vertex shader");

	free(shaderData);
	shaderData = NULL;
	
	fprintf(gpFile, "\nDEBUG:[createShaders][SUCCESS] vkCreateShaderModule() vertex shader module creted successfully");

	//fragment shader
	szFileName = "shader.frag.spv";
	fileSize = 0;
	fp = fopen(szFileName, "rb");
	if (fp == NULL)
	{
		fprintf(gpFile, "\nDEBUG:[createShaders][ERROR] Failed fopen() shader.frag.spv");
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	else
		fprintf(gpFile, "\nDEBUG:[createShaders][SUCCESS] fopen() shader.frag.spv");
	
	fseek(fp, 0L, SEEK_END);
	fileSize = ftell(fp);
	if (fileSize == 0)
	{
		fprintf(gpFile, "\nDEBUG:[createShaders][ERROR] Failed ftell() shader.frag.spv fileSize = %zd", fileSize);
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	else
		fprintf(gpFile, "\nDEBUG:[createShaders][SUCCESS] ftell() shader.frag.spv fileSize = %zd", fileSize);
	
	fseek(fp, 0L, SEEK_SET);

	char *shaderData2 = (char *)malloc(sizeof(char) * fileSize);
	if (shaderData2 == NULL)
	{
		fprintf(gpFile, "\nDEBUG:[createShaders][ERROR] Failed malloc() shaderData2");
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	else
		fprintf(gpFile, "\nDEBUG:[createShaders][SUCCESS] malloc() shaderData2");

	retVal = fread(shaderData2, sizeof(char), fileSize, fp);
	if (retVal != fileSize)
	{
		fprintf(gpFile, "\nDEBUG:[createShaders][ERROR] Failed fread() shaderData2 = %zd", retVal);
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	else
		fprintf(gpFile, "\nDEBUG:[createShaders][SUCCESS] fread() shaderData2");
	
	fclose(fp);
	fp = NULL;

	memset((void *)&vkShaderModuleCreateInfo, 0, sizeof(VkShaderModuleCreateInfo));

	vkShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	vkShaderModuleCreateInfo.pNext = NULL;
	vkShaderModuleCreateInfo.flags = 0;		//VK_SHADER_MODULE_CREATE_DEVICE_ADDRESS_BIT_EXT; //valid only for device address extension
	vkShaderModuleCreateInfo.codeSize = fileSize;
	vkShaderModuleCreateInfo.pCode = (const uint32_t *)shaderData2;

	vkResult = vkCreateShaderModule(vkDevice, &vkShaderModuleCreateInfo, NULL, &vkShaderModule_fragment_shader);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nDEBUG:[createShaders][ERROR] Failed vkCreateShaderModule() fragment shader");
		return vkResult;
	}
	else
		fprintf(gpFile, "\nDEBUG:[createShaders][SUCCESS] vkCreateShaderModule() fragment shader");
	
	free(shaderData2);
	shaderData2 = NULL;
	fprintf(gpFile, "\nDEBUG:[createShaders][SUCCESS] vkCreateShaderModule() fragment shader module creted successfully");

	fprintf(gpFile, "\nDEBUG:------------------------------------Done [createShaders()]------------------------------------\n");

	return vkResult;
}
/*--------------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------------	 
24-Descriptor-Set-Layout
--------------------------------------------------------------------------------------------------------	 
1> 
2> In initialize declare and call a UDF createDescriptorSetLayout() maintaining the convention of 
   declaring and calling it after createShader() and before createRenderPass()
3> While wrinting this UDF decalre memset and initailize() struct VkDescriptorSetLayoutCreateInfo
   perticularly its two members: 
   1> Binding count
   2> pBindings array
4> then call vkCreateDescriptorSetLayout() vulkan API with adress of above initailized structure
   and get the required vulkan object global VkDescriptorSetLayout in its last parameter.
5> In uninitialize(), call call vkDestroyDescriptorSetLayout() to destroy this vulkan object.
--------------------------------------------------------------------------------------------------------*/
VkResult createDescriptorSetLayout(void)
{
	//variable declaration
	VkResult vkResult = VK_SUCCESS;

	//code
	fprintf(gpFile, "\n\nDEBUG:------------------------------------Inside[createDescriptorSetLayout()]------------------------------------");

	//initialise descriptor set layout binding structure
	VkDescriptorSetLayoutBinding vkDescriptorSetLayoutBinding_array[2];				//0 for uniform, 1 is for texture image
	memset((void *)&vkDescriptorSetLayoutBinding_array, 0, sizeof(VkDescriptorSetLayoutBinding) * _ARRAYSIZE(vkDescriptorSetLayoutBinding_array));

	//vkDescriptorSetLayoutBinding assignments for mvp_uniforms
	vkDescriptorSetLayoutBinding_array[0].binding 			= 0; 									//binding index
	vkDescriptorSetLayoutBinding_array[0].descriptorType 	= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; 	//descriptor type
	vkDescriptorSetLayoutBinding_array[0].descriptorCount 	= 1; 									//descriptor count
	vkDescriptorSetLayoutBinding_array[0].stageFlags 		= VK_SHADER_STAGE_VERTEX_BIT; 			//shader stage flags
	vkDescriptorSetLayoutBinding_array[0].pImmutableSamplers = NULL; 								//sampler info, not used in this example

	//vkDescriptorSetLayoutBinding assignments for texture image and sampler
	vkDescriptorSetLayoutBinding_array[1].binding 			= 1; 											//binding index
	vkDescriptorSetLayoutBinding_array[1].descriptorType 	= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; 	//descriptor type
	vkDescriptorSetLayoutBinding_array[1].descriptorCount 	= 1; 											//descriptor count
	vkDescriptorSetLayoutBinding_array[1].stageFlags 		= VK_SHADER_STAGE_FRAGMENT_BIT; 				//shader stage flags
	vkDescriptorSetLayoutBinding_array[1].pImmutableSamplers = NULL; 										//sampler info, not used in this example

	//1> 
	//2> In initialize declare and call a UDF createDescriptorSetLayout() maintaining the convention of 
	//   declaring and calling it after createShader() and before createRenderPass()
	//3> While wrinting this UDF decalre memset and initailize() struct VkDescriptorSetLayoutCreateInfo
	//   perticularly its two members:
	VkDescriptorSetLayoutCreateInfo vkDescriptorSetLayoutCreateInfo;
	memset((void *)&vkDescriptorSetLayoutCreateInfo, 0, sizeof(VkDescriptorSetLayoutCreateInfo));

	vkDescriptorSetLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	vkDescriptorSetLayoutCreateInfo.pNext = NULL;
	vkDescriptorSetLayoutCreateInfo.flags = 0; 			//VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR; //valid only for push descriptor extension
	vkDescriptorSetLayoutCreateInfo.bindingCount = _ARRAYSIZE(vkDescriptorSetLayoutBinding_array); 	//binding count

	//In the shaders up till now we dont have any bindings,
	// so pbindings memmber of this structure wwas not used.
	// But in this example and hence fore we will use atleast one descriptor set, it to bind uniform buffer
	// and hence we need to use this member. with new type VkDescriptorSetLayoutBinding

	//assign the address of above vkDescriptorSetLayoutBinding to pBindings member of this structure
	//4> then call vkCreateDescriptorSetLayout() vulkan API with adress of above initailized structure
	//   and get the required vulkan object global VkDescriptorSetLayout in its last parameter
	vkDescriptorSetLayoutCreateInfo.pBindings = vkDescriptorSetLayoutBinding_array;

	vkResult = vkCreateDescriptorSetLayout(vkDevice, &vkDescriptorSetLayoutCreateInfo, NULL, &vkDescriptorSetLayout);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nDEBUG:[createDescriptorSetLayout][ERROR] Failed vkCreateDescriptorSetLayout()");
		return vkResult;
	}
	else
		fprintf(gpFile, "\nDEBUG:[createDescriptorSetLayout][SUCCESS] vkCreateDescriptorSetLayout()");

	fprintf(gpFile, "\nDEBUG:[createDescriptorSetLayout][SUCCESS] vkCreateDescriptorSetLayout()");

	fprintf(gpFile, "\nDEBUG:------------------------------------Done [createDescriptorSetLayout()]------------------------------------\n");
	return vkResult;
}
/*--------------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------------
25-Pipeline-Layout
--------------------------------------------------------------------------------------------------------
1> Declare a global vulkan object VkPipelinelayout and initailise to null
2> In initialise declare and call a UDF, createPipelineLayout() maintaining convention of declaring and calling it 
   after createDescriptorSetLayout() and before
2> while writing declare memset and initailise vkPipelineCreateInfo structure varaible with
   four important members
   1> setLayoutCount
   2> pSetLayouts
   3> pushConstantRangeCount
   4> pPushConstantRanges
3> call vkCreatePipelineLayout() vulkan api with above initailized adrress varibales.
   with VkPipelinelayout as last variable.
4> in uninitialize() destroy this pipeline using vkDestroyPipelineLayput() api.
*--------------------------------------------------------------------------------------------------------*/
VkResult createPipelineLayout(void)
{
	//variable declaration
	VkResult vkResult = VK_SUCCESS;

	//code
	fprintf(gpFile, "\n\nDEBUG:------------------------------------Inside[createPipelineLayout()]------------------------------------");

	//1> Declare a global vulkan object VkPipelinelayout and initailise to null
	vkPipelineLayout = NULL;

	//2> In initialise declare and call a UDF, createPipelineLayout() maintaining convention of declaring and calling it 
	//   after createDescriptorSetLayout() and before
	//3> while writing declare memset and initailise vkPipelineCreateInfo structure varaible with
	//   four important members
	VkPipelineLayoutCreateInfo vkPipelineLayoutCreateInfo;
	memset((void *)&vkPipelineLayoutCreateInfo, 0, sizeof(VkPipelineLayoutCreateInfo));

	vkPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	vkPipelineLayoutCreateInfo.pNext = NULL;
	vkPipelineLayoutCreateInfo.flags = 0; 								//VK_PIPELINE_LAYOUT_CREATE_INDEPENDENT_SETS_BIT_EXT; //valid only for independent sets extension
	vkPipelineLayoutCreateInfo.setLayoutCount = 1; 						//set layout count
	vkPipelineLayoutCreateInfo.pSetLayouts = &vkDescriptorSetLayout; 	//set layout array address
	vkPipelineLayoutCreateInfo.pushConstantRangeCount = 0; 				//push constant range count
	vkPipelineLayoutCreateInfo.pPushConstantRanges = NULL; 				//push constant range array address

	//3> call vkCreatePipelineLayout() vulkan api with above initailized adrress varibales.
	//   with VkPipelinelayout as last variable.
	vkResult = vkCreatePipelineLayout(vkDevice, &vkPipelineLayoutCreateInfo, NULL, &vkPipelineLayout);
	if (vkResult != VK_SUCCESS)
	{
		fprintf(gpFile, "\nDEBUG:[createPipelineLayout][ERROR] Failed vkCreatePipelineLayout()");
		return vkResult;
	}
	else
		fprintf(gpFile, "\nDEBUG:[createPipelineLayout][SUCCESS] vkCreatePipelineLayout()");

	fprintf(gpFile, "\nDEBUG:------------------------------------Done [createPipelineLayout()]------------------------------------\n");
	return vkResult;
}
/*--------------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------------*/
VkResult createDescriptorPool(void)
{
	
	//variable declaration
	VkResult vkResult = VK_SUCCESS;

	//code
	fprintf(gpFile, "\n\nDEBUG:------------------------------------Inside[createDescriptorPool()]------------------------------------");
	//NOTE: Before creating descriptor pool, Vulkan expects size of the descriptor pool.
	//1> Declare a global vulkan object VkDescriptorPool and initailise to null
	//2> Declare and memset VkDescriptorPoolSize structure.
	
	VkDescriptorPoolSize vkDescriptorPoolSize_array[2]; //1 for mvp_UBO //2 for texture
	memset((void *)&vkDescriptorPoolSize_array, 0, sizeof(VkDescriptorPoolSize)* _ARRAYSIZE(vkDescriptorPoolSize_array));

	//for mvp uniform buffer
	vkDescriptorPoolSize_array[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; //descriptor type
	vkDescriptorPoolSize_array[0].descriptorCount = 1; //descriptor count

	//for texture
	vkDescriptorPoolSize_array[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; //descriptor type
	vkDescriptorPoolSize_array[1].descriptorCount = 1; //descriptor count
	//1> Declare and memset VkDescriptorPoolCreateInfo structure.
	VkDescriptorPoolCreateInfo vkDescriptorPoolCreateInfo;
	memset((void *)&vkDescriptorPoolCreateInfo, 0, sizeof(VkDescriptorPoolCreateInfo));

	vkDescriptorPoolCreateInfo.sType 			= VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	vkDescriptorPoolCreateInfo.pNext 			= NULL;
	vkDescriptorPoolCreateInfo.flags 			= 0; //VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT; //valid only for free descriptor set extension
	vkDescriptorPoolCreateInfo.maxSets 			= 2; //max sets in the pool
	vkDescriptorPoolCreateInfo.poolSizeCount 	= _ARRAYSIZE(vkDescriptorPoolSize_array); //pool size count

	//3> Assign above VkDescriptorPoolSize structure address to ppoolSizes member of VkDescriptorPoolCreateInfo structure.
	vkDescriptorPoolCreateInfo.pPoolSizes = vkDescriptorPoolSize_array;

	//4> Now call vkCreateVulkanRenderPass() to create the actual renderpass.
	vkResult = vkCreateDescriptorPool(vkDevice, &vkDescriptorPoolCreateInfo, NULL, &vkDescriptorPool);
	if (vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[createDescriptorPool][ERROR] Failed vkCreateDescriptorPool()");
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nDEBUG:[createDescriptorPool][SUCCESS] vkCreateDescriptorPool()");
	}
	fprintf(gpFile, "\nDEBUG:------------------------------------Done [createDescriptorPool()]------------------------------------\n");
	return vkResult;

}	
/*--------------------------------------------------------------------------------------------------------*/
VkResult createDescriptorSet(void)
{
	//variable declaration
	VkResult vkResult = VK_SUCCESS;

	//code
	fprintf(gpFile, "\n\nDEBUG:------------------------------------Inside[createDescriptorSet()]------------------------------------");

	//1> Declare a global vulkan object VkDescriptorSet and initailise to null
	vkDescriptorSet_cube = NULL;

	//2> Declare and memset VkDescriptorSetAllocateInfo structure.
	VkDescriptorSetAllocateInfo vkDescriptorSetAllocateInfo;
	memset((void *)&vkDescriptorSetAllocateInfo, 0, sizeof(VkDescriptorSetAllocateInfo));

	//vkDescriptorSetAllocateInfo assignments
	vkDescriptorSetAllocateInfo.sType 				= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	vkDescriptorSetAllocateInfo.pNext 				= NULL;
	vkDescriptorSetAllocateInfo.descriptorPool 		= vkDescriptorPool; //descriptor pool
	vkDescriptorSetAllocateInfo.descriptorSetCount 	= 1; //descriptor set count
	vkDescriptorSetAllocateInfo.pSetLayouts 		= &vkDescriptorSetLayout; //descriptor set layout array address

	//3> Now call vkCreateVulkanRenderPass() to create the actual renderpass.
	vkResult = vkAllocateDescriptorSets(vkDevice, &vkDescriptorSetAllocateInfo, &vkDescriptorSet_cube);
	if (vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[createDescriptorSet][ERROR] Failed vkAllocateDescriptorSets()");
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nDEBUG:[createDescriptorSet][SUCCESS] vkAllocateDescriptorSets()");
	}

	//describe whether we want buffer as uniform or image as uniform
	//4> Declare and memset VkDescriptorBufferInfo structure.
	VkDescriptorBufferInfo vkDescriptorBufferInfo;
	memset((void *)&vkDescriptorBufferInfo, 0, sizeof(VkDescriptorBufferInfo));

	vkDescriptorBufferInfo.buffer = uniformData_cube.vkBuffer; //buffer handle
	vkDescriptorBufferInfo.offset = 0; //offset in the buffer

	//5> Assign size of the uniform buffer to range member of VkDescriptorBufferInfo structure.
	vkDescriptorBufferInfo.range = sizeof(struct MyUniformData); //size of the uniform buffer

	//for texture uniform buffer
	VkDescriptorImageInfo vkDescriptorImageInfo;
	memset((void *)&vkDescriptorImageInfo, 0, sizeof(VkDescriptorImageInfo));

	vkDescriptorImageInfo.imageLayout 	= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; 	//image layout
	vkDescriptorImageInfo.imageView 	= vkImageView_texture; 							//image view 
	vkDescriptorImageInfo.sampler 		= vkSampler_texture; 							//sampler 
	//copy the data from shader to shader or write the data to the uniform buffer
	// we prefer writing,  this is done by using vkUpdateDescriptorSets() API.


	//6> Declare and memset VkWriteDescriptorSet structure.
	VkWriteDescriptorSet vkWriteDescriptorSet_array[2]; 		//for above two structure
	memset((void *)&vkWriteDescriptorSet_array, 0, sizeof(VkWriteDescriptorSet) * _ARRAYSIZE(vkWriteDescriptorSet_array));

	vkWriteDescriptorSet_array[0].sType 				= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	vkWriteDescriptorSet_array[0].pNext 				= NULL;
	vkWriteDescriptorSet_array[0].dstSet 				= vkDescriptorSet_cube; 					//descriptor set handle
	vkWriteDescriptorSet_array[0].dstBinding 			= 0; 								//binding index
	vkWriteDescriptorSet_array[0].dstArrayElement 		= 0; 								//array element index
	vkWriteDescriptorSet_array[0].descriptorCount 		= 1; 								//descriptor count
	vkWriteDescriptorSet_array[0].descriptorType 		= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; //descriptor type
	vkWriteDescriptorSet_array[0].pBufferInfo 			= &vkDescriptorBufferInfo; 			//buffer info address
	vkWriteDescriptorSet_array[0].pImageInfo 			= NULL; 							//image info, not used in this example
	vkWriteDescriptorSet_array[0].pTexelBufferView 		= NULL; 							//texel buffer view, not used in this example

	//texture
	vkWriteDescriptorSet_array[1].sType 				= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	vkWriteDescriptorSet_array[1].pNext 				= NULL;
	vkWriteDescriptorSet_array[1].dstSet 				= vkDescriptorSet_cube; 					//descriptor set handle
	vkWriteDescriptorSet_array[1].dstBinding 			= 1; 								//binding index
	vkWriteDescriptorSet_array[1].dstArrayElement 		= 0; 								//array element index
	vkWriteDescriptorSet_array[1].descriptorCount 		= 1; 								//descriptor count
	vkWriteDescriptorSet_array[1].descriptorType 		= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER; //descriptor type
	vkWriteDescriptorSet_array[1].pBufferInfo 			= NULL; 			//buffer info address
	vkWriteDescriptorSet_array[1].pImageInfo 			= &vkDescriptorImageInfo; 			//image info address
	vkWriteDescriptorSet_array[1].pTexelBufferView 		= NULL; 							//texel buffer view, not used in this example

	//7> Now call vkUpdateDescriptorSets() to update the descriptor set with above VkWriteDescriptorSet structure.
	vkUpdateDescriptorSets(vkDevice, _ARRAYSIZE(vkWriteDescriptorSet_array), vkWriteDescriptorSet_array, 0, NULL);

	fprintf(gpFile, "\nDEBUG:------------------------------------Done [createDescriptorSet()]------------------------------------\n");
	return vkResult;
}
/*--------------------------------------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------------------------------------
16>Renderpass
--------------------------------------------------------------------------------------------------------
1> Declare and initialize VkAttachmentDescription structure array. although we have only one attachment
   ie color attachment for this example we declare it as array.

2> Declare and initialize VkAttachmentReference structure. which will have information about the attachment 
   described above.
3> Declare and initailise VkSubpassDescription structure and keep information about aboout VkAttachmentReference
   structure.
4> Declare and initailise VkRenderPassCreateInfo structure and refer VkAttachmentDescription and VkSubpassDescription
   into it.
   Remember here also we need attachment info in the form of image views which will be used by frame buffer later.
   We also need to consider/specify interdependency between subpasses if needed.
5> Now call vkCreateVulkanRenderPass() to create the actual renderpass.
   Remember 
6> In uninitialize destroy renderpass by using vkDestroyRenderPass() API.
--------------------------------------------------------------------------------------------------------*/
VkResult createRenderPass(void)
{
	//variable declaration
	VkResult vkResult = VK_SUCCESS;

	//code
	fprintf(gpFile, "\n\nDEBUG:------------------------------------Inside[createRenderPass()]------------------------------------");
	
	//1> Declare and initialize VkAttachmentDescription structure array. although we have only one attachment
	//   ie color attachment for this example we declare it as array.
	VkAttachmentDescription vkAttachmentDescription_array[2];
	memset((void *)vkAttachmentDescription_array, 0, sizeof(VkAttachmentDescription) * _ARRAYSIZE(vkAttachmentDescription_array));

	//for color attachment
	vkAttachmentDescription_array[0].flags = 0; //VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT;
	vkAttachmentDescription_array[0].format = vkFormat_color;
	vkAttachmentDescription_array[0].samples = VK_SAMPLE_COUNT_1_BIT;
	vkAttachmentDescription_array[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	vkAttachmentDescription_array[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	vkAttachmentDescription_array[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	vkAttachmentDescription_array[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	vkAttachmentDescription_array[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	vkAttachmentDescription_array[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	//for depth attachment
	vkAttachmentDescription_array[1].flags = 0; //VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT;
	vkAttachmentDescription_array[1].format = vkFormat_depth;
	vkAttachmentDescription_array[1].samples = VK_SAMPLE_COUNT_1_BIT;
	vkAttachmentDescription_array[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	vkAttachmentDescription_array[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	vkAttachmentDescription_array[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	vkAttachmentDescription_array[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	vkAttachmentDescription_array[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	vkAttachmentDescription_array[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	//2> Declare and initialize VkAttachmentReference structure. which will have information about the attachment
	//   described above.

	//for color attachment
	VkAttachmentReference vkAttachmentReference_color;
	memset((void *)&vkAttachmentReference_color, 0, sizeof(VkAttachmentReference));

	vkAttachmentReference_color.attachment = 0;
	vkAttachmentReference_color.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	//for depth attachment
	VkAttachmentReference vkAttachmentReference_depth;
	memset((void *)&vkAttachmentReference_depth, 0, sizeof(VkAttachmentReference));

	vkAttachmentReference_depth.attachment = 1;
	vkAttachmentReference_depth.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	//3> Declare and initailise VkSubpassDescription structure and keep information about aboout VkAttachmentReference
	//   structure.
	VkSubpassDescription vkSubpassDescription;
	memset((void *)&vkSubpassDescription, 0, sizeof(VkSubpassDescription));

	vkSubpassDescription.flags = 0;
	vkSubpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	vkSubpassDescription.inputAttachmentCount = 0;
	vkSubpassDescription.pInputAttachments = NULL;
	vkSubpassDescription.colorAttachmentCount = 1; //_ARRAYSIZE(vkAttachmentDescription_array);
	vkSubpassDescription.pColorAttachments = &vkAttachmentReference_color;
	vkSubpassDescription.pResolveAttachments = NULL;
	vkSubpassDescription.pDepthStencilAttachment = &vkAttachmentReference_depth;
	vkSubpassDescription.preserveAttachmentCount = 0;
	vkSubpassDescription.pPreserveAttachments = NULL;

	//4> Declare and initailise VkRenderPassCreateInfo structure and refer VkAttachmentDescription and VkSubpassDescription
	//   into it.
	//   Remember here also we need attachment info in the form of image views which will be used by frame buffer later.
	//   We also need to consider/specify interdependency between subpasses if needed.
	VkRenderPassCreateInfo vkRenderPassCreateInfo;
	memset((void *)&vkRenderPassCreateInfo, 0, sizeof(VkRenderPassCreateInfo));

	
	vkRenderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	vkRenderPassCreateInfo.flags = 0;
	vkRenderPassCreateInfo.pNext = NULL;
	vkRenderPassCreateInfo.attachmentCount = _ARRAYSIZE(vkAttachmentDescription_array);
	vkRenderPassCreateInfo.pAttachments = vkAttachmentDescription_array;
	vkRenderPassCreateInfo.subpassCount = 1;
	vkRenderPassCreateInfo.pSubpasses = &vkSubpassDescription;
	vkRenderPassCreateInfo.dependencyCount = 0;
	vkRenderPassCreateInfo.pDependencies = NULL;

	//5> Now call vkCreateVulkanRenderPass() to create the actual renderpass.
	vkResult = vkCreateRenderPass(vkDevice, &vkRenderPassCreateInfo, NULL, &vkRenderPass);
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[createCommandPool][ERROR] Failed vkCreateRenderPass()");
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nDEBUG:[createCommandPool][SUCCESS] vkCreateRenderPass()");
	}

	fprintf(gpFile, "\nDEBUG:------------------------------------Done [createRenderPass()]------------------------------------\n");
	return vkResult;
}
/*--------------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------*/
VkResult createPipeline(void)
{
	//variable declaration
	VkResult vkResult = VK_SUCCESS;

	//code
	fprintf(gpFile, "\n\nDEBUG:------------------------------------Inside[createPipeline()]------------------------------------");

	//1>Vertex Input State
	VkVertexInputBindingDescription vkVertexInputBindingDescription_array[2];
	memset((void *)&vkVertexInputBindingDescription_array, 0, sizeof(VkVertexInputBindingDescription) * _ARRAYSIZE(vkVertexInputBindingDescription_array));

	//binding 0 for position
	vkVertexInputBindingDescription_array[0].binding = 0; 								//binding index corresponding to location 0 in vertex shader
	vkVertexInputBindingDescription_array[0].stride = sizeof(float) * 3; 				//size of each vertex in bytes
	vkVertexInputBindingDescription_array[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX; 	//vertex input rate

	//binding 1 for texture coordinates
	vkVertexInputBindingDescription_array[1].binding = 1; 								//binding index corresponding to location 1 in vertex shader
	vkVertexInputBindingDescription_array[1].stride = sizeof(float) * 2; 				//size of each vertex in bytes
	vkVertexInputBindingDescription_array[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX; 	//vertex input rate

	VkVertexInputAttributeDescription vkVertexInputAttributeDescription_array[2];
	memset((void *)&vkVertexInputAttributeDescription_array, 0, sizeof(VkVertexInputAttributeDescription) * _ARRAYSIZE(vkVertexInputAttributeDescription_array));

	//attribute 0 for position
	vkVertexInputAttributeDescription_array[0].binding = 0; 							//binding index
	vkVertexInputAttributeDescription_array[0].location = 0; 							//location index
	vkVertexInputAttributeDescription_array[0].format = VK_FORMAT_R32G32B32_SFLOAT; 	//format of the vertex data
	vkVertexInputAttributeDescription_array[0].offset = 0; 								//offset of the vertex data

	//attribute 1 for texture coordinates
	vkVertexInputAttributeDescription_array[1].binding = 1; 							//binding index
	vkVertexInputAttributeDescription_array[1].location = 1; 							//location index
	vkVertexInputAttributeDescription_array[1].format = VK_FORMAT_R32G32_SFLOAT; 		//format of the texture coordinate data
	vkVertexInputAttributeDescription_array[1].offset = 0; 								//offset of the texture coordinate data

	VkPipelineVertexInputStateCreateInfo vkPipelineVertexInputStateCreateInfo;
	memset((void *)&vkPipelineVertexInputStateCreateInfo, 0, sizeof(VkPipelineVertexInputStateCreateInfo));

	vkPipelineVertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vkPipelineVertexInputStateCreateInfo.pNext = NULL;
	vkPipelineVertexInputStateCreateInfo.flags = 0; 									//VK_PIPELINE_VERTEX_INPUT_STATE_CREATE_RESERVE_1_BIT_EXT; //valid only for reserved extension
	vkPipelineVertexInputStateCreateInfo.vertexBindingDescriptionCount = _ARRAYSIZE(vkVertexInputBindingDescription_array); //binding count
	vkPipelineVertexInputStateCreateInfo.pVertexBindingDescriptions = vkVertexInputBindingDescription_array; //binding array address
	vkPipelineVertexInputStateCreateInfo.vertexAttributeDescriptionCount = _ARRAYSIZE(vkVertexInputAttributeDescription_array); //attribute count
	vkPipelineVertexInputStateCreateInfo.pVertexAttributeDescriptions = vkVertexInputAttributeDescription_array; //attribute array address

	
	//2> Input Assembly State
	VkPipelineInputAssemblyStateCreateInfo vkPipelineInputAssemblyStateCreateInfo;
	memset((void *)&vkPipelineInputAssemblyStateCreateInfo, 0, sizeof(VkPipelineInputAssemblyStateCreateInfo));
	
	vkPipelineInputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	vkPipelineInputAssemblyStateCreateInfo.pNext = NULL;
	vkPipelineInputAssemblyStateCreateInfo.flags = 0; 									//VK_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_RESERVE_1_BIT_EXT; //valid only for reserved extension
	vkPipelineInputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; //topology type
	//vkPipelineInputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE; 			//primitive restart enable or not


	//3> Rasterization State
	VkPipelineRasterizationStateCreateInfo vkPipelineRasterizationStateCreateInfo;
	memset((void *)&vkPipelineRasterizationStateCreateInfo, 0, sizeof(VkPipelineRasterizationStateCreateInfo));

	vkPipelineRasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	vkPipelineRasterizationStateCreateInfo.pNext = NULL;
	vkPipelineRasterizationStateCreateInfo.flags = 0; 								//VK_PIPELINE_RASTERIZATION_STATE_CREATE_RESERVE_1_BIT_EXT; //valid only for reserved extension
	vkPipelineRasterizationStateCreateInfo.depthClampEnable = VK_FALSE; 			//depth clamp enable or not
	vkPipelineRasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE; 		//rasterizer discard enable or not
	vkPipelineRasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL; 		//polygon mode
	vkPipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_NONE; 		//cull mode
	vkPipelineRasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; //front face
	vkPipelineRasterizationStateCreateInfo.depthBiasEnable = VK_FALSE; 			//depth bias enable or not
	vkPipelineRasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f; 		//depth bias constant factor
	vkPipelineRasterizationStateCreateInfo.depthBiasClamp = 0.0f; 				//depth bias clamp
	vkPipelineRasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f; 			//depth bias slope factor
	vkPipelineRasterizationStateCreateInfo.lineWidth = 1.0f; 					//line width


	//4> Color Blend State
	VkPipelineColorBlendAttachmentState vkPipelineColorBlendAttachmentState_array[1];
	memset((void *)&vkPipelineColorBlendAttachmentState_array, 0, sizeof(VkPipelineColorBlendAttachmentState) * _ARRAYSIZE(vkPipelineColorBlendAttachmentState_array));

	vkPipelineColorBlendAttachmentState_array[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT; //color write mask
	vkPipelineColorBlendAttachmentState_array[0].blendEnable = VK_FALSE; 								//blend enable or not


	VkPipelineColorBlendStateCreateInfo vkPipelineColorBlendStateCreateInfo;
	memset((void *)&vkPipelineColorBlendStateCreateInfo, 0, sizeof(VkPipelineColorBlendStateCreateInfo));

	vkPipelineColorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	vkPipelineColorBlendStateCreateInfo.pNext = NULL;
	vkPipelineColorBlendStateCreateInfo.flags = 0; 											//VK_PIPELINE_COLOR_BLEND_STATE_CREATE_RESERVE_1_BIT_EXT; //valid only for reserved extension
	vkPipelineColorBlendStateCreateInfo.attachmentCount = _ARRAYSIZE(vkPipelineColorBlendAttachmentState_array); //attachment count
	vkPipelineColorBlendStateCreateInfo.pAttachments = vkPipelineColorBlendAttachmentState_array; //attachment array address
	//vkPipelineColorBlendStateCreateInfo.logicOpEnable = VK_FALSE; 								//logic op enable or not

	

	//5> Viewport Scissor State
	VkPipelineViewportStateCreateInfo vkPipelineViewportStateCreateInfo;
	memset((void *)&vkPipelineViewportStateCreateInfo, 0, sizeof(VkPipelineViewportStateCreateInfo));

	vkPipelineViewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	vkPipelineViewportStateCreateInfo.pNext = NULL;
	vkPipelineViewportStateCreateInfo.flags = 0; 										//VK_PIPELINE_VIEWPORT_STATE_CREATE_RESERVE_1_BIT_EXT; //valid only for reserved extension
	vkPipelineViewportStateCreateInfo.viewportCount = 1; 								//viewport count
	
	memset((void *)&vkViewport, 0, sizeof(VkViewport)); 								    //viewport array
	vkViewport.x = 0.0f; 																	//x coordinate of viewport
	vkViewport.y = 0.0f; 																	//y coordinate of viewport
	vkViewport.width = (float)vkExtend2D_Swapchain.width; 								//width of viewport
	vkViewport.height = (float)vkExtend2D_Swapchain.height; 								//height of viewport
	vkViewport.minDepth = 0.0f; 															//min depth of viewport
	vkViewport.maxDepth = 1.0f; 															//max depth of viewport

	vkPipelineViewportStateCreateInfo.pViewports = &vkViewport; 							//viewport array address
	vkPipelineViewportStateCreateInfo.scissorCount = 1; 								//scissor count
	
	memset((void *)&vkRect2D_scissor, 0, sizeof(VkRect2D)); 										//scissor array
	vkRect2D_scissor.offset.x = 0; 																//x coordinate of scissor
	vkRect2D_scissor.offset.y = 0; 																//y coordinate of scissor
	vkRect2D_scissor.extent.width = vkExtend2D_Swapchain.width; 								//width of scissor
	vkRect2D_scissor.extent.height = vkExtend2D_Swapchain.height; 							//height of scissor

	vkPipelineViewportStateCreateInfo.pScissors = &vkRect2D_scissor; 						//scissor array address

	//6> Depth Stencil State
	//as we dont have depth yet we can ommit/skip this state.
	VkPipelineDepthStencilStateCreateInfo vkPipelineDepthStencilStateCreateInfo;
	memset((void *)&vkPipelineDepthStencilStateCreateInfo, 0, sizeof(VkPipelineDepthStencilStateCreateInfo));

	vkPipelineDepthStencilStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	vkPipelineDepthStencilStateCreateInfo.pNext = NULL;
	vkPipelineDepthStencilStateCreateInfo.flags = 0; 										//VK_PIPELINE_DEPTH_STENCIL_STATE_CREATE_RESERVE_1_BIT_EXT; //valid only for reserved extension
	vkPipelineDepthStencilStateCreateInfo.depthTestEnable = VK_TRUE; 						//depth test enable or not
	vkPipelineDepthStencilStateCreateInfo.depthWriteEnable = VK_TRUE; 						//depth write enable or not
	vkPipelineDepthStencilStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS; 				//depth compare operation
	vkPipelineDepthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE; 				//depth bounds test enable or not
	vkPipelineDepthStencilStateCreateInfo.stencilTestEnable = VK_FALSE; 					//stencil test enable or not
	vkPipelineDepthStencilStateCreateInfo.front.failOp = VK_STENCIL_OP_KEEP; 				//front stencil fail operation
	vkPipelineDepthStencilStateCreateInfo.front.passOp = VK_STENCIL_OP_KEEP; 				//front stencil pass operation
	vkPipelineDepthStencilStateCreateInfo.front.depthFailOp = VK_STENCIL_OP_KEEP; 			//front stencil depth fail operation
	vkPipelineDepthStencilStateCreateInfo.front.compareOp = VK_COMPARE_OP_ALWAYS; 			//front stencil compare operation
	vkPipelineDepthStencilStateCreateInfo.front.compareMask = 0; 							//front stencil compare mask
	vkPipelineDepthStencilStateCreateInfo.front.writeMask = 0; 								//front stencil write mask
	vkPipelineDepthStencilStateCreateInfo.front.reference = 0; 								//front stencil reference
	vkPipelineDepthStencilStateCreateInfo.back.failOp = VK_STENCIL_OP_KEEP; 				//back stencil fail operation
	vkPipelineDepthStencilStateCreateInfo.back.passOp = VK_STENCIL_OP_KEEP; 				//back stencil pass operation
	vkPipelineDepthStencilStateCreateInfo.back.depthFailOp = VK_STENCIL_OP_KEEP; 			//back stencil depth fail operation
	vkPipelineDepthStencilStateCreateInfo.back.compareOp = VK_COMPARE_OP_ALWAYS; 			//back stencil compare operation
	vkPipelineDepthStencilStateCreateInfo.back.compareMask = 0; 							//back stencil compare mask
	vkPipelineDepthStencilStateCreateInfo.back.writeMask = 0; 								//back stencil write mask
	vkPipelineDepthStencilStateCreateInfo.back.reference = 0; 								//back stencil reference
	vkPipelineDepthStencilStateCreateInfo.minDepthBounds = 0.0f; 							//min depth bounds
	vkPipelineDepthStencilStateCreateInfo.maxDepthBounds = 1.0f; 							//max depth bounds

	//7> Dynamic State
	//we can skip this state as we are not using any dynamic states.
	
	//8> Multisample State
	VkPipelineMultisampleStateCreateInfo vkPipelineMultisampleStateCreateInfo;
	memset((void *)&vkPipelineMultisampleStateCreateInfo, 0, sizeof(VkPipelineMultisampleStateCreateInfo));

	vkPipelineMultisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	vkPipelineMultisampleStateCreateInfo.pNext = NULL;
	vkPipelineMultisampleStateCreateInfo.flags = 0; 									//VK_PIPELINE_MULTISAMPLE_STATE_CREATE_RESERVE_1_BIT_EXT; //valid only for reserved extension
	vkPipelineMultisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; 	//rasterization samples


	//9> shader stage state
	VkPipelineShaderStageCreateInfo vkPipelineShaderStageCreateInfo_array[2];
	memset((void *)&vkPipelineShaderStageCreateInfo_array, 0, sizeof(VkPipelineShaderStageCreateInfo) * _ARRAYSIZE(vkPipelineShaderStageCreateInfo_array));

	vkPipelineShaderStageCreateInfo_array[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vkPipelineShaderStageCreateInfo_array[0].pNext = NULL;
	vkPipelineShaderStageCreateInfo_array[0].flags = 0; 								//VK_PIPELINE_SHADER_STAGE_CREATE_RESERVE_1_BIT_EXT; //valid only for reserved extension
	vkPipelineShaderStageCreateInfo_array[0].stage = VK_SHADER_STAGE_VERTEX_BIT; 		//shader stage
	vkPipelineShaderStageCreateInfo_array[0].module = vkShaderModule_vertex_shader; 	//shader module
	vkPipelineShaderStageCreateInfo_array[0].pName = "main"; 							//entry point name
	vkPipelineShaderStageCreateInfo_array[0].pSpecializationInfo = NULL; 				//specialization info

	vkPipelineShaderStageCreateInfo_array[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vkPipelineShaderStageCreateInfo_array[1].pNext = NULL;
	vkPipelineShaderStageCreateInfo_array[1].flags = 0; 								//VK_PIPELINE_SHADER_STAGE_CREATE_RESERVE_1_BIT_EXT; //valid only for reserved extension
	vkPipelineShaderStageCreateInfo_array[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT; 		//shader stage
	vkPipelineShaderStageCreateInfo_array[1].module = vkShaderModule_fragment_shader; 	//shader module
	vkPipelineShaderStageCreateInfo_array[1].pName = "main"; 							//entry point name
	vkPipelineShaderStageCreateInfo_array[1].pSpecializationInfo = NULL; 				//specialization info

	//10> Tessellation State
	//we can skip this state as we are not using any tessellation.

	//As pipelines are created from pipeline cache we will create a pipeline cache object.
	//11> Pipeline Cache State
	VkPipelineCacheCreateInfo vkPipelineCacheCreateInfo;
	memset((void *)&vkPipelineCacheCreateInfo, 0, sizeof(VkPipelineCacheCreateInfo));

	vkPipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	vkPipelineCacheCreateInfo.pNext = NULL;
	vkPipelineCacheCreateInfo.flags = 0; 								//VK_PIPELINE_CACHE_CREATE_RESERVE_1_BIT_EXT; //valid only for reserved extension
	vkPipelineCacheCreateInfo.initialDataSize = 0; 					//initial data size
	vkPipelineCacheCreateInfo.pInitialData = NULL; 					//initial data address

	VkPipelineCache vkPipelineCache = VK_NULL_HANDLE;

	vkResult = vkCreatePipelineCache(vkDevice, &vkPipelineCacheCreateInfo, NULL, &vkPipelineCache);
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[createPipeline][ERROR] Failed vkCreatePipelineCache()");
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nDEBUG:[createPipeline][SUCCESS] vkCreatePipelineCache()");
	}
	
	//12> create Graphics Pipeline
	VkGraphicsPipelineCreateInfo vkGraphicsPipelineCreateInfo;
	memset((void *)&vkGraphicsPipelineCreateInfo, 0, sizeof(VkGraphicsPipelineCreateInfo));
	
	vkGraphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	vkGraphicsPipelineCreateInfo.pNext = NULL;
	vkGraphicsPipelineCreateInfo.flags = 0; 														//VK_GRAPHICS_PIPELINE_CREATE_RESERVE_1_BIT_EXT; //valid only for reserved extension
	vkGraphicsPipelineCreateInfo.pVertexInputState 		= &vkPipelineVertexInputStateCreateInfo; 	//vertex input state
	vkGraphicsPipelineCreateInfo.pInputAssemblyState 	= &vkPipelineInputAssemblyStateCreateInfo; //input assembly state
	vkGraphicsPipelineCreateInfo.pViewportState 		= &vkPipelineViewportStateCreateInfo; //viewport state
	vkGraphicsPipelineCreateInfo.pRasterizationState 	= &vkPipelineRasterizationStateCreateInfo; //rasterization state
	vkGraphicsPipelineCreateInfo.pMultisampleState 		= &vkPipelineMultisampleStateCreateInfo; //multisample state
	vkGraphicsPipelineCreateInfo.pColorBlendState 		= &vkPipelineColorBlendStateCreateInfo; //color blend state
	vkGraphicsPipelineCreateInfo.pDepthStencilState 	= &vkPipelineDepthStencilStateCreateInfo; //depth stencil state
	vkGraphicsPipelineCreateInfo.pTessellationState 	= NULL; 								//tessellation state
	vkGraphicsPipelineCreateInfo.pDynamicState 			= NULL; 								//dynamic state
	
	vkGraphicsPipelineCreateInfo.stageCount = _ARRAYSIZE(vkPipelineShaderStageCreateInfo_array); //shader stage count
	vkGraphicsPipelineCreateInfo.pStages = vkPipelineShaderStageCreateInfo_array; //shader stage array address
	
	vkGraphicsPipelineCreateInfo.layout = vkPipelineLayout; 						//pipeline layout
	vkGraphicsPipelineCreateInfo.renderPass = vkRenderPass; 						//render pass
	vkGraphicsPipelineCreateInfo.subpass = 0; 									//subpass index
	vkGraphicsPipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE; 			//base pipeline handle
	vkGraphicsPipelineCreateInfo.basePipelineIndex = -1; 						//base pipeline index

	
	vkResult = vkCreateGraphicsPipelines(vkDevice, vkPipelineCache, 1, &vkGraphicsPipelineCreateInfo, NULL, &vkPipeline);
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[createPipeline][ERROR] Failed vkCreateGraphicsPipelines()");
		vkDestroyPipelineCache(vkDevice, vkPipelineCache, NULL);
		vkPipelineCache = VK_NULL_HANDLE;
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nDEBUG:[createPipeline][SUCCESS] vkCreateGraphicsPipelines()");
	}

	//13> Destroy Pipeline Cache
	vkDestroyPipelineCache(vkDevice, vkPipelineCache, NULL);
	vkPipelineCache = VK_NULL_HANDLE;
	if(vkPipelineCache != VK_NULL_HANDLE)
	{
		fprintf(gpFile, "\nDEBUG:[createPipeline][ERROR] Failed vkDestroyPipelineCache()");
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nDEBUG:[createPipeline][SUCCESS] vkDestroyPipelineCache()");
	}
	
	fprintf(gpFile, "\nDEBUG:---------------------------------Done[createPipeline()]------------------------------------");
	return vkResult;
}


/*--------------------------------------------------------------------------------------------------------
17> Frame Buffers
--------------------------------------------------------------------------------------------------------
number of frame buffers should be equal to number of swapchains
1> Declare an array of vkImageView equal to the number of attachments ie in our example it is 1.

2> Declare and initailise VkFrameBuffer structure.
3> Allocate the FrameBuffer array by malloc to size of swapchain image count.
4> Start a loop for swapchain image count and call vkCreateFrameBuffer() to create frame buffers.
5> In uninitialize() destroy frame buffers in a loop till swapchain image count. 
--------------------------------------------------------------------------------------------------------*/
VkResult createFrameBuffers(void)
{
	//variable declaration
	VkResult vkResult = VK_SUCCESS;

	//code
	fprintf(gpFile, "\n\nDEBUG:------------------------------------Inside[createFrameBuffers()]------------------------------------");

	//1> Declare an array of vkImageView equal to the number of attachments ie in our example it is 1.
	//VkImageView vkImageView_attchments_array[2];

	//3> Allocate the FrameBuffer array by malloc to size of swapchain image count.
	vkFramebuffer_array = (VkFramebuffer *)malloc(swapchainImageCount * sizeof(VkFramebuffer));
	//error checking malloc

	//4> Start a loop for swapchain image count and call vkCreateFrameBuffer() to create frame buffers.
	for(uint32_t i = 0; i < swapchainImageCount; i++)
	{
		//1> Declare an array of vkImageView equal to the number of attachments ie in our example it is 1.
		VkImageView vkImageView_attchments_array[2];
		memset((void *)vkImageView_attchments_array, 0, sizeof(VkImageView) * _ARRAYSIZE(vkImageView_attchments_array));
		
		//2> Declare and initailise VkFrameBuffer structure.
		VkFramebufferCreateInfo vkFramebufferCreateInfo;
		memset((void *)&vkFramebufferCreateInfo, 0, sizeof(VkFramebufferCreateInfo));

		vkFramebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		vkFramebufferCreateInfo.pNext = NULL;
		vkFramebufferCreateInfo.flags = 0;
		vkFramebufferCreateInfo.renderPass = vkRenderPass;
		vkFramebufferCreateInfo.attachmentCount = _ARRAYSIZE(vkImageView_attchments_array);
		vkFramebufferCreateInfo.pAttachments = vkImageView_attchments_array;
		vkFramebufferCreateInfo.width = vkExtend2D_Swapchain.width;
		vkFramebufferCreateInfo.height = vkExtend2D_Swapchain.height;
		vkFramebufferCreateInfo.layers = 1;

		//assign swapchain image view to attachment array	
		vkImageView_attchments_array[0] = swapchainImageView_array[i];
		vkImageView_attchments_array[1] = vkImageView_depth; //depth stencil image view

		vkResult = vkCreateFramebuffer(vkDevice, &vkFramebufferCreateInfo, NULL, &vkFramebuffer_array[i]);
		if(vkResult != VK_SUCCESS)
		{	
			fprintf(gpFile, "\nDEBUG:[createFrameBuffers][ERROR] Failed vkCreateFramebuffer() i = %d", i);
			return vkResult;
		}
		else
		{
			fprintf(gpFile, "\nDEBUG:[createFrameBuffers][SUCCESS] vkCreateFramebuffer() i = %d", i);
		}
	}

	fprintf(gpFile, "\nDEBUG:------------------------------------Done [createFrameBuffers()]------------------------------------\n");
	return vkResult;
}
/*--------------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------------
18-Fences-And-Semaphores
--------------------------------------------------------------------------------------------------------
1> Globally declare array of two fences of variables type pointer to VkFence. Additionally declare 2 semaphore 
   objects of type VkSemaphores.
2> In createSemaphore() declare, memset and initialize VkSemaphoreCreateInfo structure.
3> Now Call vkCreateSemaphore() 2 times to create 2 semaphore objects.
   Remember: Both will use same VkSemaphoreCreateInfo structure.
4> In createFences() function declare, memset and initialize VkFencesCreateInfo structure.
5> In this function createFences(), allocate global fence array to the size of swapchain image count using malloc.
6> Now in a loop vkCreateFence() API to initialize global fences array.
7> In uninitialize() first in a loop with swapchainImageCount as a counter destroy fence array objects using vkDestroyFence() API.
   and then actually free the fences array by using free().
8> Destroy both global semaphore objects with two separate calls vkDestroySemaphore() API.
--------------------------------------------------------------------------------------------------------*/
VkResult createSemaphores(void)
{
	//variable declaration
	VkResult vkResult = VK_SUCCESS;

	//code
	fprintf(gpFile, "\n\nDEBUG:------------------------------------Inside[createSemaphores()]------------------------------------");

	//2> In createSemaphore() declare, memset and initialize VkSemaphoreCreateInfo structure.
	VkSemaphoreCreateInfo vkSemaphoreCreateInfo;
	memset((void *)&vkSemaphoreCreateInfo, 0, sizeof(VkSemaphoreCreateInfo));

	vkSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	vkSemaphoreCreateInfo.pNext = NULL;
	vkSemaphoreCreateInfo.flags = 0;	//must be zero:reserved

	//NOTE: //   Bydefault semaphores not delared then it is type of binary semaphore.
	//   semaphores are signaled and not signaled when created. So we need to use fences to signal and wait for them.
	//   semaphores are used to synchronize the operations between command buffers and queues.(host and device operations)'
	
	//3> Now Call vkCreateSemaphore() 2 times to create 2 semaphore objects.
	//   Remember: Both will use same VkSemaphoreCreateInfo structure.
	vkResult = vkCreateSemaphore(vkDevice, &vkSemaphoreCreateInfo, NULL, &vkSemaphore_backBuffer);
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[createSemaphores][ERROR] Failed vkCreateSemaphore() first call");
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nDEBUG:[createSemaphores][SUCCESS] vkCreateSemaphore() first call");
	}

	vkResult = vkCreateSemaphore(vkDevice, &vkSemaphoreCreateInfo, NULL, &vkSemaphore_renderComplete);
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[createSemaphores][ERROR] Failed vkCreateSemaphore() second call");
		return vkResult;
	}
	else
	{
		fprintf(gpFile, "\nDEBUG:[createSemaphores][SUCCESS] vkCreateSemaphore() second call");
	}

	fprintf(gpFile, "\nDEBUG:------------------------------------Done [createSemaphores()]------------------------------------\n");
	return vkResult;
}
/*--------------------------------------------------------------------------------------------------------*/
VkResult createFences(void)
{
	//variable declaration
	VkResult vkResult = VK_SUCCESS;

	//code
	fprintf(gpFile, "\n\nDEBUG:------------------------------------Inside[createFences()]------------------------------------");

	//4> In createFences() function declare, memset and initialize VkFencesCreateInfo structure.
	VkFenceCreateInfo vkFenceCreateInfo;
	memset((void *)&vkFenceCreateInfo, 0, sizeof(VkFenceCreateInfo));

	vkFenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	vkFenceCreateInfo.pNext = NULL;
	vkFenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; //VK_FENCE_CREATE_SIGNALED_BIT; //VK_FENCE_CREATE_TIMELINE_BIT_EXT

	//NOTE: //   Bydefault fences not delared then it is type of binary fence.
	//   fences are signaled and not signaled when created. So we need to use semaphores to signal and wait for them.
	//   fences are used to synchronize the operations between command buffers and queues.(host and device operations)'
	
	//5> In this function createFences(), allocate global fence array to the size of swapchain image count using malloc.
	vkFence_array = (VkFence *)malloc(swapchainImageCount * sizeof(VkFence));
	//error checking malloc

	//6> Now in a loop vkCreateFence() API to initialize global fences array.
	for(uint32_t i = 0; i < swapchainImageCount; i++)
	{
		vkResult = vkCreateFence(vkDevice, &vkFenceCreateInfo, NULL, &vkFence_array[i]);
		if(vkResult != VK_SUCCESS)
		{	
			fprintf(gpFile, "\nDEBUG:[createFences][ERROR] Failed vkCreateFence() i = %d", i);
			return vkResult;
		}
		else
		{
			fprintf(gpFile, "\nDEBUG:[createFences][SUCCESS] vkCreateFence() i = %d", i);
		}
	}


	fprintf(gpFile, "\nDEBUG:------------------------------------Done [createFences()]------------------------------------\n");
	return vkResult;
}
/*--------------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------------
19-Build-Command-Buffers
--------------------------------------------------------------------------------------------------------
1> Start a loop with swapchainImageCount as a counter.
2> Inside the loop vkResetCommandBuffer() API to reset the command buffers.
3> Then declare memset initialise VkCommandBufferBiginInfo.
4> Now call vkBiginCommandBuffer() to record vulkan drawing related commands. do error checking.
5> Declare memset and initailise struct array of VkClearValue type.
   Remember: Internally it is union.
   Our array willbe of 1 element. this number is depends on the number of attachments to the frame buffers.
   As we have only one attachment i.e. color attachment hence the 1 element.
   It has two members ie.=> 
   // Provided by VK_VERSION_1_0
	typedef union VkClearValue {
    	VkClearColorValue           color;
    	VkClearDepthStencilValue    depthStencil;
	} VkClearValue;
	As we only have color attachment color value is meaningful and depthStencil member is meaningless.
	When there will be depth attachment it will be reversed ie depthStencil value will be meaningful.

	To this color member we need to assign VkClearColorValue structure value. To do this declare globally
	VkClearColorValue structure varaible. memset and initailise it in initailise();
	Remember: We are going to clear color member of VkClearValue structure by VkClearColorValue structure
	because in steps for 16>Renderpass we specified the .loadOp member of VkAttachmentDescription structure
	to VK_ATTACHMENT_LOAD_OP_CLEAR.
6> Then declare memset and initialize VkRenderPassBeginInfo.
7> Then begin renderpass by vkCmdBeginRenderPass() API.
   Remember: The code written inside "beginrenderpass" and "endrenderpass" itself is code the of subpass if no subpass 
   is explicitely created.
   In other words even if no subpass is explicitely created there is always one subpass inside/for the renderpass.
8> End renderpass by calling vkCmdEndRenderPass() API.
9> End the recording of the command buffer by calling vkEndCommandBuffer() API. Do error checking.
10> Close the loop.
--------------------------------------------------------------------------------------------------------*/
VkResult buildCommandBuffers(void)
{
	//variable declaration
	VkResult vkResult = VK_SUCCESS;

	//code
	fprintf(gpFile, "\n\nDEBUG:------------------------------------Inside[buildCommandBuffers()]------------------------------------");

	//1> Start a loop with swapchainImageCount as a counter.
	for(uint32_t i = 0; i < swapchainImageCount; i++)
	{
		//2> Inside the loop vkResetCommandBuffer() API to reset the command buffers.
		vkResult = vkResetCommandBuffer(vkCommandBuffer_array[i], 0);
		if(vkResult != VK_SUCCESS)
		{	
			fprintf(gpFile, "\nDEBUG:[buildCommandBuffers][ERROR] Failed vkResetCommandBuffer() i = %d", i);
			return vkResult;
		}
		else
		{
			fprintf(gpFile, "\nDEBUG:[buildCommandBuffers][SUCCESS] vkResetCommandBuffer() i = %d", i);
		}

		//3> Then declare memset initialise VkCommandBufferBiginInfo.
		VkCommandBufferBeginInfo vkCommandBufferBeginInfo;
		memset((void *)&vkCommandBufferBeginInfo, 0, sizeof(VkCommandBufferBeginInfo));

		vkCommandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		vkCommandBufferBeginInfo.pNext = NULL;
		vkCommandBufferBeginInfo.flags = 0; //VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		//4> Now call vkBiginCommandBuffer() to record vulkan drawing related commands. do error checking.
		vkResult = vkBeginCommandBuffer(vkCommandBuffer_array[i], &vkCommandBufferBeginInfo);
		if(vkResult != VK_SUCCESS)
		{	
			fprintf(gpFile, "\nDEBUG:[buildCommandBuffers][ERROR] Failed vkBeginCommandBuffer() i = %d", i);
			return vkResult;
		}
		else
		{
			fprintf(gpFile, "\nDEBUG:[buildCommandBuffers][SUCCESS] vkBeginCommandBuffer() i = %d", i);
		}

		//5> Declare memset and initailise struct array of VkClearValue type.
		VkClearValue vkClearValue_array[2];
		memset((void *)vkClearValue_array, 0, sizeof(VkClearValue) * _ARRAYSIZE(vkClearValue_array));

		vkClearValue_array[0].color = vkClearColorValue; //color member of union is used
		vkClearValue_array[1].depthStencil = vkClearDepthStencilValue; //depthStencil member of union is used


		//6> Then declare memset and initialize VkRenderPassBeginInfo.
		VkRenderPassBeginInfo vkRenderPassBeginInfo;
		memset((void *)&vkRenderPassBeginInfo, 0, sizeof(VkRenderPassBeginInfo));

		vkRenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		vkRenderPassBeginInfo.pNext = NULL;
		vkRenderPassBeginInfo.renderPass = vkRenderPass;
		vkRenderPassBeginInfo.renderArea.offset.x = 0;
		vkRenderPassBeginInfo.renderArea.offset.y = 0;
		vkRenderPassBeginInfo.renderArea.extent.width = vkExtend2D_Swapchain.width;
		vkRenderPassBeginInfo.renderArea.extent.height = vkExtend2D_Swapchain.height;
		vkRenderPassBeginInfo.clearValueCount = _ARRAYSIZE(vkClearValue_array);
		vkRenderPassBeginInfo.pClearValues = vkClearValue_array;
		vkRenderPassBeginInfo.framebuffer = vkFramebuffer_array[i];

		//7> Then begin renderpass by vkCmdBeginRenderPass() API.
		vkCmdBeginRenderPass(vkCommandBuffer_array[i], &vkRenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		

		//bind with pipeline
		vkCmdBindPipeline(vkCommandBuffer_array[i], VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline);

		//bind with vertex buffer
		VkDeviceSize vkDeviceSize_position;
		memset((void *)&vkDeviceSize_position, 0, sizeof(VkDeviceSize));

		//bind our desciptor set to the pipeline
		vkCmdBindDescriptorSets(vkCommandBuffer_array[i], VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipelineLayout, 0, 1, &vkDescriptorSet_cube, 0, NULL);

		vkCmdBindVertexBuffers(vkCommandBuffer_array[i], 0, 1, &vertexData_position.vkBuffer, &vkDeviceSize_position);

		//bind with texture buffer
		VkDeviceSize vkDeviceSize_texCoords;
		memset((void *)&vkDeviceSize_texCoords, 0, sizeof(VkDeviceSize));

		//bind our desciptor set to the pipeline
		vkCmdBindDescriptorSets(vkCommandBuffer_array[i], VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipelineLayout, 0, 1, &vkDescriptorSet_cube, 0, NULL);

		vkCmdBindVertexBuffers(vkCommandBuffer_array[i], 1, 1, &vertexData_texCoords.vkBuffer, &vkDeviceSize_texCoords);

		//here we should code the actual drawing commands to be recorded in the command buffer.
		//but we are not doing it here. we will do it in the next example.
		//we are just clearing the color buffer in this example.

		//draw call
		vkCmdDraw(vkCommandBuffer_array[i], 36, 1, 0, 0); //draw call


		//8> End renderpass by calling vkCmdEndRenderPass() API.
		vkCmdEndRenderPass(vkCommandBuffer_array[i]);

		//9> End the recording of the command buffer by calling vkEndCommandBuffer() API.
		vkResult = vkEndCommandBuffer(vkCommandBuffer_array[i]);
		if(vkResult != VK_SUCCESS)
		{	
			fprintf(gpFile, "\nDEBUG:[buildCommandBuffers][ERROR] Failed vkEndCommandBuffer() i = %d", i);
			return vkResult;
		}
		else
		{
			fprintf(gpFile, "\nDEBUG:[buildCommandBuffers][SUCCESS] vkEndCommandBuffer() i = %d", i);
		}
	}

	fprintf(gpFile, "\nDEBUG:------------------------------------Done [buildCommandBuffers()]------------------------------------\n");
	fflush(gpFile);
	return vkResult;
}
/*--------------------------------------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------------------------------------*/
VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallback(
		
	VkDebugReportFlagsEXT vkDebugReportFlagsEXT,				////flags for error, warning, etc
	VkDebugReportObjectTypeEXT vkDebugReportObjectTypeEXT,		//object type of the object that generated the message
	uint64_t object,											//object that generated the message
	size_t location,											//location of the message in the object
	int32_t messageCode,										//message code of the message
	const char *pLayerPrefix,									//prefix of the layer that generated the message
	const char *pMessage,										//message string
	void *pUserData												//user data passed to the callback function
)
{
	//code
	fprintf(gpFile, "\n\nDEBUG:[debugReportCallback]\n%s", pMessage);
	fprintf(gpFile, "\nDEBUG:[debugReportCallback] %s", pLayerPrefix);
	fprintf(gpFile, "\nDEBUG:[debugReportCallback] %d", messageCode);
	fprintf(gpFile, "\nDEBUG:[debugReportCallback] %d", vkDebugReportFlagsEXT);
	fprintf(gpFile, "\nDEBUG:[debugReportCallback] %d", vkDebugReportObjectTypeEXT);
	fprintf(gpFile, "\nDEBUG:[debugReportCallback] %lld", object);
	fprintf(gpFile, "\nDEBUG:[debugReportCallback] %zd", location);

	return VK_FALSE;
}
/*--------------------------------------------------------------------------------------------------------*/




