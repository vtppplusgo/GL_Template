#ifndef VKInternalState_h
#define VKInternalState_h

#include "../../Common.hpp"

#ifdef VULKAN_BACKEND

#include "VKUtilities.hpp"


class VKSwapchain;

struct VKGPUInternalState {
	VkInstance instance;
	VkSurfaceKHR surface;
	
	VkPhysicalDevice physicalDevice;
	VkDevice device;
	
	VkQueue graphicsQueue;
	VkQueue presentQueue;
	
	VkCommandPool commandPool;
	
	VKSwapchain * swapchain; //Todo maybe in another way... a struct?
	
	//
	//std::vector<VkCommandBuffer> commandBuffers; in swapchain too ?
	
	
	
	//std::vector<VkSemaphore> imageAvailableSemaphores;
	//std::vector<VkSemaphore> renderFinishedSemaphores;
	//std::vector<VkFence> inFlightFences;
	
	
	
	//uint32_t imageIndex;
	//VkRenderPass finalRenderPass;
	// Infos
	bool debugLayersEnabled;
	VkDeviceSize minUniformOffset;
	//int maxInFlight = 2;
	
};

#endif
#endif
