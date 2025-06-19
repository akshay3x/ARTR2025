del vk.obj 
del Application.exe
del vk.res
del DebugLog.txt

cl.exe /c /EHsc vk.c

rc vk.rc 

link.exe vk.obj vk.res user32.lib gdi32.lib /SUBSYSTEM:WINDOWS /out:Application.exe

Application.exe

