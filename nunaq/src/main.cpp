/**
 * Nunaq - Simple Window Launcher
 *
 * Creates a window and initializes Nanook.
 * That's it. Nothing more.
 */

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <nanook/Nanook.hpp>
#include <iostream>

int main(int argc, char* argv[]) {
	// Create window
	if (!glfwInit()) {
		std::cerr << "Failed to initialize GLFW\n";
		return 1;
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

	GLFWwindow* window = glfwCreateWindow(1920, 1080, "Nunaq - Qimmiit Engine", nullptr, nullptr);
	if (!window) {
		std::cerr << "Failed to create window\n";
		glfwTerminate();
		return 1;
	}

	std::cout << "Nunaq: Window created\n";

	// Initialize Nanook
	nanook::Nanook* nanook = new nanook::Nanook();
	if (!nanook->initialize(window, argc, argv)) {
		std::cerr << "Nunaq: Failed to initialize Nanook\n";
		delete nanook;
		glfwDestroyWindow(window);
		glfwTerminate();
		return 1;
	}

	// Run Nanook (handles main loop)
	nanook->run();

	// Cleanup
	std::cout << "Nunaq: Shutting down\n";
	delete nanook;
	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}
