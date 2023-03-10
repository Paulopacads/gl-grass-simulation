#define GLFW_INCLUDE_NONE
#include <cstddef>
#include <fstream>
#include <glm/ext/vector_float4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>
#include <glm/matrix.hpp>
#include <glm/vec3.hpp>
#include <iostream>
#include <iterator>
#include <ostream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GLFW/glfw3.h>
#include <vector>

#include "src/utils.hh"
#include "src/program.hh"
#include "src/obj.hh"
#include "src/camera.hh"
#include "src/mouse.hh"
#include "src/input.hh"
#include "src/shaders_init.hh"
#include "src/grass.hh"

#define HEIGHT 900
#define WIDTH 1400

std::vector<program *> programs;
grass *grass_main;
grass *grass_mini;
obj* sphere_object; 

void framebuffer_size_callback(__attribute__((unused)) GLFWwindow *window,
                               int width, int height)
{
    glViewport(0, 0, width, height);
}

GLFWwindow *init_glfw()
{
    glfwInit();
    check_gl_error(__LINE__, __FILE__);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    check_gl_error(__LINE__, __FILE__);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    check_gl_error(__LINE__, __FILE__);
    //glewExperimental = true;
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    check_gl_error(__LINE__, __FILE__);

    GLFWwindow *window =
        glfwCreateWindow(WIDTH, HEIGHT, "Grass simulation project !", NULL, NULL);
        check_gl_error(__LINE__, __FILE__);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        std::exit(1);
    }
    std::cout << "GLFW version: " << glfwGetVersionString() << std::endl;

    glfwMakeContextCurrent(window);
    check_gl_error(__LINE__, __FILE__);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    //gladLoadGL(glfwGetProcAddress);
    check_gl_error(__LINE__, __FILE__);
    return window;
}

void init_glew()
{
    int val = glewInit();
    if (val != GLEW_OK)
    {
        std::cerr << "ERROR Failed init glew: " << val << std::endl;
        exit(1);
    }
}

void init_GL()
{
    glEnable(GL_DEPTH_TEST);
    check_gl_error(__LINE__, __FILE__);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    check_gl_error(__LINE__, __FILE__);

    // glEnable(GL_CULL_FACE);
    glClearColor(135.0 / 255.0, 206.0 / 255.0, 235.0 / 255.0, 1.0);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);

    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
}

void init_shaders()
{
    // Prog bunny
    const std::string bunny_shaders[] = {"shaders/vertex_simple.shd", "shaders/fragment.shd"};
    GLenum bunny_types[] = {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER};
    program *bunny_prog = program::make_program(bunny_shaders, bunny_types, 2);
    bunny_prog->add_object(setup_bunny(bunny_prog->get_program_id()));
    programs.push_back(bunny_prog);

    // Prog grass compute
    const std::string compute_grass_shaders[] = {"shaders/grass/compute_grass.shd"};
    GLenum compute_grass_types[] = {GL_COMPUTE_SHADER};
    program *compute_grass_prog = program::make_program(compute_grass_shaders, compute_grass_types, 1);
    programs.push_back(compute_grass_prog);

    // Prog grass
    const std::string grass_shaders[] = {"shaders/grass/vertex_grass.shd", "shaders/grass/fragment_grass.shd", 
    "shaders/grass/tess_eval_grass.shd", "shaders/grass/tess_control_grass.shd", "shaders/grass/geometry_grass.shd"};
    GLenum grass_types[] = {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER, GL_TESS_EVALUATION_SHADER, GL_TESS_CONTROL_SHADER, GL_GEOMETRY_SHADER};
    program *grass_prog = program::make_program(grass_shaders, grass_types, 5);
    grass_main = new grass(glm::vec3(0.5, 0.0, 0.0), glm::vec3(3.5, 0.0, 3.0), 80, 80, grass_prog, 0.1, 0.4);
    programs.push_back(grass_prog);
    
    // Prog floor
    const std::string floor_shaders[] = {"shaders/vertex_simple.shd", "shaders/fragment.shd"};
    GLenum floor_types[] = {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER};
    program *floor_prog = program::make_program(floor_shaders, floor_types, 2);
    floor_prog->add_object(setup_floor(floor_prog->get_program_id()));
    programs.push_back(floor_prog);

    //Prog sphere
    const std::string sphere_shaders[] = {"shaders/vertex_sphere.shd", "shaders/fragment.shd"};
    GLenum sphere_types[] = {GL_VERTEX_SHADER, GL_FRAGMENT_SHADER};
    program *sphere_prog = program::make_program(sphere_shaders, sphere_types, 2);
    sphere_object = setup_sphere(sphere_prog->get_program_id());
    sphere_prog->add_object(sphere_object);
    programs.push_back(sphere_prog);

    // Prog mini_grass
    /*program *mini_grass_prog = program::make_program(grass_shaders, grass_types, 5);
    grass_mini = new grass(glm::vec3(-0.5, 0.0, 1.0), glm::vec3(-1.0, 0.0, 1.5), 8, 8, mini_grass_prog, 0.1, 0.4);
    programs.push_back(mini_grass_prog);
    
    // Mini grass compute
    program *mini_compute_grass_prog = program::make_program(compute_grass_shaders, compute_grass_types, 1);
    programs.push_back(mini_compute_grass_prog);*/

}

int main()
{
    GLFWwindow *window = init_glfw();
    init_glew();
    init_GL();
    check_gl_error(__LINE__, __FILE__);
    init_shaders();
    if (programs.size() == 0)
    {
        check_gl_error(__LINE__, __FILE__);
        std::cerr << "Programs not initialized !" << std::endl;
        std::exit(1);
    }

    Camera *camera = new Camera();

    Mouse::init_mouse(camera);
    glfwSetCursorPosCallback(window, Mouse::mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    while (!glfwWindowShouldClose(window))
    {
        Time::update_time_passed();
        
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
        // Bunny
        programs[0]->use();
        init_simple_shaders(programs[0], camera, glm::vec3(1.0, 1.0, 1.0), 0);

        // Floor
        programs[3]->use();
        init_simple_shaders(programs[3], camera, glm::vec3(155.0 / 255.0, 118.0 / 255.0, 83.0 / 255.0), 1);

        // Sphere
        programs[4]->use();
        sphere_object->scale = 0.3f;
        init_sphere_shaders(programs[4], camera, glm::vec3(0.8, 0.3, 0.3));

        // Compute shader
        programs[1]->use();
        grass_main->init_compute_shader(programs[1], sphere_object);

        //programs[6]->use();
        //grass_mini->init_compute_shader(programs[6], sphere_object);

        // Grass
        programs[2]->use();
        grass_main->init_shader(camera);

        //programs[5]->use();
        //grass_mini->init_shader(camera);

        glfwSwapBuffers(window);
        glfwPollEvents();

        process_input(window, camera, sphere_object);
    }
    glfwTerminate();
    delete grass_main;

    return 0;
}