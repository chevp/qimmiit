/**
 * Copyright (C) by Patrice Chevillat
 */

#pragma once

#ifndef OCEAN_GLTF_HPP_
#define OCEAN_GLTF_HPP_

// for faster matrix multiplications
#define GLM_FORCE_AVX  // or GLM_FORCE_SSE2 

#ifndef OCEAN_DEPENDENCIES_HPP_
#include "ocean_dependencies.hpp"
#endif

#ifndef OCEAN_TEXTURE_HPP_
#include "ocean_texture.hpp"
#endif

#ifndef OCEAN_MACROS_HPP_
#include "ocean_macros.hpp"
#endif

#ifndef OCEAN_CORE_HPP_
#include "ocean_core.hpp"
#endif

namespace oceangltf {

// Changing this value here also requires changing it in the vertex shader
#define MAX_NUM_JOINTS 128u

struct Node;

struct BoundingBox
{
	glm::vec3 min;
	glm::vec3 max;
	bool valid = false;
	BoundingBox(){};
	BoundingBox(glm::vec3 min, glm::vec3 max);
	BoundingBox getAABB(glm::mat4 m);
};

	struct TextureSampler
	{
		VkFilter magFilter;
		VkFilter minFilter;
		VkSamplerAddressMode addressModeU;
		VkSamplerAddressMode addressModeV;
		VkSamplerAddressMode addressModeW;
	};

	/*struct Texture
	{
		ocean::VulkanDevice *device;
		VkImage image;
		VkImageLayout imageLayout;
		VkDeviceMemory deviceMemory;
		VkImageView view;
		uint32_t width, height;
		uint32_t mipLevels;
		uint32_t layerCount;
		VkDescriptorImageInfo descriptor;
		VkSampler sampler;
		void updateDescriptor();
		void destroy();
		// Load a texture from a glTF image (stored as vector of chars loaded via stb_image) and generate a full mip chaing for it
		void fromGltfImage(tinygltf::Image &gltfimage, TextureSampler textureSampler, ocean::VulkanDevice *device,
						   VkQueue copyQueue);
	};*/

	struct Material
	{
		enum AlphaMode
		{
			ALPHAMODE_OPAQUE,
			ALPHAMODE_MASK,
			ALPHAMODE_BLEND
		};
		AlphaMode alphaMode = ALPHAMODE_OPAQUE;
		float alphaCutoff = 1.0f;
		float metallicFactor = 1.0f;
		float roughnessFactor = 1.0f;
		glm::vec4 baseColorFactor = glm::vec4(1.0f);
		glm::vec4 emissiveFactor = glm::vec4(1.0f);
		oceantexture::Texture *baseColorTexture;
		oceantexture::Texture *metallicRoughnessTexture;
		oceantexture::Texture *normalTexture;
		oceantexture::Texture *occlusionTexture;
		oceantexture::Texture *emissiveTexture;
		bool doubleSided = false;
		struct TexCoordSets
		{
			uint8_t baseColor = 0;
			uint8_t metallicRoughness = 0;
			uint8_t specularGlossiness = 0;
			uint8_t normal = 0;
			uint8_t occlusion = 0;
			uint8_t emissive = 0;
		} texCoordSets;

		struct Extension
		{
			oceantexture::Texture *specularGlossinessTexture;
			oceantexture::Texture *diffuseTexture;
			glm::vec4 diffuseFactor = glm::vec4(1.0f);
			glm::vec3 specularFactor = glm::vec3(0.0f);
		} extension;

		struct PbrWorkflows
		{
			bool metallicRoughness = true;
			bool specularGlossiness = false;
		} pbrWorkflows;
		VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
	};

	struct Primitive
	{
		uint32_t firstIndex;
		uint32_t indexCount;
		uint32_t vertexCount;
		Material &material;
		bool hasIndices;
		BoundingBox bb;
		Primitive(uint32_t firstIndex, uint32_t indexCount, uint32_t vertexCount, Material &material);
		void setBoundingBox(glm::vec3 min, glm::vec3 max);
	};

	struct Mesh
	{
		oceancore::VulkanDevice *device;
		std::vector<Primitive *> primitives;
		BoundingBox bb;
		BoundingBox aabb;
		struct UniformBuffer
		{
			VkBuffer buffer;
			VkDeviceMemory memory;
			VkDescriptorBufferInfo descriptor;
			VkDescriptorSet descriptorSet;
			void *mapped;
		} uniformBuffer;
		struct UniformBlock
		{
			glm::mat4 matrix;
			glm::mat4 jointMatrix[MAX_NUM_JOINTS]{};
			float jointcount{0};
		} uniformBlock;
		Mesh(oceancore::VulkanDevice *device, glm::mat4 matrix);
		~Mesh();
		void setBoundingBox(glm::vec3 min, glm::vec3 max);
	};

	struct Skin
	{
		std::string name;
		Node *skeletonRoot = nullptr;
		std::vector<glm::mat4> inverseBindMatrices;
		std::vector<Node *> joints;
	};

	enum Shader
	{
		SHADER_PBR = 1,
		SHADER_BASE_COLOR = 2
	};

	struct Node
	{
		/// <summary>
		/// root-nodes
		/// </summary>
		uint32_t id = 0; // root-nodes = 0 (Nodes owned by gltf)
		uint32_t gltfId = 0;
		int32_t animationIndex = -1;
		float animationTimer = 0.0f;
		bool display = true;
		Shader shader = SHADER_PBR;
		//////////////

		Node *parent;
		uint32_t index;
		std::vector<Node *> children;
		glm::mat4 matrix;
		std::string name;
		Mesh *mesh;
		Skin *skin;
		int32_t skinIndex = -1;
		glm::vec3 translation{};
		glm::vec3 scale{1.0f};
		glm::quat rotation{};
		BoundingBox bvh;
		BoundingBox aabb;
		glm::mat4 getMatrix(bool hasScale = true);
		glm::mat4 localMatrix(bool hasScale = true);
		void update();
		~Node();

	private:
		glm::mat4 cachedMatrix;
		bool matrixCached = false;
		bool transformChanged = false;  // Flag indicating if transform has changed

		// Function to check if any transform has changed (translation, rotation, scale)
		bool hasTransformChanged() const {
			return transformChanged;
		}

		// Function to mark the matrix cache as invalid when a transform is updated
		void invalidateMatrixCache() {
			transformChanged = true;
			matrixCached = false;
		}
	};

	struct AnimationChannel
	{
		enum PathType
		{
			TRANSLATION,
			ROTATION,
			SCALE
		};
		PathType path;
		Node *node;
		uint32_t samplerIndex;
	};

	struct AnimationSampler
	{
		enum InterpolationType
		{
			LINEAR,
			STEP,
			CUBICSPLINE
		};
		InterpolationType interpolation;
		std::vector<float> inputs;
		std::vector<glm::vec4> outputsVec4;
	};

	struct Animation
	{
		std::string name;
		std::vector<AnimationSampler> samplers;
		std::vector<AnimationChannel> channels;
		float start = std::numeric_limits<float>::max();
		float end = std::numeric_limits<float>::min();
	};

	struct Model
	{
		///
		uint32_t id = 0;
		bool display = true;
		///

		// 2023-11-08 sollte hier sein!
		// VkDescriptorSet descriptorSet;
		//

		oceancore::VulkanDevice *device;

		struct Vertex
		{
			glm::vec3 pos;
			glm::vec3 normal;
			glm::vec2 uv0;
			glm::vec2 uv1;
			glm::vec4 joint0;
			glm::vec4 weight0;
			glm::vec4 color;
		};

		struct Vertices
		{
			VkBuffer buffer = VK_NULL_HANDLE;
			VkDeviceMemory memory;
		} vertices;

		struct Indices
		{
			VkBuffer buffer = VK_NULL_HANDLE;
			VkDeviceMemory memory;
		} indices;

		glm::mat4 aabb;

		std::vector<Node *> nodes;
		std::vector<Node *> linearNodes;
		std::vector<Skin *> skins;

		std::vector<oceantexture::Texture> textures;
		std::vector<TextureSampler> textureSamplers;
		std::vector<Material> materials;
		std::vector<Animation> animations;
		std::vector<std::string> extensions;

		struct Dimensions
		{
			glm::vec3 min = glm::vec3(FLT_MAX);
			glm::vec3 max = glm::vec3(-FLT_MAX);
		} dimensions;

		struct LoaderInfo
		{
			uint32_t *indexBuffer = nullptr;
			Vertex *vertexBuffer = nullptr;
			size_t indexPos = 0;
			size_t vertexPos = 0;
		} /*loaderInfo*/;

		/// <summary>
		/// Additional Nodes
		/// </summary>
		std::vector<std::pair<uint32_t, Node *>> additionalNodes = std::vector<std::pair<uint32_t, Node *>>();
		void addNode(uint32_t nodeId, glm::vec3 pos, glm::vec3 rot);
		// void loadNode2(Node *parent, const Node &node, uint32_t nodeIndex, LoaderInfo &loaderInfo, float globalscale);
		void changePosAndRot(glm::vec3 pos, glm::vec3 rot);
		///

		void destroy(VkDevice device);
		void loadNode(Node *parent, const tinygltf::Node &node, uint32_t nodeIndex, const tinygltf::Model &model, LoaderInfo &loaderInfo, float globalscale);
		void getNodeProps(const tinygltf::Node &node, const tinygltf::Model &model, size_t &vertexCount, size_t &indexCount);
		void loadSkins(const tinygltf::Model &gltfModel);
		void loadTextures(const tinygltf::Model &gltfModel, oceancore::VulkanDevice *device, VkQueue transferQueue);
		VkSamplerAddressMode getVkWrapMode(int32_t wrapMode);
		VkFilter getVkFilterMode(int32_t filterMode);
		void loadTextureSamplers(const tinygltf::Model &gltfModel);
		void loadMaterials(const tinygltf::Model &gltfModel);
		void loadAnimations(const tinygltf::Model &gltfModel, int defaultAnimationIndex = 0);
		void loadFromFile(std::string_view filename, oceancore::VulkanDevice *device, VkQueue transferQueue, float scale = 1.0f);
		void loadFromModel(uint32_t gltfId, const tinygltf::Model *gltfModel, oceancore::VulkanDevice *device, VkQueue transferQueue, float scale = 1.0f);
		void drawNode(Node *node, VkCommandBuffer commandBuffer);
		void draw(VkCommandBuffer commandBuffer);
		void calculateBoundingBox(Node *node, Node *parent);
		void getSceneDimensions();
		void updateAnimation(uint32_t index, float time);
		void updateAnimations(void);
		// void updateNodes(void);
		Node *findNode(Node *parent, uint32_t index);
		Node *nodeFromIndex(uint32_t index);
		void updateNodes(const tinygltf::Model *model);
		oceantexture::Texture loadTextureFromGltfImage(tinygltf::Image &gltfimage, TextureSampler textureSampler,
													   oceancore::VulkanDevice *device, VkQueue copyQueue);
		// void updateNodes(const com::rsc::grpc::GltfContext* gltfContext);
	};
}

#endif