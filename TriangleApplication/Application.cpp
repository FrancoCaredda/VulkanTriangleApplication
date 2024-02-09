#include "Application.h"

#include <stdexcept>
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
	vkDestroyDevice(m_Device, nullptr);
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
	deviceInfo.ppEnabledExtensionNames = nullptr;
	deviceInfo.enabledExtensionCount = 0;
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
