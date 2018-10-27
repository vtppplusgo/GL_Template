#ifndef VKInternalState_h
#define VKInternalState_h

#include "../../Common.hpp"

#ifdef VULKAN_BACKEND

#include "VKUtilities.hpp"


class VKSwapchain;

struct VKGPUInternalState {
	
	VkInstance instance;
	VkSurfaceKHR surface;
	// Devices
	VkPhysicalDevice physicalDevice;
	VkDevice device;
	// Queues
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	
	VkCommandPool commandPool;
	VKSwapchain * swapchain;
	std::vector<VkFence> fences;
	
	// Internal references.
	VkResult currentStatus;
	VkCommandBuffer * currentCommandBuffer;
	
	// Parameters.
	bool debugLayersEnabled;
	VkDeviceSize minUniformOffset;
	uint32_t maxInFlight;
};

#endif
#endif
