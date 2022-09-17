#pragma once
#include "imgui.h"
#include "Platform/GLFW/ImGUIGLFWImpl.h"
#include "Platform/OpenGL/ImGuiOpenGLImpl.h"
#include <stdio.h>
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers
#include "Log.h"
#include<string>
#include<thread>
#include<chrono>

int InitializeImGUI();