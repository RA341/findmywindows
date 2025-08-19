#include <algorithm>
#include <iostream>
#include <ranges>
#include <string>
#include <glad/glad.h>
#include <glfw/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "tabs.h"

const auto windowTitle = "Find My Windows";

static void glfw_error_callback(const int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

bool setup_window(GLFWwindow*& window)
{
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
    {
        return true;
    }

    const auto glsl_version = "#version 330";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    window = glfwCreateWindow(1280, 720, windowTitle, nullptr, nullptr);
    if (window == nullptr)
    {
        return true;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Initialize GLAD
    if (!gladLoadGL())
    {
        fprintf(stderr, "Failed to initialize GLAD\n");
        return true;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    const ImGuiIO& io = ImGui::GetIO();
    (void)io;

    io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/segoeui.ttf", 32.0f);
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    return false;
}

void cleanup(GLFWwindow* window)
{
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}

void render(GLFWwindow* window, const ImVec4 clear_color)
{
    // Rendering
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w,
                 clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
}

std::vector<WindowInfo> launch_gui(std::vector<WindowInfo> desktops)
{
    GLFWwindow* window;
    if (setup_window(window))
    {
        return {};
    }

    constexpr auto clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Remove windows with matching title
    for (auto const& [index, value] : std::views::enumerate(desktops))
    {
        if (value.title == windowTitle)
        {
            desktops.erase(desktops.begin() + index);
        }
    }

    static int selectedIndex = 0; // Start with first item selected
    static bool focusListBox = true; // Start with focus on list

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Create fullscreen window without decorations
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);

        ImGui::Begin("MainWindow", nullptr,
                     ImGuiWindowFlags_NoDecoration |
                     ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove);

        ImGui::Text("Current windows");

        // Handle keyboard input for navigation and reordering
        if (focusListBox && !desktops.empty())
        {
            // Clamp selected index to valid range
            if (selectedIndex < 0) selectedIndex = 0;
            if (selectedIndex >= static_cast<int>(desktops.size()))
                selectedIndex = static_cast<int>(desktops.size()) - 1;

            const ImGuiIO& io = ImGui::GetIO();

            // Reordering with Alt + up/down
            if (io.KeyAlt && ImGui::IsKeyPressed(ImGuiKey_UpArrow) && selectedIndex > 0)
            {
                // printf("moving on on alt+down only");
                std::swap(desktops[selectedIndex], desktops[selectedIndex - 1]);
                selectedIndex--;
            }

            if (io.KeyAlt && ImGui::IsKeyPressed(ImGuiKey_DownArrow) && selectedIndex < static_cast<int>(desktops.
                size()) - 1)
            {
                // printf("moving on on alt+up only");
                std::swap(desktops[selectedIndex], desktops[selectedIndex + 1]);
                selectedIndex++;
            }

            // Navigation with up/down arrows
            if (!io.KeyAlt && ImGui::IsKeyPressed(ImGuiKey_DownArrow))
            {
                // printf("moving on on down only");
                selectedIndex = (selectedIndex + 1) % static_cast<int>(desktops.size());
            }
            if (!io.KeyAlt && ImGui::IsKeyPressed(ImGuiKey_UpArrow))
            {
                // printf("moving on on up only");
                selectedIndex = (selectedIndex - 1 + static_cast<int>(desktops.size())) % static_cast<int>(desktops.
                    size());
            }
        }

        // Set focus to the listbox
        if (focusListBox)
        {
            ImGui::SetNextItemOpen(true);
            focusListBox = false; // Only set focus once
        }

        if (ImGui::BeginListBox("##desktops", ImVec2(-1, -1)))
        {
            for (int i = 0; i < static_cast<int>(desktops.size()); i++)
            {
                const bool isSelected = (selectedIndex == i);

                // Highlight selected item
                if (isSelected)
                {
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f)); // Yellow text
                    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.4f, 0.8f, 0.8f)); // Blue background
                }

                if (ImGui::Selectable(desktops[i].title.c_str(), isSelected))
                {
                    selectedIndex = i;
                }

                if (isSelected)
                {
                    ImGui::PopStyleColor(2);

                    // Auto-scroll to keep selected item visible
                    if (ImGui::IsItemFocused())
                        ImGui::SetScrollHereY(0.5f);
                }
            }

            // Check if listbox is focused for keyboard input
            focusListBox = ImGui::IsWindowFocused();

            ImGui::EndListBox();
        }

        // Display instructions
        ImGui::Separator();
        ImGui::Text("Controls:");
        ImGui::Text("Up/Down: Navigate selection");
        ImGui::Text("Alt + Up/Down: Reorder items");
        if (selectedIndex >= 0 && selectedIndex < static_cast<int>(desktops.size()))
        {
            ImGui::Text("Selected: %s", desktops[selectedIndex].title.c_str());
        }

        ImGui::End();

        render(window, clear_color);
    }

    cleanup(window);
    return desktops;
}
