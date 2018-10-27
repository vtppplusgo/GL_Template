#ifndef GLGPU_h
#define GLGPU_h

#include "../../Common.hpp"
#include "../../Config.hpp"

#ifdef OPENGL_BACKEND

GLFWwindow * GLcreateWindow(const std::string & name, Config & config);



#endif
#endif
