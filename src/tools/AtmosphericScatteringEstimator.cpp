#include "Common.hpp"
#include "Config.hpp"
#include "resources/ImageUtilities.hpp"

/**
 	\defgroup AtmosphericScattering Atmospheric Scattering
 	\brief Real-time atmospheric scattering preprocess.
 	\ingroup Tools
*/

/** \brief Configuration for the atmospheric scattering precomputations.
 	\ingroup AtmosphericScattering
 */
class AtmosphericScatteringConfig : public Config {
public:
	
	/** Initialize a new config object, parsing the input arguments and filling the attributes with their values.
	 \param argc the number of input arguments.
	 \param argv a pointer to the raw input arguments.
	 */
	AtmosphericScatteringConfig(int argc, char** argv) : Config(argc, argv) {
		processArguments();
	}
	
	/**
	 Read the internal (key, [values]) populated dictionary, and transfer their values to the configuration attributes.
	 */
	void processArguments(){
		
		for(const auto & arg : _rawArguments){
			const std::string key = arg.first;
			const std::vector<std::string> & values = arg.second;
			
			if(key == "output-path"){
				outputPath = values[0];
			} else if(key == "samples"){
				samples = std::stoi(values[0]);
			}  else if(key == "resolution"){
				resolution = std::stoi(values[0]);
			}
		}
		
	}
	
public:
	
	std::string outputPath = "./scattering.exr"; ///< Lookup table output path.
	
	unsigned int samples = 256; ///< Number of samples for iterative sampling.

	unsigned int resolution = 512; ///< Output image resolution.
};


/** Perform intersection test with a centered sphere.
 \param rayOrigin the ray origin position with respect to the center of the sphere.
 \param rayDir the ray normalized direction.
 \param radius the radius of the sphere.
 \param roots if the ray intersects the sphere, stores the two roots of the associated polynomial, such that roots[0]<=roots[1].
 \return a boolean denoting if the ray intersected the sphere.
 \warning The intersection can be behind the viewer (ie in the opposite orientation along the ray direction).
 \ingroup AtmosphericScattering
 */
bool intersects(const glm::vec3 & rayOrigin, const glm::vec3 & rayDir, float radius,  glm::vec2 & roots){
	float a = glm::dot(rayDir,rayDir);
	float b = glm::dot(rayOrigin, rayDir);
	float c = glm::dot(rayOrigin, rayOrigin) - radius*radius;
	float delta = b*b - a*c;
	// No intersection if the polynome has no real roots.
	if(delta < 0.0){
		return false;
	}
	// If it intersects, return the two roots.
	float dsqrt = sqrt(delta);
	roots = (-b+glm::vec2(-dsqrt,dsqrt))/a;
	return true;
}


/**
 Compute a scattering lookup table for real-time atmosphere rendering and saves it on disk.
 \param argc the number of input arguments.
 \param argv a pointer to the raw input arguments.
 \return a general error code.
 \ingroup AtmosphericScattering
 */
int main(int argc, char** argv) {
	
	// First, init/parse/load configuration.
	AtmosphericScatteringConfig config(argc, argv);
	
	if(config.outputPath.empty()){
		Log::Error() << Log::Utilities << "Need an output path." << std::endl;
		return 3;
	}
	
	Log::Info() << Log::Utilities << "Generating scattering lookup table." << std::endl;
	
	// Parameters.
	const float groundRadius = 6371e3;
	const float topRadius = 6471e3;
	const glm::vec3 kRayleigh = glm::vec3(5.5e-6, 13.0e-6, 22.4e-6);
	const float heightRayleigh = 8000.0;
	const float heightMie = 1200.0;
	const float kMie = 21e-6;
	
	std::vector<glm::vec3> transmittanceTable(config.resolution * config.resolution);
	const unsigned int samplesCount = config.samples;
	
	for(size_t y = 0; y < config.resolution; ++y){
		for(size_t x = 0; x < config.resolution; ++x){
			// Move to 0,1.
			// No need to take care of the 0.5 shift as we are working with indices
			const float xf = float(x) / (config.resolution - 1.0);
			const float yf = float(y) / (config.resolution - 1.0);
			// Position and ray direction.
			// x becomes the height
			// y become the cosine
			const glm::vec3 currPos = glm::vec3(0.0f, (topRadius - groundRadius) * xf + groundRadius, 0.0f);
			const float cosA = 2.0f * yf - 1.0f;
			const float sinA = sqrt(1.0 - cosA*cosA);
			const glm::vec3 sunDir = -glm::normalize(glm::vec3(sinA, cosA, 0.0f));
			// Check when the ray leaves the atmosphere.
			glm::vec2 interSecondTop;
			const bool didHitSecondTop = intersects(currPos, sunDir, topRadius, interSecondTop);
			// Divide the distance traveled through the atmosphere in samplesCount parts.
			const float secondStepSize = didHitSecondTop ? interSecondTop.y/samplesCount : 0.0f;
				
			// Accumulate optical distance for both scatterings.
			float rayleighSecondDist = 0.0;
			float mieSecondDist = 0.0;
			
			// March along the secondary ray.
			for(int j = 0; j < samplesCount; ++j){
				// Compute the current position along the ray, ...
				const glm::vec3 currSecondPos = currPos + (j+0.5f) * secondStepSize * sunDir;
				// ...and its distance to the ground (as we are in planet space).
				const float currSecondHeight = glm::length(currSecondPos) - groundRadius;
				// Compute density based on the characteristic height of Rayleigh and Mie.
				const float rayleighSecondStep = exp(-currSecondHeight/heightRayleigh) * secondStepSize;
				const float mieSecondStep = exp(-currSecondHeight/heightMie) * secondStepSize;
				// Accumulate optical distances.
				rayleighSecondDist += rayleighSecondStep;
				mieSecondDist += mieSecondStep;
			}
			
			// Compute associated attenuation.
			const glm::vec3 secondaryAttenuation = exp(-(kMie * (mieSecondDist) + kRayleigh * (rayleighSecondDist)));
			transmittanceTable[config.resolution*y+x] = secondaryAttenuation;
		}
	}
	
	ImageUtilities::saveHDRImage(config.outputPath, config.resolution, config.resolution, 3, reinterpret_cast<float*>(&transmittanceTable[0]), false);
	
	Log::Info() << Log::Utilities << "Done." << std::endl;
	
	return 0;
}


