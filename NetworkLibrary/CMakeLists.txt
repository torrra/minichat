# ~ NetworkLibrary/CMakeLists.txt

# Set target name variable (library and corresponding VS project name)
if (NOT TARGET_NAME)
	get_filename_component(TARGET_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
	set(TARGET_NAME ${TARGET_NAME} PARENT_SCOPE)
endif()

# Gather project sources (headers, C++ source files and extra documentation)

file(GLOB_RECURSE TARGET_HEADER_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/*.h
	${CMAKE_CURRENT_SOURCE_DIR}/*.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/*.inl # inline files
)

file(GLOB_RECURSE TARGET_SOURCE_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/*.c
	${CMAKE_CURRENT_SOURCE_DIR}/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/*.c++
	${CMAKE_CURRENT_SOURCE_DIR}/*.cxx
)

file (GLOB_RECURSE TARGET_EXTRA_FILES
	${CMAKE_CURRENT_SOURCE_DIR}/*.md
	${CMAKE_CURRENT_SOURCE_DIR}/*.txt
)

file(GLOB TARGET_ROOT_DOCS
	${CMAKE_SOURCE_DIR}/.gitignore
	${CMAKE_SOURCE_DIR}/README.txt
)


# Group files together
set(TARGET_FILES ${TARGET_HEADER_FILES} ${TARGET_SOURCE_FILES} ${TARGET_EXTRA_FILES})

# Generate VS filters matching the file tree
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${TARGET_FILES})
source_group("Extras" FILES ${TARGET_ROOT_DOCS})
# Create library
add_library(${TARGET_NAME})

# Set sources for compilation, linking and VS filters
target_sources(${TARGET_NAME}
	PRIVATE ${TARGET_FILES}
	PRIVATE ${TARGET_ROOT_DOCS}
)
target_include_directories(${TARGET_NAME}
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Private
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Public/Network
)
target_link_libraries(${TARGET_NAME} PRIVATE Ws2_32.lib)

# Include folder variable so projects dependant on this one can access it
set(${TARGET_NAME}_INCLUDE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Public PARENT_SCOPE)

# Set warning level to max and treat warnings as errors
if (MSVC)
	target_compile_options(${TARGET_NAME} PRIVATE /W4 /WX)
else()
	message(STATUS "Not using MSVC. Compile options for ${TARGET_NAME} not set.")
endif()