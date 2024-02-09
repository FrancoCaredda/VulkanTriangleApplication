#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <string>
#include <vector>
#include <optional>

typedef struct QueueFamilyIndices_t {
	std::optional<uint32_t> graphicsIndex;
	std::optional<uint32_t> presentationIndex;

	inline bool IsCompleted() const noexcept { return graphicsIndex.has_value() && presentationIndex.has_value(); }
} QueueFamilyIndices;

class Application
{
public:
	Application() = default;
	~Application();

	void Run();
private:
	void InitGLFW();
	void InitWindow();
	void InitVkInstance();
	void InitSurface();
	void SelectDevice();
	void InitDevice();
	void InitSwapchain();
#ifdef _DEBUG
	VkDebugUtilsMessengerCreateInfoEXT GetDebugCreateInfo() const noexcept;
	void InitDebugger();

	static VkBool32 DebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT           messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT                  messageTypes,
		const VkDebugUtilsMessengerCallbackDataEXT*      pCallbackData,
		void*											 pUserData);
#endif

	std::vector<VkLayerProperties>& GetLayerProperties() const noexcept;
	std::vector<VkExtensionProperties>& GetExtensionProperties() const noexcept;

	std::vector<VkLayerProperties>& GetDeviceLayerProperties() const noexcept;
	std::vector<VkExtensionProperties>& GetDeviceExtensionProperties() const noexcept;

	VkPresentModeKHR GetPresentMode() const noexcept;
	VkSurfaceFormatKHR GetSurfaceFormat() const noexcept;
	VkExtent2D GetExtent2D() const noexcept;

	void PrintLayersAndExtensions() const noexcept;

	void RunMainLoop();
private:
	int m_Width = 600,
		m_Height = 400;

	std::string m_Title = "Triangle Application";

	GLFWwindow* m_Window;

	// Vulkan Artifacts
	VkInstance m_Instance;
	VkSurfaceKHR m_Surface;
	VkPhysicalDevice m_PhysicalDevice;
	VkDevice m_Device;
	VkQueue m_GraphicsQueue;
	VkQueue m_PresentationQueue;
	VkSwapchainKHR m_Swapchain;
#ifdef _DEBUG
	VkDebugUtilsMessengerEXT m_DebugMessenger;
#endif

	QueueFamilyIndices m_Indices;
};