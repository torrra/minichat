# ~ Root CMakeLists.txt

cmake_minimum_required(VERSION 3.24 FATAL_ERROR) # PATH_EQUAL

# VS solution name
get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)

# Do not attempt to compile if C++ 20 is not available
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project(${PROJECT_NAME})

# Create first target (library)
set(TARGET_NAME Network)
add_subdirectory(NetworkLibrary)

# Add Network to link libraries for the next targets
set(ADDITIONAL_INCLUDE_DIRECTORIES ${ADDITIONAL_INCLUDE_DIRECTORIES} ${${TARGET_NAME}_INCLUDE_DIRECTORY})
set(ADDITIONAL_LINK_LIBRARIES ${ADDITIONAL_LINK_LIBRARIES} ${TARGET_NAME})

# Configure client-side app
unset(TARGET_NAME)
add_subdirectory(MiniChatClient)

# Configure server-side app
unset(TARGET_NAME)
add_subdirectory(MiniChatServer)

# Use server app as Visual Studio startup project instead of ALL_BUILD
if (MSVC)
	set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${TARGET_NAME})
endif()