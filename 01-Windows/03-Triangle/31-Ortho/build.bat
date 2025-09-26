del vk.obj 
del DebugLog.txt 
del Application.exe
del vk.res

cl.exe /c /EHsc vk.cpp /I C:\VulkanSDK\Vulkan\Include

rc vk.rc 

link.exe vk.obj vk.res /LIBPATH:C:\VulkanSDK\Vulkan\Lib user32.lib gdi32.lib /SUBSYSTEM:WINDOWS /out:Application.exe

Application.exe

