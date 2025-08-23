#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "vk.h" 	//vk.h

//vulkan related header files
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>


//vulkan related libraries
#pragma comment (lib, "vulkan-1.lib")

//vulkan related global variables
//instance extension related
uint32_t enabledInstanceExtensionCount = 0;
//VK_KHR_SURFACE_EXTENSION_NAME
//VK_KHR_WIN32_SURFACE_EXTENSION_NAME
const char *enabledInstanceExtensionNames_Array[2];

//Vulkan Instance
VkInstance vkInstance = VK_NULL_HANDLE;

//macro definitions
#define WIN_WIDTH 800
#define WIN_HIGHT 600

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
		MessageBox(NULL, TEXT("Can Not Open Desired File..."), TEXT("Error"), MB_OK);
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
							TEXT("Vulkan: VkInstance"),
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
		fprintf(gpFile, "\nDEBUG:[WINMAIN()][ERROR] Failed initialize()");
		DestroyWindow(hwnd);
	}
	else
		fprintf(gpFile, "\nDEBUG:[WINMAIN()][SUCCESS] initialize()");

		
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
	VkResult fillInstanceExtensionNames(void);

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

	fprintf(gpFile, "\n\nDEBUG:------------------------------------Done[initialize()]------------------------------------");
	return vkResult;
}

void resize(int width, int hight)
{
	//code	
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

	//destroy vkinstance
	if(vkInstance)
	{
		vkDestroyInstance(vkInstance, NULL);
		vkInstance = VK_NULL_HANDLE;
		fprintf(gpFile, "\nDEBUG:[uninitialize()][SUCCESS] vkDestroyInstance()");
	}

	if(gpFile)
	{
		fprintf(gpFile, "\nDEBUG:[uninitialize] Debug File Closing");
		fprintf(gpFile, "\nDEBUG:------------------------------------Done[uninitialize()]------------------------------------");
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

	//function declarations
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
		instanceExtensionNames_array[i] = (char *)malloc(sizeof(char) * strlen(vkExtensionProperties_array[i].extensionName) + 1);
		memcpy(instanceExtensionNames_array[i], vkExtensionProperties_array[i].extensionName, strlen(vkExtensionProperties_array[i].extensionName) + 1);
		fprintf(gpFile, "\nDEBUG:[fillInstanceExtensionNames][INFO] Vulkan Extension Name[%d] = %s", i, instanceExtensionNames_array[i]);
	}

	//STEP 4
	//4> As not required onwords, here free VkExtensionProperties.
	free(vkExtensionProperties_array);
	vkExtensionProperties_array = NULL;

	//STEP 5
	//5> Find whether above extension names contains our 2 required extensions
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

