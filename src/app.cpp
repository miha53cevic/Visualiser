#include "app.h"

#include <cstdio>
#include <glad/glad.h>

Clock::Clock()
{
    m_start = SDL_GetTicks();
}

// Returns elapsed time in seconds
float Clock::restart()
{
    m_end = SDL_GetTicks();
    float elapsed = (m_end - m_start) / 1000.0;
    m_start = m_end;
    return elapsed;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////

App::App(const char * title, int width, int height)
    : m_screenWidth(width)
    , m_screenHeight(height)
    , m_window(nullptr)
    , m_title(title)
    , m_bFPSCounter(true)
{
    init_screen(title);
}

App::~App()
{
    SDL_DestroyWindow(m_window);
    SDL_GL_DeleteContext(m_maincontext);
    SDL_Quit();
}

void App::Run()
{
    // Get keystates
    m_keys = SDL_GetKeyboardState(nullptr);

    bool quit = false;

    // Run USER setup code
    if (!Setup())
        quit = true;

    SDL_Event e;
    while (!quit)
    {
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT || m_keys[SDL_SCANCODE_Q])
                quit = true;
            else if (e.type == SDL_WINDOWEVENT)
            {
                // Resize the opengl viewport when the window size is changed
                if (e.window.event == SDL_WINDOWEVENT_RESIZED)
                {
                    int width, height;
                    SDL_GetWindowSize(m_window, &width, &height);
                    glViewport(0, 0, width, height);
                }
                // Check for window focus
                else if (e.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) m_bFocus = true;
                else if (e.window.event == SDL_WINDOWEVENT_FOCUS_LOST)   m_bFocus = false;
            }

            // Run USER event code
            if (!Event(e))
                quit = true;
        }

        // Clear
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        // Run USER loop code
        float elapsed = m_clock.restart();
        if (!Loop(elapsed))
            quit = true;

        // Display
        SDL_GL_SwapWindow(m_window);

        // FPS Counter Option
        if (m_bFPSCounter)
        {
            std::string newTitle = m_title + " - FPS: " + std::to_string((1.0f / elapsed));
            SDL_SetWindowTitle(m_window, newTitle.c_str());
        }
        else SDL_SetWindowTitle(m_window, m_title.c_str());
    }
}

int App::ScreenWidth()
{
    return m_screenWidth;
}

int App::ScreenHeight()
{
    return m_screenHeight;
}

bool App::GetFocus()
{
    return m_bFocus;
}

Uint8 App::GetKey(SDL_Scancode code)
{
    return m_keys[code];
}

void App::FPSCounter(bool fpscounter)
{
    m_bFPSCounter = fpscounter;
}

void App::WireFrame(bool wireframe)
{
    if (wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else           glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
}

void App::VSync(bool vsync)
{
    if (vsync) SDL_GL_SetSwapInterval(1);
    else       SDL_GL_SetSwapInterval(0);
}

// Culling should be Counter-Clock Wise
// - glFrontFace(GL_CCW) by default but can be set to GL_CW
void App::Culling(bool cull)
{
    if (cull) glEnable(GL_CULL_FACE);
    else      glDisable(GL_CULL_FACE);
}

void App::ShowCursor(bool cursor)
{
    SDL_ShowCursor(cursor);
}

/**
 *  Used as a mask when testing buttons in buttonstate.
 *   - SDL_BUTTON_LEFT    Left mouse button
 *   - SDL_BUTTON_MIDDLE  Middle mouse button
 *   - SDL_BUTTON_RIGHT   Right mouse button
 */
bool App::MouseHold(int key)
{
    int x, y;
    return SDL_GetMouseState(&x, &y) == SDL_BUTTON(key);
    return false;
}

void App::sdl_die(const char * message)
{
    printf("%s: %s\n", message, SDL_GetError());
    SDL_Quit();
    exit(-1);
}

void App::init_screen(const char * title)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
        sdl_die("Couldn't initialize SDL_VIDEO");

    // Let SDL load the standard OpenGL of the system
    SDL_GL_LoadLibrary(nullptr);

    // Request an OpenGL 3.3 context
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    // Make sure we are using the core profile so we don't use deprecated functions
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    // Request a depth buffer for later
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    // Create the window
    m_window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        m_screenWidth, m_screenHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
    );

    // Create maincontext
    m_maincontext = SDL_GL_CreateContext(m_window);
    if (m_maincontext == nullptr)
        sdl_die("Failed to create OpenGL context");

    // Check OpenGL properties
    printf("[OpenGL loaded]\n");
    gladLoadGLLoader(SDL_GL_GetProcAddress);
    printf("Vendor:   %s\n", glGetString(GL_VENDOR));
    printf("Renderer: %s\n", glGetString(GL_RENDERER));
    printf("Version:  %s\n", glGetString(GL_VERSION));
    printf("\n");

    // Enable Depth testing
    glEnable(GL_DEPTH_TEST);
}

void App::setClearColor(int r, int g, int b, int a)
{
    float _r = r / 255.0f;
    float _g = g / 255.0f;
    float _b = b / 255.0f;
    float _a = a / 255.0f;
    glClearColor(_r, _g, _b, _a);
}