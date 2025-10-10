#include <coregfx/pbr/pbr_app.hpp>
#include <coregfx/overlay/overlay.hpp>
#include <coregfx/vulkan_context.hpp>
#include <coregfx/util/LunaraUtil.hpp>
#include <coregfx/core/ocean_log.hpp>
#include <coregfx/core/ocean_config.hpp>

// Screenshot functionality
#include <stb_image_write.h>
#include <fstream>
#include <ctime>

// PBR implementation headers
#include "DescriptorSetLayouts.hpp"
#include "DescriptorSets.hpp"
#include "PbrModels.hpp"
#include "Pipelines.hpp"
#include "PushConstBlockMaterial.hpp"
#include "ShaderValuesParams.hpp"
#include "Textures.hpp"
#include "UBOMatrices.hpp"
#include "UniformBufferSet.hpp"

namespace coregfx {

	OceanPbrApp::OceanPbrApp(int argc, char *argv[])
	{
		for (int32_t i = 0; i < argc - 1; i++)
		{
			args.push_back(argv[i]);

			if (strcmp(argv[i], "--ocean-server-host") == 0)
			{
				cmdLineOceanServerHost = argv[i + 1];
				oceanServerAdressFromCmdLine = true;
			}
			else if (strcmp(argv[i], "--ocean-server-port") == 0)
			{
				cmdLineOceanServerPort = atoi(argv[i + 1]);
			}
		};

		title = "OceanRendererClient";
#if defined(TINYGLTF_ENABLE_DRACO)
		ocean::info("Draco mesh compression is enabled");
#endif

		lastAnimationUpdate = std::chrono::high_resolution_clock::now();
	}
	std::shared_ptr<grpc::Channel> OceanPbrApp::buildChannel(std::string address)
	{
		grpc::ChannelArguments args;
		args.SetCompressionAlgorithm(GRPC_COMPRESS_GZIP);
		return grpc::CreateCustomChannel(address,
										 grpc::InsecureChannelCredentials(), args);
	}
	/**
	 * Lädt eine statische Seite aus der Datenbank.
	*/ 
	void OceanPbrApp::loadPageRequest(const std::string &stubId, bool updateCam = true)
	{
		/*auto l = buildLoadMessage(stubId, updateCam);
		ocean::ipcoe12::Value response;
		if (!sendEvent(l, &response))
		{
			ocean::error("loadPageRequest() cannot load stubId: " + stubId);
		}*/
	}

	/**
	 * Sendet ein Event (FormAction) zum Server
	 */
	void OceanPbrApp::sendFormActionRequest(const std::string& uri)
	{
		auto formActionMsg = buildFormActionMsg(uri);
		cgfx::Value response;
		// TODO: Fix namespace - Ipcoe17::send(formActionMsg, &response);
		// cgfx::send(formActionMsg, &response);

		ocean::info(response.DebugString());
	}

	/**
	 * Request zum Join eines GameModes.
	 */
	void OceanPbrApp::joinRequest(const std::string &gameModeIdent)
	{
		/*auto l = buildJoinMessage(gameModeIdent);
		ocean::ipcoe12::Value response;
		if (!sendEvent(l, &response))
		{
			ocean::error("joinRequest() cannot join gameMode: " + gameModeIdent);
		}*/
	}
	cgfx::UserRequest OceanPbrApp::buildFormActionMsg(const std::string& route)
	{
		cgfx::UserRequest r;
		auto map = r.mutable_formaction();
		map->set_name(route);
		return r;
	}

	OceanPbrApp::~OceanPbrApp()
	{
		vkDestroyPipeline(device, pipelines.skybox, nullptr);
		vkDestroyPipeline(device, pipelines.pbr, nullptr);
		vkDestroyPipeline(device, pipelines.pbrAlphaBlend, nullptr);

		vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
		vkDestroyDescriptorSetLayout(device, descriptorSetLayouts.scene, nullptr);
		vkDestroyDescriptorSetLayout(device, descriptorSetLayouts.material, nullptr);
		vkDestroyDescriptorSetLayout(device, descriptorSetLayouts.node, nullptr);

		pbrModels.destroy(device);

		for (auto buffer : uniformBuffers)
		{
			buffer.params.destroy();
			buffer.scene.destroy();
			buffer.skybox.destroy();
		}
		for (auto fence : waitFences)
		{
			vkDestroyFence(device, fence, nullptr);
		}
		for (auto semaphore : renderCompleteSemaphores)
		{
			vkDestroySemaphore(device, semaphore, nullptr);
		}
		for (auto semaphore : presentCompleteSemaphores)
		{
			vkDestroySemaphore(device, semaphore, nullptr);
		}

		textures.environmentCube.destroy();
		textures.irradianceCube.destroy();
		textures.prefilteredCube.destroy();
		textures.lutBrdf.destroy();
		textures.empty.destroy();

		// delete ui;
	}
	void OceanPbrApp::setFramerateCap(bool framerateCap)
	{
		this->framerateCap = framerateCap;
	}
	oceancore::Settings OceanPbrApp::buildDefaultSettings()
	{
		oceancore::Settings settings;
		settings.fullscreen = false;
		settings.multiSampling = true;
		settings.validation = true;
		settings.vsync = false;
		settings.windowHeight = 0;
		settings.windowWidth = 0;
		settings.useDiscreteGpu = false;
		return settings;
	}
	void OceanPbrApp::init(const oceancore::Settings *settings, HINSTANCE hinstance, WNDPROC wndproc)
	{
		oceanbase::VulkanExampleBase::init(settings);

		// Set edit mode flag
		editModeEnabled = settings->enableStudioMode;
		if (editModeEnabled) {
			ocean::info("Edit mode initialized - menu bar will be available");
		}

		setupWindow(hinstance, wndproc);

		prepare();
	}
	void OceanPbrApp::renderNodePipelinePbr(oceangltf::Node *node, uint32_t cbIndex,
											oceangltf::Material::AlphaMode alphaMode)
	{
		if (!node->display)
			return;

		if (node->mesh)
		{
			// Render mesh primitives
			for (auto *primitive : node->mesh->primitives)
			{
				if (primitive->material.alphaMode == alphaMode)
				{
					VkPipeline pipeline = VK_NULL_HANDLE;
					switch (alphaMode)
					{
					case oceangltf::Material::ALPHAMODE_OPAQUE:
					case oceangltf::Material::ALPHAMODE_MASK:
						pipeline = primitive->material.doubleSided ? pipelines.pbrDoubleSided : pipelines.pbr;
						break;
					case oceangltf::Material::ALPHAMODE_BLEND:
						pipeline = pipelines.pbrAlphaBlend;
						break;
					}

					if (pipeline != boundPipeline)
					{
						vkCmdBindPipeline(commandBuffers[cbIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
						boundPipeline = pipeline;
					}

					const std::vector<VkDescriptorSet> descriptorsets = {
						descriptorSets[cbIndex].scene,
						primitive->material.descriptorSet,
						node->mesh->uniformBuffer.descriptorSet,
					};
					vkCmdBindDescriptorSets(commandBuffers[cbIndex], VK_PIPELINE_BIND_POINT_GRAPHICS,
											pipelineLayout, 0, static_cast<uint32_t>(descriptorsets.size()),
											descriptorsets.data(), 0, NULL);

					// Pass material parameters as push constants
					PushConstBlockMaterial pushConstBlockMaterial{};
					pushConstBlockMaterial.emissiveFactor = primitive->material.emissiveFactor;
					// To save push constant space, availabilty and texture coordiante set are combined
					// -1 = texture not used for this material, >= 0 texture used and index of texture coordinate set
					pushConstBlockMaterial.colorTextureSet =
						primitive->material.baseColorTexture != nullptr ? primitive->material.texCoordSets.baseColor : -1;
					pushConstBlockMaterial.normalTextureSet =
						primitive->material.normalTexture != nullptr ? primitive->material.texCoordSets.normal : -1;
					pushConstBlockMaterial.occlusionTextureSet =
						primitive->material.occlusionTexture != nullptr ? primitive->material.texCoordSets.occlusion : -1;
					pushConstBlockMaterial.emissiveTextureSet =
						primitive->material.emissiveTexture != nullptr ? primitive->material.texCoordSets.emissive : -1;
					pushConstBlockMaterial.alphaMask =
						static_cast<float>(primitive->material.alphaMode == oceangltf::Material::ALPHAMODE_MASK);
					pushConstBlockMaterial.alphaMaskCutoff = primitive->material.alphaCutoff;

					// TODO: glTF specs states that metallic roughness should be preferred, even if specular glosiness is present

					if (primitive->material.pbrWorkflows.metallicRoughness)
					{
						// Metallic roughness workflow
						pushConstBlockMaterial.workflow = static_cast<float>(PBR_WORKFLOW_METALLIC_ROUGHNESS);
						pushConstBlockMaterial.baseColorFactor = primitive->material.baseColorFactor;
						pushConstBlockMaterial.metallicFactor = primitive->material.metallicFactor;
						pushConstBlockMaterial.roughnessFactor = primitive->material.roughnessFactor;
						pushConstBlockMaterial.PhysicalDescriptorTextureSet = primitive->material.metallicRoughnessTexture != nullptr ? primitive->material.texCoordSets.metallicRoughness : -1;
						pushConstBlockMaterial.colorTextureSet = primitive->material.baseColorTexture != nullptr ? primitive->material.texCoordSets.baseColor : -1;
					}

					if (primitive->material.pbrWorkflows.specularGlossiness)
					{
						// Specular glossiness workflow
						pushConstBlockMaterial.workflow = static_cast<float>(PBR_WORKFLOW_SPECULAR_GLOSINESS);
						pushConstBlockMaterial.PhysicalDescriptorTextureSet = primitive->material.extension.specularGlossinessTexture != nullptr ? primitive->material.texCoordSets.specularGlossiness : -1;
						pushConstBlockMaterial.colorTextureSet = primitive->material.extension.diffuseTexture != nullptr ? primitive->material.texCoordSets.baseColor : -1;
						pushConstBlockMaterial.diffuseFactor = primitive->material.extension.diffuseFactor;
						pushConstBlockMaterial.specularFactor = glm::vec4(primitive->material.extension.specularFactor, 1.0f);
					}

					vkCmdPushConstants(commandBuffers[cbIndex], pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstBlockMaterial), &pushConstBlockMaterial);

					if (primitive->hasIndices)
					{
						vkCmdDrawIndexed(commandBuffers[cbIndex], primitive->indexCount, 1, primitive->firstIndex, 0, 0);
					}
					else
					{
						vkCmdDraw(commandBuffers[cbIndex], primitive->vertexCount, 1, 0, 0);
					}
				}
			}
		};
		for (auto child : node->children)
		{
			renderNodePipelinePbr(child, cbIndex, alphaMode);
		}
	}

	void OceanPbrApp::renderNodePipelinePbr2(oceangltf::Node *node, uint32_t cbIndex,
											 oceangltf::Material::AlphaMode alphaMode)
	{
		if (!node->display)
			return;

		if (node->mesh)
		{
			// Render mesh primitives
			for (auto *primitive : node->mesh->primitives)
			{
				if (primitive->material.alphaMode == alphaMode)
				{
					VkPipeline pipeline = VK_NULL_HANDLE;
					switch (alphaMode)
					{
					case oceangltf::Material::ALPHAMODE_OPAQUE:
					case oceangltf::Material::ALPHAMODE_MASK:
						pipeline = primitive->material.doubleSided ? pipelines.pbrDoubleSided : pipelines.pbr;
						break;
					case oceangltf::Material::ALPHAMODE_BLEND:
						pipeline = pipelines.pbrAlphaBlend;
						break;
					}

					if (pipeline != boundPipeline)
					{
						vkCmdBindPipeline(commandBuffers[cbIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
						boundPipeline = pipeline;
					}

					const std::vector<VkDescriptorSet> descriptorsets = {
						descriptorSets[cbIndex].scene,
						primitive->material.descriptorSet,
						node->mesh->uniformBuffer.descriptorSet,
					};
					vkCmdBindDescriptorSets(commandBuffers[cbIndex], VK_PIPELINE_BIND_POINT_GRAPHICS,
											pipelineLayout, 0, static_cast<uint32_t>(descriptorsets.size()),
											descriptorsets.data(), 0, NULL);

					// Pass material parameters as push constants
					PushConstBlockMaterial pushConstBlockMaterial{};
					pushConstBlockMaterial.emissiveFactor = primitive->material.emissiveFactor;
					// To save push constant space, availabilty and texture coordiante set are combined
					// -1 = texture not used for this material, >= 0 texture used and index of texture coordinate set
					pushConstBlockMaterial.colorTextureSet =
						primitive->material.baseColorTexture != nullptr ? primitive->material.texCoordSets.baseColor : -1;
					pushConstBlockMaterial.normalTextureSet =
						primitive->material.normalTexture != nullptr ? primitive->material.texCoordSets.normal : -1;
					pushConstBlockMaterial.occlusionTextureSet =
						primitive->material.occlusionTexture != nullptr ? primitive->material.texCoordSets.occlusion : -1;
					pushConstBlockMaterial.emissiveTextureSet =
						primitive->material.emissiveTexture != nullptr ? primitive->material.texCoordSets.emissive : -1;
					pushConstBlockMaterial.alphaMask =
						static_cast<float>(primitive->material.alphaMode == oceangltf::Material::ALPHAMODE_MASK);
					pushConstBlockMaterial.alphaMaskCutoff = primitive->material.alphaCutoff;

					// TODO: glTF specs states that metallic roughness should be preferred, even if specular glosiness is present

					if (primitive->material.pbrWorkflows.metallicRoughness)
					{
						// Metallic roughness workflow
						pushConstBlockMaterial.workflow = static_cast<float>(PBR_WORKFLOW_METALLIC_ROUGHNESS);
						pushConstBlockMaterial.baseColorFactor = {0, 1.0, 0, 1}; // primitive->material.baseColorFactor;
						pushConstBlockMaterial.metallicFactor = primitive->material.metallicFactor;
						pushConstBlockMaterial.roughnessFactor = primitive->material.roughnessFactor;
						pushConstBlockMaterial.PhysicalDescriptorTextureSet = primitive->material.metallicRoughnessTexture != nullptr ? primitive->material.texCoordSets.metallicRoughness : -1;
						pushConstBlockMaterial.colorTextureSet = primitive->material.baseColorTexture != nullptr ? primitive->material.texCoordSets.baseColor : -1;
					}

					if (primitive->material.pbrWorkflows.specularGlossiness)
					{
						// Specular glossiness workflow
						pushConstBlockMaterial.workflow = static_cast<float>(PBR_WORKFLOW_SPECULAR_GLOSINESS);
						pushConstBlockMaterial.PhysicalDescriptorTextureSet = primitive->material.extension.specularGlossinessTexture != nullptr ? primitive->material.texCoordSets.specularGlossiness : -1;
						pushConstBlockMaterial.colorTextureSet = primitive->material.extension.diffuseTexture != nullptr ? primitive->material.texCoordSets.baseColor : -1;
						pushConstBlockMaterial.diffuseFactor = primitive->material.extension.diffuseFactor;
						pushConstBlockMaterial.specularFactor = glm::vec4(primitive->material.extension.specularFactor, 1.0f);
					}

					vkCmdPushConstants(commandBuffers[cbIndex], pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstBlockMaterial), &pushConstBlockMaterial);

					if (primitive->hasIndices)
					{
						vkCmdDrawIndexed(commandBuffers[cbIndex], primitive->indexCount, 1, primitive->firstIndex, 0, 0);
					}
					else
					{
						vkCmdDraw(commandBuffers[cbIndex], primitive->vertexCount, 1, 0, 0);
					}
				}
			}
		};
		for (auto child : node->children)
		{
			renderNodePipelinePbr2(child, cbIndex, alphaMode);
		}
	}

	void OceanPbrApp::recordCommandBuffers()
	{
		VkCommandBufferBeginInfo cmdBufferBeginInfo = oceaninitializer::commandBufferBeginInfo();

		VkClearValue clearValues[3];
		if (settings.multiSampling)
		{
			// clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
			clearValues[0].color = {CLEAR_COLOR[0], CLEAR_COLOR[1], CLEAR_COLOR[2], CLEAR_COLOR[3]};
			// clearValues[1].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
			clearValues[1].color = {CLEAR_COLOR[0], CLEAR_COLOR[1], CLEAR_COLOR[2], CLEAR_COLOR[3]};
			// clearValues[1].color = { { 32.0f / 255.0f, 32.0f / 255.0f, 32.0f / 255.0f, 1.0f } };
			clearValues[2].depthStencil = {1.0f, 0};
		}
		else
		{
			clearValues[0].color = {CLEAR_COLOR[0], CLEAR_COLOR[1], CLEAR_COLOR[2], CLEAR_COLOR[3]};
			clearValues[1].depthStencil = {1.0f, 0};
		}

		VkRenderPassBeginInfo renderPassBeginInfo{};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = renderPass;
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent.width = width;
		renderPassBeginInfo.renderArea.extent.height = height;
		renderPassBeginInfo.clearValueCount = settings.multiSampling ? 3 : 2;
		renderPassBeginInfo.pClearValues = clearValues;

		for (size_t i = 0; i < commandBuffers.size(); ++i)
		{
			renderPassBeginInfo.framebuffer = frameBuffers[i];

			VkCommandBuffer currentCB = commandBuffers[i];

			VK_CHECK_RESULT(vkBeginCommandBuffer(currentCB, &cmdBufferBeginInfo));
			vkCmdBeginRenderPass(currentCB, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			VkViewport viewport{};
			viewport.width = (float)width;
			viewport.height = (float)height;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;
			vkCmdSetViewport(currentCB, 0, 1, &viewport);

			VkRect2D scissor{};
			scissor.extent = {width, height};
			vkCmdSetScissor(currentCB, 0, 1, &scissor);

			VkDeviceSize offsets[1] = {0};

			if (displayBackground)
			{
				vkCmdBindDescriptorSets(currentCB, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1,
										&descriptorSets[i].skybox, 0, nullptr);
				vkCmdBindPipeline(currentCB, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelines.skybox);
				pbrModels.skybox.draw(currentCB);
			}
			/////////////////////////////////////////////////////////////////////////
			oceangltf::Model &model = pbrModels.scene;

			vkCmdBindVertexBuffers(currentCB, 0, 1, &model.vertices.buffer, offsets);
			if (model.indices.buffer != VK_NULL_HANDLE)
			{
				vkCmdBindIndexBuffer(currentCB, model.indices.buffer, 0, VK_INDEX_TYPE_UINT32);
			}

			boundPipeline = VK_NULL_HANDLE;

			// Opaque primitives first
			for (auto& node : model.nodes)
			{
				renderNodePipelinePbr(node, i, oceangltf::Material::ALPHAMODE_OPAQUE);
			}
			// for (auto node : model.additionalNodes) {
			//	renderNode(node.second, i, oceangltf::Material::ALPHAMODE_OPAQUE);
			// }
			//  Alpha masked primitives
			for (auto& node : model.nodes)
			{
				renderNodePipelinePbr(node, i, oceangltf::Material::ALPHAMODE_MASK);
			}
			// for (auto node : model.additionalNodes) {
			//	renderNode(node.second, i, oceangltf::Material::ALPHAMODE_MASK);
			// }
			//  Transparent primitives
			//  TODO: Correct depth sorting
			for (auto& node : model.nodes)
			{
				renderNodePipelinePbr(node, i, oceangltf::Material::ALPHAMODE_BLEND);
			}
			// for (auto node : model.additionalNodes) {
			//	renderNode(node.second, i, oceangltf::Material::ALPHAMODE_BLEND);
			// }

			//////////////////////////////////////////////////////////////////
			if (pbrModels.gltf2 != nullptr)
			{
				auto m2 = pbrModels.gltf2;

				vkCmdBindVertexBuffers(currentCB, 0, 1, &m2->vertices.buffer, offsets);
				if (m2->indices.buffer != VK_NULL_HANDLE)
				{
					vkCmdBindIndexBuffer(currentCB, m2->indices.buffer, 0, VK_INDEX_TYPE_UINT32);
				}

				// Opaque primitives first
				for (auto& node : m2->nodes)
				{
					renderNodePipelinePbr(node, i, oceangltf::Material::ALPHAMODE_OPAQUE);
				}
				// Alpha masked primitives
				for (auto& node : m2->nodes)
				{
					renderNodePipelinePbr(node, i, oceangltf::Material::ALPHAMODE_MASK);
				}
				// Transparent primitives
				// TODO: Correct depth sorting
				for (auto& node : m2->nodes)
				{
					renderNodePipelinePbr(node, i, oceangltf::Material::ALPHAMODE_BLEND);
				}
			}
			//////////////////////////////////////////////////////////////////

			for (int s = 0; s < pbrModels.gltfs.size(); s++)
			{
				auto m3 = &pbrModels.gltfs[s];

				if (!m3->display)
					continue;

				vkCmdBindVertexBuffers(currentCB, 0, 1, &m3->vertices.buffer, offsets);
				if (m3->indices.buffer != VK_NULL_HANDLE)
				{
					vkCmdBindIndexBuffer(currentCB, m3->indices.buffer, 0, VK_INDEX_TYPE_UINT32);
				}
				// Opaque primitives first
				for (auto& node : m3->nodes)
				{
					// if (node->id.compare("1847883423") == 0)
					//{
					// if (node->translation.x > 1)
					//	node->display = false;
					// std::cout << "p " << node->translation.x << std::endl;
					//}
					switch (node->shader)
					{
					case oceangltf::Shader::SHADER_BASE_COLOR:
						renderNodePipelinePbr2(node, i, oceangltf::Material::ALPHAMODE_OPAQUE);
						break;
					default:
					case oceangltf::Shader::SHADER_PBR:
						renderNodePipelinePbr(node, i, oceangltf::Material::ALPHAMODE_OPAQUE);
						break;
					}
				}
				// Alpha masked primitives
				for (auto& node : m3->nodes)
				{
					switch (node->shader)
					{
					case oceangltf::Shader::SHADER_BASE_COLOR:
						renderNodePipelinePbr2(node, i, oceangltf::Material::ALPHAMODE_MASK);
						break;
					default:
					case oceangltf::Shader::SHADER_PBR:
						renderNodePipelinePbr(node, i, oceangltf::Material::ALPHAMODE_MASK);
						break;
					}
				}
				// Transparent primitives
				// TODO: Correct depth sorting
				for (auto& node : m3->nodes)
				{
					switch (node->shader)
					{
					case oceangltf::Shader::SHADER_BASE_COLOR:
						renderNodePipelinePbr2(node, i, oceangltf::Material::ALPHAMODE_BLEND);
						break;
					default:
					case oceangltf::Shader::SHADER_PBR:
						renderNodePipelinePbr(node, i, oceangltf::Material::ALPHAMODE_BLEND);
						break;
					}
				}
			}

			//////////////////////////////////////////////////////////////////

			// User interface
			if (overlayTemplateId != overlayimgui::OverlayTemplate::OVERLAY_NONE)
				ui->draw(currentCB);

			vkCmdEndRenderPass(currentCB);
			VK_CHECK_RESULT(vkEndCommandBuffer(currentCB));
		}
	}

	void OceanPbrApp::loadGltf(uint32_t gltfId, std::string_view filename)
	{
		ocean::info("=== PBR APP GLTF LOADING START ===");
		ocean::info("Loading gltfId=" + std::to_string(gltfId) + " from " + std::string(filename));

		auto destroyStart = std::chrono::high_resolution_clock::now();
		pbrModels.scene.destroy(device);
		auto destroyTime = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - destroyStart).count();
		ocean::info("Scene destruction took " + std::to_string(destroyTime) + " ms");

		node_animationIndex = 4;
		node_animationTimer = 0.0f;

		auto tStart = std::chrono::high_resolution_clock::now();
		pbrModels.scene.loadFromFile(filename, vulkanDevice, queue);
		auto tFileLoad = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - tStart).count();
		ocean::info("PBR GLTF file loading took " + std::to_string(tFileLoad) + " ms");
		ocean::info("=== PBR APP GLTF LOADING COMPLETE ===");
	}

	void OceanPbrApp::loadGltf(uint32_t gltfId, const tinygltf::Model *model)
	{
		ocean::info("=== PBR APP GLTF MODEL LOADING START ===");
		ocean::info("Loading gltfId=" + std::to_string(gltfId) + " from tinygltf::Model");

		auto destroyStart = std::chrono::high_resolution_clock::now();
		pbrModels.scene.destroy(device);
		auto destroyTime = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - destroyStart).count();
		ocean::info("Scene destruction took " + std::to_string(destroyTime) + " ms");

		node_animationIndex = 4;
		node_animationTimer = 0.0f;

		auto tStart = std::chrono::high_resolution_clock::now();
		pbrModels.scene.loadFromModel(gltfId, model, vulkanDevice, queue);
		auto tFileLoad = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - tStart).count();
		ocean::info("PBR GLTF model loading took " + std::to_string(tFileLoad) + " ms");
		ocean::info("=== PBR APP GLTF MODEL LOADING COMPLETE ===");
	}

	void OceanPbrApp::addGltf3(uint32_t gltfId, const tinygltf::Model *model)
	{
		ocean::info("=== PBR ADD GLTF3 START ===");
		ocean::info("Adding GLTF model with gltfId=" + std::to_string(gltfId));
		auto tStart = std::chrono::high_resolution_clock::now();

		size_t modelCountBefore = pbrModels.gltfs.size();
		pbrModels.gltfs.push_back(oceangltf::Model());
		ocean::info("Added new model to gltfs vector (count: " + std::to_string(pbrModels.gltfs.size()) + ")");

		ocean::info("Model statistics: nodes=" + std::to_string(model->nodes.size()) +
					", materials=" + std::to_string(model->materials.size()) +
					", animations=" + std::to_string(model->animations.size()) +
					", accessors=" + std::to_string(model->accessors.size()) +
					", meshes=" + std::to_string(model->meshes.size()) +
					", textures=" + std::to_string(model->textures.size()));

		auto loadStart = std::chrono::high_resolution_clock::now();
		pbrModels.gltfs[pbrModels.gltfs.size() - 1].loadFromModel(gltfId, model, vulkanDevice, queue);
		auto loadTime = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - loadStart).count();

		auto tFileLoad = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - tStart).count();
		ocean::info("Model loading took " + std::to_string(loadTime) + " ms");
		ocean::info("Total addGltf3 operation took " + std::to_string(tFileLoad) + " ms");
		ocean::info("=== PBR ADD GLTF3 COMPLETE ===");
	}
	void OceanPbrApp::addNode(uint32_t nodeId, uint32_t gltfId, const glm::vec3 pos, const glm::vec3 rot)
	{
		ocean::info("=== PBR ADD NODE START ===");
		ocean::info("Adding nodeId=" + std::to_string(nodeId) + " to gltfId=" + std::to_string(gltfId));
		ocean::info("Position: (" + std::to_string(pos.x) + ", " + std::to_string(pos.y) + ", " + std::to_string(pos.z) + ")");
		ocean::info("Rotation: (" + std::to_string(rot.x) + ", " + std::to_string(rot.y) + ", " + std::to_string(rot.z) + ")");

		auto model = findModelByGltfId(gltfId);
		if (!model) {
			ocean::error("Failed to find model with gltfId=" + std::to_string(gltfId));
			return;
		}

		auto tStart = std::chrono::high_resolution_clock::now();
		model->addNode(nodeId, pos, rot);
		auto tFileLoad = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - tStart).count();
		ocean::info("Node addition took " + std::to_string(tFileLoad) + " ms");
		ocean::info("=== PBR ADD NODE COMPLETE ===");
	}

	oceangltf::Model *OceanPbrApp::findModelByGltfId(uint32_t gltfId)
	{
		// To do something...
		return &pbrModels.scene;
	}

	void OceanPbrApp::loadEnvironment(std::string_view filename)
	{
		ocean::info("=== PBR LOAD ENVIRONMENT START ===");
		ocean::info("Loading environment from " + std::string(filename));
		if (textures.environmentCube.image)
		{
			textures.environmentCube.destroy();
			textures.irradianceCube.destroy();
			textures.prefilteredCube.destroy();
		}
		textures.environmentCube.loadFromFile(filename, VK_FORMAT_R16G16B16A16_SFLOAT, vulkanDevice, queue);
		generateCubemaps();
	}

	void OceanPbrApp::loadAssets()
	{
		ocean::info("=== PBR LOAD ASSETS START ===");
		ocean::info("Starting asset loading process");

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
		tinygltf::asset_manager = androidApp->activity->assetManager;
		oceanutil::readDirectory(assetpath + "models", "*.gltf", scenes, true);
#else
		// const std::string assetpath = "resources/";
		const std::string assetpath = getAssetPath();
		ocean::info("Asset path resolved to: " + assetpath);
		struct stat info;
		if (stat(assetpath.c_str(), &info) != 0)
		{
			std::string msg = "Could not locate asset path in \"" + assetpath + "\".\nMake sure binary is run from correct relative directory!";
			ocean::error("CRITICAL: " + msg);
			ocean::error("Verify ASSET_ROOT in coregfx.config.yml points to a valid directory");
			throw std::runtime_error(msg);
		}
		ocean::info("Asset path validation: SUCCESS");
#endif
		ocean::info("Reading environments directory...");
		oceanutil::readDirectory(assetpath + "environments", "*.ktx", environments, false);
		ocean::info("Environments directory read complete");

		ocean::info("Loading empty texture from: " + assetpath + "textures/empty.ktx");
		textures.empty.loadFromFile(assetpath + "textures/empty.ktx", VK_FORMAT_R8G8B8A8_UNORM, vulkanDevice, queue);
		ocean::info("Empty texture loaded successfully");

		const uint32_t gltfId = 3095296745;
		// Use asset path from ConfigProvider instead of hardcoded paths
		std::string gltfFile = assetpath + "models/cross/gltf/cross.gltf";
		ocean::info("Loading GLTF file: " + gltfFile);

		std::string envMapFile = assetpath + "environments/papermill.ktx";
		// std::string envMapFile = assetpath + "environments/gcanyon_cube.ktx";

		loadGltf(gltfId, gltfFile.c_str());
		ocean::info("GLTF file loaded successfully");

		ocean::info("Loading skybox from: " + assetpath + "models/Box/glTF-Embedded/Box.gltf");
		pbrModels.skybox.loadFromFile(assetpath + "models/Box/glTF-Embedded/Box.gltf", vulkanDevice, queue);
		ocean::info("Skybox loaded successfully");

		ocean::info("Loading environment map: " + envMapFile);
		loadEnvironment(envMapFile.c_str());
		ocean::info("Environment map loaded successfully");
	}

	void OceanPbrApp::setupPerNodeDescriptorSet(oceangltf::Node *node)
	{
		if (node->mesh)
		{
			VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
			descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			descriptorSetAllocInfo.descriptorPool = descriptorPool;
			descriptorSetAllocInfo.pSetLayouts = &descriptorSetLayouts.node;
			descriptorSetAllocInfo.descriptorSetCount = 1;
			VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &descriptorSetAllocInfo, &node->mesh->uniformBuffer.descriptorSet));

			VkWriteDescriptorSet writeDescriptorSet{};
			writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			writeDescriptorSet.descriptorCount = 1;
			writeDescriptorSet.dstSet = node->mesh->uniformBuffer.descriptorSet;
			writeDescriptorSet.dstBinding = 0;
			writeDescriptorSet.pBufferInfo = &node->mesh->uniformBuffer.descriptor;

			vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, nullptr);
		}
		for (auto &child : node->children)
		{
			setupPerNodeDescriptorSet(child);
		}
	}

	void OceanPbrApp::setupDescriptors()
	{
		ocean::info("=== PBR SETUP DESCRIPTORS START ===");
		ocean::info("Setting up Vulkan descriptor sets and pools");
		buildDescriptorPool();
		buildSceneDescriptorSets();
		buildMaterialSamplers();
		buildSkybox();
	}

	void OceanPbrApp::buildDescriptorPool()
	{
		uint32_t imageSamplerCount = 0;
		uint32_t materialCount = 0;
		uint32_t meshCount = 0;

		// Environment samplers (radiance, irradiance, brdf lut)
		imageSamplerCount += 3;

		std::vector<oceangltf::Model *> modellist;

		if (pbrModels.gltf2 == nullptr)
		{
			modellist = {
				&pbrModels.skybox,
				&pbrModels.scene};
		}
		else
		{
			modellist = {
				&pbrModels.skybox,
				&pbrModels.scene,
				pbrModels.gltf2};
		}
		for (auto &model : modellist)
		{
			for (auto &material : model->materials)
			{
				imageSamplerCount += 5;
				materialCount++;
			}
			for (auto& node : model->linearNodes)
			{
				if (node->mesh)
				{
					meshCount++;
				}
			}
		}

		for (int i = 0; i < pbrModels.gltfs.size(); i++)
		{
			for (auto &material : pbrModels.gltfs[i].materials)
			{
				imageSamplerCount += 5;
				materialCount++;
			}
			for (auto& node : pbrModels.gltfs[i].linearNodes)
			{
				if (node->mesh)
				{
					meshCount++;
				}
			}
		}

		ocean::info("VkPbrExample.setupDescriptors() materialCount=" + std::to_string(materialCount) +
					" meshCount=" + std::to_string(meshCount));

		std::vector<VkDescriptorPoolSize> poolSizes = {
			{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, (4 + meshCount) * swapChain.imageCount},
			{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, imageSamplerCount * swapChain.imageCount}};
		VkDescriptorPoolCreateInfo descriptorPoolCI{};
		descriptorPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolCI.poolSizeCount = 2;
		descriptorPoolCI.pPoolSizes = poolSizes.data();
		descriptorPoolCI.maxSets = (2 + materialCount + meshCount) * swapChain.imageCount;
		VK_CHECK_RESULT(vkCreateDescriptorPool(device, &descriptorPoolCI, nullptr, &descriptorPool));
	}
	void OceanPbrApp::buildSceneDescriptorSets2()
	{
		if (pbrModels.gltf2 != nullptr)
		// Scene (matrices and environment maps)
		{
			std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
				{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
				{1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
				{2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
				{3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
				{4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
			};
			VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI{};
			descriptorSetLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			descriptorSetLayoutCI.pBindings = setLayoutBindings.data();
			descriptorSetLayoutCI.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
			// VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCI, nullptr, &descriptorSetLayouts.scene));

			for (auto i = 0; i < descriptorSets.size(); i++)
			{

				VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
				descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
				descriptorSetAllocInfo.descriptorPool = descriptorPool;
				descriptorSetAllocInfo.pSetLayouts = &descriptorSetLayouts.scene;
				descriptorSetAllocInfo.descriptorSetCount = 1;
				VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &descriptorSetAllocInfo, &descriptorSets[i].scene));

				std::array<VkWriteDescriptorSet, 5> writeDescriptorSets{};

				// if (pbrModels.gltf2 != nullptr) {
				//  MODEL2
				descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
				descriptorSetAllocInfo.descriptorPool = descriptorPool;
				descriptorSetAllocInfo.pSetLayouts = &descriptorSetLayouts.scene;
				descriptorSetAllocInfo.descriptorSetCount = 1;
				VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &descriptorSetAllocInfo, &descriptorSets[i].gltf2));

				writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				writeDescriptorSets[0].descriptorCount = 1;
				writeDescriptorSets[0].dstSet = descriptorSets[i].gltf2;
				writeDescriptorSets[0].dstBinding = 0;
				writeDescriptorSets[0].pBufferInfo = &uniformBuffers[i].scene.descriptor;

				writeDescriptorSets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeDescriptorSets[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				writeDescriptorSets[1].descriptorCount = 1;
				writeDescriptorSets[1].dstSet = descriptorSets[i].gltf2;
				writeDescriptorSets[1].dstBinding = 1;
				writeDescriptorSets[1].pBufferInfo = &uniformBuffers[i].params.descriptor;

				writeDescriptorSets[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeDescriptorSets[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				writeDescriptorSets[2].descriptorCount = 1;
				writeDescriptorSets[2].dstSet = descriptorSets[i].gltf2;
				writeDescriptorSets[2].dstBinding = 2;
				writeDescriptorSets[2].pImageInfo = &textures.irradianceCube.descriptor;

				writeDescriptorSets[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeDescriptorSets[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				writeDescriptorSets[3].descriptorCount = 1;
				writeDescriptorSets[3].dstSet = descriptorSets[i].gltf2;
				writeDescriptorSets[3].dstBinding = 3;
				writeDescriptorSets[3].pImageInfo = &textures.prefilteredCube.descriptor;

				writeDescriptorSets[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeDescriptorSets[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				writeDescriptorSets[4].descriptorCount = 1;
				writeDescriptorSets[4].dstSet = descriptorSets[i].gltf2;
				writeDescriptorSets[4].dstBinding = 4;
				writeDescriptorSets[4].pImageInfo = &textures.lutBrdf.descriptor;

				vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, NULL);
				//}
			}
		}
	}

	void OceanPbrApp::buildSceneDescriptorSets()
	{
		// Scene (matrices and environment maps)
		{
			std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
				{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
				{1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
				{2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
				{3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
				{4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
			};
			VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI{};
			descriptorSetLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			descriptorSetLayoutCI.pBindings = setLayoutBindings.data();
			descriptorSetLayoutCI.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
			VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCI, nullptr, &descriptorSetLayouts.scene));

			for (auto i = 0; i < descriptorSets.size(); i++)
			{

				// SCENE
				VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
				descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
				descriptorSetAllocInfo.descriptorPool = descriptorPool;
				descriptorSetAllocInfo.pSetLayouts = &descriptorSetLayouts.scene;
				descriptorSetAllocInfo.descriptorSetCount = 1;
				VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &descriptorSetAllocInfo, &descriptorSets[i].scene));

				std::array<VkWriteDescriptorSet, 5> writeDescriptorSets{};

				writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				writeDescriptorSets[0].descriptorCount = 1;
				writeDescriptorSets[0].dstSet = descriptorSets[i].scene;
				writeDescriptorSets[0].dstBinding = 0;
				writeDescriptorSets[0].pBufferInfo = &uniformBuffers[i].scene.descriptor;

				writeDescriptorSets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeDescriptorSets[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				writeDescriptorSets[1].descriptorCount = 1;
				writeDescriptorSets[1].dstSet = descriptorSets[i].scene;
				writeDescriptorSets[1].dstBinding = 1;
				writeDescriptorSets[1].pBufferInfo = &uniformBuffers[i].params.descriptor;

				writeDescriptorSets[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeDescriptorSets[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				writeDescriptorSets[2].descriptorCount = 1;
				writeDescriptorSets[2].dstSet = descriptorSets[i].scene;
				writeDescriptorSets[2].dstBinding = 2;
				writeDescriptorSets[2].pImageInfo = &textures.irradianceCube.descriptor;

				writeDescriptorSets[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeDescriptorSets[3].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				writeDescriptorSets[3].descriptorCount = 1;
				writeDescriptorSets[3].dstSet = descriptorSets[i].scene;
				writeDescriptorSets[3].dstBinding = 3;
				writeDescriptorSets[3].pImageInfo = &textures.prefilteredCube.descriptor;

				writeDescriptorSets[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeDescriptorSets[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				writeDescriptorSets[4].descriptorCount = 1;
				writeDescriptorSets[4].dstSet = descriptorSets[i].scene;
				writeDescriptorSets[4].dstBinding = 4;
				writeDescriptorSets[4].pImageInfo = &textures.lutBrdf.descriptor;

				vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()),
									   writeDescriptorSets.data(), 0, NULL);
			}
		}
	}
	void OceanPbrApp::buildMaterialSamplers()
	{
		// Material (samplers)
		std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
			{0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
			{1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
			{2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
			{3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
			{4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr},
		};
		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI{};
		descriptorSetLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorSetLayoutCI.pBindings = setLayoutBindings.data();
		descriptorSetLayoutCI.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
		VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCI, nullptr, &descriptorSetLayouts.material));

		buildPerMaterialDescriptorSets(pbrModels.scene.materials);

		// model 2
		if (pbrModels.gltf2 != nullptr)
		{
			buildPerMaterialDescriptorSets(pbrModels.gltf2->materials);
		}

		for (int i = 0; i < pbrModels.gltfs.size(); i++)
		{
			buildPerMaterialDescriptorSets(pbrModels.gltfs[i].materials);
		}

		buildModelNodeMatrices();
	}
	void OceanPbrApp::buildSkybox()
	{
		// Skybox (fixed set)
		for (auto i = 0; i < uniformBuffers.size(); i++)
		{
			VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
			descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			descriptorSetAllocInfo.descriptorPool = descriptorPool;
			descriptorSetAllocInfo.pSetLayouts = &descriptorSetLayouts.scene;
			descriptorSetAllocInfo.descriptorSetCount = 1;
			VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &descriptorSetAllocInfo, &descriptorSets[i].skybox));

			std::array<VkWriteDescriptorSet, 3> writeDescriptorSets{};

			writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			writeDescriptorSets[0].descriptorCount = 1;
			writeDescriptorSets[0].dstSet = descriptorSets[i].skybox;
			writeDescriptorSets[0].dstBinding = 0;
			writeDescriptorSets[0].pBufferInfo = &uniformBuffers[i].skybox.descriptor;

			writeDescriptorSets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptorSets[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			writeDescriptorSets[1].descriptorCount = 1;
			writeDescriptorSets[1].dstSet = descriptorSets[i].skybox;
			writeDescriptorSets[1].dstBinding = 1;
			writeDescriptorSets[1].pBufferInfo = &uniformBuffers[i].params.descriptor;

			writeDescriptorSets[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptorSets[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			writeDescriptorSets[2].descriptorCount = 1;
			writeDescriptorSets[2].dstSet = descriptorSets[i].skybox;
			writeDescriptorSets[2].dstBinding = 2;
			writeDescriptorSets[2].pImageInfo = &textures.prefilteredCube.descriptor;

			vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
		}
	}

	void OceanPbrApp::buildModelNodeMatrices()
	{
		// Model node (matrices)
		std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings = {
			{0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1, VK_SHADER_STAGE_VERTEX_BIT, nullptr},
		};
		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI{};
		descriptorSetLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorSetLayoutCI.pBindings = setLayoutBindings.data();
		descriptorSetLayoutCI.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
		VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCI,
													nullptr, &descriptorSetLayouts.node));

		// Per-Node descriptor set
		for (auto &node : pbrModels.scene.nodes)
		{
			setupPerNodeDescriptorSet(node);
		}
		if (pbrModels.gltf2 != nullptr)
		{
			// Per-Node descriptor set
			for (oceangltf::Node *node : pbrModels.gltf2->nodes)
			{
				setupPerNodeDescriptorSet(node);
			}
		}
		for (int i = 0; i < pbrModels.gltfs.size(); i++)
		{
			for (oceangltf::Node *node : pbrModels.gltfs[i].nodes)
			{
				setupPerNodeDescriptorSet(node);
			}
		}
	}
	void OceanPbrApp::buildPerMaterialDescriptorSets(std::vector<oceangltf::Material> &materials)
	{
		// Per-Material descriptor sets
		for (auto &material : materials)
		{
			VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
			descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			descriptorSetAllocInfo.descriptorPool = descriptorPool;
			descriptorSetAllocInfo.pSetLayouts = &descriptorSetLayouts.material;
			descriptorSetAllocInfo.descriptorSetCount = 1;
			VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &descriptorSetAllocInfo, &material.descriptorSet));

			std::vector<VkDescriptorImageInfo> imageDescriptors = {
				textures.empty.descriptor,
				textures.empty.descriptor,
				material.normalTexture ? material.normalTexture->descriptor : textures.empty.descriptor,
				material.occlusionTexture ? material.occlusionTexture->descriptor : textures.empty.descriptor,
				material.emissiveTexture ? material.emissiveTexture->descriptor : textures.empty.descriptor};

			// TODO: glTF specs states that metallic roughness should be preferred, even if specular glosiness is present

			if (material.pbrWorkflows.metallicRoughness)
			{
				if (material.baseColorTexture)
				{
					imageDescriptors[0] = material.baseColorTexture->descriptor;
				}
				if (material.metallicRoughnessTexture)
				{
					imageDescriptors[1] = material.metallicRoughnessTexture->descriptor;
				}
			}

			if (material.pbrWorkflows.specularGlossiness)
			{
				if (material.extension.diffuseTexture)
				{
					imageDescriptors[0] = material.extension.diffuseTexture->descriptor;
				}
				if (material.extension.specularGlossinessTexture)
				{
					imageDescriptors[1] = material.extension.specularGlossinessTexture->descriptor;
				}
			}

			std::array<VkWriteDescriptorSet, 5> writeDescriptorSets{};
			for (size_t i = 0; i < imageDescriptors.size(); i++)
			{
				writeDescriptorSets[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeDescriptorSets[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				writeDescriptorSets[i].descriptorCount = 1;
				writeDescriptorSets[i].dstSet = material.descriptorSet;
				writeDescriptorSets[i].dstBinding = static_cast<uint32_t>(i);
				writeDescriptorSets[i].pImageInfo = &imageDescriptors[i];
			}

			vkUpdateDescriptorSets(device, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, NULL);
		}
	}
	void OceanPbrApp::preparePipelines()
	{
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCI{};
		inputAssemblyStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyStateCI.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		VkPipelineRasterizationStateCreateInfo rasterizationStateCI{};
		rasterizationStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizationStateCI.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizationStateCI.cullMode = VK_CULL_MODE_BACK_BIT;
		rasterizationStateCI.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizationStateCI.lineWidth = 1.0f;

		VkPipelineColorBlendAttachmentState blendAttachmentState{};
		blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		blendAttachmentState.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlendStateCI{};
		colorBlendStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendStateCI.attachmentCount = 1;
		colorBlendStateCI.pAttachments = &blendAttachmentState;

		VkPipelineDepthStencilStateCreateInfo depthStencilStateCI{};
		depthStencilStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilStateCI.depthTestEnable = VK_FALSE;
		depthStencilStateCI.depthWriteEnable = VK_FALSE;
		depthStencilStateCI.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		depthStencilStateCI.front = depthStencilStateCI.back;
		depthStencilStateCI.back.compareOp = VK_COMPARE_OP_ALWAYS;

		VkPipelineViewportStateCreateInfo viewportStateCI{};
		viewportStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportStateCI.viewportCount = 1;
		viewportStateCI.scissorCount = 1;

		VkPipelineMultisampleStateCreateInfo multisampleStateCI{};
		multisampleStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;

		if (settings.multiSampling)
		{
			multisampleStateCI.rasterizationSamples = VK_SAMPLE_COUNT_4_BIT;
		}

		std::vector<VkDynamicState> dynamicStateEnables = {
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR};
		VkPipelineDynamicStateCreateInfo dynamicStateCI{};
		dynamicStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicStateCI.pDynamicStates = dynamicStateEnables.data();
		dynamicStateCI.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());

		// Pipeline layout
		const std::vector<VkDescriptorSetLayout> setLayouts = {
			descriptorSetLayouts.scene, descriptorSetLayouts.material, descriptorSetLayouts.node};
		VkPipelineLayoutCreateInfo pipelineLayoutCI{};
		pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCI.setLayoutCount = static_cast<uint32_t>(setLayouts.size());
		pipelineLayoutCI.pSetLayouts = setLayouts.data();
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.size = sizeof(PushConstBlockMaterial);
		pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		pipelineLayoutCI.pushConstantRangeCount = 1;
		pipelineLayoutCI.pPushConstantRanges = &pushConstantRange;
		VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutCI, nullptr, &pipelineLayout));

		// Vertex bindings an attributes
		VkVertexInputBindingDescription vertexInputBinding = {0, sizeof(oceangltf::Model::Vertex), VK_VERTEX_INPUT_RATE_VERTEX};
		std::vector<VkVertexInputAttributeDescription> vertexInputAttributes = {
			{0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0},
			{1, 0, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 3},
			{2, 0, VK_FORMAT_R32G32_SFLOAT, sizeof(float) * 6},
			{3, 0, VK_FORMAT_R32G32_SFLOAT, sizeof(float) * 8},
			{4, 0, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 10},
			{5, 0, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 14},
			{6, 0, VK_FORMAT_R32G32B32A32_SFLOAT, sizeof(float) * 18}};
		VkPipelineVertexInputStateCreateInfo vertexInputStateCI{};
		vertexInputStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputStateCI.vertexBindingDescriptionCount = 1;
		vertexInputStateCI.pVertexBindingDescriptions = &vertexInputBinding;
		vertexInputStateCI.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexInputAttributes.size());
		vertexInputStateCI.pVertexAttributeDescriptions = vertexInputAttributes.data();

		// Pipelines
		std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;

		VkGraphicsPipelineCreateInfo pipelineCI{};
		pipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineCI.layout = pipelineLayout;
		pipelineCI.renderPass = renderPass;
		pipelineCI.pInputAssemblyState = &inputAssemblyStateCI;
		pipelineCI.pVertexInputState = &vertexInputStateCI;
		pipelineCI.pRasterizationState = &rasterizationStateCI;
		pipelineCI.pColorBlendState = &colorBlendStateCI;
		pipelineCI.pMultisampleState = &multisampleStateCI;
		pipelineCI.pViewportState = &viewportStateCI;
		pipelineCI.pDepthStencilState = &depthStencilStateCI;
		pipelineCI.pDynamicState = &dynamicStateCI;
		pipelineCI.stageCount = static_cast<uint32_t>(shaderStages.size());
		pipelineCI.pStages = shaderStages.data();

		if (settings.multiSampling)
		{
			multisampleStateCI.rasterizationSamples = VK_SAMPLE_COUNT_4_BIT;
		}

		// Skybox pipeline (background cube)
		shaderStages = {
			oceanutil::loadShader(device, "skybox.vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
			oceanutil::loadShader(device, "skybox.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT)};
		VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCI,
												  nullptr, &pipelines.skybox));
		for (auto &shaderStage : shaderStages)
		{
			vkDestroyShaderModule(device, shaderStage.module, nullptr);
		}

		// PBR pipeline
		shaderStages = {
			oceanutil::loadShader(device, "pbr.vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
			oceanutil::loadShader(device, "pbr_khr.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT)};
		depthStencilStateCI.depthWriteEnable = VK_TRUE;
		depthStencilStateCI.depthTestEnable = VK_TRUE;
		VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCI, nullptr,
												  &pipelines.pbr));
		rasterizationStateCI.cullMode = VK_CULL_MODE_NONE;
		VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCI, nullptr,
												  &pipelines.pbrDoubleSided));

		rasterizationStateCI.cullMode = VK_CULL_MODE_NONE;
		blendAttachmentState.blendEnable = VK_TRUE;
		blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
		blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		blendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
		VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCI, nullptr, &pipelines.pbrAlphaBlend));

		for (const auto &shaderStage : shaderStages)
		{
			vkDestroyShaderModule(device, shaderStage.module, nullptr);
		}
	}

	/*
		Generate a BRDF integration map storing roughness/NdotV as a look-up-table
	*/
	void OceanPbrApp::generateBRDFLUT()
	{
		auto tStart = std::chrono::high_resolution_clock::now();

		const VkFormat format = VK_FORMAT_R16G16_SFLOAT;
		const int32_t dim = 512;

		// Image
		VkImageCreateInfo imageCI{};
		imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCI.imageType = VK_IMAGE_TYPE_2D;
		imageCI.format = format;
		imageCI.extent.width = dim;
		imageCI.extent.height = dim;
		imageCI.extent.depth = 1;
		imageCI.mipLevels = 1;
		imageCI.arrayLayers = 1;
		imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCI.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		VK_CHECK_RESULT(vkCreateImage(device, &imageCI, nullptr, &textures.lutBrdf.image));
		VkMemoryRequirements memReqs;
		vkGetImageMemoryRequirements(device, textures.lutBrdf.image, &memReqs);
		VkMemoryAllocateInfo memAllocInfo{};
		memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memAllocInfo.allocationSize = memReqs.size;
		memAllocInfo.memoryTypeIndex = vulkanDevice->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		VK_CHECK_RESULT(vkAllocateMemory(device, &memAllocInfo, nullptr, &textures.lutBrdf.deviceMemory));
		VK_CHECK_RESULT(vkBindImageMemory(device, textures.lutBrdf.image, textures.lutBrdf.deviceMemory, 0));

		// View
		VkImageViewCreateInfo viewCI{};
		viewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewCI.format = format;
		viewCI.subresourceRange = {};
		viewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewCI.subresourceRange.levelCount = 1;
		viewCI.subresourceRange.layerCount = 1;
		viewCI.image = textures.lutBrdf.image;
		VK_CHECK_RESULT(vkCreateImageView(device, &viewCI, nullptr, &textures.lutBrdf.view));

		// Sampler
		VkSamplerCreateInfo samplerCI{};
		samplerCI.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerCI.magFilter = VK_FILTER_LINEAR;
		samplerCI.minFilter = VK_FILTER_LINEAR;
		samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerCI.minLod = 0.0f;
		samplerCI.maxLod = 1.0f;
		samplerCI.maxAnisotropy = 1.0f;
		samplerCI.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		VK_CHECK_RESULT(vkCreateSampler(device, &samplerCI, nullptr, &textures.lutBrdf.sampler));

		// FB, Att, RP, Pipe, etc.
		VkAttachmentDescription attDesc{};
		// Color attachment
		attDesc.format = format;
		attDesc.samples = VK_SAMPLE_COUNT_1_BIT;
		attDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		attDesc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		VkAttachmentReference colorReference = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

		VkSubpassDescription subpassDescription{};
		subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpassDescription.colorAttachmentCount = 1;
		subpassDescription.pColorAttachments = &colorReference;

		// Use subpass dependencies for layout transitions
		std::array<VkSubpassDependency, 2> dependencies;
		dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[0].dstSubpass = 0;
		dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
		dependencies[1].srcSubpass = 0;
		dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
		dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

		// Create the actual renderpass
		VkRenderPassCreateInfo renderPassCI{};
		renderPassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassCI.attachmentCount = 1;
		renderPassCI.pAttachments = &attDesc;
		renderPassCI.subpassCount = 1;
		renderPassCI.pSubpasses = &subpassDescription;
		renderPassCI.dependencyCount = 2;
		renderPassCI.pDependencies = dependencies.data();

		VkRenderPass renderpass;
		VK_CHECK_RESULT(vkCreateRenderPass(device, &renderPassCI, nullptr, &renderpass));

		VkFramebufferCreateInfo framebufferCI{};
		framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCI.renderPass = renderpass;
		framebufferCI.attachmentCount = 1;
		framebufferCI.pAttachments = &textures.lutBrdf.view;
		framebufferCI.width = dim;
		framebufferCI.height = dim;
		framebufferCI.layers = 1;

		VkFramebuffer framebuffer;
		VK_CHECK_RESULT(vkCreateFramebuffer(device, &framebufferCI, nullptr, &framebuffer));

		// Desriptors
		VkDescriptorSetLayout descriptorsetlayout;
		VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI{};
		descriptorSetLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCI, nullptr, &descriptorsetlayout));

		// Pipeline layout
		VkPipelineLayout pipelinelayout;
		VkPipelineLayoutCreateInfo pipelineLayoutCI{};
		pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCI.setLayoutCount = 1;
		pipelineLayoutCI.pSetLayouts = &descriptorsetlayout;
		VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutCI, nullptr, &pipelinelayout));

		// Pipeline
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCI{};
		inputAssemblyStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyStateCI.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		VkPipelineRasterizationStateCreateInfo rasterizationStateCI{};
		rasterizationStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizationStateCI.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizationStateCI.cullMode = VK_CULL_MODE_NONE;
		rasterizationStateCI.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizationStateCI.lineWidth = 1.0f;

		VkPipelineColorBlendAttachmentState blendAttachmentState{};
		blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		blendAttachmentState.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlendStateCI{};
		colorBlendStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendStateCI.attachmentCount = 1;
		colorBlendStateCI.pAttachments = &blendAttachmentState;

		VkPipelineDepthStencilStateCreateInfo depthStencilStateCI{};
		depthStencilStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilStateCI.depthTestEnable = VK_FALSE;
		depthStencilStateCI.depthWriteEnable = VK_FALSE;
		depthStencilStateCI.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		depthStencilStateCI.front = depthStencilStateCI.back;
		depthStencilStateCI.back.compareOp = VK_COMPARE_OP_ALWAYS;

		VkPipelineViewportStateCreateInfo viewportStateCI{};
		viewportStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportStateCI.viewportCount = 1;
		viewportStateCI.scissorCount = 1;

		VkPipelineMultisampleStateCreateInfo multisampleStateCI{};
		multisampleStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleStateCI.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		std::vector<VkDynamicState> dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
		VkPipelineDynamicStateCreateInfo dynamicStateCI{};
		dynamicStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicStateCI.pDynamicStates = dynamicStateEnables.data();
		dynamicStateCI.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());

		VkPipelineVertexInputStateCreateInfo emptyInputStateCI{};
		emptyInputStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

		std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;

		VkGraphicsPipelineCreateInfo pipelineCI{};
		pipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineCI.layout = pipelinelayout;
		pipelineCI.renderPass = renderpass;
		pipelineCI.pInputAssemblyState = &inputAssemblyStateCI;
		pipelineCI.pVertexInputState = &emptyInputStateCI;
		pipelineCI.pRasterizationState = &rasterizationStateCI;
		pipelineCI.pColorBlendState = &colorBlendStateCI;
		pipelineCI.pMultisampleState = &multisampleStateCI;
		pipelineCI.pViewportState = &viewportStateCI;
		pipelineCI.pDepthStencilState = &depthStencilStateCI;
		pipelineCI.pDynamicState = &dynamicStateCI;
		pipelineCI.stageCount = 2;
		pipelineCI.pStages = shaderStages.data();

		// Look-up-table (from BRDF) pipeline
		shaderStages = {
			oceanutil::loadShader(device, "genbrdflut.vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
			oceanutil::loadShader(device, "genbrdflut.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT)};
		VkPipeline pipeline;
		VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCI, nullptr, &pipeline));
		for (auto shaderStage : shaderStages)
		{
			vkDestroyShaderModule(device, shaderStage.module, nullptr);
		}

		// Render
		VkClearValue clearValues[1];
		clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};

		VkRenderPassBeginInfo renderPassBeginInfo{};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = renderpass;
		renderPassBeginInfo.renderArea.extent.width = dim;
		renderPassBeginInfo.renderArea.extent.height = dim;
		renderPassBeginInfo.clearValueCount = 1;
		renderPassBeginInfo.pClearValues = clearValues;
		renderPassBeginInfo.framebuffer = framebuffer;

		VkCommandBuffer cmdBuf = vulkanDevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
		vkCmdBeginRenderPass(cmdBuf, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.width = (float)dim;
		viewport.height = (float)dim;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		VkRect2D scissor{};
		scissor.extent.width = dim;
		scissor.extent.height = dim;

		vkCmdSetViewport(cmdBuf, 0, 1, &viewport);
		vkCmdSetScissor(cmdBuf, 0, 1, &scissor);
		vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		vkCmdDraw(cmdBuf, 3, 1, 0, 0);
		vkCmdEndRenderPass(cmdBuf);
		vulkanDevice->flushCommandBuffer(cmdBuf, queue);

		vkQueueWaitIdle(queue);

		vkDestroyPipeline(device, pipeline, nullptr);
		vkDestroyPipelineLayout(device, pipelinelayout, nullptr);
		vkDestroyRenderPass(device, renderpass, nullptr);
		vkDestroyFramebuffer(device, framebuffer, nullptr);
		vkDestroyDescriptorSetLayout(device, descriptorsetlayout, nullptr);

		textures.lutBrdf.descriptor.imageView = textures.lutBrdf.view;
		textures.lutBrdf.descriptor.sampler = textures.lutBrdf.sampler;
		textures.lutBrdf.descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		textures.lutBrdf.device = vulkanDevice;

		auto tEnd = std::chrono::high_resolution_clock::now();
		auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
		ocean::info("Generating BRDF LUT took " + std::to_string(tDiff) + " ms");
	}

	/*
		Offline generation for the cube maps used for PBR lighting
		- Irradiance cube map
		- Pre-filterd environment cubemap
	*/
	void OceanPbrApp::generateCubemaps()
	{
		enum Target
		{
			IRRADIANCE = 0,
			PREFILTEREDENV = 1
		};

		for (uint32_t target = 0; target < PREFILTEREDENV + 1; target++)
		{

			oceantexture::TextureCubeMap cubemap;

			auto tStart = std::chrono::high_resolution_clock::now();

			VkFormat format;
			int32_t dim;

			switch (target)
			{
			case IRRADIANCE:
				format = VK_FORMAT_R32G32B32A32_SFLOAT;
				dim = 64;
				break;
			case PREFILTEREDENV:
				format = VK_FORMAT_R16G16B16A16_SFLOAT;
				dim = 512;
				break;
			};

			const uint32_t numMips = static_cast<uint32_t>(floor(log2(dim))) + 1;

			// Create target cubemap
			{
				// Image
				VkImageCreateInfo imageCI{};
				imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
				imageCI.imageType = VK_IMAGE_TYPE_2D;
				imageCI.format = format;
				imageCI.extent.width = dim;
				imageCI.extent.height = dim;
				imageCI.extent.depth = 1;
				imageCI.mipLevels = numMips;
				imageCI.arrayLayers = 6;
				imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
				imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
				imageCI.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
				imageCI.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
				VkResult a = vkCreateImage(device, &imageCI, nullptr, &cubemap.image);
				VK_CHECK_RESULT(a);
				VkMemoryRequirements memReqs;
				vkGetImageMemoryRequirements(device, cubemap.image, &memReqs);
				VkMemoryAllocateInfo memAllocInfo{};
				memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
				memAllocInfo.allocationSize = memReqs.size;
				memAllocInfo.memoryTypeIndex = vulkanDevice->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
				VK_CHECK_RESULT(vkAllocateMemory(device, &memAllocInfo, nullptr, &cubemap.deviceMemory));
				VK_CHECK_RESULT(vkBindImageMemory(device, cubemap.image, cubemap.deviceMemory, 0));

				// View
				VkImageViewCreateInfo viewCI{};
				viewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				viewCI.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
				viewCI.format = format;
				viewCI.subresourceRange = {};
				viewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				viewCI.subresourceRange.levelCount = numMips;
				viewCI.subresourceRange.layerCount = 6;
				viewCI.image = cubemap.image;
				VK_CHECK_RESULT(vkCreateImageView(device, &viewCI, nullptr, &cubemap.view));

				// Sampler
				VkSamplerCreateInfo samplerCI{};
				samplerCI.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
				samplerCI.magFilter = VK_FILTER_LINEAR;
				samplerCI.minFilter = VK_FILTER_LINEAR;
				samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
				samplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
				samplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
				samplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
				samplerCI.minLod = 0.0f;
				samplerCI.maxLod = static_cast<float>(numMips);
				samplerCI.maxAnisotropy = 1.0f;
				samplerCI.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
				VK_CHECK_RESULT(vkCreateSampler(device, &samplerCI, nullptr, &cubemap.sampler));
			}

			// FB, Att, RP, Pipe, etc.
			VkAttachmentDescription attDesc{};
			// Color attachment
			attDesc.format = format;
			attDesc.samples = VK_SAMPLE_COUNT_1_BIT;
			attDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attDesc.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attDesc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			VkAttachmentReference colorReference = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

			VkSubpassDescription subpassDescription{};
			subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpassDescription.colorAttachmentCount = 1;
			subpassDescription.pColorAttachments = &colorReference;

			// Use subpass dependencies for layout transitions
			std::array<VkSubpassDependency, 2> dependencies;
			dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[0].dstSubpass = 0;
			dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
			dependencies[1].srcSubpass = 0;
			dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			// Renderpass
			VkRenderPassCreateInfo renderPassCI{};
			renderPassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassCI.attachmentCount = 1;
			renderPassCI.pAttachments = &attDesc;
			renderPassCI.subpassCount = 1;
			renderPassCI.pSubpasses = &subpassDescription;
			renderPassCI.dependencyCount = 2;
			renderPassCI.pDependencies = dependencies.data();
			VkRenderPass renderpass;
			VK_CHECK_RESULT(vkCreateRenderPass(device, &renderPassCI, nullptr, &renderpass));

			struct Offscreen
			{
				VkImage image;
				VkImageView view;
				VkDeviceMemory memory;
				VkFramebuffer framebuffer;
			} offscreen;

			// Create offscreen framebuffer
			{
				// Image
				VkImageCreateInfo imageCI{};
				imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
				imageCI.imageType = VK_IMAGE_TYPE_2D;
				imageCI.format = format;
				imageCI.extent.width = dim;
				imageCI.extent.height = dim;
				imageCI.extent.depth = 1;
				imageCI.mipLevels = 1;
				imageCI.arrayLayers = 1;
				imageCI.samples = VK_SAMPLE_COUNT_1_BIT;
				imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
				imageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				imageCI.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
				imageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
				VK_CHECK_RESULT(vkCreateImage(device, &imageCI, nullptr, &offscreen.image));
				VkMemoryRequirements memReqs;
				vkGetImageMemoryRequirements(device, offscreen.image, &memReqs);
				VkMemoryAllocateInfo memAllocInfo{};
				memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
				memAllocInfo.allocationSize = memReqs.size;
				memAllocInfo.memoryTypeIndex = vulkanDevice->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
				VK_CHECK_RESULT(vkAllocateMemory(device, &memAllocInfo, nullptr, &offscreen.memory));
				VK_CHECK_RESULT(vkBindImageMemory(device, offscreen.image, offscreen.memory, 0));

				// View
				VkImageViewCreateInfo viewCI{};
				viewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				viewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
				viewCI.format = format;
				viewCI.flags = 0;
				viewCI.subresourceRange = {};
				viewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				viewCI.subresourceRange.baseMipLevel = 0;
				viewCI.subresourceRange.levelCount = 1;
				viewCI.subresourceRange.baseArrayLayer = 0;
				viewCI.subresourceRange.layerCount = 1;
				viewCI.image = offscreen.image;
				VK_CHECK_RESULT(vkCreateImageView(device, &viewCI, nullptr, &offscreen.view));

				// Framebuffer
				VkFramebufferCreateInfo framebufferCI{};
				framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				framebufferCI.renderPass = renderpass;
				framebufferCI.attachmentCount = 1;
				framebufferCI.pAttachments = &offscreen.view;
				framebufferCI.width = dim;
				framebufferCI.height = dim;
				framebufferCI.layers = 1;
				VK_CHECK_RESULT(vkCreateFramebuffer(device, &framebufferCI, nullptr, &offscreen.framebuffer));

				VkCommandBuffer layoutCmd = vulkanDevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
				VkImageMemoryBarrier imageMemoryBarrier{};
				imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				imageMemoryBarrier.image = offscreen.image;
				imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				imageMemoryBarrier.srcAccessMask = 0;
				imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				imageMemoryBarrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
				vkCmdPipelineBarrier(layoutCmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
				vulkanDevice->flushCommandBuffer(layoutCmd, queue, true);
			}

			// Descriptors
			VkDescriptorSetLayout descriptorsetlayout;
			VkDescriptorSetLayoutBinding setLayoutBinding = {0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, VK_SHADER_STAGE_FRAGMENT_BIT, nullptr};
			VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCI{};
			descriptorSetLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			descriptorSetLayoutCI.pBindings = &setLayoutBinding;
			descriptorSetLayoutCI.bindingCount = 1;
			VK_CHECK_RESULT(vkCreateDescriptorSetLayout(device, &descriptorSetLayoutCI, nullptr, &descriptorsetlayout));

			// Descriptor Pool
			VkDescriptorPoolSize poolSize = {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1};
			VkDescriptorPoolCreateInfo descriptorPoolCI{};
			descriptorPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			descriptorPoolCI.poolSizeCount = 1;
			descriptorPoolCI.pPoolSizes = &poolSize;
			descriptorPoolCI.maxSets = 2;
			VkDescriptorPool descriptorpool;
			VK_CHECK_RESULT(vkCreateDescriptorPool(device, &descriptorPoolCI, nullptr, &descriptorpool));

			// Descriptor sets
			VkDescriptorSet descriptorset;
			VkDescriptorSetAllocateInfo descriptorSetAllocInfo{};
			descriptorSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			descriptorSetAllocInfo.descriptorPool = descriptorpool;
			descriptorSetAllocInfo.pSetLayouts = &descriptorsetlayout;
			descriptorSetAllocInfo.descriptorSetCount = 1;
			VK_CHECK_RESULT(vkAllocateDescriptorSets(device, &descriptorSetAllocInfo, &descriptorset));
			VkWriteDescriptorSet writeDescriptorSet{};
			writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			writeDescriptorSet.descriptorCount = 1;
			writeDescriptorSet.dstSet = descriptorset;
			writeDescriptorSet.dstBinding = 0;
			writeDescriptorSet.pImageInfo = &textures.environmentCube.descriptor;
			vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, nullptr);

			struct PushBlockIrradiance
			{
				glm::mat4 mvp;
				float deltaPhi = (2.0f * float(M_PI)) / 180.0f;
				float deltaTheta = (0.5f * float(M_PI)) / 64.0f;
			} pushBlockIrradiance;

			struct PushBlockPrefilterEnv
			{
				glm::mat4 mvp;
				float roughness;
				uint32_t numSamples = 32u;
			} pushBlockPrefilterEnv;

			// Pipeline layout
			VkPipelineLayout pipelinelayout;
			VkPushConstantRange pushConstantRange{};
			pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

			switch (target)
			{
			case IRRADIANCE:
				pushConstantRange.size = sizeof(PushBlockIrradiance);
				break;
			case PREFILTEREDENV:
				pushConstantRange.size = sizeof(PushBlockPrefilterEnv);
				break;
			};

			VkPipelineLayoutCreateInfo pipelineLayoutCI{};
			pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipelineLayoutCI.setLayoutCount = 1;
			pipelineLayoutCI.pSetLayouts = &descriptorsetlayout;
			pipelineLayoutCI.pushConstantRangeCount = 1;
			pipelineLayoutCI.pPushConstantRanges = &pushConstantRange;
			VK_CHECK_RESULT(vkCreatePipelineLayout(device, &pipelineLayoutCI, nullptr, &pipelinelayout));

			// Pipeline
			VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCI{};
			inputAssemblyStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			inputAssemblyStateCI.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

			VkPipelineRasterizationStateCreateInfo rasterizationStateCI{};
			rasterizationStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			rasterizationStateCI.polygonMode = VK_POLYGON_MODE_FILL;
			rasterizationStateCI.cullMode = VK_CULL_MODE_NONE;
			rasterizationStateCI.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
			rasterizationStateCI.lineWidth = 1.0f;

			VkPipelineColorBlendAttachmentState blendAttachmentState{};
			blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			blendAttachmentState.blendEnable = VK_FALSE;

			VkPipelineColorBlendStateCreateInfo colorBlendStateCI{};
			colorBlendStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			colorBlendStateCI.attachmentCount = 1;
			colorBlendStateCI.pAttachments = &blendAttachmentState;

			VkPipelineDepthStencilStateCreateInfo depthStencilStateCI{};
			depthStencilStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			depthStencilStateCI.depthTestEnable = VK_FALSE;
			depthStencilStateCI.depthWriteEnable = VK_FALSE;
			depthStencilStateCI.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
			depthStencilStateCI.front = depthStencilStateCI.back;
			depthStencilStateCI.back.compareOp = VK_COMPARE_OP_ALWAYS;

			VkPipelineViewportStateCreateInfo viewportStateCI{};
			viewportStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			viewportStateCI.viewportCount = 1;
			viewportStateCI.scissorCount = 1;

			VkPipelineMultisampleStateCreateInfo multisampleStateCI{};
			multisampleStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			multisampleStateCI.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

			std::vector<VkDynamicState> dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
			VkPipelineDynamicStateCreateInfo dynamicStateCI{};
			dynamicStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
			dynamicStateCI.pDynamicStates = dynamicStateEnables.data();
			dynamicStateCI.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());

			// Vertex input state
			VkVertexInputBindingDescription vertexInputBinding = {0, sizeof(oceangltf::Model::Vertex), VK_VERTEX_INPUT_RATE_VERTEX};
			VkVertexInputAttributeDescription vertexInputAttribute = {0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0};

			VkPipelineVertexInputStateCreateInfo vertexInputStateCI{};
			vertexInputStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			vertexInputStateCI.vertexBindingDescriptionCount = 1;
			vertexInputStateCI.pVertexBindingDescriptions = &vertexInputBinding;
			vertexInputStateCI.vertexAttributeDescriptionCount = 1;
			vertexInputStateCI.pVertexAttributeDescriptions = &vertexInputAttribute;

			std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;

			VkGraphicsPipelineCreateInfo pipelineCI{};
			pipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			pipelineCI.layout = pipelinelayout;
			pipelineCI.renderPass = renderpass;
			pipelineCI.pInputAssemblyState = &inputAssemblyStateCI;
			pipelineCI.pVertexInputState = &vertexInputStateCI;
			pipelineCI.pRasterizationState = &rasterizationStateCI;
			pipelineCI.pColorBlendState = &colorBlendStateCI;
			pipelineCI.pMultisampleState = &multisampleStateCI;
			pipelineCI.pViewportState = &viewportStateCI;
			pipelineCI.pDepthStencilState = &depthStencilStateCI;
			pipelineCI.pDynamicState = &dynamicStateCI;
			pipelineCI.stageCount = 2;
			pipelineCI.pStages = shaderStages.data();
			pipelineCI.renderPass = renderpass;

			shaderStages[0] = oceanutil::loadShader(device, "filtercube.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
			switch (target)
			{
			case IRRADIANCE:
				shaderStages[1] = oceanutil::loadShader(device, "irradiancecube.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
				break;
			case PREFILTEREDENV:
				shaderStages[1] = oceanutil::loadShader(device, "prefilterenvmap.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
				break;
			};
			VkPipeline pipeline;
			VK_CHECK_RESULT(vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipelineCI, nullptr, &pipeline));
			for (auto shaderStage : shaderStages)
			{
				vkDestroyShaderModule(device, shaderStage.module, nullptr);
			}

			// Render cubemap
			VkClearValue clearValues[1];
			clearValues[0].color = {{0.0f, 0.0f, 0.2f, 0.0f}};

			VkRenderPassBeginInfo renderPassBeginInfo{};
			renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassBeginInfo.renderPass = renderpass;
			renderPassBeginInfo.framebuffer = offscreen.framebuffer;
			renderPassBeginInfo.renderArea.extent.width = dim;
			renderPassBeginInfo.renderArea.extent.height = dim;
			renderPassBeginInfo.clearValueCount = 1;
			renderPassBeginInfo.pClearValues = clearValues;

			std::vector<glm::mat4> matrices = {
				glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
				glm::rotate(glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f)), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
				glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
				glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
				glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
				glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
			};

			VkCommandBuffer cmdBuf = vulkanDevice->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, false);

			VkViewport viewport{};
			viewport.width = (float)dim;
			viewport.height = (float)dim;
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;

			VkRect2D scissor{};
			scissor.extent.width = dim;
			scissor.extent.height = dim;

			VkImageSubresourceRange subresourceRange{};
			subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			subresourceRange.baseMipLevel = 0;
			subresourceRange.levelCount = numMips;
			subresourceRange.layerCount = 6;

			// Change image layout for all cubemap faces to transfer destination
			{
				vulkanDevice->beginCommandBuffer(cmdBuf);
				VkImageMemoryBarrier imageMemoryBarrier{};
				imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				imageMemoryBarrier.image = cubemap.image;
				imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				imageMemoryBarrier.srcAccessMask = 0;
				imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				imageMemoryBarrier.subresourceRange = subresourceRange;
				vkCmdPipelineBarrier(cmdBuf, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
				vulkanDevice->flushCommandBuffer(cmdBuf, queue, false);
			}

			for (uint32_t m = 0; m < numMips; m++)
			{
				for (uint32_t f = 0; f < 6; f++)
				{

					vulkanDevice->beginCommandBuffer(cmdBuf);

					viewport.width = static_cast<float>(dim * std::pow(0.5f, m));
					viewport.height = static_cast<float>(dim * std::pow(0.5f, m));
					vkCmdSetViewport(cmdBuf, 0, 1, &viewport);
					vkCmdSetScissor(cmdBuf, 0, 1, &scissor);

					// Render scene from cube face's point of view
					vkCmdBeginRenderPass(cmdBuf, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

					// Pass parameters for current pass using a push constant block
					switch (target)
					{
					case IRRADIANCE:
						pushBlockIrradiance.mvp = glm::perspective((float)(M_PI / 2.0), 1.0f, 0.1f, 512.0f) * matrices[f];
						vkCmdPushConstants(cmdBuf, pipelinelayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushBlockIrradiance), &pushBlockIrradiance);
						break;
					case PREFILTEREDENV:
						pushBlockPrefilterEnv.mvp = glm::perspective((float)(M_PI / 2.0), 1.0f, 0.1f, 512.0f) * matrices[f];
						pushBlockPrefilterEnv.roughness = (float)m / (float)(numMips - 1);
						vkCmdPushConstants(cmdBuf, pipelinelayout, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushBlockPrefilterEnv), &pushBlockPrefilterEnv);
						break;
					};

					vkCmdBindPipeline(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
					vkCmdBindDescriptorSets(cmdBuf, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelinelayout, 0, 1, &descriptorset, 0, NULL);

					VkDeviceSize offsets[1] = {0};

					pbrModels.skybox.draw(cmdBuf);

					vkCmdEndRenderPass(cmdBuf);

					VkImageSubresourceRange subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
					subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					subresourceRange.baseMipLevel = 0;
					subresourceRange.levelCount = numMips;
					subresourceRange.layerCount = 6;

					{
						VkImageMemoryBarrier imageMemoryBarrier{};
						imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
						imageMemoryBarrier.image = offscreen.image;
						imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
						imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
						imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
						imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
						imageMemoryBarrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
						vkCmdPipelineBarrier(cmdBuf, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
					}

					// Copy region for transfer from framebuffer to cube face
					VkImageCopy copyRegion{};

					copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					copyRegion.srcSubresource.baseArrayLayer = 0;
					copyRegion.srcSubresource.mipLevel = 0;
					copyRegion.srcSubresource.layerCount = 1;
					copyRegion.srcOffset = {0, 0, 0};

					copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					copyRegion.dstSubresource.baseArrayLayer = f;
					copyRegion.dstSubresource.mipLevel = m;
					copyRegion.dstSubresource.layerCount = 1;
					copyRegion.dstOffset = {0, 0, 0};

					copyRegion.extent.width = static_cast<uint32_t>(viewport.width);
					copyRegion.extent.height = static_cast<uint32_t>(viewport.height);
					copyRegion.extent.depth = 1;

					vkCmdCopyImage(
						cmdBuf,
						offscreen.image,
						VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
						cubemap.image,
						VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
						1,
						&copyRegion);

					{
						VkImageMemoryBarrier imageMemoryBarrier{};
						imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
						imageMemoryBarrier.image = offscreen.image;
						imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
						imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
						imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
						imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
						imageMemoryBarrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
						vkCmdPipelineBarrier(cmdBuf, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
					}

					vulkanDevice->flushCommandBuffer(cmdBuf, queue, false);
				}
			}

			{
				vulkanDevice->beginCommandBuffer(cmdBuf);
				VkImageMemoryBarrier imageMemoryBarrier{};
				imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				imageMemoryBarrier.image = cubemap.image;
				imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				imageMemoryBarrier.dstAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
				imageMemoryBarrier.subresourceRange = subresourceRange;
				vkCmdPipelineBarrier(cmdBuf, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &imageMemoryBarrier);
				vulkanDevice->flushCommandBuffer(cmdBuf, queue, false);
			}

			vkDestroyRenderPass(device, renderpass, nullptr);
			vkDestroyFramebuffer(device, offscreen.framebuffer, nullptr);
			vkFreeMemory(device, offscreen.memory, nullptr);
			vkDestroyImageView(device, offscreen.view, nullptr);
			vkDestroyImage(device, offscreen.image, nullptr);
			vkDestroyDescriptorPool(device, descriptorpool, nullptr);
			vkDestroyDescriptorSetLayout(device, descriptorsetlayout, nullptr);
			vkDestroyPipeline(device, pipeline, nullptr);
			vkDestroyPipelineLayout(device, pipelinelayout, nullptr);

			cubemap.descriptor.imageView = cubemap.view;
			cubemap.descriptor.sampler = cubemap.sampler;
			cubemap.descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			cubemap.device = vulkanDevice;

			switch (target)
			{
			case IRRADIANCE:
				textures.irradianceCube = cubemap;
				break;
			case PREFILTEREDENV:
				textures.prefilteredCube = cubemap;
				shaderValuesParams.prefilteredCubeMipLevels = static_cast<float>(numMips);
				break;
			};

			auto tEnd = std::chrono::high_resolution_clock::now();
			auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
			ocean::info("Generating cube map with " + std::to_string(numMips) + " mip levels took " + std::to_string(tDiff) + " ms");
		}
	}

	/*
		Prepare and initialize uniform buffers containing shader parameters
	*/
	void OceanPbrApp::prepareUniformBuffers()
	{
		for (auto &uniformBuffer : uniformBuffers)
		{
			uniformBuffer.scene.create(vulkanDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, sizeof(shaderValuesScene));
			uniformBuffer.skybox.create(vulkanDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, sizeof(shaderValuesSkybox));
			uniformBuffer.params.create(vulkanDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, sizeof(shaderValuesParams));

			// uniformBuffer.gltf2.create(vulkanDevice, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, sizeof(shaderValuesScene));
		}
		updateUniformBuffers();
	}

	void OceanPbrApp::updateUniformBuffers()
	{
		// Scene
		shaderValuesScene.projection = camera.matrices.perspective;
		shaderValuesScene.view = camera.matrices.view;

		// Center and scale model

		// float scale = (1.0f / std::max(pbrModels.scene.aabb[0][0], std::max(pbrModels.scene.aabb[1][1], pbrModels.scene.aabb[2][2]))) * 0.5f;
		// glm::vec3 translate = -glm::vec3(pbrModels.scene.aabb[3][0], pbrModels.scene.aabb[3][1], pbrModels.scene.aabb[3][2]);
		// translate += -0.5f * glm::vec3(pbrModels.scene.aabb[0][0], pbrModels.scene.aabb[1][1], pbrModels.scene.aabb[2][2]);

		shaderValuesScene.model = glm::mat4(1.0f);
		if (centerAndScaleModel)
		{
			float scale = (1.0f / (std::max)(pbrModels.scene.aabb[0][0], (std::max)(pbrModels.scene.aabb[1][1], pbrModels.scene.aabb[2][2]))) * 0.5f;
			glm::vec3 translate = -glm::vec3(pbrModels.scene.aabb[3][0], pbrModels.scene.aabb[3][1], pbrModels.scene.aabb[3][2]);
			translate += -0.5f * glm::vec3(pbrModels.scene.aabb[0][0], pbrModels.scene.aabb[1][1], pbrModels.scene.aabb[2][2]);
			shaderValuesScene.model[0][0] = scale;
			shaderValuesScene.model[1][1] = scale;
			shaderValuesScene.model[2][2] = scale;
			shaderValuesScene.model = glm::translate(shaderValuesScene.model, translate);
		}

		shaderValuesScene.camPos = glm::vec3(
			-camera.position.z * sin(glm::radians(camera.rotation.y)) * cos(glm::radians(camera.rotation.x)),
			-camera.position.z * sin(glm::radians(camera.rotation.x)),
			camera.position.z * cos(glm::radians(camera.rotation.y)) * cos(glm::radians(camera.rotation.x)));

		// Skybox
		shaderValuesSkybox.projection = camera.matrices.perspective;
		shaderValuesSkybox.view = camera.matrices.view;
		shaderValuesSkybox.model = glm::mat4(glm::mat3(camera.matrices.view));
	}

	void OceanPbrApp::updateParams()
	{
		shaderValuesParams.lightDir = glm::vec4(
			sin(glm::radians(lightSource.rotation.x)) * cos(glm::radians(lightSource.rotation.y)),
			sin(glm::radians(lightSource.rotation.y)),
			cos(glm::radians(lightSource.rotation.x)) * cos(glm::radians(lightSource.rotation.y)),
			0.0f);
	}

	void OceanPbrApp::windowResized()
	{
		recordCommandBuffers();
		vkDeviceWaitIdle(device);
		updateUniformBuffers();
		updateOverlay();
	}

	void OceanPbrApp::updateOverlay()
	{
		// Initialize ImGui IO for all UI elements
		ImGuiIO &io = ImGui::GetIO();
		io.DisplaySize = ImVec2((float)width, (float)height);
		io.DeltaTime = frameTimer;
		io.MousePos = ImVec2(mousePos.x, mousePos.y);
		io.MouseDown[0] = mouseButtons.left;
		io.MouseDown[1] = mouseButtons.right;

		ui->pushConstBlock.scale = glm::vec2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
		ui->pushConstBlock.translate = glm::vec2(-1.0f);

		float scale = 1.0f;
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
		scale = (float)vks::android::screenDensity / (float)ACONFIGURATION_DENSITY_MEDIUM;
#endif

		// Always call NewFrame before any ImGui functions
		ImGui::NewFrame();

		// Always show edit mode menu bar if enabled
		if (editModeEnabled) {
			updateStudioModeMenuBar();
		}

		switch (overlayTemplateId)
		{
		case overlayimgui::OverlayTemplate::OVERLAY_NONE:
			// No additional overlays to render
			break;
		case overlayimgui::OverlayTemplate::OVERLAY_DEBUG_PERFORMANCE:
			updateOverlayDebugPerformance();
			break;
		default:
		case overlayimgui::OverlayTemplate::OVERLAY_DEBUG_SHADER:
			updateOverlayDebugShader();
		}

		// Always call Render() exactly once per frame at the end
		ImGui::Render();

		// Handle ImGui draw data processing
		handleImGuiDrawData();
	}

	void OceanPbrApp::handleImGuiDrawData()
	{
		ImDrawData *imDrawData = ImGui::GetDrawData();

		// Check if ui buffers need to be recreated
		if (imDrawData)
		{
			VkDeviceSize vertexBufferSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
			VkDeviceSize indexBufferSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);

			bool updateBuffers = (ui->vertexBuffer.buffer == VK_NULL_HANDLE) || (ui->vertexBuffer.count != imDrawData->TotalVtxCount) || (ui->indexBuffer.buffer == VK_NULL_HANDLE) || (ui->indexBuffer.count != imDrawData->TotalIdxCount);

			if (updateBuffers)
			{
				vkDeviceWaitIdle(device);
				if (ui->vertexBuffer.buffer)
				{
					ui->vertexBuffer.destroy();
				}
				ui->vertexBuffer.create(vulkanDevice, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, vertexBufferSize);
				ui->vertexBuffer.count = imDrawData->TotalVtxCount;
				if (ui->indexBuffer.buffer)
				{
					ui->indexBuffer.destroy();
				}
				ui->indexBuffer.create(vulkanDevice, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, indexBufferSize);
				ui->indexBuffer.count = imDrawData->TotalIdxCount;
			}

			// Upload data
			ImDrawVert *vtxDst = (ImDrawVert *)ui->vertexBuffer.mapped;
			ImDrawIdx *idxDst = (ImDrawIdx *)ui->indexBuffer.mapped;
			for (int n = 0; n < imDrawData->CmdListsCount; n++)
			{
				const ImDrawList *cmd_list = imDrawData->CmdLists[n];
				memcpy(vtxDst, cmd_list->VtxBuffer.Data, cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
				memcpy(idxDst, cmd_list->IdxBuffer.Data, cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
				vtxDst += cmd_list->VtxBuffer.Size;
				idxDst += cmd_list->IdxBuffer.Size;
			}

			ui->vertexBuffer.flush();
			ui->indexBuffer.flush();

			// Check if command buffers need to be updated due to buffer changes
			vkDeviceWaitIdle(device);
			recordCommandBuffers();
		}
	}

	/*
		Update ImGui user interface
	*/
	void OceanPbrApp::updateOverlayDebugShader()
	{
		ImGuiIO &io = ImGui::GetIO();
		ImVec2 lastDisplaySize = io.DisplaySize;

		bool updateShaderParams = false;
		bool updateCBs = false;
		float scale = 1.0f;

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
		scale = (float)vks::android::screenDensity / (float)ACONFIGURATION_DENSITY_MEDIUM;
#endif

		ImGui::SetNextWindowPos(ImVec2(10, 50));
		ImGui::SetNextWindowSize(ImVec2(200 * scale, (pbrModels.scene.animations.size() > 0 ? 440 : 360) * scale), ImGuiSetCond_Always);

		ImGui::Begin("Ocean Renderer", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

		ImGui::PushItemWidth(100.0f * scale);

		if (ui->checkbox("Update Cam", &updateCamOnReceiveRendererDto))
		{
		}

		static char str0[128] = "Hello, world!";
		if (ImGui::InputText("input text", str0, IM_ARRAYSIZE(str0)))
		{
		}

		if (ui->button("Load"))
		{
			loadPageRequest("65a2aaf35058b63992ce78d1", updateCamOnReceiveRendererDto);
		}

		ImGui::SameLine();

		if (ui->button("Save"))
		{
		}

		ImGui::SameLine();

		if (ui->button("Clear"))
		{
			loadPageRequest("6580cf79d59ffd187c3ebe90");
		}

		ui->text("scapestation.com");
		ui->text("%.1d fps (%.2f ms)", lastFPS, (1000.0f / lastFPS));

		if (ui->header("Scene"))
		{
#if defined(VK_USE_PLATFORM_ANDROID_KHR)
			if (ui->combo("File", selectedScene, scenes))
			{
				vkDeviceWaitIdle(device);
				loadScene(scenes[selectedScene]);
				setupDescriptors();
				updateCBs = true;
			}
#else

			/*if (ui->button("Open gltf file"))
			{
				std::string filename = "";
#if defined(_WIN32)

				char buffer[MAX_PATH];
				OPENFILENAME ofn;
				ZeroMemory(&buffer, sizeof(buffer));
				ZeroMemory(&ofn, sizeof(ofn));
				ofn.lStructSize = sizeof(ofn);
				ofn.lpstrFilter = L"glTF files\0*.gltf;*.glb\0";
				ofn.lpstrFile = CA2CT(buffer);
				ofn.nMaxFile = MAX_PATH;
				ofn.lpstrTitle = L"Select a glTF file to load";
				ofn.Flags = OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
				// if (GetOpenFileNameA(&ofn)) {
				//	filename = buffer;
				// }
#elif defined(__linux__) && !defined(VK_USE_PLATFORM_ANDROID_KHR)
				char buffer[1024];
				FILE *file = popen("zenity --title=\"Select a glTF file to load\" --file-filter=\"glTF files | *.gltf *.glb\" --file-selection", "r");
				if (file)
				{
					while (fgets(buffer, sizeof(buffer), file))
					{
						filename += buffer;
					};
					filename.erase(std::remove(filename.begin(), filename.end(), '\n'), filename.end());
					std::cout << filename << std::endl;
				}
#endif
				if (!filename.empty())
				{
					vkDeviceWaitIdle(device);
					loadGltf(0, filename);
					// loadScene(filename);
					setupDescriptors();
					updateCBs = true;
				}
			}*/
#endif
			if (ui->combo("Environment", selectedEnvironment, environments))
			{
				vkDeviceWaitIdle(device);
				loadEnvironment(environments[selectedEnvironment]);
				setupDescriptors();
				updateCBs = true;
			}
		}

		if (ui->header("Environment"))
		{
			if (ui->checkbox("Background", &displayBackground))
			{
				updateShaderParams = true;
			}
			if (ui->slider("Exposure", &shaderValuesParams.exposure, 0.1f, 10.0f))
			{
				updateShaderParams = true;
			}
			if (ui->slider("Gamma", &shaderValuesParams.gamma, 0.1f, 4.0f))
			{
				updateShaderParams = true;
			}
			if (ui->slider("IBL", &shaderValuesParams.scaleIBLAmbient, 0.0f, 1.0f))
			{
				updateShaderParams = true;
			}
		}

		if (ui->header("Debug view"))
		{
			const std::vector<std::string> debugNamesInputs = {
				"none", "Base color", "Normal", "Occlusion", "Emissive", "Metallic", "Roughness"};
			if (ui->combo("Inputs", &debugViewInputs, debugNamesInputs))
			{
				shaderValuesParams.debugViewInputs = static_cast<float>(debugViewInputs);
				updateShaderParams = true;
			}
			const std::vector<std::string> debugNamesEquation = {
				"none", "Diff (l,n)", "F (l,h)", "G (l,v,h)", "D (h)", "Specular"};
			if (ui->combo("PBR equation", &debugViewEquation, debugNamesEquation))
			{
				shaderValuesParams.debugViewEquation = static_cast<float>(debugViewEquation);
				updateShaderParams = true;
			}
		}

		if (pbrModels.scene.animations.size() > 0)
		{
			if (ui->header("Animations"))
			{
				ui->checkbox("Animate", &node_animate);
				std::vector<std::string> animationNames;
				for (auto animation : pbrModels.scene.animations)
				{
					animationNames.push_back(animation.name);
				}
				ui->combo("Animation", &node_animationIndex, animationNames);
			}
		}

		// Render the button with the custom style
		renderStyledButton();

		ImGui::PopItemWidth();
		ImGui::End();

		// overlaycustom::CustomUI::renderImGuiFromProto(layout);

		CallbackFunc lambdaCallback = [this](const std::string& uri) {
			sendFormActionRequest(uri);
		};

		// Call renderImGuiFromProto with the layout and an empty activeItemId
		// TODO: The callback function is not being used here - may need to be integrated differently
		ImStudio::renderImGuiFromProto(layout, "");

		// Draw data is now handled centrally in updateOverlay()

		if (lastDisplaySize.x != io.DisplaySize.x || lastDisplaySize.y != io.DisplaySize.y)
		{
			updateCBs = true;
		}

		if (updateCBs)
		{
			vkDeviceWaitIdle(device);
			recordCommandBuffers();
			vkDeviceWaitIdle(device);
		}

		if (updateShaderParams)
		{
			updateParams();
		}

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
		if (mouseButtons.left)
		{
			mouseButtons.left = false;
		}
#endif
	}
	// Function to simulate a button with the properties from CSS
	void OceanPbrApp::renderStyledButton() {
		// Set button colors based on the CSS
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.6f, 0.86f, 1.0f));  // background-color: #3498db
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.7f, 0.96f, 1.0f));  // hovered color
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.2f, 0.5f, 0.76f, 1.0f));  // active color
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f)); // color: white

		// Set button size based on CSS width and height
		ImGui::PushItemWidth(100);
		ImVec2 buttonSize(100, 50);  // width: 100px, height: 50px

		// Render button
		if (ImGui::Button("Styled Button", buttonSize)) {
			// Button click logic
		}

		// Restore original colors after the button is rendered
		ImGui::PopStyleColor(4);  // Pop all the pushed styles (button + text)
	}
	void OceanPbrApp::updateOverlayDebugPerformance()
	{
		ImGuiIO &io = ImGui::GetIO();
		ImVec2 lastDisplaySize = io.DisplaySize;

		bool updateShaderParams = false;
		bool updateCBs = false;
		float scale = 1.0f;

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
		scale = (float)vks::android::screenDensity / (float)ACONFIGURATION_DENSITY_MEDIUM;
#endif

		ImGui::SetNextWindowPos(ImVec2(10, 50));
		ImGui::SetNextWindowSize(ImVec2(200 * scale, (pbrModels.scene.animations.size() > 0 ? 440 : 360) * scale), ImGuiSetCond_Always);

		ImGui::Begin("Ocean Renderer", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

		ImGui::PushItemWidth(100.0f * scale);

		if (ui->checkbox("Update Cam", &updateCamOnReceiveRendererDto))
		{
		}

		static char str0[128] = "Hello, world!";
		if (ImGui::InputText("input text", str0, IM_ARRAYSIZE(str0)))
		{
		}
		if (ui->button("Load"))
		{
			loadPageRequest("65a2aaf35058b63992ce78d1", updateCamOnReceiveRendererDto);
		}

		ImGui::SameLine();

		// nach einem Join kann mit einer bestimmten oid auch nachträglich einem stubContext
		// beigetreten werden!
		if (ui->button("Load-Context"))
		{
			loadPageRequest("65c6843ec03aef10cf481467", updateCamOnReceiveRendererDto);
		}

		// join game mode
		if (ui->button("Join"))
		{
			joinRequest("GM100A");
		}

		ImGui::SameLine();

		// select game mode
		if (ui->button("Select"))
		{
			// auswahl eines game modes
		}

		if (ui->button("Save"))
		{
		}

		ImGui::SameLine();

		ImGui::SameLine();

		if (ui->button("Clear"))
		{
			loadPageRequest("6580cf79d59ffd187c3ebe90");
		}

		ui->text("scapestation.com");
		ui->text("%.1d fps (%.2f ms)", lastFPS, (1000.0f / lastFPS));

		if (pbrModels.scene.animations.size() > 0)
		{
			if (ui->header("Animations"))
			{
				ui->checkbox("Animate", &node_animate);
				std::vector<std::string> animationNames;
				for (auto animation : pbrModels.scene.animations)
				{
					animationNames.push_back(animation.name);
				}
				ui->combo("Animation", &node_animationIndex, animationNames);
			}
		}

		ImGui::PopItemWidth();
		ImGui::End();

		// Draw data is now handled centrally in updateOverlay()

		if (lastDisplaySize.x != io.DisplaySize.x || lastDisplaySize.y != io.DisplaySize.y)
		{
			updateCBs = true;
		}

		if (updateCBs)
		{
			vkDeviceWaitIdle(device);
			recordCommandBuffers();
			vkDeviceWaitIdle(device);
		}

		if (updateShaderParams)
		{
			updateParams();
		}
	}

	void OceanPbrApp::updatePageInfoOverlay()
	{
		ImGuiIO &io = ImGui::GetIO();

		ImVec2 lastDisplaySize = io.DisplaySize;
		io.DisplaySize = ImVec2((float)width, (float)height);
		io.DeltaTime = frameTimer;

		io.MousePos = ImVec2(mousePos.x, mousePos.y);
		io.MouseDown[0] = mouseButtons.left;
		io.MouseDown[1] = mouseButtons.right;

		ui->pushConstBlock.scale = glm::vec2(2.0f / io.DisplaySize.x, 2.0f / io.DisplaySize.y);
		ui->pushConstBlock.translate = glm::vec2(-1.0f);

		bool updateShaderParams = false;
		bool updateCBs = false;
		float scale = 1.0f;

#if defined(VK_USE_PLATFORM_ANDROID_KHR)
		scale = (float)vks::android::screenDensity / (float)ACONFIGURATION_DENSITY_MEDIUM;
#endif
		ImGui::NewFrame();

		ImGui::SetNextWindowPos(ImVec2(10, 80));
		ImGui::SetNextWindowSize(ImVec2(200 * scale, 360 * scale), ImGuiSetCond_Always);

		ImGui::Begin("Ocean Renderer", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

		ImGui::PushItemWidth(100.0f * scale);

		ImGui::SameLine();

		ui->text("scapestation.com");

		ImGui::PopItemWidth();
		ImGui::End();
	}

	void OceanPbrApp::render()
	{
		if (!prepared)
		{
			return;
		}

		// if (framerateCap)
		//	Sleep(10);

		updateOverlay();

		VK_CHECK_RESULT(vkWaitForFences(device, 1, &waitFences[node_frameIndex], VK_TRUE, UINT64_MAX));
		VK_CHECK_RESULT(vkResetFences(device, 1, &waitFences[node_frameIndex]));

		VkResult acquire = swapChain.acquireNextImage(presentCompleteSemaphores[node_frameIndex], &currentBuffer);
		if ((acquire == VK_ERROR_OUT_OF_DATE_KHR) || (acquire == VK_SUBOPTIMAL_KHR))
		{
			windowResize();
		}
		else
		{
			VK_CHECK_RESULT(acquire);
		}

		// Update UBOs
		updateUniformBuffers();
		UniformBufferSet currentUB = uniformBuffers[currentBuffer];
		memcpy(currentUB.scene.mapped, &shaderValuesScene, sizeof(shaderValuesScene));
		memcpy(currentUB.params.mapped, &shaderValuesParams, sizeof(shaderValuesParams));
		memcpy(currentUB.skybox.mapped, &shaderValuesSkybox, sizeof(shaderValuesSkybox));

		// if (pbrModels.gltf2 != nullptr)
		//	 memcpy(currentUB.gltf2.mapped, &shaderValuesScene, sizeof(shaderValuesScene));

		const VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pWaitDstStageMask = &waitDstStageMask;
		submitInfo.pWaitSemaphores = &presentCompleteSemaphores[node_frameIndex];
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &renderCompleteSemaphores[node_frameIndex];
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pCommandBuffers = &commandBuffers[currentBuffer];
		submitInfo.commandBufferCount = 1;
		VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, waitFences[node_frameIndex]));

		VkResult present = swapChain.queuePresent(queue, currentBuffer, renderCompleteSemaphores[node_frameIndex]);
		if (!((present == VK_SUCCESS) || (present == VK_SUBOPTIMAL_KHR)))
		{
			if (present == VK_ERROR_OUT_OF_DATE_KHR)
			{
				windowResize();
				return;
			}
			else
			{
				VK_CHECK_RESULT(present);
			}
		}

		// Handle delayed screenshot capture
		if (screenshotRequested) {
			framesSinceStart++;
			if (framesSinceStart >= screenshotDelayFrames) {
				ocean::info("Screenshot - triggering capture after " + std::to_string(framesSinceStart) + " frames");
				captureScreenshot(screenshotFilename);
				screenshotRequested = false; // Reset for next screenshot
				framesSinceStart = 0;
			}
		}

		node_frameIndex += 1;
		node_frameIndex %= renderAhead;

		if (!paused)
		{
			if (node_rotate)
			{
				node_rot.y += frameTimer * 35.0f;
				if (node_rot.y > 360.0f)
				{
					node_rot.y -= 360.0f;
				}
			}

			if (node_animate)
				animation();

			updateParams();
		}
		if (camera.updated || node_updated)
		{
			updateUniformBuffers();
		}
	}
	void OceanPbrApp::animation()
	{
		auto tEnd = std::chrono::high_resolution_clock::now();
		auto tDiff = std::chrono::duration<double, std::milli>(tEnd - lastAnimationUpdate).count();

		// Begrenzung der Update-Zyklen, da sonst Frame-Drop auf 7 fps
		bool hasUpdate = (tDiff > animationUpdateIntervalMilliseconds);
		if (hasUpdate)
		{
			lastAnimationUpdate = tEnd;
		}
		/*if (pbrModels.scene.animations.size() > 0) {
			node_animationTimer += frameTimer;
			if (node_animationTimer > pbrModels.scene.animations[node_animationIndex].end) {
				node_animationTimer -= pbrModels.scene.animations[node_animationIndex].end;
			}
			pbrModels.scene.updateAnimation(node_animationIndex, node_animationTimer);
		}
		if (pbrModels.gltf2 != nullptr && pbrModels.gltf2->animations.size() > 0) {
			node_animationTimer += frameTimer;
			if (node_animationTimer > pbrModels.gltf2->animations[node_animationIndex].end) {
				node_animationTimer -= pbrModels.gltf2->animations[node_animationIndex].end;
			}
			pbrModels.gltf2->updateAnimation(node_animationIndex, node_animationTimer);
		}*/

		// depr
		// node_animationTimer += frameTimer;
		// if (node_animationTimer > pbrModels.gltfs[i].animations[node_animationIndex].end) {
		//	node_animationTimer -= pbrModels.gltfs[i].animations[node_animationIndex].end;
		//}
		///

		for (int i = 0; i < pbrModels.gltfs.size(); i++)
		{
			if (pbrModels.gltfs[i].animations.size() > 0)
			{
				for (int s = 0; s < pbrModels.gltfs[i].nodes.size(); s++)
				{
					if (pbrModels.gltfs[i].nodes[s]->animationIndex != -1)
					{
						assert(pbrModels.gltfs[i].nodes[s]->animationIndex < pbrModels.gltfs[i].animations.size());
						updateNodeAnimation(pbrModels.gltfs[i].nodes[s],
											pbrModels.gltfs[i].animations[pbrModels.gltfs[i].nodes[s]->animationIndex].end);
					}
				}
				if (hasUpdate)
					pbrModels.gltfs[i].updateAnimations();
			}
		}
	}
	void OceanPbrApp::updateNodeAnimation(oceangltf::Node *node, float end)
	{
		node->animationTimer += frameTimer;
		if (node->animationTimer > end)
		{
			node->animationTimer -= end;
		}

		// for (auto& child : node->children) {
		//	updateNodeAnimation(child, maxAnimations);
		// }
	}

	void OceanPbrApp::reloadEnvironment(std::string_view envMapFile)
	{
		ocean::info("=== PBR RELOAD ENVIRONMENT START ===");
		ocean::info("Reloading environment map from: " + std::string(envMapFile));
		vkDeviceWaitIdle(device);

		assert(envMapFile.find(".ktx") != std::string::npos);

		loadEnvironment(envMapFile);
		setupDescriptors();
		recordCommandBuffers();
	}

	/**
	 * Diese Id bezieht sich auf die Gltf-Datei und nicht auf das nodeGroup!
	 */
	void OceanPbrApp::updateGltf(uint32_t gltfId, std::string filename)
	{
		ocean::info("=== PBR UPDATE GLTF START ===");
		ocean::info("Updating gltfId=" + std::to_string(gltfId) + " from filename=" + filename);
		vkDeviceWaitIdle(device);
		loadGltf(gltfId, filename);
		setupDescriptors();
		recordCommandBuffers();
	}
	void OceanPbrApp::insertGltfModel(uint32_t gltfId, const tinygltf::Model *model)
	{
		ocean::info("=== PBR INSERT GLTF MODEL START ===");
		ocean::info("Inserting GLTF model with gltfId=" + std::to_string(gltfId));
		vkDeviceWaitIdle(device);
		loadGltf(gltfId, model);
		setupDescriptors();
		recordCommandBuffers();
	}
	void OceanPbrApp::insertSecondGltf(uint32_t gltfId, const tinygltf::Model *model)
	{
		std::cout << "VkPbrExample.insertSecondGltf(gltfId=" << gltfId << ")" << std::endl;
		vkDeviceWaitIdle(device);
		// loadSecondGltf(gltfId, model);
		setupDescriptors();
		recordCommandBuffers();
	}
	void OceanPbrApp::insertGltf3(uint32_t gltfId, const tinygltf::Model *model)
	{
		std::cout << "VkPbrExample.insertGltf3(gltfId=" << gltfId << ")" << std::endl;
		vkDeviceWaitIdle(device);
		addGltf3(gltfId, model);
		setupDescriptors();
		recordCommandBuffers();
	}
	void OceanPbrApp::insertGltf3(std::vector<std::pair<uint32_t, tinygltf::Model>> *modelContext)
	{
		std::cout << "VkPbrExample.insertGltf3(modelContext.size=" << modelContext->size() << ")" << std::endl;
		
		for (size_t i = 0; i < pbrModels.gltfs.size(); i++)
		{
			for (size_t s = 0; s < modelContext->size(); s++)
			{
				if (pbrModels.gltfs[i].id == modelContext->at(s).first)
				{
					std::cout << "VkPbrExample.insertGltf3(gltfId=" << modelContext->at(s).first << ") already loaded. You must delete it first or use putGltf" << std::endl;
					return;
				}
			}
		}

		vkDeviceWaitIdle(device);

		for (size_t i = 0; i < modelContext->size(); i++)
		{
			addGltf3(modelContext->at(i).first, &modelContext->at(i).second);
		}
		setupDescriptors();
		recordCommandBuffers();
	}
	void OceanPbrApp::insertGltf3(std::vector<std::tuple<uint32_t, GltfEngine::GltfService::GltfStatus, tinygltf::Model>> *modelContext)
	{
		std::cout << "VkPbrExample.insertGltf3(modelContext.size=" << modelContext->size() << ")" << std::endl;
		auto tStart = std::chrono::high_resolution_clock::now();

		vkDeviceWaitIdle(device);

		int counter = 0;
		for (uint32_t i = 0; i < modelContext->size(); i++)
		{
			addGltf3(std::get<0>(modelContext->at(i)), &std::get<2>(modelContext->at(i)));
			counter++;
		}
		setupDescriptors();
		recordCommandBuffers();
		auto tEnd = std::chrono::high_resolution_clock::now();
		auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
		std::cout << "VkPbrExample.insertGltf3 inserted " << counter << " gltfs in " << tDiff << "ms" << std::endl;
	}
	void OceanPbrApp::removeGltf3(std::vector<uint32_t> *modelIds)
	{
		std::cout << "VkPbrExample.removeGltf3(modelIds.size=" << modelIds->size() << ")" << std::endl;

		vkDeviceWaitIdle(device);

		for (uint32_t i = 0; i < pbrModels.gltfs.size(); i++)
		{
			for (uint32_t s = 0; s < modelIds->size(); s++)
			{
				if (pbrModels.gltfs.at(i).id == modelIds->at(s))
				{
					pbrModels.gltfs.at(i).destroy(device);
				}
			}
		}
		setupDescriptors();
		recordCommandBuffers();
	}
	void OceanPbrApp::updateGltf3(std::vector<std::tuple<uint32_t, GltfEngine::GltfService::GltfStatus, tinygltf::Model>> *modelContext)
	{
		// DEPRECATED
		vkDeviceWaitIdle(device);
		if (modelContext->size() != pbrModels.gltfs.size())
		{
			return;
		}
		for (uint32_t i = 0; i < modelContext->size(); i++)
		{
			assert(std::get<0>(modelContext->at(i)) == pbrModels.gltfs.at(i).id);
			pbrModels.gltfs.at(i).updateNodes(&std::get<2>(modelContext->at(i)));
		}
		// setupDescriptors();
		recordCommandBuffers();
	}
	void OceanPbrApp::updateNodesFromEntities(const std::unordered_map<std::string, nyx::Entity>& entities) {
		ocean::info("updateNodesFromEntities: syncing ECS transforms to GLTF nodes...");

		vkDeviceWaitIdle(device);  // Block rendering before making updates

		for (const auto& [id, entity] : entities) {
			const auto* transform = lunara_utils::getComponent<nyx::TransformComponent>(entity);
			const auto* model = lunara_utils::getComponent<nyx::ModelComponent>(entity);
			if (!transform || !model) continue;

			int gltfId = std::stoi(model->model_id());

			// Find the GLTF object that owns this gltf_id
			for (auto& gltf : pbrModels.gltfs) {
				if (gltf.id != gltfId) continue;

				// Find node that matches the entity id
				for (auto& node : gltf.nodes) {
					if (node && node->id == std::stoi(id)) {
						node->translation.x = transform->pos().x();
						node->translation.y = transform->pos().y();
						node->translation.z = transform->pos().z();

						if (transform->has_rot()) {
							node->rotation.x = transform->rot().x();
							node->rotation.y = transform->rot().y();
							node->rotation.z = transform->rot().z();
							node->rotation.w = transform->rot().w();
						}

						node->update();  // Commit transform update
						break;
					}
				}

				break; // No need to check other GLTFs if one matched
			}
		}

		recordCommandBuffers();
	}

	void OceanPbrApp::updateNodes3(com::context::grpc::DatabaseContext* gltfContext)
	{
		// std::cout << "VkPbrExample.updateNodes3()" << gltfContext->DebugString() << std::endl;

		vkDeviceWaitIdle(device);

		for (uint32_t i = 0; i < gltfContext->nodes_size(); i++)
		{
			for (uint32_t s = 0; s < pbrModels.gltfs.size(); s++)
			{
				auto node = &gltfContext->nodes().at(i);
				if (node->gltfid() == pbrModels.gltfs.at(s).id)
				{
					for (uint32_t k = 0; k < pbrModels.gltfs.at(s).nodes.size(); k++)
					{
						if (node->id() == pbrModels.gltfs.at(s).nodes.at(k)->id)
						{
							auto n = pbrModels.gltfs.at(s).nodes.at(k);
							n->translation.x = node->px();
							n->translation.y = node->py();
							n->translation.z = node->pz();
							// n->rotation.x = node->rx();
							// n->rotation.y = node->ry();
							// n->rotation.z = node->rz();
							// n->rotation.w = node->rw();
							n->update();

							s = MAXUINT32 - 1; // continue with next node of gltfContext->node
							k = MAXUINT32 - 1;
							break;
						}
					}
				}
			}
		}
		recordCommandBuffers();
	}
	void OceanPbrApp::updateNodes3(com::context::grpc::GltfContext* gltfContext)
	{
		vkDeviceWaitIdle(device);

		// Precompute the size of the vectors to avoid calling size() repeatedly
		const auto& nodes = gltfContext->nodes();
		const auto& gltfs = pbrModels.gltfs;

		for (const auto& node : nodes) {
			const uint32_t gltfId = node.node().gltfid();

			// Use a map for faster lookups if gltfs is large
			auto gltfIt = std::find_if(gltfs.begin(), gltfs.end(),
				[&gltfId](const auto& gltf) { return gltf.id == gltfId; });

			if (gltfIt == gltfs.end()) {
				continue; // Continue with the next node if no matching GLTF found
			}

			const auto& gltf = *gltfIt;
			const auto& nodeIds = gltf.nodes;

			auto nodeIt = std::find_if(nodeIds.begin(), nodeIds.end(),
				[&node](const auto& n) { return n->id == node.id(); });

			if (nodeIt != nodeIds.end()) {
				auto* n = *nodeIt;
				n->translation.x = node.node().px();
				n->translation.y = node.node().py();
				n->translation.z = node.node().pz();
				n->rotation.x = node.node().rx();
				n->rotation.y = node.node().ry();
				n->rotation.z = node.node().rz();
				n->rotation.w = node.node().rw();

				// No need to manually adjust loop counters; continue to the next node
			}
		}

		recordCommandBuffers();
	}

	void OceanPbrApp::removeSecondGltfModel(uint32_t gltfId)
	{
		std::cout << "VkPbrExample.removeSecondGltf(gltfId=" << gltfId << ")" << std::endl;
		vkDeviceWaitIdle(device);
		removeSecondGltf(gltfId);
		setupDescriptors();
		recordCommandBuffers();
	}
	void OceanPbrApp::removeSecondGltf(uint32_t gltfId)
	{
		if (pbrModels.gltf2 != nullptr)
		{
			pbrModels.gltf2->destroy(device);
			delete pbrModels.gltf2;
			pbrModels.gltf2 = nullptr;
		}
	}
	/// <summary>
	/// Nur �nderungen der Nodes!
	/// </summary>
	void OceanPbrApp::updateGltfModel(uint32_t gltfId, const tinygltf::Model *model)
	{
		vkDeviceWaitIdle(device);
		// auto tStart = std::chrono::high_resolution_clock::now();
		pbrModels.scene.updateNodes(model);
		// pbrModels.scene.updateNodes();
		recordCommandBuffers();
		// auto t = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - tStart).count();
		// std::cout << "VkPbrExample.putGltfNode() Updating node took " << t << " ms" << std::endl;
	}
	void OceanPbrApp::updateGlbFromMemory(uint32_t gltfId, const unsigned char *bytes)
	{
		assert(false);
	}
	void OceanPbrApp::removeGltf(uint32_t gltfId)
	{
		ocean::info("=== PBR REMOVE GLTF START ===");
		ocean::info("Removing GLTF model with gltfId=" + std::to_string(gltfId));
		vkDeviceWaitIdle(device);
		pbrModels.scene.destroy(device);
		setupDescriptors();
		recordCommandBuffers();
	}
	void OceanPbrApp::clearGltfs()
	{
		ocean::info("=== PBR CLEAR GLTFS START ===");
		ocean::info("Clearing all GLTF models");
		vkDeviceWaitIdle(device);
		for (uint32_t i = 0; i < pbrModels.gltfs.size(); i++)
		{
			pbrModels.gltfs[i].destroy(device);
		}
		pbrModels.gltfs.clear();
		setupDescriptors();
		recordCommandBuffers();
	}
	void OceanPbrApp::clearGltfById(uint32_t gltfId)
	{
		std::cout << "VkPbrExample.clearGltfById gltfId=" << gltfId << std::endl;
		vkDeviceWaitIdle(device);

		for (uint32_t i = 0; i < pbrModels.gltfs.size(); i++)
		{
			if (pbrModels.gltfs[i].id == gltfId)
			{
				pbrModels.gltfs[i].destroy(device);
				pbrModels.gltfs.erase(pbrModels.gltfs.begin() + i);
			}
		}

		setupDescriptors();
		recordCommandBuffers();
	}
	void OceanPbrApp::updateNode(uint32_t nodeId, const google::protobuf::RepeatedField<float> pos,
		const google::protobuf::RepeatedField<float> rot)
	{
		updateNode(nodeId, {pos[0], pos[1], pos[2]}, {rot[0], rot[1], rot[2]});
	}
	void OceanPbrApp::updateNode(uint32_t nodeId, const glm::vec3 pos, const glm::vec3 rot)
	{
		/*vkDeviceWaitIdle(device);
		// pbrModels.scene.changePosAndRot(pos, rot);
		bool found = false;
		for (auto& model : pbrModels.gltfs)
		{
			for (auto& node : model.nodes)
			{
				if (node->id.compare(std::to_string(nodeId)) == 0)
				{
					node->translation = pos;
					node->rotation = rot;
					found = true;
					break;
				}
			}
			if (found)
				break;
		}
		recordCommandBuffers();*/
	}
	void OceanPbrApp::insertNode(uint32_t nodeId, uint32_t gltfId, const glm::vec3 pos, const glm::vec3 rot)
	{
		vkDeviceWaitIdle(device);
		addNode(nodeId, gltfId, pos, rot);
		recordCommandBuffers();
	}
	void OceanPbrApp::removeNode(uint32_t nodeId)
	{
		assert(false);
	}
	
	HWND OceanPbrApp::setupWindow(HINSTANCE hinstance, WNDPROC wndproc)
	{
		return oceanbase::VulkanExampleBase::setupWindow(hinstance, wndproc);
	}
	void OceanPbrApp::closeWindow()
	{
		oceanbase::VulkanExampleBase::sendKey(WM_CLOSE);
	}
	bool OceanPbrApp::render2()
	{
		return oceanbase::VulkanExampleBase::render2();
	}
	oceancore::Camera *OceanPbrApp::getCamera(void)
	{
		return &camera;
	}

	void OceanPbrApp::updateCamera(const glm::vec3 pos, 
		const glm::vec3 rot, float fov,float znear, float zfar)
	{
		camera.setPosition(pos);
		camera.setRotation(rot);
	}
	void OceanPbrApp::updateCamera(const glm::vec3 pos, const glm::vec3 rot)
	{
		// Debug: Log if camera is being set to origin (which shows blue background)
		if (pos.x == 0.0f && pos.y == 0.0f && pos.z == 0.0f) {
			ocean::error("Camera position set to origin (0,0,0) - may show blue background instead of scene");
		}
		camera.setPosition(pos);
		camera.setRotation(rot);
	}
	void OceanPbrApp::updateCamera(const google::protobuf::RepeatedField<float> pos,
								   const google::protobuf::RepeatedField<float> rot)
	{
		updateCamera({pos[0], pos[1], pos[2]}, {rot[0], rot[1], rot[2]});
	}
	void OceanPbrApp::changeOverlay(overlayimgui::OverlayTemplate newOverlay)
	{
		overlayTemplateId = newOverlay;
	}
	void OceanPbrApp::handleMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		oceanbase::VulkanExampleBase::handleMessages(hWnd, uMsg, wParam, lParam);
	}

	void OceanPbrApp::prepare()
	{
		oceanbase::VulkanExampleBase::prepare();

		camera.type = oceancore::Camera::CameraType::lookat;
		// camera.type = oceancore::Camera::CameraType::firstperson;

		// camera.setPerspective(45.0f, (float)width / (float)height, 0.1f, 256.0f);
		camera.setPerspective(45.0f, (float)width / (float)height, 0.001f, 256.0f);
		// camera.setPerspective(10.0f, (float)width / (float)height, 0.001f, 256.0f); // ortho
		camera.rotationSpeed = 0.25f;
		camera.movementSpeed = 0.1f;
		camera.setPosition({0.0f, 3.0f, 15.0f});  // Better default position to see scene objects
		camera.setRotation({0.0f, 0.0f, 0.0f});

		// This ensures that there are enough fences to handle up to 
		// renderAhead frames. For example, if you're rendering 
		// 2 or 3 frames ahead, you'll need that many fences 
		// to synchronize the CPU/GPU properly.
		waitFences.resize(renderAhead);
		// These semaphores signal that the image has been acquired 
		// from the swapchain and is ready for rendering.
		presentCompleteSemaphores.resize(renderAhead);
		// These semaphores signal that the rendering has completed 
		// and the image is ready for presentation to the screen.
		renderCompleteSemaphores.resize(renderAhead);
		// This ensures that there is one command buffer for each 
		// image in the swapchain. Each command buffer corresponds 
		// to one image in the swapchain and will be used to record 
		// rendering commands for that image.
		commandBuffers.resize(swapChain.imageCount);
		// These are per-frame uniform buffers. You need one uniform 
		// buffer for each swapchain image because the uniform data 
		// might change every frame (e.g., camera matrices, lighting data).
		uniformBuffers.resize(swapChain.imageCount);
		// You need a descriptor set for each frame (or swapchain image) 
		// because the resources bound by the descriptors (like uniform 
		// buffers, textures, etc.) may change every frame.
		descriptorSets.resize(swapChain.imageCount);
		
		// Command buffer execution fences
		for (auto &waitFence : waitFences)
		{
			VkFenceCreateInfo fenceCI{VK_STRUCTURE_TYPE_FENCE_CREATE_INFO, nullptr, VK_FENCE_CREATE_SIGNALED_BIT};
			VK_CHECK_RESULT(vkCreateFence(device, &fenceCI, nullptr, &waitFence));
		}
		// Queue ordering semaphores
		for (auto &semaphore : presentCompleteSemaphores)
		{
			VkSemaphoreCreateInfo semaphoreCI{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, nullptr, 0};
			VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCI, nullptr, &semaphore));
		}
		for (auto &semaphore : renderCompleteSemaphores)
		{
			VkSemaphoreCreateInfo semaphoreCI{VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO, nullptr, 0};
			VK_CHECK_RESULT(vkCreateSemaphore(device, &semaphoreCI, nullptr, &semaphore));
		}
		// Command buffers
		{
			VkCommandBufferAllocateInfo cmdBufAllocateInfo{};
			cmdBufAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			cmdBufAllocateInfo.commandPool = cmdPool;
			cmdBufAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			cmdBufAllocateInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());
			VK_CHECK_RESULT(vkAllocateCommandBuffers(device, &cmdBufAllocateInfo, commandBuffers.data()));
		}

		loadAssets();
		generateBRDFLUT();
		generateCubemaps();
		ocean::info("Preparing uniform buffers...");
		prepareUniformBuffers();
		ocean::info("Uniform buffers prepared");

		ocean::info("Setting up descriptors...");
		setupDescriptors();
		ocean::info("Descriptors setup complete");

		ocean::info("Preparing pipelines...");
		preparePipelines();
		ocean::info("Pipelines prepared");

		ocean::info("Creating ImGui UI...");
		ui = new overlayimgui::UI(vulkanDevice, renderPass, queue, pipelineCache, VK_SAMPLE_COUNT_4_BIT);
		ocean::info("ImGui UI created");
		// layout = ImStudio::createDefaultLayout2();

		ocean::info("Updating overlay...");
		updateOverlay();
		ocean::info("Overlay updated");

		ocean::info("Recording command buffers...");
		recordCommandBuffers();
		ocean::info("Command buffers recorded");

		prepared = true;
		ocean::info("=== PREPARE COMPLETE ===");
	}
	void OceanPbrApp::createWindow(const oceancore::Settings *settings)
	{
		assert(false);
	}

	void OceanPbrApp::requestScreenshot(const std::string& filename)
	{
		ocean::info("Screenshot - requested: " + filename + " (will capture after " + std::to_string(screenshotDelayFrames) + " frames)");
		screenshotRequested = true;
		screenshotFilename = filename;
		framesSinceStart = 0; // Reset frame counter
	}

	void OceanPbrApp::dumpSceneState()
	{
		ocean::info("Scene state - dumping to ocean log");

		// Write header with timestamp
		auto now = std::chrono::system_clock::now();
		auto time_t = std::chrono::system_clock::to_time_t(now);
		auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

		ocean::info("================================================================================");
		ocean::info("ELYRION SCENE STATE LOG");
		ocean::info("================================================================================");
		ocean::info("Timestamp: " + std::to_string(timestamp) + " ms since epoch");
		char timeBuffer[100];
		ctime_s(timeBuffer, sizeof(timeBuffer), &time_t);
		std::string timeStr = timeBuffer;
		timeStr.pop_back(); // Remove trailing newline
		ocean::info("Time: " + timeStr);
		ocean::info("Frame: " + std::to_string(framesSinceStart));
		ocean::info("");

		// === VULKAN STATE ===
		ocean::info("=== VULKAN STATE ===");
		ocean::info("SwapChain Format: " + std::to_string(swapChain.colorFormat));
		ocean::info("SwapChain ColorSpace: " + std::to_string(swapChain.colorSpace));
		ocean::info("SwapChain Extent: " + std::to_string(swapChain.extent.width) + "x" + std::to_string(swapChain.extent.height));
		ocean::info("SwapChain Image Count: " + std::to_string(swapChain.imageCount));
		ocean::info("Current Buffer Index: " + std::to_string(currentBuffer));
		ocean::info("Frame Index: " + std::to_string(node_frameIndex));
		ocean::info("Render Ahead: " + std::to_string(renderAhead));
		ocean::info("Window Dimensions: " + std::to_string(width) + "x" + std::to_string(height));
		ocean::info("Prepared: " + std::string(prepared ? "true" : "false"));
		ocean::info("Paused: " + std::string(paused ? "true" : "false"));
		ocean::info("");

		// === CAMERA STATE ===
		ocean::info("=== CAMERA STATE ===");
		oceancore::Camera* cam = getCamera();
		if (cam) {
			ocean::info("Camera Position: [" + std::to_string(cam->position.x) + ", " + std::to_string(cam->position.y) + ", " + std::to_string(cam->position.z) + "]");
			ocean::info("Camera Rotation: [" + std::to_string(cam->rotation.x) + ", " + std::to_string(cam->rotation.y) + ", " + std::to_string(cam->rotation.z) + "]");
			ocean::info("Camera Type: " + std::string(cam->type == oceancore::Camera::CameraType::lookat ? "lookat" : "firstperson"));
			ocean::info("Camera Updated: " + std::string(cam->updated ? "true" : "false"));
			ocean::info("Camera Movement Speed: " + std::to_string(cam->movementSpeed));
			ocean::info("Camera Rotation Speed: " + std::to_string(cam->rotationSpeed));
		} else {
			ocean::info("Camera: NULL");
		}
		ocean::info("");

		// === NODE/ENTITY STATE ===
		ocean::info("=== NODE/ENTITY STATE ===");
		ocean::info("Node Rotation Y: " + std::to_string(node_rot.y));
		ocean::info("Node Rotate Enabled: " + std::string(node_rotate ? "true" : "false"));
		ocean::info("Node Animation Index: " + std::to_string(node_animationIndex));
		ocean::info("Node Animation Timer: " + std::to_string(node_animationTimer));
		ocean::info("Node Animate Enabled: " + std::string(node_animate ? "true" : "false"));
		ocean::info("Node Updated: " + std::string(node_updated ? "true" : "false"));
		ocean::info("");

		// === GLTF MODELS ===
		ocean::info("=== GLTF MODELS ===");
		ocean::info("Main Scene Model:");
		ocean::info("  ID: " + pbrModels.scene.id);
		ocean::info("  Display: " + std::string(pbrModels.scene.display ? "true" : "false"));
		ocean::info("  Nodes: " + std::to_string(pbrModels.scene.nodes.size()));
		ocean::info("  Linear Nodes: " + std::to_string(pbrModels.scene.linearNodes.size()));
		ocean::info("  Materials: " + std::to_string(pbrModels.scene.materials.size()));
		ocean::info("  Textures: " + std::to_string(pbrModels.scene.textures.size()));
		ocean::info("  Texture Samplers: " + std::to_string(pbrModels.scene.textureSamplers.size()));
		ocean::info("  Animations: " + std::to_string(pbrModels.scene.animations.size()));
		ocean::info("  Skins: " + std::to_string(pbrModels.scene.skins.size()));

		ocean::info("Skybox Model:");
		ocean::info("  ID: " + pbrModels.skybox.id);
		ocean::info("  Display: " + std::string(pbrModels.skybox.display ? "true" : "false"));
		ocean::info("  Nodes: " + std::to_string(pbrModels.skybox.nodes.size()));
		ocean::info("  Linear Nodes: " + std::to_string(pbrModels.skybox.linearNodes.size()));
		ocean::info("  Materials: " + std::to_string(pbrModels.skybox.materials.size()));
		ocean::info("  Textures: " + std::to_string(pbrModels.skybox.textures.size()));

		ocean::info("Additional GLTF Models: " + std::to_string(pbrModels.gltfs.size()));
		for (size_t i = 0; i < pbrModels.gltfs.size(); ++i) {
			const auto& model = pbrModels.gltfs[i];
			ocean::info("GLTF[" + std::to_string(i) + "]:");
			ocean::info("  ID: " + model.id);
			ocean::info("  Display: " + std::string(model.display ? "true" : "false"));
			ocean::info("  Nodes: " + std::to_string(model.nodes.size()));
			ocean::info("  Linear Nodes: " + std::to_string(model.linearNodes.size()));
			ocean::info("  Materials: " + std::to_string(model.materials.size()));
			ocean::info("  Textures: " + std::to_string(model.textures.size()));
			ocean::info("  Animations: " + std::to_string(model.animations.size()));
		}
		ocean::info("");

		// === LIGHTING ===
		ocean::info("=== LIGHTING ===");
		ocean::info("Selected Environment: " + selectedEnvironment);
		ocean::info("Available Environments: " + std::to_string(environments.size()));
		for (const auto& env : environments) {
			ocean::info("  " + env.first + " -> " + env.second);
		}
		ocean::info("");

		// === PBR SETTINGS ===
		ocean::info("=== PBR SETTINGS ===");
		ocean::info("Debug View Inputs: " + std::to_string(debugViewInputs));
		ocean::info("Debug View Equation: " + std::to_string(debugViewEquation));
		ocean::info("Push Constant Material:");
		ocean::info("  BaseColorFactor: [" + std::to_string(pushConstBlockMaterial.baseColorFactor.x) + ", " +
		                  std::to_string(pushConstBlockMaterial.baseColorFactor.y) + ", " +
		                  std::to_string(pushConstBlockMaterial.baseColorFactor.z) + ", " +
		                  std::to_string(pushConstBlockMaterial.baseColorFactor.w) + "]");
		ocean::info("  EmissiveFactor: [" + std::to_string(pushConstBlockMaterial.emissiveFactor.x) + ", " +
		                  std::to_string(pushConstBlockMaterial.emissiveFactor.y) + ", " +
		                  std::to_string(pushConstBlockMaterial.emissiveFactor.z) + "]");
		ocean::info("  DiffuseFactor: [" + std::to_string(pushConstBlockMaterial.diffuseFactor.x) + ", " +
		                  std::to_string(pushConstBlockMaterial.diffuseFactor.y) + ", " +
		                  std::to_string(pushConstBlockMaterial.diffuseFactor.z) + "]");
		ocean::info("  SpecularFactor: [" + std::to_string(pushConstBlockMaterial.specularFactor.x) + ", " +
		                  std::to_string(pushConstBlockMaterial.specularFactor.y) + ", " +
		                  std::to_string(pushConstBlockMaterial.specularFactor.z) + "]");
		ocean::info("");

		// === VULKAN RESOURCES ===
		ocean::info("=== VULKAN RESOURCES ===");
		ocean::info("Command Buffers: " + std::to_string(commandBuffers.size()));
		ocean::info("Uniform Buffer Sets: " + std::to_string(uniformBuffers.size()));
		ocean::info("Wait Fences: " + std::to_string(waitFences.size()));
		ocean::info("Render Complete Semaphores: " + std::to_string(renderCompleteSemaphores.size()));
		ocean::info("Present Complete Semaphores: " + std::to_string(presentCompleteSemaphores.size()));
		ocean::info("Descriptor Sets: " + std::to_string(descriptorSets.size()));
		ocean::info("Bound Pipeline: " + std::string(boundPipeline != VK_NULL_HANDLE ? "Set" : "NULL"));
		ocean::info("");

		// === SCREENSHOT REQUEST STATE ===
		ocean::info("=== SCREENSHOT STATE ===");
		ocean::info("Screenshot Requested: " + std::string(screenshotRequested ? "true" : "false"));
		ocean::info("Screenshot Filename: " + screenshotFilename);
		ocean::info("Screenshot Delay Frames: " + std::to_string(screenshotDelayFrames));
		ocean::info("Frames Since Start: " + std::to_string(framesSinceStart));
		ocean::info("");

		ocean::info("================================================================================");
		ocean::info("END OF SCENE STATE LOG");
		ocean::info("================================================================================");
	}

	void OceanPbrApp::captureScreenshot(const std::string& filename)
	{
		ocean::info("Screenshot - capturing to: " + filename);

		// Dump scene state to ocean log (trace level)
		dumpSceneState();

		try {
			// Get current swapchain image
			VkImage srcImage = swapChain.images[currentBuffer];

			// Get swapchain image format and dimensions
			VkFormat imageFormat = swapChain.colorFormat;
			VkExtent2D imageExtent = { width, height };

			// Create staging buffer for image data
			VkDeviceSize imageSize = width * height * 4; // RGBA

			VkBuffer stagingBuffer;
			VkDeviceMemory stagingBufferMemory;

			// Create staging buffer
			VkBufferCreateInfo bufferInfo = {};
			bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferInfo.size = imageSize;
			bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT;
			bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

			if (vkCreateBuffer(device, &bufferInfo, nullptr, &stagingBuffer) != VK_SUCCESS) {
				ocean::error("Screenshot - failed to create staging buffer");
				return;
			}

			// Allocate memory for staging buffer
			VkMemoryRequirements memRequirements;
			vkGetBufferMemoryRequirements(device, stagingBuffer, &memRequirements);

			VkMemoryAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = memRequirements.size;
			allocInfo.memoryTypeIndex = vulkanDevice->getMemoryType(memRequirements.memoryTypeBits,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

			if (vkAllocateMemory(device, &allocInfo, nullptr, &stagingBufferMemory) != VK_SUCCESS) {
				vkDestroyBuffer(device, stagingBuffer, nullptr);
				ocean::error("Screenshot - failed to allocate staging buffer memory");
				return;
			}

			vkBindBufferMemory(device, stagingBuffer, stagingBufferMemory, 0);

			// Create command buffer for copy operation
			VkCommandBufferAllocateInfo cmdBufAllocInfo = {};
			cmdBufAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			cmdBufAllocInfo.commandPool = cmdPool;
			cmdBufAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			cmdBufAllocInfo.commandBufferCount = 1;

			VkCommandBuffer copyCmd;
			vkAllocateCommandBuffers(device, &cmdBufAllocInfo, &copyCmd);

			VkCommandBufferBeginInfo cmdBufInfo = {};
			cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

			vkBeginCommandBuffer(copyCmd, &cmdBufInfo);

			// Transition image layout for transfer
			VkImageMemoryBarrier barrier = {};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = srcImage;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer = 0;
			barrier.subresourceRange.layerCount = 1;
			barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			vkCmdPipelineBarrier(copyCmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

			// Copy image to buffer
			VkBufferImageCopy region = {};
			region.bufferOffset = 0;
			region.bufferRowLength = 0;
			region.bufferImageHeight = 0;
			region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			region.imageSubresource.mipLevel = 0;
			region.imageSubresource.baseArrayLayer = 0;
			region.imageSubresource.layerCount = 1;
			region.imageOffset = { 0, 0, 0 };
			region.imageExtent = { width, height, 1 };

			vkCmdCopyImageToBuffer(copyCmd, srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, stagingBuffer, 1, &region);

			// Transition back to present layout
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;

			vkCmdPipelineBarrier(copyCmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);

			vkEndCommandBuffer(copyCmd);

			// Submit command buffer
			VkSubmitInfo submitInfo = {};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &copyCmd;

			vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
			vkQueueWaitIdle(queue);

			// Map memory and read pixel data
			void* data;
			vkMapMemory(device, stagingBufferMemory, 0, imageSize, 0, &data);

			// Save as PNG using stb_image_write
			if (stbi_write_png(filename.c_str(), width, height, 4, data, width * 4)) {
				ocean::info("Screenshot - saved successfully: " + filename);
			} else {
				ocean::error("Screenshot - failed to save PNG: " + filename);
			}

			vkUnmapMemory(device, stagingBufferMemory);

			// Cleanup
			vkFreeCommandBuffers(device, cmdPool, 1, &copyCmd);
			vkDestroyBuffer(device, stagingBuffer, nullptr);
			vkFreeMemory(device, stagingBufferMemory, nullptr);

		} catch (const std::exception& e) {
			ocean::error("Screenshot - exception: " + std::string(e.what()));
		}
	}

	void OceanPbrApp::updateStudioModeMenuBar()
	{
		try {
			ui->beginMainMenuBar();

		if (ui->beginMenu("File")) {
			if (ui->menuItem("New Scene", "Ctrl+N")) {
				ocean::info("Studio Menu - New Scene requested");
			}
			if (ui->menuItem("Open Scene", "Ctrl+O")) {
				ocean::info("Studio Menu - Open Scene requested");
			}
			if (ui->menuItem("Save Scene", "Ctrl+S")) {
				ocean::info("Studio Menu - Save Scene requested");
			}
			if (ui->menuItem("Save Scene As...", "Ctrl+Shift+S")) {
				ocean::info("Studio Menu - Save Scene As requested");
			}
			ImGui::Separator();
			if (ui->menuItem("Import GLTF...", nullptr)) {
				ocean::info("Studio Menu - Import GLTF requested");
			}
			if (ui->menuItem("Export Scene...", nullptr)) {
				ocean::info("Studio Menu - Export Scene requested");
			}
			ImGui::Separator();
			if (ui->menuItem("Exit", "Alt+F4")) {
				ocean::info("Studio Menu - Exit requested");
				PostQuitMessage(0);
			}
			ui->endMenu();
		}

		if (ui->beginMenu("View")) {
			if (ui->menuItem("Wireframe Mode", "W")) {
				ocean::info("Studio Menu - Wireframe mode toggled");
			}
			if (ui->menuItem("Show Grid", "G")) {
				ocean::info("Studio Menu - Grid visibility toggled");
			}
			if (ui->menuItem("Show Debug Info", "D")) {
				ocean::info("Studio Menu - Debug info toggled");
				// Toggle between debug overlays
				if (overlayTemplateId == overlayimgui::OverlayTemplate::OVERLAY_DEBUG_SHADER) {
					overlayTemplateId = overlayimgui::OverlayTemplate::OVERLAY_DEBUG_PERFORMANCE;
				} else {
					overlayTemplateId = overlayimgui::OverlayTemplate::OVERLAY_DEBUG_SHADER;
				}
			}
			ImGui::Separator();
			if (ui->menuItem("Reset Camera", "R")) {
				ocean::info("Studio Menu - Reset camera requested");
				// Reset camera to default position
				updateCamera(glm::vec3(0.0f, 0.0f, 10.0f), glm::vec3(0.0f, 0.0f, 0.0f));
			}
			ui->endMenu();
		}

		if (ui->beginMenu("Scene")) {
			if (ui->menuItem("Add Light", nullptr)) {
				ocean::info("Studio Menu - Add light requested");
			}
			if (ui->menuItem("Add Object", nullptr)) {
				ocean::info("Studio Menu - Add object requested");
			}
			ImGui::Separator();
			if (ui->menuItem("Scene Properties", nullptr)) {
				ocean::info("Studio Menu - Scene properties requested");
			}
			ui->endMenu();
		}

		if (ui->beginMenu("Render")) {
			if (ui->menuItem("Take Screenshot", "F12")) {
				ocean::info("Studio Menu - Screenshot requested");
				requestScreenshot("studio_screenshot.png");
			}
			if (ui->menuItem("Render Animation", nullptr)) {
				ocean::info("Studio Menu - Render animation requested");
			}
			ImGui::Separator();
			if (ui->menuItem("Render Settings", nullptr)) {
				ocean::info("Studio Menu - Render settings requested");
			}
			ui->endMenu();
		}

		if (ui->beginMenu("Tools")) {
			if (ui->menuItem("🔧 IO Monitor", nullptr)) {
				ocean::info("Studio Menu - IO Monitor requested");
				if (onToolLaunchRequested) {
					ocean::info("  Callback is set - invoking with tool ID: io-monitor");
					onToolLaunchRequested("io-monitor");
				} else {
					ocean::error("  Callback is NULL - cannot launch tool!");
					ocean::error("  Make sure edit mode is enabled and callback was registered");
				}
			}
			if (ui->menuItem("📐 Scene Editor", nullptr)) {
				ocean::info("Studio Menu - Scene Editor requested");
				if (onToolLaunchRequested) {
					onToolLaunchRequested("scene-editor");
				}
			}
			if (ui->menuItem("🎬 Showcase", nullptr)) {
				ocean::info("Studio Menu - Showcase requested");
				if (onToolLaunchRequested) {
					onToolLaunchRequested("showcase");
				}
			}
			if (ui->menuItem("🔨 Build Lab", nullptr)) {
				ocean::info("Studio Menu - Build Lab requested");
				if (onToolLaunchRequested) {
					onToolLaunchRequested("build-lab");
				}
			}
			if (ui->menuItem("✨ Material Editor", nullptr)) {
				ocean::info("Studio Menu - Material Editor requested");
				if (onToolLaunchRequested) {
					onToolLaunchRequested("material-editor");
				}
			}
			ui->endMenu();
		}

		if (ui->beginMenu("Help")) {
			if (ui->menuItem("About", nullptr)) {
				ocean::info("Studio Menu - About requested");
			}
			if (ui->menuItem("Controls", "F1")) {
				ocean::info("Studio Menu - Controls help requested");
			}
			ui->endMenu();
		}

			ui->endMainMenuBar();
		} catch (const std::exception& e) {
			ocean::error("Studio Mode Menu Bar Exception: " + std::string(e.what()));
			ocean::error("This may be due to ImGui frame scope issues - ensure NewFrame() is called before UI functions");
		}
	}

	void OceanPbrApp::setTitle(const std::string& newTitle)
	{
		VulkanExampleBase::setTitle(newTitle);
	}
};