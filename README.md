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
### Selecting a physical device
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
