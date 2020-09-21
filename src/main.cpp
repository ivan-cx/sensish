// dear imgui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// If you are new to dear imgui, see examples/README.txt and documentation at the top of imgui.cpp.
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>

// About Desktop OpenGL function loaders:
//  Modern desktop OpenGL doesn't have a standard portable header file to load OpenGL function pointers.
//  Helper libraries are often used for this purpose! Here we are supporting a few common ones (gl3w, glew, glad).
//  You may use another loader/header of your choice (glext, glLoadGen, etc.), or chose to manually implement your own.
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h>            // Initialize with gl3wInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h>            // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h>          // Initialize with gladLoadGL()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
#define GLFW_INCLUDE_NONE       // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#include <glbinding/Binding.h>  // Initialize with glbinding::Binding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
#define GLFW_INCLUDE_NONE       // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#include <glbinding/glbinding.h>// Initialize with glbinding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif

// Include glfw3.h after our OpenGL definitions
#include <GLFW/glfw3.h>

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

#include <stb_image.h>
#include <stb_image_write.h>

#include "open_file.h"
#include "text.h"
#include "texture.h"

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

struct Graphics {
  Texture tex;
  unsigned char *workingTextureData = nullptr;
  unsigned char *sourceTextureData = nullptr;
  int channels = 0;
};

void applyWatermark(Graphics &graphics, const WatermarkText &wmText,
                    float wmColor[4], int wmLetterHeight) {
  deleteTexture(graphics.tex);
  memcpy(graphics.workingTextureData, graphics.sourceTextureData,
         graphics.tex.width * graphics.tex.height * graphics.channels);
  watermark_draw_text(graphics.workingTextureData, graphics.tex.width,
                      graphics.tex.height, graphics.channels, wmText, wmColor,
                      wmLetterHeight);
  // TODO render text
  graphics.tex = createTexture(graphics.workingTextureData, graphics.tex.width,
                               graphics.tex.height, graphics.channels);
}

int main() {
  // Setup window
  glfwSetErrorCallback(glfw_error_callback);
  if (!glfwInit())
    return 1;

  // Decide GL+GLSL versions
  // GL 3.0 + GLSL 130
  const char *glsl_version = "#version 130";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+
  // only glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+
  // only
  GLFWwindow *window;
  {
    // get size of available screen area:
    int x, y, width, height;
    glfwGetMonitorWorkarea(glfwGetPrimaryMonitor(), &x, &y, &width, &height);
    // Create window with graphics context
    window = glfwCreateWindow(width, height, "sensish", NULL, NULL);
  }
  if (window == NULL)
    return 1;
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1); // Enable vsync

  // Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
  bool err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
  bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
  bool err = gladLoadGL() == 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
  bool err = false;
  glbinding::Binding::initialize();
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
  bool err = false;
  glbinding::initialize([](const char *name) {
    return (glbinding::ProcAddress)glfwGetProcAddress(name);
  });
#else
  bool err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader
                    // is likely to requires some form of initialization.
#endif
  if (err) {
    fprintf(stderr, "Failed to initialize OpenGL loader!\n");
    return 1;
  }
  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  io.FontAllowUserScaling = true;

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();

  // Setup Platform/Renderer bindings
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

  // Load Fonts
  // - If no fonts are loaded, dear imgui will use the default font. You can
  // also load multiple fonts and use ImGui::PushFont()/PopFont() to select
  // them.
  // - AddFontFromFileTTF() will return the ImFont* so you can store it if you
  // need to select the font among multiple.
  // - If the file cannot be loaded, the function will return NULL. Please
  // handle those errors in your application (e.g. use an assertion, or display
  // an error and quit).
  // - The fonts will be rasterized at a given size (w/ oversampling) and stored
  // into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which
  // ImGui_ImplXXXX_NewFrame below will call.
  // - Read 'docs/FONTS.txt' for more instructions and details.
  // - Remember that in C/C++ if you want to include a backslash \ in a string
  // literal you need to write a double backslash \\ !
  // io.Fonts->AddFontDefault();
  {
#define FONT "./fonts/Roboto-Medium.ttf"
    static const ImWchar range_all_unicode[]{0x0020, 0xFFFF, 0};
    io.Fonts->AddFontFromFileTTF(FONT, 28.0f, NULL, range_all_unicode);
    watermark_init_font(FONT);
  }
  // ImFont* font =
  // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f,
  // NULL, io.Fonts->GetGlyphRangesJapanese()); IM_ASSERT(font != NULL);

  // Our state
  ImVec4 clearColor = ImVec4(0.45f, 0.45f, 0.50f, 1.00f);
  std::filesystem::path currentDirPath{getDefaultWorkingDirectory()};
  std::filesystem::path filePath{};
  int width, height;
  Graphics gfx;

  WatermarkText wmText;
  float wmColor[4]{0.8, 0.1, 0.1, 0.7};
  int wmLetterHeight = 64;
#define OUTPUT_FILENAME_BUF_SIZE 1024
  char outputFilename[OUTPUT_FILENAME_BUF_SIZE] = {0};

  // Main loop
  while (!glfwWindowShouldClose(window)) {
    // Poll and handle events (inputs, window resize, etc.)
    // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to
    // tell if dear imgui wants to use your inputs.
    // - When io.WantCaptureMouse is true, do not dispatch mouse input data to
    // your main application.
    // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input
    // data to your main application. Generally you may always pass all inputs
    // to dear imgui, and hide them from your application based on those two
    // flags.
    glfwPollEvents();

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Application logic here:
    if (filePath == "")
      filePath = openFile(currentDirPath);

    if (filePath != "") {
      if (gfx.sourceTextureData == nullptr) {
        snprintf(outputFilename, OUTPUT_FILENAME_BUF_SIZE, "sensish_%s.png",
                 filePath.stem().c_str());
        gfx.sourceTextureData =
            stbi_load(filePath.c_str(), &width, &height, &gfx.channels, 0);
        if (gfx.sourceTextureData) {
          gfx.tex =
              createTexture(gfx.sourceTextureData, width, height, gfx.channels);
          gfx.workingTextureData =
              (unsigned char *)malloc(width * height * gfx.channels);
        }
      }
    }
    if (gfx.sourceTextureData) {
      if (ImGui::Begin("Document")) {
        ImGui::Image((void *)(intptr_t)gfx.tex.id, ImVec2(width, height));
      }
      ImGui::End();
      if (ImGui::Begin("Process")) {
        ImGui::InputText("Watermark Text #1", wmText.lines[0], TEXT_LINE_SIZE);
        ImGui::InputText("Watermark Text #2", wmText.lines[1], TEXT_LINE_SIZE);
        ImGui::InputText("Watermark Text #3", wmText.lines[2], TEXT_LINE_SIZE);
        ImGui::ColorEdit4("Watermark color", wmColor);
        ImGui::InputInt("Letter height in pixels", &wmLetterHeight, 1, 5);
        // TODO font size
        if (ImGui::Button("Preview")) {
          applyWatermark(gfx, wmText, wmColor, wmLetterHeight);
        }
        if (ImGui::Button("Save")) {
          applyWatermark(gfx, wmText, wmColor, wmLetterHeight);
          stbi_write_png(outputFilename, width, height, gfx.channels,
                         gfx.workingTextureData, width * gfx.channels);
        }
        ImGui::SameLine();
        ImGui::InputText("Ouput file", outputFilename,
                         OUTPUT_FILENAME_BUF_SIZE);
        ImGui::Separator();
        if (ImGui::Button("Select another file")) {
          filePath = "";
          stbi_image_free(gfx.sourceTextureData);
          gfx.sourceTextureData = nullptr;
        }
      }
      ImGui::End();
    }

    // Rendering
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(clearColor.x, clearColor.y, clearColor.z, clearColor.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
  }

  stbi_image_free(gfx.sourceTextureData);
  free(gfx.workingTextureData);

  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
