#include "GLGPU.hpp"

#ifdef OPENGL_BACKEND
#include "GLUtilities.hpp"

GLFWwindow * GLcreateWindow(const std::string & name, Config & config){
	// Initialize glfw, which will create and setup an OpenGL context.
	if (!glfwInit()) {
		Log::Error() << Log::OpenGL << "Could not start GLFW3" << std::endl;
		return nullptr;
	}
	
	GLFWwindow* window;
	
	glfwWindowHint (GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint (GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint (GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint (GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	
	if(config.fullscreen){
		const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
		glfwWindowHint(GLFW_RED_BITS, mode->redBits);
		glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
		glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
		glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
		window = glfwCreateWindow(mode->width, mode->height, name.c_str(), glfwGetPrimaryMonitor(), NULL);
	} else {
		// Create a window with a given size. Width and height are defined in the configuration.
		window = glfwCreateWindow(config.initialWidth, config.initialHeight, name.c_str(), NULL, NULL);
	}
	
	if (!window) {
		Log::Error() << Log::OpenGL << "Could not open window with GLFW3" << std::endl;
		glfwTerminate();
		return nullptr;
	}
	

	// Bind the OpenGL context and the new window.
	glfwMakeContextCurrent(window);
	
	if (gl3wInit()) {
		Log::Error() << Log::OpenGL << "Failed to initialize OpenGL" << std::endl;
		return nullptr;
	}
	if (!gl3wIsSupported(3, 2)) {
		Log::Error() << Log::OpenGL << "OpenGL 3.2 not supported\n" << std::endl;
		return nullptr;
	}
	
	// Default OpenGL state, just in case.
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_BLEND);
	
	return window;
}

#endif
