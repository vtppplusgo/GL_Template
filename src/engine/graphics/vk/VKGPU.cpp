#include "VKGPU.hpp"

#include <array>

#ifdef VULKAN_BACKEND
#include "VKUtilities.hpp"
#include "VKSwapchain.hpp"

#include "VKInternalState.hpp"


VKGPUInternalState vkState;

GLFWwindow * VKcreateWindow(const std::string & name, Config & config){
	// Initialize GLFW, which will create and setup an OpenGL context.
	if (!glfwInit()) {
		Log::Error() << Log::OpenGL << "Could not start GLFW3" << std::endl;
		return nullptr;
	}
	
	GLFWwindow* window;
	// Don't create an OpenGL context.
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	
	// Create the window at the right size.
	if(config.fullscreen){
		const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
		glfwWindowHint(GLFW_RED_BITS, mode->redBits);
		glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
		glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
		glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
		window = glfwCreateWindow(mode->width, mode->height, name.c_str(), glfwGetPrimaryMonitor(), nullptr);
	} else {
		// Create a window with a given size. Width and height are defined in the configuration.
		window = glfwCreateWindow(config.initialWidth, config.initialHeight, name.c_str(), nullptr, nullptr);
	}
	
	if(!window){
		Log::Error() << "Unable to create GLFW window." << std::endl;
		return nullptr;
	}
	
	// Debug setup.
	// Vulkan provide validations layers that can check if we do any mistake.
#ifdef DEBUG
	vkState.debugLayersEnabled = true;
#else
	vkState.debugLayersEnabled = false;
#endif

	// Check if the validation layers are needed and available.
	if(vkState.debugLayersEnabled && !VKUtilities::checkValidationLayerSupport()){
		Log::Error() << "Validation layers required and unavailable." << std::endl;
		vkState.debugLayersEnabled = false;
	}
	
	// Create the Vulkan instance (context).
	VKUtilities::createInstance("Test Vulkan", vkState.debugLayersEnabled, vkState.instance);
	
	// Create the surface backing the window.
	if(glfwCreateWindowSurface(vkState.instance, window, nullptr, &vkState.surface) != VK_SUCCESS) {
		Log::Error() << "Unable to create the surface." << std::endl;
		return nullptr;
	}

	// Obtain the best possible physical device.
	VKUtilities::createPhysicalDevice(vkState.instance, vkState.surface, vkState.physicalDevice, vkState.minUniformOffset);
	
	// Queues: we need them to submit commands. We need a graphics one and a presentation one (can be the same).
	VKUtilities::ActiveQueues queues = VKUtilities::getGraphicsQueueFamilyIndex(vkState.physicalDevice, vkState.surface);
	std::set<int> selectedQueues = queues.getIndices();
	
	// Device setup: we can request additional features.
	VkPhysicalDeviceFeatures deviceFeatures = {};
	deviceFeatures.samplerAnisotropy = VK_TRUE;
	// Create the logical device.
	VKUtilities::createDevice(vkState.physicalDevice, selectedQueues, deviceFeatures, vkState.device, vkState.debugLayersEnabled);
	// Get references to the queues.
	vkGetDeviceQueue(vkState.device, queues.graphicsQueue, 0, &vkState.graphicsQueue);
	vkGetDeviceQueue(vkState.device, queues.presentQueue, 0, &vkState.presentQueue);
	
	// Command pool: where our command buffers will be allocated.
	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = queues.graphicsQueue;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	if(vkCreateCommandPool(vkState.device, &poolInfo, nullptr, &vkState.commandPool) != VK_SUCCESS) {
		Log::Error() << "Unable to create command pool." << std::endl;
	}
	
	// Create the swapchain.
	VKSwapchain * swapchain = new VKSwapchain(vkState, config.initialWidth, config.initialHeight);
	vkState.swapchain = swapchain;
	
	// Create fences to ensure that we do not reuse a command buffer currently in use by the GPU.
	vkState.fences.resize(vkState.maxInFlight);
	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	for(size_t i = 0; i < vkState.fences.size(); i++) {
		if(vkCreateFence(vkState.device, &fenceInfo, nullptr, &vkState.fences[i]) != VK_SUCCESS) {
			Log::Error() << "Unable to create  fences." << std::endl;
		}
	}
	
	vkState.currentStatus = VK_SUCCESS;
	return window;
}


bool VKacquireNextFrame(){
	// Wait for the current command buffer to be done.
	vkWaitForFences(vkState.device, 1, &vkState.fences[vkState.swapchain->currentFrame()], VK_TRUE, std::numeric_limits<uint64_t>::max());
	// Acquire frame infos from swap chain.
	VkRenderPassBeginInfo infos = {};
	VkResult status = vkState.swapchain->acquireNextFrame(vkState, infos);
	// Save the acquisition result status.
	vkState.currentStatus = status;
	// Indicate if we can encode a new frame.
	bool success = (status == VK_SUCCESS || status == VK_SUBOPTIMAL_KHR);
	if(success){
		// Start the command buffer.
		// To mimic the "old way" we will only use one, and rewrite it each frame. This is suboptimal but we are not a commercial game engine.
		VkCommandBufferBeginInfo beginInfo = {};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		const auto & finalCommandBuffer = vkState.swapchain->getCommandBuffer();
		vkBeginCommandBuffer(finalCommandBuffer, &beginInfo);
		// Save a pointer to the current command buffer.
		vkState.currentCommandBuffer = &vkState.swapchain->getCommandBuffer();
		
		// Temporary: example for debug.
		// Complete final pass infos.
		std::array<VkClearValue, 2> clearValues = {};
		clearValues[0].color = { { 0.0f, 1.0f, 0.0f, 1.0f} };
		clearValues[1].depthStencil = { 1.0f, 0 };
		infos.clearValueCount = static_cast<uint32_t>(clearValues.size());
		infos.pClearValues = clearValues.data();
		// Submit final pass.
		vkCmdBeginRenderPass(finalCommandBuffer, &infos, VK_SUBPASS_CONTENTS_INLINE);
		vkCmdEndRenderPass(finalCommandBuffer);
	}
	return success;
}

bool VKswap(bool resizedDetected, unsigned int width, unsigned int height){
	// We have been able to acquire a frame and write to a command buffer.
	if(vkState.currentStatus == VK_SUCCESS || vkState.currentStatus == VK_SUBOPTIMAL_KHR){
		// Finalize the current command buffer.
		vkEndCommandBuffer(*vkState.currentCommandBuffer);
		// Get the current frame semaphores.
		const auto & startSemaphore = vkState.swapchain->getStartSemaphore();
		const auto & endSemaphore = vkState.swapchain->getEndSemaphore();
		// Submit the command buffer.
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		// The semaphore will wait until the GPU reach the very end of the frame generation (color output).
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &startSemaphore;
		const VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.pWaitDstStageMask = waitStages;
		// We submit a unique command buffer.
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = vkState.currentCommandBuffer;
		// Semaphore for when the command buffer is done, so that we can present the image.
		VkSemaphore signalSemaphores[] = { endSemaphore };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;
		// Add the fence so that we don't reuse the command buffer while it's in use.
		vkResetFences(vkState.device, 1, &vkState.fences[vkState.swapchain->currentFrame()]);
		// Submit the command buffer.
		vkQueueSubmit(vkState.graphicsQueue, 1, &submitInfo, vkState.fences[vkState.swapchain->currentFrame()]);
		// Present the finalized frame on screen.
		VkPresentInfoKHR presentInfo = {};
		vkState.swapchain->presentCurrentFrame(vkState, presentInfo);
		VkResult status = vkQueuePresentKHR(vkState.presentQueue, &presentInfo);
		vkState.currentStatus = status;
	}
	// The swapchain must be resized, either because we were told explicitely or because the swapchain is suboptimal/out of date.
	if(vkState.currentStatus == VK_ERROR_OUT_OF_DATE_KHR || vkState.currentStatus == VK_SUBOPTIMAL_KHR || resizedDetected){
		vkState.swapchain->resize(vkState, width, height);
	} else if (vkState.currentStatus != VK_SUCCESS) {
		Log::Error() << "Error while rendering or presenting." << std::endl;
		return false;
	}
	// Advance the swapchain frame index.
	vkState.swapchain->step();
	return true;
}

void VKclean(){
	// Wait for the device to finish all jobs.
	vkDeviceWaitIdle(vkState.device);
	// Clean the swapchain first.
	vkState.swapchain->clean(vkState);
	delete vkState.swapchain;
	// Then the other objects.
	for(size_t i = 0; i < vkState.fences.size(); i++) {
		vkDestroyFence(vkState.device, vkState.fences[i], nullptr);
	}
	vkDestroyCommandPool(vkState.device, vkState.commandPool, nullptr);
	vkDestroyDevice(vkState.device, nullptr);
	if(vkState.debugLayersEnabled){
		VKUtilities::cleanupDebug(vkState.instance);
	}
	vkDestroySurfaceKHR(vkState.instance, vkState.surface, nullptr);
	vkDestroyInstance(vkState.instance, nullptr);
}

#endif
