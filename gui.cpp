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

    io.Fonts->AddFontFromFileTTF("C:/Windows/Fonts/verdana.ttf", 20.0f, nullptr, io.Fonts->GetGlyphRangesDefault());
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
    glClearColor(
        clear_color.x * clear_color.w,
        clear_color.y * clear_color.w,
        clear_color.z * clear_color.w,
        clear_color.w
    );
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

    // Enhanced color scheme - Grey primary, Red secondary
    constexpr auto clear_color = ImVec4(0.15f, 0.15f, 0.18f, 1.00f); // Dark grey background

    // Set up custom ImGui style
    ImGuiStyle& style = ImGui::GetStyle();

    // Primary colors (Grey tones)
    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.12f, 0.12f, 0.15f, 0.95f); // Dark grey window background
    style.Colors[ImGuiCol_ChildBg] = ImVec4(0.10f, 0.10f, 0.12f, 0.90f); // Darker grey for child windows
    style.Colors[ImGuiCol_PopupBg] = ImVec4(0.12f, 0.12f, 0.15f, 0.98f); // Popup background
    style.Colors[ImGuiCol_Border] = ImVec4(0.25f, 0.25f, 0.28f, 0.80f); // Light grey borders
    style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f); // No shadow
    style.Colors[ImGuiCol_FrameBg] = ImVec4(0.18f, 0.18f, 0.22f, 0.85f); // Frame background
    style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.22f, 0.22f, 0.26f, 0.90f); // Frame hover
    style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.26f, 0.26f, 0.30f, 0.95f); // Frame active

    // Secondary colors (Red accents)
    style.Colors[ImGuiCol_Header] = ImVec4(0.65f, 0.20f, 0.20f, 0.70f); // Red header
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.75f, 0.25f, 0.25f, 0.80f); // Red header hover
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.85f, 0.30f, 0.30f, 0.90f); // Red header active
    style.Colors[ImGuiCol_Button] = ImVec4(0.55f, 0.18f, 0.18f, 0.65f); // Red button
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.65f, 0.22f, 0.22f, 0.75f); // Red button hover
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.75f, 0.26f, 0.26f, 0.85f); // Red button active

    // Selection colors (Red theme)
    style.Colors[ImGuiCol_CheckMark] = ImVec4(0.85f, 0.30f, 0.30f, 1.00f); // Red checkmark
    style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.75f, 0.25f, 0.25f, 0.85f); // Red slider
    style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.85f, 0.30f, 0.30f, 0.95f); // Red slider active

    // Text colors
    style.Colors[ImGuiCol_Text] = ImVec4(0.92f, 0.92f, 0.94f, 1.00f); // Light grey text
    style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.52f, 1.00f); // Disabled text
    style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.65f, 0.20f, 0.20f, 0.35f); // Red text selection

    // Separator and resize grip
    style.Colors[ImGuiCol_Separator] = ImVec4(0.35f, 0.35f, 0.38f, 0.60f); // Grey separator
    style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.75f, 0.25f, 0.25f, 0.80f); // Red separator hover
    style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.85f, 0.30f, 0.30f, 1.00f); // Red separator active
    style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.65f, 0.20f, 0.20f, 0.25f); // Red resize grip
    style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.75f, 0.25f, 0.25f, 0.67f); // Red resize grip hover
    style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.85f, 0.30f, 0.30f, 0.95f); // Red resize grip active

    // Enhanced styling parameters
    style.WindowRounding = 6.0f; // Rounded corners
    style.FrameRounding = 4.0f; // Rounded frames
    style.PopupRounding = 4.0f; // Rounded popups
    style.ScrollbarRounding = 4.0f; // Rounded scrollbars
    style.GrabRounding = 4.0f; // Rounded grab handles
    style.TabRounding = 4.0f; // Rounded tabs
    style.WindowBorderSize = 1.0f; // Thin borders
    style.FrameBorderSize = 1.0f; // Frame borders
    style.PopupBorderSize = 1.0f; // Popup borders
    style.WindowPadding = ImVec2(12.0f, 12.0f); // More padding
    style.FramePadding = ImVec2(8.0f, 4.0f); // Frame padding
    style.ItemSpacing = ImVec2(8.0f, 6.0f); // Item spacing
    style.ItemInnerSpacing = ImVec2(6.0f, 4.0f); // Inner spacing

    // Remove windows with matching title
    for (auto const& [index, value] : std::views::enumerate(desktops))
    {
        if (value.title == windowTitle)
        {
            desktops.erase(desktops.begin() + index);
        }
    }

    static int selectedIndex = 0;
    static bool focusListBox = true;
    static bool set_initial_focus = true;

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

        // Styled separator
        ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.65f, 0.20f, 0.20f, 0.60f));
        ImGui::Separator();
        ImGui::PopStyleColor();

        ImGui::Spacing();

        // Enhanced instruction panel with better formatting
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.08f, 0.08f, 0.10f, 0.85f));
        ImGui::BeginChild("Instructions", ImVec2(0, 80), true, ImGuiWindowFlags_NoScrollbar);

        const auto appname = "Find My Windows";
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.85f, 0.30f, 0.30f, 1.00f)); // Red title
        ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize(appname).x) * 0.5f);
        ImGui::Text(appname);
        ImGui::PopStyleColor();

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.75f, 0.75f, 0.77f, 1.00f));
        const auto instructions = {"TAB to Close", "^/v Navigate", "ALT+^/v Reorder"};

        auto offset = 0;
        for (const auto ins : instructions)
        {
            ImGui::Text(ins);
            offset += 150;
            ImGui::SameLine(offset);
        }
        ImGui::PopStyleColor();

        ImGui::EndChild();
        ImGui::PopStyleColor();

        ImGui::Spacing();

        constexpr auto max_shortcuts = 9;

        if (!desktops.empty())
        {
            // Clamp selected index to valid range
            if (selectedIndex < 0) selectedIndex = 0;
            if (selectedIndex >= static_cast<int>(desktops.size()))
                selectedIndex = static_cast<int>(desktops.size()) - 1;

            const ImGuiIO& io = ImGui::GetIO();

            if (ImGui::IsKeyPressed(ImGuiKey_Tab))
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
            focusListBox = false;
        }

        // Enhanced list box with custom styling
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 8.0f));
        if (ImGui::BeginListBox("##desktops", ImVec2(-1, -1)))
        {
            for (int i = 0; i < static_cast<int>(desktops.size()); i++)
            {
                const bool isSelected = selectedIndex == i;

                std::string label;
                std::string shortcut;
                if (i < max_shortcuts)
                {
                    shortcut = std::format("CTRL {}", i + 1);
                    label = desktops[i].title;
                }
                else
                {
                    label = desktops[i].title;
                }

                std::string fullLabel = shortcut.empty() ? label : std::format("[{}] {}", shortcut, label);
                if (isSelected)
                {
                    if (ImGui::Selectable("##hidden", isSelected))
                    {
                        selectedIndex = i;
                    }

                    ImGui::SameLine(ImGui::GetStyle().ItemInnerSpacing.x);
                    ImGui::Text("%s", fullLabel.c_str());
                }
                else
                {
                    // Regular selectable for non-selected items
                    if (ImGui::Selectable(fullLabel.c_str(), isSelected))
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
        ImGui::PopStyleVar();

        ImGui::End();

        render(window, clear_color);
    }

    cleanup(window);
    return desktops;
}
