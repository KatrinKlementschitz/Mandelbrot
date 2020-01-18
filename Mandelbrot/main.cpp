#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "shader_s.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <windows.h>

const char* vertexShaderSource = "#version 330 core\n#extension GL_ARB_gpu_shader_fp64 : enable\n#pragma optionNV(fastmath off)\n#pragma optionNV(fastprecision off)\nlayout (location = 0) in highp vec3 aPos;\n\nout highp vec4 pos;\nout double x;\n\nvoid main()\n{\n\tpos = highp vec4(aPos, 1.0);\n\tx = aPos.x;\n\tgl_Position = pos;\n};\n\0";
const char* fragmentShaderSource = "#version 330 core\n#extension GL_ARB_gpu_shader_fp64 : enable\n#pragma optionNV(fastmath off)\n#pragma optionNV(fastprecision off)\nprecision highp float;\nout highp vec4 FragColor;\nin vec4 pos;\nuniform highp mat4 trans;\n\nvoid main()\n{\t\n\thighp vec4 position = trans * pos;\n    double factor = 1.5;\n\tint MaxIterations = 200;\n \n\tdouble c_re = position.x;\n\tdouble c_im = position.y;\n\t\n\tdouble Z_re = c_re, Z_im = c_im;\n\n\tfor(int n=0; n<MaxIterations; ++n)\n\t{\n\t\tdouble Z_re2 = Z_re*Z_re, Z_im2 = Z_im*Z_im;\n\t\tif(Z_re2 + Z_im2 > 4)\n\t\t{\n\t\t\tdouble t = double(n)/double(MaxIterations);\n\t\t\tt -= (log(log(float(Z_re2 + Z_im2)) / log(4.0)) / log(2.0))/MaxIterations;\n\t\t\tdouble r = 9.47*(1.0-t)*t*t*t;\n\t\t\tdouble g = 15.96*(1.0-t)*(1.0-t)*t*t;\n\t\t\tdouble b = 9.44*(1.0-t)*(1.0-t)*(1.0-t)*t;\n\t\t\tFragColor = highp vec4(r,g,b, 1.0f);\n\t\t\treturn;\n\t\t}\n\t\t\n\t\tZ_im = 2*Z_re*Z_im + c_im;\n\t\tZ_re = Z_re2 - Z_im2 + c_re;\n\t}\n\n\tFragColor = highp vec4(0.f,0.f,0.f, 1.0f);\n}\n\0";


int Width = 800;
int Height = 800;
bool pressed = false;
double oldmouse[2];
double offset[2];
double mouse[2];
glm::mat4 trans = glm::mat4(1.0f);

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    Width = width;
    Height = height;
}


void CursorCallback(GLFWwindow* window, double x, double y)
{
    mouse[0] = x / (Width / 2) - 1;
    mouse[1] = y / (Height / 2) - 1;
    if(pressed){
        offset[0] = oldmouse[0] - mouse[0];
        offset[1] = oldmouse[1] - mouse[1];
        oldmouse[0] = mouse[0];
        oldmouse[1] = mouse[1];
        trans = glm::translate(trans, glm::vec3(offset[0], offset[1] * -1, 0.0f));
    }
}

void MouseWheelCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (yoffset < 0){
        trans = glm::scale(trans, glm::vec3(1.5,1.5,1.5));
    }
    else{
        trans = glm::scale(trans, glm::vec3(1/1.5, 1/1.5, 1/1.5));
    }
}

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS){
        oldmouse[0] = mouse[0];
        oldmouse[1] = mouse[1];
        pressed = true;
    }
    else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE){
        oldmouse[0] = mouse[0];
        oldmouse[1] = mouse[1];
        pressed = false;
    }

}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void HideConsole()
{
    HWND hWnd = GetConsoleWindow();
    ShowWindow(hWnd, SW_HIDE);
}

int main()
{
    HideConsole();
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    


    GLFWwindow* window = glfwCreateWindow(Width, Height, "Mandelbrot", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, CursorCallback);
    glfwSetScrollCallback(window, MouseWheelCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    int nrAttributes;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nrAttributes);
    std::cout << "Maximum number of vertex attributes supported: " << nrAttributes << std::endl;

    Shader ourShader;
    ourShader.ShaderFromCString(vertexShaderSource, fragmentShaderSource);

    float vertices[] = {
        // positions
         1.f,  1.f, 0.0f,
         1.f, -1.f, 0.0f,
        -1.f, -1.f, 0.0f,
        -1.f,  1.f, 0.0f
    };
    unsigned int indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);


    ourShader.use(); 

    

    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        ourShader.setMatrix("trans", trans);
        ourShader.use();

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
	

    glfwTerminate();
    return 0;
}
