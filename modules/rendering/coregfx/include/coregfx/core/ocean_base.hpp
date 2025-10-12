/*
 * Vulkan Example base class
 *
 * Copyright (C) 2016 by Sascha Willems - www.saschawillems.de
 *
 * This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */

#pragma once

#ifndef OCEAN_BASE_HPP_
#define OCEAN_BASE_HPP_

#ifndef OCEAN_SWAP_CHAIN_HPP_
#include "ocean_swap_chain.hpp"
#endif

#ifndef OCEAN_KEYCODES_HPP_
#include "ocean_keycodes.hpp"
#endif

#include <memory>

// Forward declarations for utility classes
namespace coregfx {
namespace platform {
    class InputHandler;
    class FramebufferManager;
    class CommandBufferManager;
    class SynchronizationManager;
    class WindowResizeManager;
}
}

namespace oceanbase
{
	class VulkanExampleBase
	{
	private:
		float fpsTimer = 0.0f;
		uint32_t frameCounter = 0;
		uint32_t destWidth;
		uint32_t destHeight;
		bool resizing = false;
		void handleMouseMove(int32_t x, int32_t y);
		PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallback;
		PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallback;
		VkDebugReportCallbackEXT debugReportCallback;
		struct MultisampleTarget
		{
			struct
			{
				VkImage image;
				VkImageView view;
				VkDeviceMemory memory;
			} color;
			struct
			{
				VkImage image;
				VkImageView view;
				VkDeviceMemory memory;
			} depth;
		} multisampleTarget;

	protected:
		VkInstance instance;
		VkPhysicalDevice physicalDevice;
		VkPhysicalDeviceProperties deviceProperties;
		VkPhysicalDeviceFeatures deviceFeatures;
		VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
		VkDevice device;
		oceancore::VulkanDevice *vulkanDevice;
		VkQueue queue;
		VkFormat depthFormat;
		VkCommandPool cmdPool;
		VkRenderPass renderPass;
		std::vector<VkFramebuffer> frameBuffers;
		uint32_t currentBuffer = 0;
		VkDescriptorPool descriptorPool;
		VkPipelineCache pipelineCache;
		oceanswapchain::VulkanSwapChain swapChain;
		std::string title = "Vulkan Example";
		std::string name = "vulkanExample";

		// Refactored member variables
		std::vector<VkCommandBuffer> drawCmdBuffers;
		VkSemaphore semaphoreImageAvailable;
		VkFence waitFence;

		// Manager classes (using unique_ptr for proper type management)
		std::unique_ptr<coregfx::platform::CommandBufferManager> commandBufferManager;
		std::unique_ptr<coregfx::platform::SynchronizationManager> synchronizationManager;
		std::unique_ptr<coregfx::platform::WindowResizeManager> windowResizeManager;

		void windowResize();
		int getIndexOfBestGraphicalUnit(std::vector<VkPhysicalDevice>& devices);
		void parseCommandLineArgs();

	public:
		static std::vector<const char *> args;
		bool prepared = false;
		bool framerateCap = true;
		uint32_t width = 1280;
		uint32_t height = 720;
		float frameTimer = 1.0f;
		oceancore::Camera camera;
		glm::vec2 mousePos;
		bool paused = false;
		uint32_t lastFPS = 0;

		struct oceancore::Settings settings;

		// Utility classes for refactored functionality
		std::unique_ptr<coregfx::platform::InputHandler> inputHandler;
		std::unique_ptr<coregfx::platform::FramebufferManager> framebufferManager;

		struct DepthStencil
		{
			VkImage image;
			VkDeviceMemory mem;
			VkImageView view;
		} depthStencil;

		struct GamePadState
		{
			glm::vec2 axisLeft = glm::vec2(0.0f);
			glm::vec2 axisRight = glm::vec2(0.0f);
		} gamePadState;

		struct MouseButtons
		{
			bool left = false;
			bool right = false;
			bool middle = false;
		} mouseButtons;

		// OS specific
#if defined(_WIN32)
		HWND window;
		HINSTANCE windowInstance;
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
		// true if application has focused, false if moved to background
		bool focused = false;
		std::string androidProduct;
		struct TouchPoint
		{
			int32_t id;
			float x;
			float y;
			bool down = false;
		};
		float pinchDist = 0.0f;
		std::array<TouchPoint, 2> touchPoints;
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
		wl_display *display = nullptr;
		wl_registry *registry = nullptr;
		wl_compositor *compositor = nullptr;
		wl_shell *shell = nullptr;
		wl_seat *seat = nullptr;
		wl_pointer *pointer = nullptr;
		wl_keyboard *keyboard = nullptr;
		wl_surface *surface = nullptr;
		wl_shell_surface *shell_surface = nullptr;
		bool quit = false;

#elif defined(_DIRECT2DISPLAY)
		bool quit = false;
#elif defined(VK_USE_PLATFORM_XCB_KHR)
		bool quit = false;
		xcb_connection_t *connection;
		xcb_screen_t *screen;
		xcb_window_t window;
		xcb_intern_atom_reply_t *atom_wm_delete_window;
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
		NSWindow *window;
#endif

#if defined(_WIN32)
		HWND setupWindow(HINSTANCE hinstance, WNDPROC wndproc);
		void handleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
		static int32_t handleAppInput(struct android_app *app, AInputEvent *event);
		static void handleAppCommand(android_app *app, int32_t cmd);
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
		wl_shell_surface *setupWindow();
		void initWaylandConnection();
		static void registryGlobalCb(void *data, struct wl_registry *registry,
									 uint32_t name, const char *interface, uint32_t version);
		void registryGlobal(struct wl_registry *registry, uint32_t name,
							const char *interface, uint32_t version);
		static void registryGlobalRemoveCb(void *data, struct wl_registry *registry,
										   uint32_t name);
		static void seatCapabilitiesCb(void *data, wl_seat *seat, uint32_t caps);
		void seatCapabilities(wl_seat *seat, uint32_t caps);
		static void pointerEnterCb(void *data, struct wl_pointer *pointer,
								   uint32_t serial, struct wl_surface *surface, wl_fixed_t sx,
								   wl_fixed_t sy);
		static void pointerLeaveCb(void *data, struct wl_pointer *pointer,
								   uint32_t serial, struct wl_surface *surface);
		static void pointerMotionCb(void *data, struct wl_pointer *pointer,
									uint32_t time, wl_fixed_t sx, wl_fixed_t sy);
		void pointerMotion(struct wl_pointer *pointer,
						   uint32_t time, wl_fixed_t sx, wl_fixed_t sy);
		static void pointerButtonCb(void *data, struct wl_pointer *wl_pointer,
									uint32_t serial, uint32_t time, uint32_t button, uint32_t state);
		void pointerButton(struct wl_pointer *wl_pointer,
						   uint32_t serial, uint32_t time, uint32_t button, uint32_t state);
		static void pointerAxisCb(void *data, struct wl_pointer *wl_pointer,
								  uint32_t time, uint32_t axis, wl_fixed_t value);
		void pointerAxis(struct wl_pointer *wl_pointer,
						 uint32_t time, uint32_t axis, wl_fixed_t value);
		static void keyboardKeymapCb(void *data, struct wl_keyboard *keyboard,
									 uint32_t format, int fd, uint32_t size);
		static void keyboardEnterCb(void *data, struct wl_keyboard *keyboard,
									uint32_t serial, struct wl_surface *surface, struct wl_array *keys);
		static void keyboardLeaveCb(void *data, struct wl_keyboard *keyboard,
									uint32_t serial, struct wl_surface *surface);
		static void keyboardKeyCb(void *data, struct wl_keyboard *keyboard,
								  uint32_t serial, uint32_t time, uint32_t key, uint32_t state);
		void keyboardKey(struct wl_keyboard *keyboard,
						 uint32_t serial, uint32_t time, uint32_t key, uint32_t state);
		static void keyboardModifiersCb(void *data, struct wl_keyboard *keyboard,
										uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched,
										uint32_t mods_locked, uint32_t group);

#elif defined(_DIRECT2DISPLAY)
		//
#elif defined(VK_USE_PLATFORM_XCB_KHR)
		xcb_window_t setupWindow();
		void initxcbConnection();
		void handleEvent(const xcb_generic_event_t *event);
#elif defined(VK_USE_PLATFORM_MACOS_MVK)
		NSWindow *setupWindow();
		void mouseDragged(float x, float y);
		void windowWillResize(float x, float y);
		void windowDidResize();
#endif

		VulkanExampleBase();
		void setTitle(const std::string& newTitle);
		virtual ~VulkanExampleBase();

		// void init(const com::island::grpc::Settings* settings);
		void init(const oceancore::Settings *settings);

		virtual VkResult createInstance(bool enableValidation);
		virtual void render() = 0;
		bool render2();
		virtual void windowResized();
		virtual void setupFrameBuffer();
		virtual void prepare();
		virtual void fileDropped(std::string filename);
		void sendKey(UINT uMsg);
		void initSwapchain();
		void setupSwapChain();

		void renderLoop();
		void renderFrame();

		// Refactored methods
		void createCommandPool();
		void createCommandBuffers();
		void createSynchronizationPrimitives();
		void mouseMoved(int32_t x, int32_t y);

		void virtual createWindow(const oceancore::Settings *settings) = 0;
		// virtual void createWindow(const com::island::grpc::Settings* settings) = 0;
		// virtual void render() = 0;
	};
}

// Simplified Ocean wrapper for applications
namespace oceancore
{
	// Ocean class - a simplified wrapper around VulkanExampleBase
	// This class provides a simpler interface for applications that don't need
	// to extend the base class directly
	class Ocean : public oceanbase::VulkanExampleBase
	{
	public:
		bool viewUpdated = false;  // Set to true when window should close

		Ocean() : oceanbase::VulkanExampleBase() {}
		virtual ~Ocean() {}

		// Simplified prepare method that takes settings by value
		bool prepare(const oceancore::Settings& settings)
		{
			// Initialize the base class with settings
			init(&settings);
			VulkanExampleBase::prepare();
			return prepared;
		}

		// Override render to do nothing (apps can override if needed)
		virtual void render() override {}

		// Override createWindow to create a basic window
		virtual void createWindow(const oceancore::Settings* settings) override
		{
			// Window creation is handled by the base class init
			// This is platform-specific and will be handled in the implementation
		}

		// Platform-independent message handling wrapper
		void handleMessages()
		{
#if defined(_WIN32)
			// Windows requires a message loop - this should be called from WndProc
			// For now, we'll just check if window should close
			// The actual message handling is done through Windows message pump
#else
			// On other platforms, handleMessages might not exist or work differently
#endif
		}
	};
}

#endif