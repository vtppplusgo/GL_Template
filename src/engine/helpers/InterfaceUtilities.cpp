#include "InterfaceUtilities.hpp"
#include "../Common.hpp"
#include "../input/InputCallbacks.hpp"
#include "../input/Input.hpp"
#include "../graphics/GPU.hpp"

#include <nfd/nfd.h>


namespace Interface {
	

	void setupImGui(GLFWwindow * window){
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
#if defined(OPENGL_BACKEND)
		ImGui_ImplGlfw_InitForOpenGL(window, false);
		ImGui_ImplOpenGL3_Init("#version 150");
#elif defined(VULKAN_BACKEND)
		ImGui_ImplGlfw_InitForVulkan(window, false);
		
#endif
		ImGui::StyleColorsDark();
	}
		
	void beginFrame(){
#if defined(OPENGL_BACKEND)
		ImGui_ImplOpenGL3_NewFrame();
#elif defined(VULKAN_BACKEND)
#endif
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}
	
	void endFrame(){
		ImGui::Render();
#if defined(OPENGL_BACKEND)
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#elif defined(VULKAN_BACKEND)
#endif
		
	}
	
	void clean(){
#if defined(OPENGL_BACKEND)
		ImGui_ImplOpenGL3_Shutdown();
#elif defined(VULKAN_BACKEND)
#endif
		
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}
	
	GLFWwindow* initWindow(const std::string & name, Config & config){
		
		GLFWwindow * window = GPU::device().createWindow(name, config);
		
		// Setup callbacks for various interactions and inputs.
		glfwSetFramebufferSizeCallback(window, resize_callback);	// Resizing the window
		glfwSetKeyCallback(window, key_callback);					// Pressing a key
		glfwSetCharCallback(window, char_callback);					// Outputing a text char (for ImGui)
		glfwSetMouseButtonCallback(window, mouse_button_callback);	// Clicking the mouse buttons
		glfwSetCursorPosCallback(window, cursor_pos_callback);		// Moving the cursor
		glfwSetScrollCallback(window, scroll_callback);				// Scrolling
		glfwSetJoystickCallback(joystick_callback);					// Joystick
		glfwSetWindowIconifyCallback(window, iconify_callback); 	// Window minimization
		glfwSwapInterval(config.vsync ? 1 : 0);						// 60 FPS V-sync
		
		/// \todo Rethink the way we can enable/disable ImGui?
		//setupImGui(window);
		
		// Check the window size (if we are on a screen smaller than the initial size).
		int wwidth, wheight;
		glfwGetWindowSize(window, &wwidth, &wheight);
		config.initialWidth = wwidth;
		config.initialHeight = wheight;
		
		// On HiDPI screens, we have to consider the internal resolution for all framebuffers size.
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		config.screenResolution = glm::vec2(width, height);
		// Compute point density by computing the ratio.
		config.screenDensity = (float)width/(float)config.initialWidth;
		Input::manager().densityEvent(config.screenDensity);
		// Update the resolution.
		Input::manager().resizeEvent(width, height);
		
		return window;
	}
	
	
	bool showPicker(const PickerMode mode, const std::string & startPath, std::string & outPath, const std::string & extensions){
		nfdchar_t *outPathRaw = NULL;
		nfdresult_t result = NFD_CANCEL;
		outPath = "";
		
#ifdef _WIN32
		const std::string internalStartPath = "";
#else
		const std::string internalStartPath = startPath;
#endif
		if(mode == Load){
			result = NFD_OpenDialog(extensions.empty() ? NULL : extensions.c_str(), internalStartPath.c_str(), &outPathRaw);
		} else if(mode == Save){
			result = NFD_SaveDialog(extensions.empty() ? NULL : extensions.c_str(), internalStartPath.c_str(), &outPathRaw);
		} else if(mode == Directory){
			result = NFD_PickFolder(internalStartPath.c_str(), &outPathRaw);
		}
		
		if (result == NFD_OKAY) {
			outPath = std::string(outPathRaw);
			free(outPathRaw);
			return true;
		} else if (result == NFD_CANCEL) {
			// Cancelled by user, nothing to do.
		} else {
			// Real error.
			Log::Error() << "Unable to present system picker (" <<  std::string(NFD_GetError()) << ")." << std::endl;
		}
		free(outPathRaw);
		return false;
	}
}








