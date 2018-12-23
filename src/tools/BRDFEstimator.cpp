#include "Common.hpp"
#include "helpers/GenerationUtilities.hpp"
#include "input/Input.hpp"
#include "renderers/utils/Renderer2D.hpp"
#include "renderers/utils/RendererCube.hpp"
#include "scenes/Scenes.hpp"


/**
 \defgroup BRDFEstimator BRDF Estimation
 \brief Perform cubemap GGX convolution, precompute BRDF lookup table.
 \see GLSL::Frag::Cubemap_convo
 \see GLSL::Frag::Brdf_sampler
 \ingroup Tools
 */

/** \brief Configuration for the BRDF preprocess tool.
 \ingroup BRDFEstimator
 */
class BRDFEstimatorConfig : public RenderingConfig {
public:
	
	/** Initialize a new config object, parsing the input arguments and filling the attributes with their values.
	 \param argc the number of input arguments.
	 \param argv a pointer to the raw input arguments.
	 \note The initial width and height are set to 512px.
	 */
	BRDFEstimatorConfig(int argc, char** argv) : RenderingConfig(argc, argv) {
		processArguments();
		initialWidth = 512;
		initialHeight = 512;
	}
	
	/**
	 Read the internal (key, [values]) populated dictionary, and transfer their values to the configuration attributes.
	 */
	void processArguments(){
		
		for(const auto & arg : _rawArguments){
			const std::string key = arg.first;
			const std::vector<std::string> & values = arg.second;
			
			if(key == "cubemap-name"){
				cubemapName = values[0];
			} else if(key == "output-path"){
				outputPath = values[0];
			} else if(key == "brdf"){
				precomputeBRDF = true;
			}
		}
		
	}
	
public:
	
	std::string cubemapName = ""; ///< Base name of the cubemap to process.
	
	std::string outputPath = ""; ///< Result output path.
	
	bool precomputeBRDF = false; ///< Toggles the computation of the BRDF lookup table.

};

/**
 Compute either a series of cubemaps convolved with a BRDF using increasing roughness values, or generate a linearized BRDF lookup table.
 \param argc the number of input arguments.
 \param argv a pointer to the raw input arguments.
 \return a general error code.
 \ingroup BRDFEstimator
 */
int main(int argc, char** argv) {
	
	// First, init/parse/load configuration.
	BRDFEstimatorConfig config(argc, argv);
	
	// Coherent config state check.
	if(!config.precomputeBRDF && config.cubemapName.empty()){
		Log::Error() << Log::Utilities << "Need a cubemap resource name." << std::endl;
		return 2;
	}
	if(config.outputPath.empty()){
		Log::Error() << Log::Utilities << "Need a destination path." << std::endl;
		return 3;
	}
	
	// Initialize glfw, which will create and setup an OpenGL context.
	if (!glfwInit()) {
		Log::Error() << Log::OpenGL << "Could not start GLFW3" << std::endl;
		return 1;
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_VISIBLE, GL_FALSE); // We hide the window as all processing will be done in offscreen framebuffers.
	
	GLFWwindow* window = glfwCreateWindow((int)config.initialWidth, (int)config.initialHeight,"GL_Template", NULL, NULL);
	
	if (!window) {
		Log::Error() << Log::OpenGL << "Could not open window with GLFW3" << std::endl;
		glfwTerminate();
		return 1;
	}

	// Bind the OpenGL context and the new window.
	glfwMakeContextCurrent(window);

	if (gl3wInit()) {
		Log::Error() << Log::OpenGL << "Failed to initialize OpenGL" << std::endl;
		return -1;
	}
	if (!gl3wIsSupported(3, 2)) {
		Log::Error() << Log::OpenGL << "OpenGL 3.2 not supported\n" << std::endl;
		return -1;
	}
	
	
	// Initialize random generator;
	Random::seed();
	// Query the renderer identifier, and the supported OpenGL version.
	const GLubyte* rendererString = glGetString(GL_RENDERER);
	const GLubyte* versionString = glGetString(GL_VERSION);
	Log::Info() << Log::OpenGL << "Internal renderer: " << rendererString << "." << std::endl;
	Log::Info() << Log::OpenGL << "Version supported: " << versionString << "." << std::endl;
	
	Input::manager().update();
	
	// Two cases:
	// - compute the two coefficients of the BRDF linear approximation.
	// - apply BRDF convolution to an existing envmap.
	const bool precomputeBRDF = config.precomputeBRDF;
	const unsigned int outputWidth = config.initialWidth;
	const unsigned int outputHeight = config.initialHeight;
	
	if(precomputeBRDF){
		std::shared_ptr<Renderer2D> renderer(new Renderer2D(config, "brdf_sampler", outputWidth, outputHeight, GL_RG32F));
		renderer->update();
		renderer->draw();
		renderer->save(config.outputPath);
		renderer->clean();
		
	} else {
		
		const std::string cubemapName = config.cubemapName;
		
		std::shared_ptr<RendererCube> renderer(new RendererCube(config, cubemapName, "cubemap_convo", outputWidth, outputHeight, GL_RGB32F));
		renderer->update();
		
		// Generate convolution map for increments of roughness.
		unsigned int count = 0;
		for(float rr = 0.0f; rr < 1.1f; rr += 0.2f){
			glUseProgram(Resources::manager().getProgram("cubemap_convo")->id());
			glUniform1f(Resources::manager().getProgram("cubemap_convo")->uniform("mimapRoughness"), rr);
			glUseProgram(0);
			
			const unsigned int powe = (unsigned int)std::pow(2, count);
			const unsigned int localWidth = outputWidth/powe;
			const unsigned int localHeight = outputHeight/powe;
			
			renderer->drawCube(localWidth, localHeight, config.outputPath + cubemapName + "-" + std::to_string(rr));
			
			++count;
		}
		renderer->clean();
	}
	
	// Handle quitting.
	glfwSetWindowShouldClose(window, GL_TRUE);
	
	// Remove the window.
	glfwDestroyWindow(window);
	
	// Close GL context and any other GLFW resources.
	glfwTerminate();
	
	Log::Info() << Log::Utilities << "Done." << std::endl;
	
	return 0;
}


