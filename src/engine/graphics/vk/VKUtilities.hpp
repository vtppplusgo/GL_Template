#ifndef VKUtilities_h
#define VKUtilities_h
#include <map>
#include <set>
#include "../../Common.hpp"
#include "../../resources/MeshUtilities.hpp"

#ifdef VULKAN_BACKEND


class VKUtilities {
public:
	
	static bool checkValidationLayerSupport();
	static int createInstance(const std::string & name, const bool debugEnabled, VkInstance & instance);
	static void cleanupDebug(VkInstance & instance);
	
	struct ActiveQueues{
		int graphicsQueue = -1;
		int presentQueue = -1;
		
		const bool isComplete() const {
			return graphicsQueue >= 0 && presentQueue >= 0;
		}
		
		const std::set<int> getIndices() const {
			return { graphicsQueue, presentQueue };
		}
	};
	
	// TODO move to swapachain i guess
	struct SwapchainSupportDetails {
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};
	struct SwapchainParameters {
		SwapchainSupportDetails support;
		VkExtent2D extent;
		VkSurfaceFormatKHR surface;
		VkPresentModeKHR mode;
		uint32_t count;
	};
	
	static VkShaderModule createShaderModule(VkDevice device, const std::string& path);
	
	static int createBuffer(const VkPhysicalDevice & physicalDevice, const VkDevice & device, const VkDeviceSize & size, const VkBufferUsageFlags & usage, const VkMemoryPropertyFlags & properties, VkBuffer & buffer, VkDeviceMemory & bufferMemory);
	static void setupBuffers(const VkPhysicalDevice & physicalDevice, const VkDevice & device, const VkCommandPool & commandPool, const VkQueue & graphicsQueue, const Mesh & mesh, VkBuffer & vertexBuffer, VkDeviceMemory & vertexBufferMemory, VkBuffer & indexBuffer, VkDeviceMemory & indexBufferMemory);
	static VkSampler createSampler(const VkDevice & device, const VkFilter filter, const VkSamplerAddressMode mode, const uint32_t mipCount);
	static void generateMipmaps(VkImage & image, const int32_t width, const int32_t height, const bool cube, const uint32_t mipCount, const VkFormat format, const VkPhysicalDevice & physicalDevice, const VkDevice & device, const VkCommandPool & commandPool, const VkQueue & graphicsQueue);
	static void createTexture(const void * image, const uint32_t width, const uint32_t height, const bool cube, const uint32_t mipCount,  const VkPhysicalDevice & physicalDevice, const VkDevice & device, const VkCommandPool & commandPool, const VkQueue & graphicsQueue, VkImage & textureImage, VkDeviceMemory & textureMemory, VkImageView & textureView);
	
	static VkFormat findDepthFormat(const VkPhysicalDevice & physicalDevice);
	static int createImage(const VkPhysicalDevice & physicalDevice, const VkDevice & device, const uint32_t & width, const uint32_t & height, const uint32_t & mipCount, const VkFormat & format, const VkImageTiling & tiling, const VkImageUsageFlags & usage, const VkMemoryPropertyFlags & properties, const bool cube, VkImage & image, VkDeviceMemory & imageMemory);
	static void transitionImageLayout(const VkDevice & device, const VkCommandPool & commandPool, const VkQueue & queue, VkImage & image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, const bool cube, const uint32_t & mipCount);
	static VkImageView createImageView(const VkDevice & device, const VkImage & image, const VkFormat format, const VkImageAspectFlags aspectFlags, const bool cube, const uint32_t & mipCount);
	
	static int createPhysicalDevice(VkInstance & instance, VkSurfaceKHR & surface, VkPhysicalDevice & physicalDevice, VkDeviceSize & minUniformOffset);
	static int createDevice(VkPhysicalDevice & physicalDevice, std::set<int> & queuesIds, VkPhysicalDeviceFeatures & features, VkDevice & device, bool debugLayersEnabled);
	static ActiveQueues getGraphicsQueueFamilyIndex(VkPhysicalDevice device, VkSurfaceKHR surface);
	
	
	/// Swapchain
	static SwapchainParameters generateSwapchainParameters(VkPhysicalDevice & physicalDevice, VkSurfaceKHR & surface, const int width, const int height);
	static int createSwapchain(SwapchainParameters & parameters, VkSurfaceKHR & surface, VkDevice & device, ActiveQueues & queues,VkSwapchainKHR & swapchain, VkSwapchainKHR oldSwapchain = VK_NULL_HANDLE);
	static SwapchainSupportDetails querySwapchainSupport(VkPhysicalDevice adevice, VkSurfaceKHR asurface);
	
private:
	static uint32_t findMemoryType(const uint32_t typeFilter, const VkMemoryPropertyFlags & properties, const VkPhysicalDevice & physicalDevice);
	static std::vector<const char*> getRequiredInstanceExtensions(const bool enableValidationLayers);
	static bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	static VkFormat findSupportedFormat(const VkPhysicalDevice & physicalDevice, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	static VkCommandBuffer beginOneShotCommandBuffer( const VkDevice & device,  const VkCommandPool & commandPool);
	static void endOneShotCommandBuffer(VkCommandBuffer & commandBuffer,  const VkDevice & device,  const VkCommandPool & commandPool,  const VkQueue & queue);
	static bool hasStencilComponent(VkFormat format);
	static bool isDeviceSuitable(VkPhysicalDevice adevice, VkSurfaceKHR asurface);
	static VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, const int width, const int height);
	static VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	static VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);
	
	static void copyBuffer(const VkBuffer & srcBuffer, const VkBuffer & dstBuffer, const VkDeviceSize & size, const VkDevice & device, const VkCommandPool & commandPool, const VkQueue & queue);
	static void copyBufferToImage(const VkBuffer & srcBuffer, const VkImage & dstImage, const uint32_t & width, const uint32_t & height, const VkDevice & device, const VkCommandPool & commandPool, const VkQueue & queue, const bool cube);
private:
	
	static VkDebugReportCallbackEXT callback;
};

#endif
#endif
