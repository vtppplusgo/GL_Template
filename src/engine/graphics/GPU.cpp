#include "GPU.hpp"

#include "vk/VKGPU.hpp"
#include "gl/GLGPU.hpp"

#include "../input/Input.hpp"

// Singleton.
GPU& GPU::device(){
	static GPU* gpu = new GPU();
	return *gpu;
}

GPU::GPU(){
	
}

GLFWwindow * GPU::createWindow(const std::string & name, Config & config){
#if defined(VULKAN_BACKEND)
	return VKcreateWindow(name, config);
#elif defined(OPENGL_BACKEND)
	return GLcreateWindow(name, config);
#endif
}

void GPU::swap(GLFWwindow * window){
#if defined(VULKAN_BACKEND)
	if(Input::manager().resized()){
		int width = 0, height = 0;
		while(width == 0 || height == 0) {
			glfwGetFramebufferSize(window, &width, &height);
			glfwWaitEvents();
		}
		Input::manager().resizeEvent(width, height);
		VKresizeSwapchain(width, height);
	}
#elif defined(OPENGL_BACKEND)
	glfwSwapBuffers(window);
#endif

}

void GPU::clean(){
#if defined(VULKAN_BACKEND)
	VKclean();
#elif defined(OPENGL_BACKEND)
	// Nothing to do.
#endif
}
