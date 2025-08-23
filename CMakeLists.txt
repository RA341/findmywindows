cmake_minimum_required(VERSION 3.14) # Lowered for broader compatibility, 4.0 is not a real version.
project(findmywindows)

set(CMAKE_CXX_STANDARD 23) # Changed to 23 as C++26 is not fully supported by compilers yet.

add_executable(findmywindows main.cpp
        gui.cpp
        gui.h
        tabs.cpp
        tabs.h
        file.cpp
        file.h
)

find_package(imgui CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)

target_link_libraries(findmywindows PRIVATE
        glfw
        glad::glad
        imgui::imgui
)

#-------------------------------------------------------------------
# 1. INSTALLATION RULES
#-------------------------------------------------------------------

set_target_properties(findmywindows PROPERTIES
        WIN32_EXECUTABLE $<CONFIG:Release>
)

# This command specifies that the final executable 'findmywindows.exe'
# should be installed into a 'bin' directory inside the final installation folder.
install(TARGETS findmywindows
        RUNTIME DESTINATION bin
)

# GLFW3 has a DLL that needs to be included with your application.
# This finds the DLL and installs it right next to your executable.
install(FILES ${GLFW_DLL_PATH} # Assumes glfw3 package exposes this, may need adjustment
        DESTINATION bin
        COMPONENT Runtime
)

# It's good practice to include the license files of your dependencies.
# Create a 'licenses' directory in your source tree and place them there.
#install(DIRECTORY licenses/ DESTINATION licenses)


#-------------------------------------------------------------------
# 2. CPACK CONFIGURATION
#-------------------------------------------------------------------

# A unique name for your application installer
set(CPACK_PACKAGE_NAME "FindMyWindows")

# The name of the organization creating the application
set(CPACK_PACKAGE_VENDOR "Your Company Name")

# A short description of your application
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A utility to find and manage windows.")

# Version number for your application
set(CPACK_PACKAGE_VERSION "1.0.0")

# Contact information
#set(CPACK_PACKAGE_CONTACT "support@example.com")

# The license file to be displayed during installation
#set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")

# An optional text file for a "ReadMe" step in the installer
# set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.txt")

# Set the installer generator to NSIS
set(CPACK_GENERATOR "NSIS")

# The final name of the installer executable
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-win64")

# An icon for the installer itself (.ico file)
# set(CPACK_NSIS_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/path/to/installer_icon.ico")

# An icon for the uninstaller (.ico file)
# set(CPACK_NSIS_MUI_UNIICON "${CMAKE_CURRENT_SOURCE_DIR}/path/to/uninstaller_icon.ico")

# Ask the user to create a desktop shortcut
#set(CPACK_NSIS_CREATE_ICONS_EXTRA "CreateShortCut '$DESKTOP\\${CPACK_PACKAGE_NAME}.lnk' '$INSTDIR\\bin\\findmywindows.exe'")

# Add a Start Menu shortcut
#set(CPACK_NSIS_MENU_LINKS "$INSTDIR\\bin\\findmywindows.exe" "${CPACK_PACKAGE_NAME}")
set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS
        "WriteRegStr HKCU \\\"Software\\\\Microsoft\\\\Windows\\\\CurrentVersion\\\\Run\\\" \\\"${CPACK_PACKAGE_NAME}\\\" '\"$INSTDIR\\\\bin\\\\findmywindows.exe\"'"
)

# It's critical to clean up the registry on uninstallation.
# This command deletes the registry value we created during installation.
set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS
        "DeleteRegValue HKCU \\\"Software\\\\Microsoft\\\\Windows\\\\CurrentVersion\\\\Run\\\" \\\"${CPACK_PACKAGE_NAME}\\\""
)

# Include the CPack module to process the above settings
include(CPack)