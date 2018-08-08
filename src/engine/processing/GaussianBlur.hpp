#ifndef GaussianBlur_h
#define GaussianBlur_h
#include "../Common.hpp"
#include "Blur.hpp"


class GaussianBlur : public Blur {

public:

	/// Init function
	GaussianBlur(unsigned int width, unsigned int height, unsigned int depth, GLuint format, GLuint type, GLuint preciseFormat);

	/// Draw function
	void process(const GLuint textureId);
	
	/// Clean function
	void clean() const;

	/// Handle screen resizing
	void resize(unsigned int width, unsigned int height);
	
private:
	
	ScreenQuad _blurScreen;
	std::shared_ptr<Framebuffer> _finalFramebuffer;
	ScreenQuad _combineScreen;
	std::vector<std::shared_ptr<Framebuffer>> _frameBuffers;
	std::vector<std::shared_ptr<Framebuffer>> _frameBuffersBlur;
	
};

#endif
