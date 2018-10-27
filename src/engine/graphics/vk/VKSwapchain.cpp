//
//  VKSwapchain.cpp
//  GL_Template
//
//  Created by Simon Rodriguez on 14/07/2018.
//  Copyright © 2018 Simon Rodriguez. All rights reserved.
//

#include "VKSwapchain.hpp"
#include <array>

#ifdef VULKAN_BACKEND

VKSwapchain::VKSwapchain(VKGPUInternalState & state, const int width, const int height) {
	_currentFrame = 0;
	
	// Setup the swapchain framebuffers/views/etc.
	setup(state, width, height);
	
	// We only need to create the "render done" and "frame presented" semaphores once.
	_imageAvailableSemaphores.resize(_maxInFlight);
	_renderFinishedSemaphores.resize(_maxInFlight);
	
	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	
	// Create as many semaphores of each type as we have frames in flight on the GPU.
	for(size_t i = 0; i < _maxInFlight; i++) {
		VkResult sem1 = vkCreateSemaphore(state.device, &semaphoreInfo, nullptr, &_imageAvailableSemaphores[i]);
		VkResult sem2 = vkCreateSemaphore(state.device, &semaphoreInfo, nullptr, &_renderFinishedSemaphores[i]);
		if(sem1 != VK_SUCCESS || sem2 != VK_SUCCESS ) {
			Log::Error() << "Unable to create semaphores." << std::endl;
		}
	}
}



void VKSwapchain::setup(VKGPUInternalState & state, const int width, const int height) {
	
	// Get the queues, we will have command buffers to submit.
	VKUtilities::ActiveQueues queues = VKUtilities::getGraphicsQueueFamilyIndex(state.physicalDevice, state.surface);
	
	// Setup swapchain.
	// Obtain the best parameters for the current surface and size.
	_parameters = VKUtilities::generateSwapchainParameters(state.physicalDevice, state.surface, width, height);
	// Create the swapchain.
	VKUtilities::createSwapchain(_parameters, state.surface, state.device, queues, _swapchain);
	_maxInFlight = _parameters.count;
	
	// Create the final render pass, that must write to the swapchain framebuffers.
	createFinalRenderpass(state);
	
	// Create the depth buffer.
	// Find the best format.
	VkFormat depthFormat = VKUtilities::findDepthFormat(state.physicalDevice);
	// Create the backing image and image views, and set the imaget to be used as a depth/stencil attachment.
	VKUtilities::createImage(state.physicalDevice, state.device, _parameters.extent.width, _parameters.extent.height, 1, depthFormat , VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, false, _depthImage, _depthImageMemory);
	_depthImageView = VKUtilities::createImageView(state.device, _depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, false, 1);
	VKUtilities::transitionImageLayout(state.device, state.commandPool, state.graphicsQueue, _depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, false, 1);
	
	// Retrieve images from the swap chain (we don't create them).
	// First we get their number.
	vkGetSwapchainImagesKHR(state.device, _swapchain, &_parameters.count, nullptr);
	_maxInFlight = _parameters.count;
	_swapchainImages.resize(_maxInFlight);
	Log::Info() << "Swapchain using " << _maxInFlight << " images."<< std::endl;
	// Then we really query them.
	vkGetSwapchainImagesKHR(state.device, _swapchain, &_parameters.count, _swapchainImages.data());
	// Create views for each image.
	_swapchainImageViews.resize(_maxInFlight);
	for(size_t i = 0; i < _maxInFlight; i++) {
		_swapchainImageViews[i] = VKUtilities::createImageView(state.device, _swapchainImages[i], _parameters.surface.format, VK_IMAGE_ASPECT_COLOR_BIT, false, 1);
	}
	// From the images, we can create framebuffers.
	_swapchainFramebuffers.resize(_maxInFlight);
	for(size_t i = 0; i < _maxInFlight; ++i){
		// They each use a color swapchain image view and a depth image view.
		std::array<VkImageView, 2> attachments = { _swapchainImageViews[i], _depthImageView };
		// Create the framebuffer.
		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = finalRenderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = _parameters.extent.width;
		framebufferInfo.height = _parameters.extent.height;
		framebufferInfo.layers = 1;
		if(vkCreateFramebuffer(state.device, &framebufferInfo, nullptr, &_swapchainFramebuffers[i]) != VK_SUCCESS) {
			Log::Error() << "Unable to create swap framebuffers." << std::endl;
		}
	}
	
	// For each frame, we need a command buffer to render into it.
	_commandBuffers.resize(_maxInFlight);
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = state.commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = static_cast<uint32_t>(_commandBuffers.size());
	if(vkAllocateCommandBuffers(state.device, &allocInfo, _commandBuffers.data()) != VK_SUCCESS) {
		Log::Error() << "Unable to create command buffers." << std::endl;
	}
	
	state.maxInFlight = _maxInFlight;
}

void VKSwapchain::createFinalRenderpass(VKGPUInternalState & state){
	// Final rendering pass.
	// \note The attachment descriptions and the render pass infos don't have to link to the image views.
	// Depth attachment.
	VkAttachmentDescription depthAttachment = {};
	depthAttachment.format = VKUtilities::findDepthFormat(state.physicalDevice);
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	VkAttachmentReference depthAttachmentRef = {};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	
	// Color attachment.
	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = _parameters.surface.format;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	
	// We have one subpass.
	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef; // linked to the layout in fragment shader.
	subpass.pDepthStencilAttachment = &depthAttachmentRef;
	
	// Dependencies: between what happens before this pass and the beginning of the subpass. When we need to read/write the color attachment, wait until the final color output is done.
	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	
	// Create the render pass.
	std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;
	
	if(vkCreateRenderPass(state.device, &renderPassInfo, nullptr, &finalRenderPass) != VK_SUCCESS) {
		Log::Error() << "Unable to create render pass." << std::endl;
	}
}

void VKSwapchain::resize(VKGPUInternalState & state, const int width, const int height){
	// Avoid unecessary work.
	if(width == _parameters.extent.width && height == _parameters.extent.height){
		return;
	}
	// \bug: Some semaphores can leave the queue eternally waiting.
	vkDeviceWaitIdle(state.device);
	unsetup(state);
	// Recreate swapchain.
	setup(state, width, height);
}

VkResult VKSwapchain::acquireNextFrame(VKGPUInternalState & state, VkRenderPassBeginInfo & infos){
	// Acquire the next available frame from the swapchain.
	VkResult status = vkAcquireNextImageKHR(state.device, _swapchain, std::numeric_limits<uint64_t>::max(), _imageAvailableSemaphores[_currentFrame], VK_NULL_HANDLE, &_imageIndex);
	if(status != VK_SUCCESS && status != VK_SUBOPTIMAL_KHR) {
		return status;
	}
	
	// Partially fill render pass infos with internal data.
	infos = {};
	infos.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	infos.renderPass = finalRenderPass;
	infos.framebuffer = _swapchainFramebuffers[_imageIndex];
	infos.renderArea.offset = { 0, 0 };
	infos.renderArea.extent = _parameters.extent;
	return status;
}

void VKSwapchain::presentCurrentFrame(VKGPUInternalState & state, VkPresentInfoKHR & presentInfo){
	// Present on swap chain.
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	// Wait for the command buffer to be executed.
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &_renderFinishedSemaphores[_currentFrame];
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &_swapchain;
	presentInfo.pImageIndices = &_imageIndex;
}

VKSwapchain::~VKSwapchain() {
}

void VKSwapchain::clean(VKGPUInternalState & state){
	unsetup(state);
	for(size_t i = 0; i < _renderFinishedSemaphores.size(); i++) {
		vkDestroySemaphore(state.device, _renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(state.device, _imageAvailableSemaphores[i], nullptr);
	}
}

void VKSwapchain::unsetup(VKGPUInternalState & state) {
	// Destroy everything thing that rely on the swapchain image size.
	for(size_t i = 0; i < _maxInFlight; i++) {
		vkDestroyFramebuffer(state.device, _swapchainFramebuffers[i], nullptr);
	}
	vkFreeCommandBuffers(state.device, state.commandPool, static_cast<uint32_t>(_commandBuffers.size()), _commandBuffers.data());
	
	vkDestroyRenderPass(state.device, finalRenderPass, nullptr);
	vkDestroyImageView(state.device, _depthImageView, nullptr);
	for(size_t i = 0; i < _swapchainImageViews.size(); i++) {
		vkDestroyImageView(state.device, _swapchainImageViews[i], nullptr);
	}
	vkDestroyImage(state.device, _depthImage, nullptr);
	vkFreeMemory(state.device, _depthImageMemory, nullptr);
	vkDestroySwapchainKHR(state.device, _swapchain, nullptr);
}

#endif
