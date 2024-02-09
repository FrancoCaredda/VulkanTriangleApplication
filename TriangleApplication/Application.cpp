#include "Application.h"

#include <stdexcept>
#include <algorithm>
#include <iostream>

#ifdef _DEBUG
static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pMessenger)
{
	PFN_vkCreateDebugUtilsMessengerEXT func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));

	if (func == nullptr)
		return VK_ERROR_UNKNOWN;

	return func(instance, pCreateInfo, pAllocator, pMessenger);
}

static VkResult DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
	PFN_vkDestroyDebugUtilsMessengerEXT func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));

	if (func == nullptr)
		return VK_ERROR_UNKNOWN;

	func(instance, debugMessenger, pAllocator);

	return VK_SUCCESS;
}
#endif

Application::~Application()
{
	for (auto imageView : m_ImageViews)
		vkDestroyImageView(m_Device, imageView, nullptr);

	vkDestroySwapchainKHR(m_Device, m_Swapchain, nullptr);
	vkDestroyDevice(m_Device, nullptr);
	vkDestroySurfaceKHR(m_Instance, m_Surface, nullptr);
#ifdef _DEBUG
	DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
#endif
	vkDestroyInstance(m_Instance, nullptr);
	glfwDestroyWindow(m_Window);
	glfwTerminate();
}

void Application::Run()
{
	InitGLFW();
	InitWindow();
	InitVkInstance();
	InitSurface();
	SelectDevice();
	InitDevice();
	InitSwapchain();
	InitImageViews();

	PrintLayersAndExtensions();
#ifdef _DEBUG
	InitDebugger();
#endif // _DEBUG


	RunMainLoop();
}

void Application::InitGLFW()
{
	if (!glfwInit())
		throw std::runtime_error::exception("GLFW hasn't been initialized!");
}

void Application::InitWindow()
{
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	m_Window = glfwCreateWindow(m_Width, m_Height, m_Title.c_str(), nullptr, nullptr);

	if (m_Window == nullptr)
		throw std::runtime_error::exception("Window hasn't been created");
}

void Application::InitVkInstance()
{
	// Vulkan Extensions and Layers
	const char* extensions[] = {
		"VK_EXT_debug_utils",
		"VK_KHR_surface",
		"VK_KHR_win32_surface"
	};

	const char* layers[] = {
		"VK_LAYER_KHRONOS_validation"
	};

	VkApplicationInfo applicationInfo{};
	applicationInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	applicationInfo.apiVersion = VK_API_VERSION_1_3;
	applicationInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
	applicationInfo.pEngineName = nullptr;
	applicationInfo.pApplicationName = m_Title.c_str();

	VkInstanceCreateInfo instanceInfo{};
	instanceInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceInfo.pApplicationInfo = &applicationInfo;
	instanceInfo.enabledExtensionCount = sizeof(extensions) / sizeof(const char*);
	instanceInfo.ppEnabledExtensionNames = extensions;

#ifdef _DEBUG
	VkDebugUtilsMessengerCreateInfoEXT debugInfo = GetDebugCreateInfo();

	instanceInfo.enabledLayerCount = sizeof(layers) / sizeof(const char*);
	instanceInfo.ppEnabledLayerNames = layers;

	instanceInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugInfo;
#else
	instanceInfo.enabledLayerCount = 0;
	instanceInfo.ppEnabledLayerNames = nullptr;
#endif // _DEBUG

	if (vkCreateInstance(&instanceInfo, nullptr, &m_Instance) != VK_SUCCESS)
		throw std::runtime_error::exception("Instance hasn't been created!");
}

void Application::InitSurface()
{
	if (glfwCreateWindowSurface(m_Instance, m_Window, nullptr, &m_Surface) != VK_SUCCESS)
		throw std::runtime_error::exception("Surface hasn't been created!");
}

void Application::SelectDevice()
{
	std::vector<VkPhysicalDevice> devices;

	uint32_t count;
	vkEnumeratePhysicalDevices(m_Instance, &count, nullptr);

	devices.resize(count);
	vkEnumeratePhysicalDevices(m_Instance, &count, devices.data());

	for (auto device : devices)
	{
		VkPhysicalDeviceProperties properties{};
		VkPhysicalDeviceFeatures features{};

		vkGetPhysicalDeviceProperties(device, &properties);
		vkGetPhysicalDeviceFeatures(device, &features);

		if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			m_PhysicalDevice = device;
	}

	if (m_PhysicalDevice == VK_NULL_HANDLE)
		throw std::runtime_error::exception("Physical device hasn't been found!");

	std::vector<VkQueueFamilyProperties> familyProps;

	uint32_t queueFamilyPropsCount;
	vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyPropsCount, nullptr);

	familyProps.resize(queueFamilyPropsCount);
	vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &queueFamilyPropsCount, familyProps.data());

	for (int i = 0; i < familyProps.size(); i++)
	{
		auto& familyProp = familyProps[i];

		VkBool32 presentationQueueSupported;
		vkGetPhysicalDeviceSurfaceSupportKHR(m_PhysicalDevice, i, m_Surface, &presentationQueueSupported);

		if (familyProp.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			m_Indices.graphicsIndex = i;

		if (presentationQueueSupported)
			m_Indices.presentationIndex = i;
	}
}

void Application::InitDevice()
{
	const char* extensions[] = {
		"VK_KHR_swapchain"
	};

	float priority = 1.0;
	
	uint32_t queueIndices[] = {
		m_Indices.graphicsIndex.value(),
		m_Indices.presentationIndex.value()
	};
	VkDeviceQueueCreateInfo queueCreateInfos[2];

	for (int i = 0; i < sizeof(queueIndices) / sizeof(uint32_t); i++)
	{
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueIndices[i];
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &priority;

		queueCreateInfos[i] = queueCreateInfo;
	}

	VkPhysicalDeviceFeatures features{};
	vkGetPhysicalDeviceFeatures(m_PhysicalDevice, &features);

	VkDeviceCreateInfo deviceInfo{};
	deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceInfo.queueCreateInfoCount = sizeof(queueIndices) / sizeof(uint32_t);
	deviceInfo.pQueueCreateInfos = queueCreateInfos;
	deviceInfo.ppEnabledLayerNames = nullptr;
	deviceInfo.ppEnabledExtensionNames = extensions;
	deviceInfo.enabledExtensionCount = sizeof(extensions) / sizeof(const char*);
	deviceInfo.pEnabledFeatures = &features;

#ifdef _DEBUG
	const char* layers[] = {
		"VK_LAYER_KHRONOS_validation"
	};

	auto debugInfo = GetDebugCreateInfo();
	deviceInfo.enabledLayerCount = 1;
	deviceInfo.ppEnabledLayerNames = layers;
#endif // _DEBUG

	

	if (vkCreateDevice(m_PhysicalDevice, &deviceInfo, nullptr, &m_Device) != VK_SUCCESS)
		throw std::runtime_error::exception("Device hasn't been created!");

	vkGetDeviceQueue(m_Device, m_Indices.graphicsIndex.value(), 0, &m_GraphicsQueue);
	vkGetDeviceQueue(m_Device, m_Indices.presentationIndex.value(), 0, &m_PresentationQueue);
}

void Application::InitSwapchain()
{
	uint32_t queueIndices[] = {
		m_Indices.graphicsIndex.value(),
		m_Indices.presentationIndex.value()
	};


	m_PresentMode = GetPresentMode();
	m_Extent = GetExtent2D();
	m_Format = GetSurfaceFormat();

	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_PhysicalDevice, m_Surface, &surfaceCapabilities);

	VkSwapchainCreateInfoKHR swapchainInfo{};
	swapchainInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainInfo.surface = m_Surface;
	swapchainInfo.presentMode = m_PresentMode;
	swapchainInfo.imageExtent = m_Extent;
	swapchainInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	swapchainInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchainInfo.imageFormat = m_Format.format;
	swapchainInfo.imageColorSpace = m_Format.colorSpace;
	swapchainInfo.imageArrayLayers = 1;
	swapchainInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainInfo.minImageCount = surfaceCapabilities.minImageCount;
	swapchainInfo.clipped = VK_FALSE;
	swapchainInfo.oldSwapchain = VK_NULL_HANDLE;

	if ((queueIndices[0] == queueIndices[1]))
	{
		swapchainInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchainInfo.pQueueFamilyIndices = nullptr;
		swapchainInfo.queueFamilyIndexCount = 0;
	}
	else
	{
		swapchainInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchainInfo.pQueueFamilyIndices = queueIndices;
		swapchainInfo.queueFamilyIndexCount = 2; 
	}
	

	if (vkCreateSwapchainKHR(m_Device, &swapchainInfo, nullptr, &m_Swapchain) != VK_SUCCESS)
		throw std::runtime_error::exception("Swapchain hasn't been created!");
}

void Application::InitImageViews()
{
	uint32_t count;
	vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &count, nullptr);

	std::vector<VkImage> images(count);
	vkGetSwapchainImagesKHR(m_Device, m_Swapchain, &count, images.data());

	m_ImageViews.resize(count);
	for (int i = 0; i < images.size(); i++)
	{
		VkImageViewCreateInfo imageViewInfo{};
		imageViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		imageViewInfo.image = image;
		imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		imageViewInfo.format = m_Format.format;
		imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageViewInfo.subresourceRange.baseArrayLayer = 0;
		imageViewInfo.subresourceRange.baseMipLevel = 0;
		imageViewInfo.subresourceRange.layerCount = 1;
		imageViewInfo.subresourceRange.levelCount = 1;
		imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

		if (vkCreateImageView(m_Device, &imageViewInfo, nullptr, &m_ImageViews[i]) != VK_SUCCESS)
			throw std::runtime_error::exception("An image view hasn't been created!");
	}
}

#ifdef _DEBUG
VkDebugUtilsMessengerCreateInfoEXT Application::GetDebugCreateInfo() const noexcept
{
	VkDebugUtilsMessengerCreateInfoEXT debugInfo{};
	debugInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	debugInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
	debugInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
	debugInfo.pfnUserCallback = DebugCallback;

	return debugInfo;
}

void Application::InitDebugger()
{
	VkDebugUtilsMessengerCreateInfoEXT debugInfo = GetDebugCreateInfo();

	if (CreateDebugUtilsMessengerEXT(m_Instance, &debugInfo, nullptr, &m_DebugMessenger) != VK_SUCCESS)
		throw std::runtime_error::exception("Debug messenger hasn't been created!");
}

VkBool32 Application::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageTypes, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	if (messageSeverity != VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
		std::cerr << pCallbackData->pMessage << "\n\n";

	return 0;
}
#endif // _DEBUG

std::vector<VkLayerProperties>& Application::GetLayerProperties() const noexcept
{
	static std::vector<VkLayerProperties> properties;

	if (properties.empty())
	{
		uint32_t count;
		vkEnumerateInstanceLayerProperties(&count, nullptr);

		properties.resize(count);
		vkEnumerateInstanceLayerProperties(&count, properties.data());
	}

	return properties;
}

std::vector<VkExtensionProperties>& Application::GetExtensionProperties() const noexcept
{
	static std::vector<VkExtensionProperties> properties;

	if (properties.empty())
	{
		uint32_t count;
		vkEnumerateInstanceExtensionProperties(nullptr, &count, nullptr);

		properties.resize(count);
		vkEnumerateInstanceExtensionProperties(nullptr, &count, properties.data());
	}

	return properties;
}

std::vector<VkLayerProperties>& Application::GetDeviceLayerProperties() const noexcept
{
	static std::vector<VkLayerProperties> properties;

	if (properties.empty())
	{
		uint32_t count;
		vkEnumerateDeviceLayerProperties(m_PhysicalDevice, &count, nullptr);

		properties.resize(count);
		vkEnumerateDeviceLayerProperties(m_PhysicalDevice, &count, properties.data());
	}

	return properties;
}

std::vector<VkExtensionProperties>& Application::GetDeviceExtensionProperties() const noexcept
{
	static std::vector<VkExtensionProperties> properties;

	if (properties.empty())
	{
		uint32_t count;
		vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &count, nullptr);

		properties.resize(count);
		vkEnumerateDeviceExtensionProperties(m_PhysicalDevice, nullptr, &count, properties.data());
	}

	return properties;
}

VkPresentModeKHR Application::GetPresentMode() const noexcept
{
	uint32_t count;
	vkGetPhysicalDeviceSurfacePresentModesKHR(m_PhysicalDevice, m_Surface, &count, nullptr);

	std::vector<VkPresentModeKHR> presentModes(count);
	vkGetPhysicalDeviceSurfacePresentModesKHR(m_PhysicalDevice, m_Surface, &count, presentModes.data());

	if (std::find(presentModes.cbegin(), presentModes.cend(), VK_PRESENT_MODE_MAILBOX_KHR) != presentModes.cend())
		return VK_PRESENT_MODE_MAILBOX_KHR;

	return VK_PRESENT_MODE_FIFO_KHR;
}

VkSurfaceFormatKHR Application::GetSurfaceFormat() const noexcept
{
	uint32_t count;
	vkGetPhysicalDeviceSurfaceFormatsKHR(m_PhysicalDevice, m_Surface, &count, nullptr);

	std::vector<VkSurfaceFormatKHR> formats(count);
	vkGetPhysicalDeviceSurfaceFormatsKHR(m_PhysicalDevice, m_Surface, &count, formats.data());

	for (auto& format : formats)
	{
		if (format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR && format.format == VK_FORMAT_B8G8R8_SRGB)
			return format;
	}

	return formats[0];
}

VkExtent2D Application::GetExtent2D() const noexcept
{
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_PhysicalDevice, m_Surface, &surfaceCapabilities);

	if (surfaceCapabilities.currentExtent.width == std::numeric_limits<uint32_t>::max()) 
	{
		return surfaceCapabilities.currentExtent;
	}

	int width, height;
	glfwGetFramebufferSize(m_Window, &width, &height);

	VkExtent2D extent{};
	extent.width = std::clamp(static_cast<uint32_t>(width), 
							  surfaceCapabilities.minImageExtent.width, 
							  surfaceCapabilities.currentExtent.width);
	extent.height = std::clamp(static_cast<uint32_t>(width),
							   surfaceCapabilities.minImageExtent.height,
							   surfaceCapabilities.currentExtent.height);

	return extent;
}

void Application::PrintLayersAndExtensions() const noexcept
{
	{
		std::cout << "[INSTANCE EXTENSIONS]:" << "\n\n";
		auto& extensionProperties = GetExtensionProperties();

		for (auto& prop : extensionProperties)
			std::cout << prop.extensionName << "\n";
	}

	{
		std::cout << "[INSTANCE LAYERS]:" << "\n\n";
		auto& layerProperties = GetLayerProperties();

		for (auto& prop : layerProperties)
			std::cout << prop.layerName << "\n";
	}

	{
		std::cout << "[DEVICE EXTENSIONS]:" << "\n\n";
		auto& extensionProperties = GetDeviceExtensionProperties();

		for (auto& prop : extensionProperties)
			std::cout << prop.extensionName << "\n";
	}

	{
		std::cout << "[DEVICE LAYERS]:" << "\n\n";
		auto& layerProperties = GetDeviceLayerProperties();

		for (auto& prop : layerProperties)
			std::cout << prop.layerName << "\n";
	}
}

void Application::RunMainLoop()
{
	while (!glfwWindowShouldClose(m_Window))
	{
		glfwPollEvents();
	}
}
