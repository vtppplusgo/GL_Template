#ifndef GPU_h
#define GPU_h

#include "../Common.hpp"
#include "../Config.hpp"

class GPU {
public:
	
	/** Singleton accessor.
	 \return the GPU device.
	 */
	static GPU& device();
	
	GPU();
	
	GLFWwindow * createWindow(const std::string & name, Config & config);
	
	void clean();
	
	bool nextFrame();
	
	bool swap(GLFWwindow * window);
	
private:
	
	/** Destructor (disabled). */
	~GPU(){};
	
	/** Assignment operator (disabled). */
	GPU& operator= (const GPU&) = delete;
	
	/** Copy constructor (disabled). */
	GPU (const GPU&) = delete;
};


#endif
