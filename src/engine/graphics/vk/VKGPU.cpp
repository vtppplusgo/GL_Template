#include "VKGPU.hpp"

#ifdef VULKAN_BACKEND
#include "VKUtilities.hpp"
#include "VKSwapchain.hpp"

#include "VKInternalState.hpp"

VKGPUInternalState vkState;

GLFWwindow * VKcreateWindow(const std::string & name, Config & config){
	// Initialize glfw, which will create and setup an OpenGL context.
	if (!glfwInit()) {
		Log::Error() << Log::OpenGL << "Could not start GLFW3" << std::endl;
		return nullptr;
	}
	
	GLFWwindow* window;
	
	// Don't create an OpenGL context.
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	
	
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
	
	/// Vulkan instance creation.
	VKUtilities::createInstance("Test Vulkan", vkState.debugLayersEnabled, vkState.instance);
	
	/// Surface window setup.
	if(glfwCreateWindowSurface(vkState.instance, window, nullptr, &vkState.surface) != VK_SUCCESS) {
		Log::Error() << "Unable to create the surface." << std::endl;
		return nullptr;
	}

	/// Physical device.
	VKUtilities::createPhysicalDevice(vkState.instance, vkState.surface, vkState.physicalDevice, vkState.minUniformOffset);
	
	/// Logical device.
	// Queue setup.
	VKUtilities::ActiveQueues queues = VKUtilities::getGraphicsQueueFamilyIndex(vkState.physicalDevice, vkState.surface);
	std::set<int> selectedQueues = queues.getIndices();
	// Device features we want.
	VkPhysicalDeviceFeatures deviceFeatures = {};
	deviceFeatures.samplerAnisotropy = VK_TRUE;
	/// Create the logical device.
	VKUtilities::createDevice(vkState.physicalDevice, selectedQueues, deviceFeatures, vkState.device, vkState.debugLayersEnabled);
	/// Get references to the queues.
	vkGetDeviceQueue(vkState.device, queues.graphicsQueue, 0, &vkState.graphicsQueue);
	vkGetDeviceQueue(vkState.device, queues.presentQueue, 0, &vkState.presentQueue);
	
	/// Command pool.
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
	return window;
}

void VKresizeSwapchain(const unsigned int width, const unsigned int height){
	vkState.swapchain->resize(vkState, width, height);
}

void VKclean(){
	
	vkState.swapchain->clean(vkState);
	vkDestroyCommandPool(vkState.device, vkState.commandPool, nullptr);
	vkDestroyDevice(vkState.device, nullptr);
	
	if(vkState.debugLayersEnabled){
		VKUtilities::cleanupDebug(vkState.instance);
	}
	vkDestroySurfaceKHR(vkState.instance, vkState.surface, nullptr);
	vkDestroyInstance(vkState.instance, nullptr);
}

#endif
