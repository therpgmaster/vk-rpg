# vk-rpg

## Build configuration
#### Headers
Add the following as include directories:
```
Source files root (example path: C:/dev/vk-rpg/Src)
Vulkan SDK (example path: C:/dev/VulkanSDK/1.2.170.0/Include)
GLFW (example path: C:/dev/glfw-3.3.3.bin.WIN64/include)
GLM (example path: C:/dev/glm)
```
#### Libraries
Add the following as linker library directories:
```
Vulkan SDK (example path: C:/dev/VulkanSDK/1.2.170.0/Lib)
GLFW (example path: C:/dev/glfw-3.3.3.bin.WIN64/lib-vc2019)
```
These libraries should be added as linker input dependencies:
```
vulkan-1.lib
glfw3.lib
```

Let the linker to ignore these default libraries (Windows):
```
MSVCRT
libucrtd
```

Runtime library: Multi-threaded Debug DLL (/MDd) or Multi-threaded DLL (/MD) 
depending for debug/release configuration


*///////
Author's path settings
///////
G:\VulkanDev\VulkanSDK\1.2.170.0\Include
G:\VulkanDev\glfw-3.3.3.bin.WIN64\include
G:\VulkanDev\glm
///////
G:\VulkanDev\VulkanSDK\1.2.170.0\Lib
G:\VulkanDev\glfw-3.3.3.bin.WIN64\lib-vc2019
///////*


