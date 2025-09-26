#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
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

UniformData uniformData;

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
VkDescriptorSet vkDescriptorSet = VK_NULL_HANDLE;

//Pipeline
VkViewport vkViewport;
VkRect2D vkRect2D_scissor;
VkPipeline vkPipeline = VK_NULL_HANDLE;

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

//main
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	//function declations
	VkResult initialize(void);
	VkResult display(void);
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
		fprintf(gpFile, "\nDEBUG:[WinMain()][SUCCESS] Log File Created Successfully");
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
	fprintf(gpFile, "\nDEBUG:[WinMain()][SUCCESS] Class Registered Successfully");
	//SystemParametersInfo
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rect, 0);

	HorPos = (rect.right - WIN_WIDTH) / 2;
	VerPos = (rect.bottom - WIN_HIGHT) / 2;

	//creating window
	hwnd = CreateWindowEx(	WS_EX_APPWINDOW,
							szAppName,
							TEXT("Vulkan| 03-Triangle-31-Ortho"),
							WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
							HorPos,
							VerPos,
							WIN_WIDTH,
							WIN_HIGHT,
							NULL,
							NULL,
							hInstance,
							NULL);

	fprintf(gpFile, "\nDEBUG:[WinMain()][SUCCESS] CreateWindowEx() Done -> Window Created");

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
	void toggleFullscreen(void);
	void uninitialize(void);
	VkResult resize(int, int);

	//code
	switch(iMsg)
	{
		case WM_CREATE:
			fprintf(gpFile, "\nDEBUG:[WndProc()] WM_CREATE Recieved");
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
					toggleFullscreen();
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
			fprintf(gpFile, "\nDEBUG:[WndProc()] WM_QUIT Recieved");
			//uninitialize();
			PostQuitMessage(0);
			break;

		case WM_DESTROY:
			fprintf(gpFile, "\nDEBUG:[WndProc()] WM_DESTROY Recieved");
			uninitialize();
			PostQuitMessage(0);
			break;
	}

	return(DefWindowProc(hwnd, iMsg, wParam, lParam));
}

void toggleFullscreen(void)
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
	
	//create fences
	vkResult = createFences();
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[initialize][ERROR] Failed createFences()");
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	else
		fprintf(gpFile, "\nDEBUG:[initialize][SUCCESS] createFences() Completed");

	//initialize clearColorValues.
	memset((void *)&vkClearColorValue, 0, sizeof(VkClearColorValue));
	
	vkClearColorValue.float32[0] = 0.0f;	//red
	vkClearColorValue.float32[1] = 0.0f;	//green	
	vkClearColorValue.float32[2] = 1.0f; 	//blue
	vkClearColorValue.float32[3] = 1.0f;	//alpha  //analogous to glClearColor()
	
	fprintf(gpFile, "\nDEBUG:[initialize][SUCCESS] vkClearColorValue initialized successfully");

	//build command buffers
	vkResult = buildCommandBuffers();
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[initialize][ERROR] Failed buildCommandBuffers()");
		return VK_ERROR_INITIALIZATION_FAILED;
	}
	else
		fprintf(gpFile, "\nDEBUG:[initialize][SUCCESS] buildCommandBuffers() Completed");
	
	//initialization complete
	bInitialized = TRUE;
	fprintf(gpFile, "\nDEBUG:[initialize][SUCCESS] bInitialized = TRUE");

	fprintf(gpFile, "\nDEBUG:------------------------------------Done [initialize()]------------------------------------\n\n");
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
	
	//destroy swapchain
	if(vkSwapchainKHR)
	{
		vkDestroySwapchainKHR(vkDevice, vkSwapchainKHR, NULL);
		vkSwapchainKHR = VK_NULL_HANDLE;
		fprintf(gpFile, "\nDEBUG:[resize()][SUCCESS] vkDestroySwapchainKHR()");
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

void uninitialize(void)
{
	//code
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
		if(uniformData.vkBuffer)
		{
			vkDestroyBuffer(vkDevice, uniformData.vkBuffer, NULL);
			uniformData.vkBuffer = VK_NULL_HANDLE;
			fprintf(gpFile, "\nDEBUG:[uninitialize()][SUCCESS] vkDestroyBuffer() = uniform.vkBuffer");
		}


		//destroy uniform memory
		if(uniformData.vkDeviceMemory)
		{
			vkFreeMemory(vkDevice, uniformData.vkDeviceMemory, NULL);
			uniformData.vkDeviceMemory = VK_NULL_HANDLE;
			fprintf(gpFile, "\nDEBUG:[uninitialize()][SUCCESS] vkFreeMemory() = uniform.vkDeviceMemory");
		}

		//destroy pipeline layout
		if(vkPipelineLayout)
		{
			vkDestroyPipelineLayout(vkDevice, vkPipelineLayout, NULL);
			vkPipelineLayout = VK_NULL_HANDLE;
			fprintf(gpFile, "\nDEBUG:[uninitialize()][SUCCESS] vkDestroyPipelineLayout()");
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

		//commad bufffers
		if(vkCommandBuffer_array)
		{
			vkFreeCommandBuffers(vkDevice, vkCommandPool, swapchainImageCount, vkCommandBuffer_array);
			fprintf(gpFile, "\nDEBUG:[uninitialize()][SUCCESS] vkFreeCommandBuffers()");

			if(vkCommandBuffer_array)
			{
				free(vkCommandBuffer_array);
				vkCommandBuffer_array = VK_NULL_HANDLE;
			}
		}
		
		//command pool
		if(vkCommandPool)
		{
			vkDestroyCommandPool(vkDevice, vkCommandPool, NULL);
			vkCommandPool = VK_NULL_HANDLE;
			fprintf(gpFile, "\nDEBUG:[uninitialize()][SUCCESS] vkDestroyCommandPool()");
		}

		//destroy image views
		for(uint32_t i = 0; i < swapchainImageCount; i++)
		{
			vkDestroyImageView(vkDevice, swapchainImageView_array[i], NULL);
			fprintf(gpFile, "\nDEBUG:[uninitialize()][SUCCESS] vkDestroyImageView() = %d", i);
		}
		
		if(swapchainImageView_array)
		{
			free(swapchainImageView_array);
			swapchainImageView_array = NULL;
			fprintf(gpFile, "\nDEBUG:[uninitialize()][SUCCESS] swapchainImageView_array freed");
		}


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
	
		//vkDevice
		vkDestroyDevice(vkDevice, NULL);
		vkDevice = VK_NULL_HANDLE;
		fprintf(gpFile, "\nDEBUG:[uninitialize()][SUCCESS] vkDestroyDevice()");
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
		fprintf(gpFile, "\nDEBUG:[uninitialize()] Debug File Closed\n");
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
in initialise()
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
	if(vkExtensionProperties_array == NULL)
	{
		fprintf(gpFile, "\nDEBUG:[fillInstanceExtensionNames][ERROR] Failed to allocate memory for VkExtensionProperties array");
		return VK_ERROR_OUT_OF_HOST_MEMORY; //return hardcoded failure
	}
	else
		fprintf(gpFile, "\nDEBUG:[fillInstanceExtensionNames][SUCCESS] Memory allocated for VkExtensionProperties array");

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
	if(instanceExtensionNames_array == NULL)
	{
		fprintf(gpFile, "\nDEBUG:[fillInstanceExtensionNames][ERROR] Failed to allocate memory for instanceExtensionNames_array");
		return VK_ERROR_OUT_OF_HOST_MEMORY; //return hardcoded failure
	}
	else
		fprintf(gpFile, "\nDEBUG:[fillInstanceExtensionNames][SUCCESS] Memory allocated for instanceExtensionNames_array");
	
	for(uint32_t i = 0; i < instanceExtensionCount; i++)
	{
		//allocate memory for each string and copy the string from vkExtensionProperties_array to instanceExtensionNames_array
		instanceExtensionNames_array[i] = (char *)malloc(sizeof(char) * strlen(vkExtensionProperties_array[i].extensionName) + 1);
		if(instanceExtensionNames_array[i] == NULL)
		{
			fprintf(gpFile, "\nDEBUG:[fillInstanceExtensionNames][ERROR] Failed to allocate memory for instanceExtensionNames_array[%d]", i);
			return VK_ERROR_OUT_OF_HOST_MEMORY; //return hardcoded failure
		}
				
		//copy extension name from vkExtensionProperties_array to instanceExtensionNames_array
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
	//variable declarations
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

	//3> Allocate VkPhysicalDeviceArray array according physical device count
	vkPhysicalDevice_array = (VkPhysicalDevice *)malloc(physicalDeviceCount * sizeof(VkPhysicalDevice));
	if(vkPhysicalDevice_array == NULL)
	{
		fprintf(gpFile, "\nDEBUG:[getPhysicalDevice][ERROR] Failed to allocate memory for vkPhysicalDevice_array");
		vkResult = VK_ERROR_OUT_OF_HOST_MEMORY; //return hardcoded failure
		return vkResult;
	}
	else
		fprintf(gpFile, "\nDEBUG:[getPhysicalDevice][SUCCESS] Memory allocated for vkPhysicalDevice_array");

	//4> Call vkEnumeratePhysicalDevices() again to fill above array (physical device names).
	vkResult = vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, vkPhysicalDevice_array);
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[getPhysicalDevice][ERROR] Failed vkEnumeratePhysicalDevices() Second Call");
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}
	else
		fprintf(gpFile, "\nDEBUG:[getPhysicalDevice][SUCCESS] vkEnumeratePhysicalDevices() Second Call");

	
	//5> Start Loop using physical device count and physical device array.
	//NOTE: Declare a boolean bFound variable before this loop which will decide whether we found desired physical device or not.
	VkBool32 bFound = VK_FALSE;
	for(uint32_t i = 0; i < physicalDeviceCount; i++)
	{
		uint32_t queueCount = UINT32_MAX;

		//returns void.
		vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice_array[i], &queueCount, NULL);
		
		VkQueueFamilyProperties *vkQueueFamilyProperties_array = NULL;
		vkQueueFamilyProperties_array = (VkQueueFamilyProperties *)malloc(queueCount * sizeof(VkQueueFamilyProperties));
		if(vkQueueFamilyProperties_array == NULL)
		{
			fprintf(gpFile, "\nDEBUG:[getPhysicalDevice][ERROR] Failed to allocate memory for vkQueueFamilyProperties_array");
			vkResult = VK_ERROR_OUT_OF_HOST_MEMORY; //return hardcoded failure
			return vkResult;
		}

		vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice_array[i], &queueCount, vkQueueFamilyProperties_array);

		VkBool32 *isQueueSurfaceSupported_array = NULL;
		isQueueSurfaceSupported_array = (VkBool32 *)malloc(queueCount * sizeof(VkBool32));
		if(isQueueSurfaceSupported_array == NULL)
		{
			fprintf(gpFile, "\nDEBUG:[getPhysicalDevice][ERROR] Failed to allocate memory for isQueueSurfaceSupported_array");
			vkResult = VK_ERROR_OUT_OF_HOST_MEMORY; //return hardcoded failure
			return vkResult;
		}

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
		fprintf(gpFile, "\nDEBUG:[getPhysicalDevice][SUCCESS] Graphics enabled vkPhysicalDevice_Selected index= %d", graphicsQueueFamilyIndex_Selected);
		fprintf(gpFile, "\nDEBUG:[getPhysicalDevice][SUCCESS] Graphics enabled graphicsQueueFamilyIndex_Selected = %d", graphicsQueueFamilyIndex_Selected);

		fprintf(gpFile, "\nDEBUG:[getPhysicalDevice][SUCCESS] Found graphics enabled physical device");
	}
	else
	{
		free(vkPhysicalDevice_array);
		vkPhysicalDevice_array = NULL;
		fprintf(gpFile, "\nDEBUG:[getPhysicalDevice][Failed] with graphics enabled");
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
		return vkResult;
	}

	//8>get physical device memory properties
	memset((void *)&vkPhysicalDeviceMemoryProperties, 0, sizeof(VkPhysicalDeviceMemoryProperties));

	vkGetPhysicalDeviceMemoryProperties(vkPhysicalDevice_Selected, &vkPhysicalDeviceMemoryProperties);

	//9> Declare a local structure variable VkPhysicalDeviceFeatures. Memset it and initialize it by calling vkGetPhysicalDeviceFeatures().
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
		fprintf(gpFile, "\nDEBUG:[getPhysicalDevicePresentModeKHR][SUCCESS] vkGetPhysicalDeviceSurfacePresentModesKHR() first call presentModeCount = %d", presentModeCount);


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

	fprintf(gpFile, "\nDEBUG:------------------------------------Done [createImagesAndImageViews()]------------------------------------\n");
	return vkResult;
}
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
	fprintf(gpFile, "\n\nDEBUG:------------------------------------Inside[createCommandBuffers()]------------------------------------");
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

	fprintf(gpFile, "\nDEBUG:------------------------------------Done [createCommandBuffers()]------------------------------------\n");
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
	float triangle_position[] = 
	{
		   0.0f,   50.0f,  0.0f,
		 -50.0f,  -50.0f,  0.0f,
		  50.0f,  -50.0f,  0.0f
	};

	memset((void *)&vertexData_position, 0, sizeof(VertexData));
	//5> memset our global vertexData_position. delcare and memset struct VkBufferCreateInfo()
	VkBufferCreateInfo vkBufferCreateInfo;
	memset((void *)&vkBufferCreateInfo, 0, sizeof(VkBufferCreateInfo));

	//   it has 8 members, we will use 5. out of 2 are very important.
	vkBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vkBufferCreateInfo.pNext = NULL;
	vkBufferCreateInfo.flags = 0; //valid flags are used in scatterd buffers/sparse buffers
	vkBufferCreateInfo.size = sizeof(triangle_position); //size of the buffer in bytes
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
	memcpy(data, triangle_position, sizeof(triangle_position));
	fprintf(gpFile, "\nDEBUG:[createVertexBuffers][SUCCESS] memcpy()");

	//16> To complete this memory mapped IO finally call vkUnmapMemory() API.
	vkUnmapMemory(vkDevice, vertexData_position.vkDeviceMemory);

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
	memset((void *)&uniformData, 0, sizeof(struct UniformData));

	//6> Call vkCreateBuffer() vulkan API in the ".vkBuffer" member of our global struct.
	vkResult = vkCreateBuffer(vkDevice, &vkBufferCreateInfo, NULL, &uniformData.vkBuffer);
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

	vkGetBufferMemoryRequirements(vkDevice, uniformData.vkBuffer, &vkMemoryRequirements);

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
	vkResult = vkAllocateMemory(vkDevice, &vkMemoryAllocateInfo, NULL, &uniformData.vkDeviceMemory);
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
	vkResult = vkBindBufferMemory(vkDevice, uniformData.vkBuffer, uniformData.vkDeviceMemory, 0);
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
	myUniformData.modelMatrix 		= glm::mat4(1.0f);		//identity matrix
	myUniformData.viewMatrix 		= glm::mat4(1.0f);		//identity matrix
	myUniformData.projectionMatrix 	= glm::mat4(1.0f);		//identity matrix

	glm::mat4 orthographicProjectionMatrix = glm::mat4(1.0f);	//identity matrix

	if(winWidth <= winHeight)
	{
		orthographicProjectionMatrix = glm::ortho( -100.0f, 
													100.0f, 
												   -100.0f * ((float)winHeight / (float)winWidth), 
												    100.0f * ((float)winHeight / (float)winWidth), 
												   -100.0f, 
												    100.0f);
	}
	else
	{
		orthographicProjectionMatrix = glm::ortho( -100.0f * ((float)winWidth / (float)winHeight), 
													100.0f * ((float)winWidth / (float)winHeight), 
												   -100.0f, 
												    100.0f, 
												   -100.0f, 
												    100.0f);
	}


	myUniformData.projectionMatrix = orthographicProjectionMatrix;

	//map the uniform buffer memory
	//fprintf(gpFile, "\nDEBUG:------------------------------------Mapping Uniform Buffer Memory------------------------------------");
	void *data = NULL;
	
	vkResult = vkMapMemory(vkDevice, uniformData.vkDeviceMemory, 0, sizeof(struct UniformData), 0, &data);
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[updateUniformBuffer][ERROR] Failed vkMapMemory()");
		return vkResult;
	}

	//copy data to the mapped memory
	memcpy(data, &myUniformData, sizeof(MyUniformData));
	//fprintf(gpFile, "\nDEBUG:[updateUniformBuffer][SUCCESS] memcpy()");

	//unmap the memory
	vkUnmapMemory(vkDevice, uniformData.vkDeviceMemory);

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
	VkDescriptorSetLayoutBinding vkDescriptorSetLayoutBinding;
	memset((void *)&vkDescriptorSetLayoutBinding, 0, sizeof(VkDescriptorSetLayoutBinding));

	//vkDescriptorSetLayoutBinding assignments
	vkDescriptorSetLayoutBinding.descriptorType 	= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; //descriptor type
	vkDescriptorSetLayoutBinding.binding 			= 0; //binding index
	vkDescriptorSetLayoutBinding.descriptorCount 	= 1; //descriptor count
	vkDescriptorSetLayoutBinding.stageFlags 		= VK_SHADER_STAGE_VERTEX_BIT; //shader stage flags
	vkDescriptorSetLayoutBinding.pImmutableSamplers = NULL; //sampler info, not used in this example

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
	vkDescriptorSetLayoutCreateInfo.bindingCount = 1; 	//binding count

	//In the shaders up till now we dont have any bindings,
	// so pbindings memmber of this structure wwas not used.
	// But in this example and hence fore we will use atleast one descriptor set, it to bind uniform buffer
	// and hence we need to use this member. with new type VkDescriptorSetLayoutBinding

	//assign the address of above vkDescriptorSetLayoutBinding to pBindings member of this structure
	//4> then call vkCreateDescriptorSetLayout() vulkan API with adress of above initailized structure
	//   and get the required vulkan object global VkDescriptorSetLayout in its last parameter
	vkDescriptorSetLayoutCreateInfo.pBindings = &vkDescriptorSetLayoutBinding;

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
--------------------------------------------------------------------------------------------------------
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
--------------------------------------------------------------------------------------------------------*/
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
	
	VkDescriptorPoolSize vkDescriptorPoolSize;
	memset((void *)&vkDescriptorPoolSize, 0, sizeof(VkDescriptorPoolSize));

	vkDescriptorPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; //descriptor type
	vkDescriptorPoolSize.descriptorCount = 1; //descriptor count

	//1> Declare and memset VkDescriptorPoolCreateInfo structure.
	VkDescriptorPoolCreateInfo vkDescriptorPoolCreateInfo;
	memset((void *)&vkDescriptorPoolCreateInfo, 0, sizeof(VkDescriptorPoolCreateInfo));

	vkDescriptorPoolCreateInfo.sType 			= VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	vkDescriptorPoolCreateInfo.pNext 			= NULL;
	vkDescriptorPoolCreateInfo.flags 			= 0; //VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT; //valid only for free descriptor set extension
	vkDescriptorPoolCreateInfo.maxSets 			= 1; //max sets in the pool
	vkDescriptorPoolCreateInfo.poolSizeCount 	= 1; //pool size count

	//3> Assign above VkDescriptorPoolSize structure address to ppoolSizes member of VkDescriptorPoolCreateInfo structure.
	vkDescriptorPoolCreateInfo.pPoolSizes = &vkDescriptorPoolSize;

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
	vkDescriptorSet = NULL;

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
	vkResult = vkAllocateDescriptorSets(vkDevice, &vkDescriptorSetAllocateInfo, &vkDescriptorSet);
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

	vkDescriptorBufferInfo.buffer = uniformData.vkBuffer; //buffer handle
	vkDescriptorBufferInfo.offset = 0; //offset in the buffer

	//5> Assign size of the uniform buffer to range member of VkDescriptorBufferInfo structure.
	vkDescriptorBufferInfo.range = sizeof(struct MyUniformData); //size of the uniform buffer

	
	//copy the data from shader to shader or write the data to the uniform buffer
	// we prefer writing,  this is done by using vkUpdateDescriptorSets() API.


	//6> Declare and memset VkWriteDescriptorSet structure.
	VkWriteDescriptorSet vkWriteDescriptorSet;

	memset((void *)&vkWriteDescriptorSet, 0, sizeof(VkWriteDescriptorSet));

	vkWriteDescriptorSet.sType 					= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	vkWriteDescriptorSet.pNext 					= NULL;
	vkWriteDescriptorSet.dstSet 				= vkDescriptorSet; //descriptor set handle
	vkWriteDescriptorSet.dstBinding 			= 0; //binding index
	vkWriteDescriptorSet.dstArrayElement 		= 0; //array element index
	vkWriteDescriptorSet.descriptorCount 		= 1; //descriptor count
	vkWriteDescriptorSet.descriptorType 		= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER; //descriptor type
	vkWriteDescriptorSet.pBufferInfo 			= &vkDescriptorBufferInfo; //buffer info address
	vkWriteDescriptorSet.pImageInfo 			= NULL; //image info, not used in this example
	vkWriteDescriptorSet.pTexelBufferView 		= NULL; //texel buffer view, not used in this example

	//7> Now call vkUpdateDescriptorSets() to update the descriptor set with above VkWriteDescriptorSet structure.
	vkUpdateDescriptorSets(vkDevice, 1, &vkWriteDescriptorSet, 0, NULL);

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
	VkAttachmentDescription vkAttachmentDescription_array[1];
	memset((void *)vkAttachmentDescription_array, 0, sizeof(VkAttachmentDescription) * _ARRAYSIZE(vkAttachmentDescription_array));

	vkAttachmentDescription_array[0].flags = 0; //VK_ATTACHMENT_DESCRIPTION_MAY_ALIAS_BIT;
	vkAttachmentDescription_array[0].format = vkFormat_color;
	vkAttachmentDescription_array[0].samples = VK_SAMPLE_COUNT_1_BIT;
	vkAttachmentDescription_array[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	vkAttachmentDescription_array[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	vkAttachmentDescription_array[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	vkAttachmentDescription_array[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	vkAttachmentDescription_array[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	vkAttachmentDescription_array[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	//2> Declare and initialize VkAttachmentReference structure. which will have information about the attachment
	//   described above.

	VkAttachmentReference vkAttachmentReference;
	memset((void *)&vkAttachmentReference, 0, sizeof(VkAttachmentReference));

	vkAttachmentReference.attachment = 0;
	vkAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	//3> Declare and initailise VkSubpassDescription structure and keep information about aboout VkAttachmentReference
	//   structure.
	VkSubpassDescription vkSubpassDescription;
	memset((void *)&vkSubpassDescription, 0, sizeof(VkSubpassDescription));

	vkSubpassDescription.flags = 0;
	vkSubpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	vkSubpassDescription.inputAttachmentCount = 0;
	vkSubpassDescription.pInputAttachments = NULL;
	vkSubpassDescription.colorAttachmentCount = _ARRAYSIZE(vkAttachmentDescription_array);
	vkSubpassDescription.pColorAttachments = &vkAttachmentReference;
	vkSubpassDescription.pResolveAttachments = NULL;
	vkSubpassDescription.pDepthStencilAttachment = NULL;
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
	VkVertexInputBindingDescription vkVertexInputBindingDescription_array[1];
	memset((void *)&vkVertexInputBindingDescription_array, 0, sizeof(VkVertexInputBindingDescription) * _ARRAYSIZE(vkVertexInputBindingDescription_array));

	vkVertexInputBindingDescription_array[0].binding = 0; 								//binding index
	vkVertexInputBindingDescription_array[0].stride = sizeof(float) * 3; 				//size of each vertex in bytes
	vkVertexInputBindingDescription_array[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX; 	//vertex input rate

	VkVertexInputAttributeDescription vkVertexInputAttributeDescription_array[1];
	memset((void *)&vkVertexInputAttributeDescription_array, 0, sizeof(VkVertexInputAttributeDescription) * _ARRAYSIZE(vkVertexInputAttributeDescription_array));

	vkVertexInputAttributeDescription_array[0].binding = 0; 							//binding index
	vkVertexInputAttributeDescription_array[0].location = 0; 							//location index
	vkVertexInputAttributeDescription_array[0].format = VK_FORMAT_R32G32B32_SFLOAT; 	//format of the vertex data
	vkVertexInputAttributeDescription_array[0].offset = 0; 								//offset of the vertex data
	
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
	vkPipelineRasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT; 		//cull mode
	vkPipelineRasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_CLOCKWISE; //front face
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
	vkGraphicsPipelineCreateInfo.flags = 0; 									//VK_GRAPHICS_PIPELINE_CREATE_RESERVE_1_BIT_EXT; //valid only for reserved extension
	vkGraphicsPipelineCreateInfo.pVertexInputState 		= &vkPipelineVertexInputStateCreateInfo; //vertex input state
	vkGraphicsPipelineCreateInfo.pInputAssemblyState 	= &vkPipelineInputAssemblyStateCreateInfo; //input assembly state
	vkGraphicsPipelineCreateInfo.pViewportState 		= &vkPipelineViewportStateCreateInfo; //viewport state
	vkGraphicsPipelineCreateInfo.pRasterizationState 	= &vkPipelineRasterizationStateCreateInfo; //rasterization state
	vkGraphicsPipelineCreateInfo.pMultisampleState 		= &vkPipelineMultisampleStateCreateInfo; //multisample state
	vkGraphicsPipelineCreateInfo.pColorBlendState 		= &vkPipelineColorBlendStateCreateInfo; //color blend state
	vkGraphicsPipelineCreateInfo.pDepthStencilState 	= NULL; 								//depth stencil state
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
	VkImageView vkImageView_attchments_array[1];
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

	//3> Allocate the FrameBuffer array by malloc to size of swapchain image count.
	vkFramebuffer_array = (VkFramebuffer *)malloc(swapchainImageCount * sizeof(VkFramebuffer));
	//error checking malloc

	//4> Start a loop for swapchain image count and call vkCreateFrameBuffer() to create frame buffers.
	for(uint32_t i = 0; i < swapchainImageCount; i++)
	{
		vkImageView_attchments_array[0] = swapchainImageView_array[i];

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
		VkClearValue vkClearValue_array[1];
		memset((void *)vkClearValue_array, 0, sizeof(VkClearValue) * _ARRAYSIZE(vkClearValue_array));

		vkClearValue_array[0].color = vkClearColorValue; //color member of union is used

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
		VkDeviceSize vkDeviceSize_offset;
		memset((void *)&vkDeviceSize_offset, 0, sizeof(VkDeviceSize));

		//bind our desciptor set to the pipeline
		vkCmdBindDescriptorSets(vkCommandBuffer_array[i], VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipelineLayout, 0, 1, &vkDescriptorSet, 0, NULL);

		vkCmdBindVertexBuffers(vkCommandBuffer_array[i], 0, 1, &vertexData_position.vkBuffer, &vkDeviceSize_offset);

		//here we should code the actual drawing commands to be recorded in the command buffer.
		//but we are not doing it here. we will do it in the next example.
		//we are just clearing the color buffer in this example.

		//draw call
		vkCmdDraw(vkCommandBuffer_array[i], 3, 1, 0, 0); //draw call


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
	fprintf(gpFile, "\nDEBUG:[debugReportCallback] pLayerPrefix 				:%s", pLayerPrefix);
	fprintf(gpFile, "\nDEBUG:[debugReportCallback] messageCode  				:%d", messageCode);
	fprintf(gpFile, "\nDEBUG:[debugReportCallback] vkDebugReportFlagsEXT 		:%d", vkDebugReportFlagsEXT);
	fprintf(gpFile, "\nDEBUG:[debugReportCallback] vkDebugReportObjectTypeEXT 	:%d", vkDebugReportObjectTypeEXT);
	fprintf(gpFile, "\nDEBUG:[debugReportCallback] object 						:%lld", object);
	fprintf(gpFile, "\nDEBUG:[debugReportCallback] location 					:%zd", location);

	return VK_FALSE;
}
/*--------------------------------------------------------------------------------------------------------*/




