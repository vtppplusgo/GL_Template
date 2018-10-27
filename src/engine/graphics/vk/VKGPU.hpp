#ifndef VKGPU_h
#define VKGPU_h

#include "../../Common.hpp"
#include "../../Config.hpp"

#ifdef VULKAN_BACKEND

GLFWwindow * VKcreateWindow(const std::string & name, Config & config);

bool VKacquireNextFrame();

bool VKswap(bool resizedDetected, unsigned int width, unsigned int height);

void VKclean();

#endif
#endif
