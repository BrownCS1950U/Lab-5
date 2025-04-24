#pragma once


#include "mesh.h"
#include "audio.h"
#include <GLFW/glfw3.h>

#include "Quad.h"

namespace gl {
class Window {
public:

    ~Window();
    static void resize_window(GLFWwindow* window, int width, int height);
    static void keyboard(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void scroll(GLFWwindow * window, double xoffset, double yoffset);
    static void cursor_enter_callback(GLFWwindow* window, int entered);
    static void mouse(GLFWwindow * window, double xpos, double ypos);
    static void drag_drop(GLFWwindow * window, int count, const char** paths);
    static int initialize(const std::string& filename);
    static AudioEngine& audio();
    static void display();
    static void update();
    static bool isActive();

private:
    // Variables to hold state
    static float sense;
    static bool active_cursor;
    static bool cursorInsideWindow;
    static GLuint shaderProgram;
    static GLuint terrainProgram;
    static int render_mode;
    static bool keys[1024];
    static int window_width, window_height;
    static int current_vp_height, current_vp_width;
    static Quad quad;

    static int skyboxIndex; // default to "Clouds"
    static const char* skyboxes[];


    static GLFWwindow* glfwWindow;
    static std::vector<DataTex> m_data;
};
}
