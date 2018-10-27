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

#ifdef VULKAN_BACKEND

#include "VKUtilities.hpp"
#include "VKInternalState.hpp"

class VKSwapchain {
public:
	
	VKSwapchain(VKGPUInternalState & state, const int width, const int height);
	
	~VKSwapchain();
	
	void resize(VKGPUInternalState & state, const int width, const int height);
	
	void clean(VKGPUInternalState & state);
	
	VkResult acquireNextFrame(VKGPUInternalState & state, VkRenderPassBeginInfo & infos);
	
	void presentCurrentFrame(VKGPUInternalState & state, VkPresentInfoKHR & presentInfo);
	
	const uint32_t currentFrame() const { return _currentFrame; }
	
	void step(){ _currentFrame = (_currentFrame + 1) % _maxInFlight; }
	
	VkCommandBuffer & getCommandBuffer(){ return _commandBuffers[_imageIndex]; }
	VkSemaphore & getStartSemaphore(){ return _imageAvailableSemaphores[_currentFrame]; }
	VkSemaphore & getEndSemaphore(){ return _renderFinishedSemaphores[_currentFrame]; }
	
private:
	
	void setup(VKGPUInternalState & state, const int width, const int height);
	
	void unsetup(VKGPUInternalState & state);
	
	void createFinalRenderpass(VKGPUInternalState & state);
	
	VKUtilities::SwapchainParameters _parameters;
	std::vector<VkCommandBuffer> _commandBuffers;
	
	VkSwapchainKHR _swapchain;
	std::vector<VkImage> _swapchainImages;
	std::vector<VkImageView> _swapchainImageViews;
	std::vector<VkFramebuffer> _swapchainFramebuffers;
	
	VkImage _depthImage;
	VkDeviceMemory _depthImageMemory;
	VkImageView _depthImageView;
	
	uint32_t _maxInFlight;
	uint32_t _currentFrame;
	uint32_t _imageIndex; // /!\ different from current frame.
	
	VkRenderPass finalRenderPass;
	
	std::vector<VkSemaphore> _imageAvailableSemaphores;
	std::vector<VkSemaphore> _renderFinishedSemaphores;
	
};

#endif
#endif 
