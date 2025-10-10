/**
 * Copyright (C) by Patrice Chevillat
 */

#pragma once
#include <coregfx/ocean.hpp>
#include <coregfx/core/ocean_base.hpp>
#include <coregfx/overlay/overlay.hpp>
#include <corecrt_math_defines.h>
#include <coregfx/gltf/gltf_service.hpp>
#include <string_view>
// Include PBR structure definitions
#include "../../../src/pbr/DescriptorSetLayouts.hpp"
#include "../../../src/pbr/DescriptorSets.hpp"
#include "../../../src/pbr/PbrModels.hpp"
#include "../../../src/pbr/Pipelines.hpp"
#include "../../../src/pbr/PushConstBlockMaterial.hpp"
#include "../../../src/pbr/ShaderValuesParams.hpp"
#include "../../../src/pbr/Textures.hpp"
#include "../../../src/pbr/UBOMatrices.hpp"
#include "../../../src/pbr/UniformBufferSet.hpp"
#include "cgfx.pb.h"
#include "cgfx.grpc.pb.h"
#include <chrono>
#include "context_engine.pb.h"
#include <coregfx/ipcoe17/ipcoe17.hpp>
#include <coregfx/imstudio/imstudio.hpp>
#include "nyx1.pb.h"

namespace coregfx
{
	using CallbackFunc = std::function<void(const std::string&)>;

	class OceanPbrApp : protected oceanbase::VulkanExampleBase
	{
	private:
		bool oceanServerAdressFromCmdLine = false;
		std::string cmdLineOceanServerHost;
		std::int32_t cmdLineOceanServerPort;

		/// <summary>
		/// Config-Settings from __argv
		/// </summary>
		bool updateCamOnReceiveRendererDto = true;
		///

		Textures textures;
		UBOMatrices shaderValuesScene;
		UBOMatrices shaderValuesSkybox;
		ShaderValuesParams shaderValuesParams;
		VkPipelineLayout pipelineLayout;
		Pipelines pipelines;
		VkPipeline boundPipeline = VK_NULL_HANDLE;
		DescriptorSetLayouts descriptorSetLayouts;
		std::vector<DescriptorSets> descriptorSets;
		std::vector<VkCommandBuffer> commandBuffers;
		std::vector<UniformBufferSet> uniformBuffers;
		std::vector<VkFence> waitFences;
		std::vector<VkSemaphore> renderCompleteSemaphores;
		std::vector<VkSemaphore> presentCompleteSemaphores;

		PbrModels pbrModels;

		const uint32_t renderAhead = 2;
		uint32_t node_frameIndex = 0;

		int32_t node_animationIndex = 12;
		float node_animationTimer = 0.0f;
		bool node_animate = true;

		bool displayBackground = false;
		bool centerAndScaleModel = false;

		oceancore::LightSource lightSource;

		// overlay
		overlayimgui::OverlayTemplate overlayTemplateId = overlayimgui::OverlayTemplate::OVERLAY_DEBUG_SHADER;
		overlayimgui::UI *ui;
		//

		bool node_rotate = true;  // aktive Rotation
		bool node_updated = true; // bei Veränderung der Pos/Rot muss der Node neu gezeichnet werden!
		glm::vec3 node_rot = glm::vec3(0.0f);
		glm::vec3 node_pos = glm::vec3(0.0f);

		enum PBRWorkflows
		{
			PBR_WORKFLOW_METALLIC_ROUGHNESS = 0,
			PBR_WORKFLOW_SPECULAR_GLOSINESS = 1
		};

		PushConstBlockMaterial pushConstBlockMaterial;

		std::map<std::string, std::string> environments;
		std::string selectedEnvironment = "papermill";

		int32_t debugViewInputs;
		int32_t debugViewEquation;

		const int32_t animationUpdateIntervalMilliseconds = 100;
		std::chrono::steady_clock::time_point lastAnimationUpdate;
		HWND setupWindow(HINSTANCE hinstance, WNDPROC wndproc);

	public:
		cgfx::Layout layout;

		// Studio mode tool launcher callback
		std::function<void(const std::string& toolId)> onToolLaunchRequested;

		OceanPbrApp(int argc, char *argv[]);
		~OceanPbrApp();
		void init(const oceancore::Settings* settings, HINSTANCE hinstance, WNDPROC wndproc);
		void insertGltf3(uint32_t gltfId, const tinygltf::Model* model);
		void insertGltf3(std::vector<std::pair<uint32_t, tinygltf::Model>>* modelContext);
		void insertGltf3(std::vector<std::tuple<uint32_t, GltfEngine::GltfService::GltfStatus, tinygltf::Model>>* modelContext);
		void removeGltf3(std::vector<uint32_t>* modelIds);
		void updateGltf3(std::vector<std::tuple<uint32_t, GltfEngine::GltfService::GltfStatus, tinygltf::Model>>* modelContext);
		void updateNodes3(com::context::grpc::DatabaseContext* gltfContext);
		void updateNodesFromEntities(const std::unordered_map<std::string, nyx::Entity>& entities);
		void updateNodes3(com::context::grpc::GltfContext* gltfContext);
		void updateGltfModel(uint32_t gltfId, const tinygltf::Model* model);
		void clearGltfs();
		void clearGltfById(uint32_t gltfId);
		void updateNode(uint32_t nodeId, const glm::vec3 pos, const glm::vec3 rot);
		void updateNode(uint32_t nodeId, const google::protobuf::RepeatedField<float> pos,
			const google::protobuf::RepeatedField<float> rot);
		void insertNode(uint32_t nodeId, uint32_t gltfId, const glm::vec3 pos, const glm::vec3 rot);
		void removeNode(uint32_t nodeId);

		void closeWindow();
		bool render2();
		oceancore::Camera* getCamera(void);
		void updateCamera(const glm::vec3 pos, const glm::vec3 rot);
		void updateCamera(const glm::vec3 pos,
			const glm::vec3 rot, float fov, float znear, float zfar);
		void updateCamera(const google::protobuf::RepeatedField<float> pos,
			const google::protobuf::RepeatedField<float> rot);
		void setTitle(const std::string& newTitle);
		void handleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		void changeOverlay(overlayimgui::OverlayTemplate newOverlay);
		void prepare();
		static oceancore::Settings buildDefaultSettings();
		bool isPrepared() {
			return prepared;
		}

		// Screenshot functionality
		void captureScreenshot(const std::string& filename = "screenshot.png");
		void requestScreenshot(const std::string& filename = "screenshot.png");
		void dumpSceneState();

		// Vulkan device access for external rendering (data-driven renderer)
		oceancore::VulkanDevice* getVulkanDevice() { return vulkanDevice; }

	private:
		void renderNodePipelinePbr(oceangltf::Node *node, uint32_t cbIndex, oceangltf::Material::AlphaMode alphaMode);
		void renderNodePipelinePbr2(oceangltf::Node *node, uint32_t cbIndex, oceangltf::Material::AlphaMode alphaMode);
		void recordCommandBuffers();
		void loadGltf(uint32_t gltfId, std::string_view filename);
		void loadGltf(uint32_t gltfId, const tinygltf::Model *model);
		void addGltf3(uint32_t gltfId, const tinygltf::Model *model);
		void addNode(uint32_t nodeId, uint32_t gltfId, const glm::vec3 pos, const glm::vec3 rot);
		void loadEnvironment(std::string_view filename);
		void loadAssets();
		void setupPerNodeDescriptorSet(oceangltf::Node *node);
		void setupDescriptors();
		void buildDescriptorPool();
		void buildSceneDescriptorSets2();
		void buildSceneDescriptorSets();
		void buildMaterialSamplers();
		void buildSkybox();
		void buildModelNodeMatrices();
		void buildPerMaterialDescriptorSets(std::vector<oceangltf::Material> &materials);
		void preparePipelines();
		void generateBRDFLUT();
		void generateCubemaps();
		void prepareUniformBuffers();
		void updateUniformBuffers();
		void updateParams();
		void windowResized();
		void updateOverlay();
		void updateOverlayDebugPerformance();
		void updateOverlayDebugShader();
		void updateStudioModeMenuBar();
		void handleImGuiDrawData();
		virtual void render();
		void animation();
		void updateNodeAnimation(oceangltf::Node *node, float end);
		void setFramerateCap(bool framerateCap);
		void reloadEnvironment(std::string_view envMapFile);
		void updateGltf(uint32_t gltfId, std::string filename);
		void insertGltfModel(uint32_t gltfId, const tinygltf::Model *model);
		void insertSecondGltf(uint32_t gltfId, const tinygltf::Model *model);
		void removeSecondGltfModel(uint32_t gltfId);
		void removeSecondGltf(uint32_t gltfId);
		void removeGltf(uint32_t gltfId);
		// void createWindow(const com::island::grpc::Settings *settings);
		void createWindow(const oceancore::Settings *settings);
		void updateGlbFromMemory(uint32_t gltfId, const unsigned char *bytes);
		oceangltf::Model *findModelByGltfId(uint32_t gltfId);
		// grpc
		std::shared_ptr<grpc::Channel> buildChannel(std::string address);
		void loadPageRequest(const std::string &stubId, bool updateCam);
		void sendFormActionRequest(const std::string& uri);
		void joinRequest(const std::string &gameModeIdent);
		cgfx::UserRequest buildFormActionMsg(const std::string& route);
		void updatePageInfoOverlay();
		void renderStyledButton();

		// Screenshot timing control
		bool screenshotRequested = false;
		std::string screenshotFilename;
		uint32_t framesSinceStart = 0;
		uint32_t screenshotDelayFrames = 10; // Wait 10 frames before taking screenshot

		// Edit mode functionality
		bool editModeEnabled = false;
	};
}