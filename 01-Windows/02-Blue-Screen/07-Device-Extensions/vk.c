#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "vk.h" 	//vk.h

//vulkan related header files
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>


//vulkan related libraries
#pragma comment (lib, "vulkan-1.lib")


//---------------------------------------------------------------------------------------------
//vulkan related global variables
//instance extension related
uint32_t enabledInstanceExtensionCount = 0;
//VK_KHR_SURFACE_EXTENSION_NAME
//VK_KHR_WIN32_SURFACE_EXTENSION_NAME
const char *enabledInstanceExtensionNames_Array[2];

VkInstance vkInstance = VK_NULL_HANDLE;
VkSurfaceKHR vkSurfaceKHR = VK_NULL_HANDLE;

//Vulkan Physical device related global variables
VkPhysicalDevice vkPhysicalDevice_Selected = VK_NULL_HANDLE;
uint32_t graphicsQueueFamilyIndex_Selected = UINT32_MAX;
VkPhysicalDeviceMemoryProperties vkPhysicalDeviceMemoryProperties;

uint32_t physicalDeviceCount = 0;
VkPhysicalDevice *vkPhysicalDevice_array = NULL;

//vulkan related global variables
//device extension related
uint32_t enabledDeviceExtensionCount = 0;
//VK_KHR_SWAPCHAIN_EXTENSION_NAME
//VK_KHR_WIN32_SURFACE_EXTENSION_NAME
const char *enabledDeviceExtensionNames_Array[1];



//macro definitions
#define WIN_WIDTH 800
#define WIN_HIGHT 600


//---------------------------------------------------------------------------------------------
//window procedure
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

//global variable declarations
HWND ghwnd = NULL;

DWORD dwStyle;
//WINDOWPLACEMENT wpPrev = { };
WINDOWPLACEMENT wpPrev = {sizeof(WINDOWPLACEMENT)};

BOOL gbFullscreen = FALSE;
BOOL gbActiveWindow = FALSE;
FILE *gpFile = NULL;

const char *gpszAppName = "VulkanApp";

//main
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	//function declations
	VkResult initialize(void);
	void display(void);
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
							TEXT("Vulkan | 07-Device-Extensions"),
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

		
	ShowWindow(hwnd, iCmdShow);
	fprintf(gpFile, "\nDEBUG:[WINMAIN()][SUCCESS] ShowWindow()");
	SetForegroundWindow(hwnd);
	fprintf(gpFile, "\nDEBUG:[WINMAIN()][SUCCESS] SetForegroundWindow()");
	SetFocus(hwnd);
	fprintf(gpFile, "\nDEBUG:[WINMAIN()][SUCCESS] SetFocus()");

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
				display();
			}
		}
	}

	return((int)msg.wParam);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	//function declarations
	void toggleFullscreen(void);
	void uninitialize(void);
	void resize(int, int);
	void display(void);

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
			resize(LOWORD(lParam), HIWORD(lParam));
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
	VkResult fillDeviceExtensionNames(void);


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
		fprintf(gpFile, "\nDEBUG:[initialize][SUCCESS] createVulkanInstance()");

	//Presetation Surface Steps
	vkResult = getSupportedSurface();
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[initialize][ERROR] Failed getSupportedSurface()");
		return vkResult;
	}
	else
		fprintf(gpFile, "\nDEBUG:[initialize][SUCCESS] getSupportedSurface()");

	//Get/select required physical device and its queue family index
	vkResult = getPhysicalDevice();
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[initialize][ERROR] Failed getPhysicalDevice()");
		return vkResult;
	}
	else
		fprintf(gpFile, "\nDEBUG:[initialize][SUCCESS] getPhysicalDevice()");

	//print vulkan info
	vkResult = printVkInfo();
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[initialize][ERROR] Failed printVkInfo()");
		return vkResult;
	}
	else
		fprintf(gpFile, "\nDEBUG:[initialize][SUCCESS] printVkInfo()");

	//device extensions
	vkResult = fillDeviceExtensionNames();
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[initialize][ERROR] Failed fillDeviceExtensionNames()");
		return vkResult;
	}
	else
		fprintf(gpFile, "\nDEBUG:[initialize][SUCCESS] fillDeviceExtensionNames()");
	
	

	fprintf(gpFile, "\nDEBUG:------------------------------------Done [initialize()]------------------------------------\n\n");
	return vkResult;	
}

void resize(int width, int hight)
{
	
}

void display(void)
{
	
}

void update()
{
	//code
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
	//destroy vkSurfaceKHR
	if(vkSurfaceKHR)
	{
		vkDestroySurfaceKHR(vkInstance, vkSurfaceKHR, NULL);
		vkSurfaceKHR = VK_NULL_HANDLE;
		fprintf(gpFile, "\nDEBUG:[uninitialize()][SUCCESS] vkDestroySurfaceKHR()");
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
		instanceExtensionNames_array[i] = (char *)malloc(sizeof(char) * strlen(vkExtensionProperties_array[i].extensionName) + 1);
		if(instanceExtensionNames_array[i] == NULL)
		{
			fprintf(gpFile, "\nDEBUG:[fillInstanceExtensionNames][ERROR] Failed to allocate memory for instanceExtensionNames_array[%d]", i);
			return VK_ERROR_OUT_OF_HOST_MEMORY; //return hardcoded failure
		}
				
		//copy extension name from vkExtensionProperties_array to instanceExtensionNames_array
		memcpy(instanceExtensionNames_array[i], vkExtensionProperties_array[i].extensionName, strlen(vkExtensionProperties_array[i].extensionName) + 1);
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

	
	vkPhysicalDevice_array = (VkPhysicalDevice *)malloc(physicalDeviceCount * sizeof(VkPhysicalDevice));
	if(vkPhysicalDevice_array == NULL)
	{
		fprintf(gpFile, "\nDEBUG:[getPhysicalDevice][ERROR] Failed to allocate memory for vkPhysicalDevice_array");
		vkResult = VK_ERROR_OUT_OF_HOST_MEMORY; //return hardcoded failure
		return vkResult;
	}
	else
		fprintf(gpFile, "\nDEBUG:[getPhysicalDevice][SUCCESS] Memory allocated for vkPhysicalDevice_array");

	vkResult = vkEnumeratePhysicalDevices(vkInstance, &physicalDeviceCount, vkPhysicalDevice_array);
	if(vkResult != VK_SUCCESS)
	{	
		fprintf(gpFile, "\nDEBUG:[getPhysicalDevice][ERROR] Failed vkEnumeratePhysicalDevices() Second Call");
		vkResult = VK_ERROR_INITIALIZATION_FAILED;
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
					fprintf(gpFile, "\nDEBUG:[getPhysicalDevice][SUCCESS] Graphics enabled vkPhysicalDevice_Selected index = %d", i);
					fprintf(gpFile, "\nDEBUG:[getPhysicalDevice][SUCCESS] Graphics enabled graphicsQueueFamilyIndex_Selected index = %d", graphicsQueueFamilyIndex_Selected);
					break;
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

