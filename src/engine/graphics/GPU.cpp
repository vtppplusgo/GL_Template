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

bool GPU::nextFrame(){
	
#if defined(VULKAN_BACKEND)
	return VKacquireNextFrame();
#elif defined(OPENGL_BACKEND)
	// Nothing to do.
	// Temporary: example for debug.
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
	glClearDepth(1.0f);
	glClearStencil(0);
	glViewport(0.0f, 0.0f, Input::manager().size()[0], Input::manager().size()[1]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#endif
	return true;
	
}

bool GPU::swap(GLFWwindow * window){
	
#if defined(VULKAN_BACKEND)
	// Detect resizes here, to resize the swapchain framebuffers.
	const bool resizeDetected = Input::manager().resized();
	int width = 0, height = 0;
	// Query the new size.
	if(resizeDetected){
		while(width == 0 || height == 0) {
			glfwGetFramebufferSize(window, &width, &height);
			glfwWaitEvents();
		}
		Input::manager().resizeEvent(width, height);
		width = Input::manager().size()[0];
		height = Input::manager().size()[1];
	}
	// Swap.
	return VKswap(resizeDetected, width, height);
#elif defined(OPENGL_BACKEND)
	// Here we just rely on GLFW.
	glfwSwapBuffers(window);
#endif
	return true;
	
}

void GPU::clean(){
	
#if defined(VULKAN_BACKEND)
	VKclean();
#elif defined(OPENGL_BACKEND)
	// Nothing to do.
#endif
	
}
