#ifndef DeferredRenderer_h
#define DeferredRenderer_h
#include "Renderer.hpp"
#include "Framebuffer.hpp"
#include "input/Camera.hpp"
#include "ScreenQuad.hpp"

#include "Gbuffer.hpp"
#include "Blur.hpp"
#include "AmbientQuad.hpp"

#include <gl3w/gl3w.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <memory>



class DeferredRenderer : public Renderer {

public:

	~DeferredRenderer();

	/// Init function
	DeferredRenderer(Config & config, std::shared_ptr<Scene> & scene);

	/// Draw function
	void draw();
	
	void update();
	
	void physics(double fullTime, double frameTime);

	/// Clean function
	void clean() const;

	/// Handle screen resizing
	void resize(int width, int height);
	
	
private:
	
	Camera _camera;

	std::shared_ptr<Gbuffer> _gbuffer;
	std::shared_ptr<Blur> _blurBuffer;
	std::shared_ptr<Framebuffer> _ssaoFramebuffer;
	std::shared_ptr<Framebuffer> _ssaoBlurFramebuffer;
	std::shared_ptr<Framebuffer> _sceneFramebuffer;
	std::shared_ptr<Framebuffer> _bloomFramebuffer;
	std::shared_ptr<Framebuffer> _toneMappingFramebuffer;
	std::shared_ptr<Framebuffer> _fxaaFramebuffer;
	
	AmbientQuad _ambientScreen;
	ScreenQuad _ssaoBlurScreen;
	ScreenQuad _bloomScreen;
	ScreenQuad _toneMappingScreen;
	ScreenQuad _fxaaScreen;
	ScreenQuad _finalScreen;
	
};

#endif
