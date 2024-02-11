# The First Vulkan Application
Vulkan is a low-level graphics API designed by Khronos Group to substitute OpenGL. 
Vulkan gives us a low-level abstraction layer on top of the GPU which means that a lot of explicit work has to be done to visualize something to the screen.
## The application structure
The VulkanTriangleApplication project is divided in two main folders which are "TriangleApplication" and "Shaders". 
Under the "TriangleApplication" folder you will find the "Application.h" and "Application.cpp" files that contain the declaration and implementation of the "Application" class.
## Setting up the main components
To start working with Vulkan, some object has to be created:
1. Connection with the driver `VkInstance`;
2. Debugger object (not mandatory, but very useful in the development) `VkDebugUtilsMessengerEXT`;
3. Find a physical device `VkPhysicalDevice`;
4. Create a logical device `VkDevice`.
### Creating a connection with the driver
The `VkInstance` handle describes a connection with the Vulkan's driver.
To create it, an instance of the `VkInstanceCreateInfo` structure has to be filled and the `vkCreateInstance` function has to be called.

> Every single Vulkan object has its own CreateInfo structure and create/destroy functions.  
> Follow this pattern `Vk<object's name>CreateInfo` and `vkCreate<object's name>`/`vkDestroy<object's name>`.

Extensions are additional dynamic libraries for Vulkan that add additional functionality.  
Layers are additional functionality inside the application that can be disabled if you like.

[Check the InitVkInstance function](./TriangleApplication/Application.cpp#L101)
### Creating a debug object
The debug layer gives us possibility to recieve any errors from the driver and GPU and correct them.
Activating it can hit the performance, so make sure it is disabled when the final version is shipped.  
To create the validation layer, we need to require the `"VK_EXT_debug_utils"` extension and enable the `"VK_LAYER_KHRONOS_validation"` layer.

> If you want to print all extensions and layers names see the PrintLayersAndExtensions function  

To create the validation layer, we need:
1. Define a handler of the `VkDebugUtilsMessengerEXT` type;
2. Fill in the `VkDebugUtilsMessengerCreateInfoEXT` structure. [See the GetDebugCreateInfo function](./TriangleApplication/Application.cpp#L632);
3. Define the callback function. [See the DebugCallback function](./TriangleApplication/Application.cpp#L656);
4. Call the `vkCreateDebugUtilsMessengerEXT` function. [See the InitDebugger function](./TriangleApplication/Application.cpp#L648).

> To use the functions with the -EXT suffix, you have to load them dynamicly. [See the CreateDebugUtilsMessengerEXT function](./TriangleApplication/Application.cpp#L9)

### Selecting a physical device
Now the next is to select a physical device. Your platform can have multiple graphics devices with different sets of properties and available features. It can influence your choice. You can write an algorithm that will select the most suitable device for your tasks or give a choice to the user.

[Check the SelectDevice function](./TriangleApplication/Application.cpp#L154)
### Creating a logical device
## Creating the surface of the window
### Creating the surface
### Creating a swapchain
### Creating image views
## Rendering a triangle to the screen
### Configuring a pipeline
### Creating framebuffers
### Creating command buffers
### Submitting commands to a queue and drawing
