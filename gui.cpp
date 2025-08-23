#include <algorithm>
#include <format>
#include <iostream>
#include <ranges>
#include <string>
#include <glad/glad.h>
#include <glfw/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "tabs.h"
#include "icon.h"

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

    window = glfwCreateWindow(720, 480, windowTitle, nullptr, nullptr);
    if (window == nullptr)
    {
        return true;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    GLFWimage icon;
    icon.width = 32; // Match your export size
    icon.height = 44; // Match your export size
    icon.pixels = static_cast<unsigned char*>(icon_rgba);
    glfwSetWindowIcon(window, 1, &icon);

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

    io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/segoeui.ttf", 24.0f);
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
    static bool set_initial_focus = true; // Renamed for clarity

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->Pos);
        ImGui::SetNextWindowSize(viewport->Size);

        if (set_initial_focus)
        {
            ImGui::SetNextWindowFocus();
            set_initial_focus = false;
        }

        ImGui::Begin(
            "MainWindow",
            nullptr,
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove
        );


        // Display instructions
        ImGui::Separator();
        ImGui::Text("Current windows");
        ImGui::SameLine(0, 5);
        ImGui::Text("Up/Down to select, ESC to close");

        if (!desktops.empty())
        {
            // Clamp selected index to valid range
            if (selectedIndex < 0) selectedIndex = 0;
            if (selectedIndex >= static_cast<int>(desktops.size()))
                selectedIndex = static_cast<int>(desktops.size()) - 1;

            const ImGuiIO& io = ImGui::GetIO();

            if (ImGui::IsKeyPressed(ImGuiKey_Escape))
            {
                glfwSetWindowShouldClose(window, GL_TRUE);
            }

            // Reordering with Alt + up/down
            if (io.KeyAlt && ImGui::IsKeyPressed(ImGuiKey_UpArrow) && selectedIndex > 0)
            {
                std::swap(desktops[selectedIndex], desktops[selectedIndex - 1]);
                selectedIndex--;
            }

            if (io.KeyAlt && ImGui::IsKeyPressed(ImGuiKey_DownArrow) && selectedIndex < static_cast<int>(desktops.
                size()) - 1)
            {
                std::swap(desktops[selectedIndex], desktops[selectedIndex + 1]);
                selectedIndex++;
            }

            // Navigation with up/down arrows
            if (!io.KeyAlt && ImGui::IsKeyPressed(ImGuiKey_DownArrow))
            {
                selectedIndex = (selectedIndex + 1) % static_cast<int>(desktops.size());
            }

            if (!io.KeyAlt && ImGui::IsKeyPressed(ImGuiKey_UpArrow))
            {
                selectedIndex = (selectedIndex - 1 + static_cast<int>(desktops.size())) % static_cast<int>(desktops.
                    size());
            }
        }

        if (focusListBox)
        {
            ImGui::SetNextItemOpen(true);
            focusListBox = false; // Only set focus once
        }

        if (ImGui::BeginListBox("##desktops", ImVec2(-1, -1)))
        {
            for (int i = 0; i < static_cast<int>(desktops.size()); i++)
            {
                const bool isSelected = selectedIndex == i;

                std::string label;
                if (i < 4)
                {
                    label = std::format("CTRL {} = {}", i + 1, desktops[i].title);
                }
                else
                {
                    label = std::format("{}", desktops[i].title);
                }

                // Custom selectable with right-aligned help text
                if (isSelected)
                {
                    // Get available width
                    float availableWidth = ImGui::GetContentRegionAvail().x;

                    // Measure text widths
                    const auto helpText = "Alt + Up/Down to reorder";
                    const float helpWidth = ImGui::CalcTextSize(helpText).x;
                    const float titleWidth = ImGui::CalcTextSize(label.c_str()).x;

                    // Calculate spacing needed to right-align help text
                    const float spacing = availableWidth - titleWidth - helpWidth - ImGui::GetStyle().ItemSpacing.x;

                    // Create the selectable item
                    if (ImGui::Selectable("##hidden", isSelected))
                    {
                        selectedIndex = i;
                    }

                    // Draw the title and help text on the same line as the selectable
                    ImGui::SameLine(ImGui::GetStyle().ItemInnerSpacing.x);
                    ImGui::Text("%s", label.c_str());

                    if (spacing > 0)
                    {
                        ImGui::SameLine(ImGui::GetStyle().ItemInnerSpacing.x + titleWidth + spacing);
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 0.0f, 1.0f)); // Yellow text
                        ImGui::Text("%s", helpText);
                        ImGui::PopStyleColor();
                    }
                }
                else
                {
                    // Regular selectable for non-selected items
                    if (ImGui::Selectable(label.c_str(), isSelected))
                    {
                        selectedIndex = i;
                    }
                }

                // Auto-scroll to keep selected item visible
                if (isSelected && ImGui::IsItemFocused())
                {
                    ImGui::SetScrollHereY(0.5f);
                }
            }

            ImGui::EndListBox();
        }

        ImGui::End();

        render(window, clear_color);
    }

    cleanup(window);
    return desktops;
}
