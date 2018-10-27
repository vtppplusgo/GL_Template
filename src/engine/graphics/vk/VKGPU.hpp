#ifndef VKGPU_h
#define VKGPU_h

#include "../../Common.hpp"
#include "../../Config.hpp"

#ifdef VULKAN_BACKEND

GLFWwindow * VKcreateWindow(const std::string & name, Config & config);

void VKresizeSwapchain(const unsigned int width, const unsigned int height);

void VKclean();

#endif
#endif
