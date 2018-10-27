#include "Common.hpp"

#include "helpers/GenerationUtilities.hpp"
#include "input/Input.hpp"
#include "input/InputCallbacks.hpp"
#include "input/ControllableCamera.hpp"
#include "helpers/InterfaceUtilities.hpp"
#include "Config.hpp"
#include "graphics/GPU.hpp"

int main(int argc, char** argv) {
	
	// First, init/parse/load configuration.
	Config config(argc, argv);
	if(!config.logPath.empty()){
		Log::setDefaultFile(config.logPath);
	}
	Log::setDefaultVerbose(config.logVerbose);
	
	GLFWwindow* window = Interface::initWindow("VulkanTest", config);
	if(!window){
		return -1;
	}
	
	// Initialize random generator;
	Random::seed();
	
	
	// Setup the timer.
	double timer = glfwGetTime();
	double fullTime = 0.0;
	double remainingTime = 0.0;
	const double dt = 1.0/120.0; // Small physics timestep.
	
	//std::shared_ptr<ProgramInfos> program = Resources::manager().getProgram("object_basic");
	//MeshInfos mesh = Resources::manager().getMesh("light_sphere");
	ControllableCamera camera;
	camera.projection(config.screenResolution[0]/config.screenResolution[1], 1.34f, 0.1f, 100.0f);
	
	// Start the display/interaction loop.
	while (!glfwWindowShouldClose(window)) {
		// Update events (inputs,...).
		Input::manager().update();
		// Handle quitting.
		if(Input::manager().pressed(Input::KeyEscape)){
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		// Start a new frame for the interface.
		//Interface::beginFrame();
		// Reload resources.
		if(Input::manager().triggered(Input::KeyP)){
			//Resources::manager().reload();
		}
		
		// Compute the time elapsed since last frame
		const double currentTime = glfwGetTime();
		double frameTime = currentTime - timer;
		timer = currentTime;
		camera.update();
		
		// Physics simulation
		// First avoid super high frametime by clamping.
		if(frameTime > 0.2){ frameTime = 0.2; }
		// Accumulate new frame time.
		remainingTime += frameTime;
		// Instead of bounding at dt, we lower our requirement (1 order of magnitude).
		while(remainingTime > 0.2*dt){
			double deltaTime = fmin(remainingTime, dt);
			// Update physics and camera.
			camera.physics(deltaTime);
			// Update timers.
			fullTime += deltaTime;
			remainingTime -= deltaTime;
		}
		
		// Render.
		const bool gpuReady = GPU::device().nextFrame();
		if(gpuReady){
			
		}
		const bool gpuOk = GPU::device().swap(window);
		if(!gpuOk){
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
	}
	

	// Clean up instance and surface.
	GPU::device().clean();
	// Clean up GLFW.
	glfwDestroyWindow(window);
	glfwTerminate();
	
	return 0;
}


