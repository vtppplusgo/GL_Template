//
//  Swapchain.hpp
//  GL_Template
//
//  Created by Simon Rodriguez on 14/07/2018.
//  Copyright © 2018 Simon Rodriguez. All rights reserved.
//

#ifndef Swapchain_hpp
#define Swapchain_hpp

#include "../../Common.hpp"

#include "VKUtilities.hpp"
#include "VKInternalState.hpp"

class VKSwapchain {
public:
	
	VKSwapchain(VKGPUInternalState & state, const int width, const int height);
	
	~VKSwapchain();
	
	void resize(VKGPUInternalState & state, const int width, const int height);
	
	void clean(VKGPUInternalState & state);
	
private:
	
	void setup(VKGPUInternalState & state, const int width, const int height);
	
	void unsetup(VKGPUInternalState & state);
	
	void createFinalRenderpass(VKGPUInternalState & state);
	
	VKUtilities::SwapchainParameters _parameters;
	//VkSurfaceKHR _surface;
	//VkQueue _presentQueue;
	std::vector<VkCommandBuffer> _commandBuffers;
	
	VkSwapchainKHR _swapchain;
	std::vector<VkImage> _swapchainImages;
	std::vector<VkImageView> _swapchainImageViews;
	std::vector<VkFramebuffer> _swapchainFramebuffers;
	VkImage _depthImage;
	VkDeviceMemory _depthImageMemory;
	VkImageView _depthImageView;
	uint32_t _maxInFlight;
	
	VkRenderPass finalRenderPass;
	//std::vector<VkSemaphore> _imageAvailableSemaphores;
	//std::vector<VkSemaphore> _renderFinishedSemaphores;
	//std::vector<VkFence> _inFlightFences;
	
	//uint32_t currentFrame;
	
};

#endif /* Swapchain_hpp */
