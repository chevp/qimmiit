/**
 * Vulkan glTF model and texture loading class based on tinyglTF (https://github.com/syoyo/tinygltf)
 *
 * Copyright (C) 2018-2022 by Sascha Willems - www.saschawillems.de
 *
 * This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */

#include <coregfx/core/ocean_gltf.hpp>

namespace oceangltf
{
	// Bounding box

	BoundingBox::BoundingBox(glm::vec3 min, glm::vec3 max) : min(min), max(max){};

	BoundingBox BoundingBox::getAABB(glm::mat4 m)
	{
		glm::vec3 min = glm::vec3(m[3]);
		glm::vec3 max = min;
		glm::vec3 v0, v1;

		glm::vec3 right = glm::vec3(m[0]);
		v0 = right * this->min.x;
		v1 = right * this->max.x;
		min += glm::min(v0, v1);
		max += glm::max(v0, v1);

		glm::vec3 up = glm::vec3(m[1]);
		v0 = up * this->min.y;
		v1 = up * this->max.y;
		min += glm::min(v0, v1);
		max += glm::max(v0, v1);

		glm::vec3 back = glm::vec3(m[2]);
		v0 = back * this->min.z;
		v1 = back * this->max.z;
		min += glm::min(v0, v1);
		max += glm::max(v0, v1);

		return BoundingBox(min, max);
	}

	// Primitive
	Primitive::Primitive(uint32_t firstIndex, uint32_t indexCount, uint32_t vertexCount, Material &material) : firstIndex(firstIndex), indexCount(indexCount), vertexCount(vertexCount), material(material)
	{
		hasIndices = indexCount > 0;
	};

	void Primitive::setBoundingBox(glm::vec3 min, glm::vec3 max)
	{
		bb.min = min;
		bb.max = max;
		bb.valid = true;
	}

	// Mesh
	Mesh::Mesh(oceancore::VulkanDevice *device, glm::mat4 matrix)
	{
		this->device = device;
		this->uniformBlock.matrix = matrix;
		VK_CHECK_RESULT(device->createBuffer(
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			sizeof(uniformBlock),
			&uniformBuffer.buffer,
			&uniformBuffer.memory,
			"mesh",
			&uniformBlock));
		VK_CHECK_RESULT(vkMapMemory(device->logicalDevice, uniformBuffer.memory, 0, sizeof(uniformBlock), 0, &uniformBuffer.mapped));
		uniformBuffer.descriptor = {uniformBuffer.buffer, 0, sizeof(uniformBlock)};
	};

	Mesh::~Mesh()
	{
		vkDestroyBuffer(device->logicalDevice, uniformBuffer.buffer, nullptr);
		vkFreeMemory(device->logicalDevice, uniformBuffer.memory, nullptr);
		for (Primitive *p : primitives)
			delete p;
	}

	void Mesh::setBoundingBox(glm::vec3 min, glm::vec3 max)
	{
		bb.min = min;
		bb.max = max;
		bb.valid = true;
	}
	// Node
	
	// Model
	/*glm::mat4 Node::localMatrix(bool hasScale)
	{
		// if (hasScale)
		return glm::translate(glm::mat4(1.0f), translation) * glm::mat4(rotation) * glm::scale(glm::mat4(1.0f), scale) * matrix;

		// return glm::translate(glm::mat4(1.0f), translation) * glm::mat4(rotation) * matrix;
	}*/
	glm::mat4 Node::localMatrix(bool hasScale)
	{
		// Start with identity matrix and apply transformations
		glm::mat4 localMat = glm::mat4(1.0f);

		// Translation
		localMat = glm::translate(localMat, translation);

		// Rotation
		localMat *= glm::mat4(rotation);

		// Conditional scaling, only apply if hasScale is true
		if (hasScale)
		{
			localMat = glm::scale(localMat, scale);
		}

		// Apply the additional matrix if present
		return localMat * matrix;
	}
	/**
	 * Pro Parent muss diese while-Schlaufe eine Matrix-Multiplikation ausf�hren. Daher
	 * sind Animationen mit vielen Childs sehr langsam! Stattdessen Skins f�r bessere
	 * Performance benutzen.
	 */
	glm::mat4 Node::getMatrix(bool hasScale)
	{
		glm::mat4 m = localMatrix(hasScale);
		Node *p = parent;
		while (p)
		{
			m = p->localMatrix(hasScale) * m;
			p = p->parent;
		}
		return m;
	}

	/*glm::mat4 Node::getMatrix(bool hasScale)
	{
		// Check if the cached matrix is up-to-date, if not recompute
		if (!matrixCached || hasTransformChanged())
		{
			// Compute local matrix
			glm::mat4 m = localMatrix(hasScale);

			// Traverse the parent hierarchy and accumulate transformations
			Node* p = parent;
			while (p)
			{
				m = p->localMatrix(hasScale) * m;
				p = p->parent;
			}

			// Cache the result to avoid recomputation
			cachedMatrix = m;
			matrixCached = true;
			transformChanged = false;  // Reset the transform change flag
		}

		// Return the cached matrix
		return cachedMatrix;
	}*/

	void Node::update()
	{
		if (mesh)
		{
			glm::mat4 m = getMatrix();
			mesh->uniformBlock.matrix = m;

			if (skin)
			{
				// Update join matrices
				/*glm::mat4 inverseTransform = glm::inverse(m);
				size_t numJoints = std::min((uint32_t)skin->joints.size(), MAX_NUM_JOINTS);
				for (size_t i = 0; i < numJoints; i++)
				{
					Node *jointNode = skin->joints[i];
					glm::mat4 jointMat = jointNode->getMatrix(false) * skin->inverseBindMatrices[i];
					// glm::mat4 jointMat = jointNode->parentMatrixValue * skin->inverseBindMatrices[i];
					jointMat = inverseTransform * jointMat;
					mesh->uniformBlock.jointMatrix[i] = jointMat;
				}
				mesh->uniformBlock.jointcount = (float)numJoints;
				memcpy(mesh->uniformBuffer.mapped, &mesh->uniformBlock, sizeof(mesh->uniformBlock));*/
				
				/*// Cache inverse matrix (assuming `m` doesn't change during the loop)
				glm::mat4 inverseTransform = glm::inverse(m);

				// Limit the number of joints processed to a maximum
				size_t numJoints = std::min((uint32_t)skin->joints.size(), MAX_NUM_JOINTS);

				// Pointer to the joint matrix array to reduce repeated lookups
				glm::mat4* jointMatrixArray = mesh->uniformBlock.jointMatrix;

				// Use parallel processing for large joint counts if applicable
#pragma omp parallel for
				for (size_t i = 0; i < numJoints; i++)
				{
					// Retrieve the joint node and its inverse bind matrix
					Node* jointNode = skin->joints[i];

					// Compute the joint matrix (no need to reassign jointMat)
					glm::mat4 jointMat = inverseTransform * (jointNode->getMatrix(false) * skin->inverseBindMatrices[i]);

					// Write directly to the joint matrix array (sequential memory access)
					jointMatrixArray[i] = jointMat;
				}

				// Update joint count and copy data to uniform buffer
				mesh->uniformBlock.jointcount = static_cast<float>(numJoints);
				memcpy(mesh->uniformBuffer.mapped, &mesh->uniformBlock, sizeof(mesh->uniformBlock));*/
				
				// Enable SIMD optimizations for GLM
// #define GLM_FORCE_SSE2  // or GLM_FORCE_AVX depending on your hardware

// Cache inverse matrix (assuming `m` doesn't change during the loop)
				glm::mat4 inverseTransform = glm::inverse(m);

				// Limit the number of joints processed to a maximum
				size_t numJoints = std::min((uint32_t)skin->joints.size(), MAX_NUM_JOINTS);

				// Pointer to the joint matrix array to reduce repeated lookups
				glm::mat4* jointMatrixArray = mesh->uniformBlock.jointMatrix;

				// Precompute the joint matrices outside the loop if they remain constant
				std::vector<glm::mat4> cachedJointMatrices(numJoints);
				for (size_t i = 0; i < numJoints; i++) {
					Node* jointNode = skin->joints[i];
					cachedJointMatrices[i] = jointNode->getMatrix(false);  // Cache the result
				}

				// Use parallel processing to accelerate matrix computation
#pragma omp parallel for
				for (size_t i = 0; i < numJoints; i++)
				{
					// Retrieve the cached joint node matrix and its inverse bind matrix
					const glm::mat4& jointNodeMatrix = cachedJointMatrices[i];
					const glm::mat4& inverseBindMatrix = skin->inverseBindMatrices[i];

					// Compute the joint matrix efficiently
			glm::mat4 jointMat = inverseTransform * (jointNodeMatrix * inverseBindMatrix);

					// Write directly to the joint matrix array (sequential memory access)
					jointMatrixArray[i] = jointMat;
				}

				// Update joint count and copy data to uniform buffer
				mesh->uniformBlock.jointcount = static_cast<float>(numJoints);
				memcpy(mesh->uniformBuffer.mapped, &mesh->uniformBlock, sizeof(mesh->uniformBlock));
			}
			else
			{
				memcpy(mesh->uniformBuffer.mapped, &m, sizeof(glm::mat4));
			}
		}

		for (auto &child : children)
		{
			child->update();
		}
	}

	Node::~Node()
	{
		if (mesh)
		{
			delete mesh;
		}
		for (auto &child : children)
		{
			delete child;
		}
	}


	/// <summary>
	/// For adding an additionalNode
	/// </summary>
	void Model::addNode(uint32_t nodeId, glm::vec3 pos, glm::vec3 rot)
	{
		if (nodes.empty()) {
			ocean::error("Model::addNode failed - No nodes to clone");
			throw std::runtime_error("Model::addNode: No nodes in model");
		}

		// Clone ALL root nodes (GLTF models can have multiple root nodes in the scene)
		// Apply the transform to create a group of cloned nodes
		for (size_t i = 0; i < nodes.size(); i++)
		{
			Node *originalNode = nodes[i];
			Node *newNode = new Node{};
			newNode->index = originalNode->index;
			newNode->parent = nullptr;
			newNode->name = originalNode->name;
			newNode->skinIndex = originalNode->skinIndex;
			newNode->matrix = originalNode->getMatrix();

			// Apply transform only to first root node (others are relative to it)
			if (i == 0) {
				newNode->translation = pos;
				newNode->rotation = rot;
				newNode->scale = glm::vec3(1.0f);
			} else {
				newNode->translation = originalNode->translation;
				newNode->rotation = originalNode->rotation;
				newNode->scale = originalNode->scale;
			}

			newNode->mesh = originalNode->mesh;
			newNode->skin = originalNode->skin;

			if (newNode->mesh)
			{
				newNode->update();
			}

			// All cloned nodes go into additionalNodes with the same nodeId
			additionalNodes.push_back(std::pair<uint32_t, Node *>(nodeId, newNode));
		}
	}

	void Model::changePosAndRot(glm::vec3 pos, glm::vec3 rot)
	{
		assert(nodes.size() > 0);

		nodes[0]->translation = pos;
		nodes[0]->rotation = rot;
		nodes[0]->scale = glm::vec3(1.0f);
	}

	void Model::destroy(VkDevice device)
	{
		if (vertices.buffer != VK_NULL_HANDLE)
		{
			vkDestroyBuffer(device, vertices.buffer, nullptr);
			vkFreeMemory(device, vertices.memory, nullptr);
			vertices.buffer = VK_NULL_HANDLE;
		}
		if (indices.buffer != VK_NULL_HANDLE)
		{
			vkDestroyBuffer(device, indices.buffer, nullptr);
			vkFreeMemory(device, indices.memory, nullptr);
			indices.buffer = VK_NULL_HANDLE;
		}
		for (auto texture : textures)
		{
			texture.destroy();
		}
		textures.resize(0);
		textureSamplers.resize(0);
		for (auto node : nodes)
		{
			delete node;
		}
		materials.resize(0);
		animations.resize(0);
		nodes.resize(0);
		linearNodes.resize(0);
		extensions.resize(0);
		for (auto skin : skins)
		{
			delete skin;
		}
		skins.resize(0);
		additionalNodes.resize(0);
	};

	void Model::loadNode(Node *parent, const tinygltf::Node &node, uint32_t nodeIndex, const tinygltf::Model &model,
						 LoaderInfo &loaderInfo, float globalscale)
	{
		// std::cout << "oceangltf.loadNode nodeId: " << node.id << std::endl;
		Node *newNode = new Node{};
		newNode->index = nodeIndex;
		newNode->parent = parent;
		newNode->name = node.name;
		newNode->skinIndex = node.skin;
		newNode->matrix = glm::mat4(1.0f);
		newNode->id = node.id;
		
		if (node.shader == "base")
		{
			newNode->shader = Shader::SHADER_BASE_COLOR;
		}
		else
		{
			newNode->shader = Shader::SHADER_PBR;
		}

		// Generate local node matrix
		glm::vec3 translation = glm::vec3(0.0f);
		if (node.translation.size() == 3)
		{
			translation = glm::make_vec3(node.translation.data());
			newNode->translation = translation;
		}
		glm::mat4 rotation = glm::mat4(1.0f);
		if (node.rotation.size() == 4)
		{
			glm::quat q = glm::make_quat(node.rotation.data());
			newNode->rotation = glm::mat4(q);
		}
		glm::vec3 scale = glm::vec3(1.0f);
		if (node.scale.size() == 3)
		{
			scale = glm::make_vec3(node.scale.data());
			newNode->scale = scale;
		}
		if (node.matrix.size() == 16)
		{
			newNode->matrix = glm::make_mat4x4(node.matrix.data());
		};

		// Node with children
		if (node.children.size() > 0)
		{
			for (size_t i = 0; i < node.children.size(); i++)
			{
				loadNode(newNode, model.nodes[node.children[i]], node.children[i], model, loaderInfo, globalscale);
			}
		}

		// Node contains mesh data
		if (node.mesh > -1)
		{
			const tinygltf::Mesh mesh = model.meshes[node.mesh];
			Mesh *newMesh = new Mesh(device, newNode->matrix);
			for (size_t j = 0; j < mesh.primitives.size(); j++)
			{
				const tinygltf::Primitive &primitive = mesh.primitives[j];
				uint32_t vertexStart = static_cast<uint32_t>(loaderInfo.vertexPos);
				uint32_t indexStart = static_cast<uint32_t>(loaderInfo.indexPos);
				uint32_t indexCount = 0;
				uint32_t vertexCount = 0;
				glm::vec3 posMin{};
				glm::vec3 posMax{};
				bool hasSkin = false;
				bool hasIndices = primitive.indices > -1;
				// Vertices
				{
					const float *bufferPos = nullptr;
					const float *bufferNormals = nullptr;
					const float *bufferTexCoordSet0 = nullptr;
					const float *bufferTexCoordSet1 = nullptr;
					const float *bufferColorSet0 = nullptr;
					const void *bufferJoints = nullptr;
					const float *bufferWeights = nullptr;

					int posByteStride;
					int normByteStride;
					int uv0ByteStride;
					int uv1ByteStride;
					int color0ByteStride;
					int jointByteStride;
					int weightByteStride;

					int jointComponentType;

					// Position attribute is required
					assert(primitive.attributes.find("POSITION") != primitive.attributes.end());

					const tinygltf::Accessor &posAccessor = model.accessors[primitive.attributes.find("POSITION")->second];
					const tinygltf::BufferView &posView = model.bufferViews[posAccessor.bufferView];
					bufferPos = reinterpret_cast<const float *>(&(model.buffers[posView.buffer].data[posAccessor.byteOffset + posView.byteOffset]));
					posMin = glm::vec3(posAccessor.minValues[0], posAccessor.minValues[1], posAccessor.minValues[2]);
					posMax = glm::vec3(posAccessor.maxValues[0], posAccessor.maxValues[1], posAccessor.maxValues[2]);
					vertexCount = static_cast<uint32_t>(posAccessor.count);
					posByteStride = posAccessor.ByteStride(posView) ? (posAccessor.ByteStride(posView) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3);

					if (primitive.attributes.find("NORMAL") != primitive.attributes.end())
					{
						const tinygltf::Accessor &normAccessor = model.accessors[primitive.attributes.find("NORMAL")->second];
						const tinygltf::BufferView &normView = model.bufferViews[normAccessor.bufferView];
						bufferNormals = reinterpret_cast<const float *>(&(model.buffers[normView.buffer].data[normAccessor.byteOffset + normView.byteOffset]));
						normByteStride = normAccessor.ByteStride(normView) ? (normAccessor.ByteStride(normView) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3);
					}

					// UVs
					if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end())
					{
						const tinygltf::Accessor &uvAccessor = model.accessors[primitive.attributes.find("TEXCOORD_0")->second];
						const tinygltf::BufferView &uvView = model.bufferViews[uvAccessor.bufferView];
						bufferTexCoordSet0 = reinterpret_cast<const float *>(&(model.buffers[uvView.buffer].data[uvAccessor.byteOffset + uvView.byteOffset]));
						uv0ByteStride = uvAccessor.ByteStride(uvView) ? (uvAccessor.ByteStride(uvView) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC2);
					}
					if (primitive.attributes.find("TEXCOORD_1") != primitive.attributes.end())
					{
						const tinygltf::Accessor &uvAccessor = model.accessors[primitive.attributes.find("TEXCOORD_1")->second];
						const tinygltf::BufferView &uvView = model.bufferViews[uvAccessor.bufferView];
						bufferTexCoordSet1 = reinterpret_cast<const float *>(&(model.buffers[uvView.buffer].data[uvAccessor.byteOffset + uvView.byteOffset]));
						uv1ByteStride = uvAccessor.ByteStride(uvView) ? (uvAccessor.ByteStride(uvView) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC2);
					}

					// Vertex colors
					if (primitive.attributes.find("COLOR_0") != primitive.attributes.end())
					{
						const tinygltf::Accessor &accessor = model.accessors[primitive.attributes.find("COLOR_0")->second];
						const tinygltf::BufferView &view = model.bufferViews[accessor.bufferView];
						bufferColorSet0 = reinterpret_cast<const float *>(&(model.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
						color0ByteStride = accessor.ByteStride(view) ? (accessor.ByteStride(view) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3);
					}

					// Skinning
					// Joints
					if (primitive.attributes.find("JOINTS_0") != primitive.attributes.end())
					{
						const tinygltf::Accessor &jointAccessor = model.accessors[primitive.attributes.find("JOINTS_0")->second];
						const tinygltf::BufferView &jointView = model.bufferViews[jointAccessor.bufferView];
						bufferJoints = &(model.buffers[jointView.buffer].data[jointAccessor.byteOffset + jointView.byteOffset]);
						jointComponentType = jointAccessor.componentType;
						jointByteStride = jointAccessor.ByteStride(jointView) ? (jointAccessor.ByteStride(jointView) / tinygltf::GetComponentSizeInBytes(jointComponentType)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC4);
					}

					if (primitive.attributes.find("WEIGHTS_0") != primitive.attributes.end())
					{
						const tinygltf::Accessor &weightAccessor = model.accessors[primitive.attributes.find("WEIGHTS_0")->second];
						const tinygltf::BufferView &weightView = model.bufferViews[weightAccessor.bufferView];
						bufferWeights = reinterpret_cast<const float *>(&(model.buffers[weightView.buffer].data[weightAccessor.byteOffset + weightView.byteOffset]));
						weightByteStride = weightAccessor.ByteStride(weightView) ? (weightAccessor.ByteStride(weightView) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC4);
					}

					hasSkin = (bufferJoints && bufferWeights);

					for (size_t v = 0; v < posAccessor.count; v++)
					{
						// std::cout << "posAccessor.count=" << posAccessor.count << " loaderInfo.vertexPos=" << loaderInfo.vertexPos << std::endl;
						Vertex &vert = loaderInfo.vertexBuffer[loaderInfo.vertexPos];
						vert.pos = glm::vec4(glm::make_vec3(&bufferPos[v * posByteStride]), 1.0f);
						vert.normal = glm::normalize(glm::vec3(bufferNormals ? glm::make_vec3(&bufferNormals[v * normByteStride]) : glm::vec3(0.0f)));
						vert.uv0 = bufferTexCoordSet0 ? glm::make_vec2(&bufferTexCoordSet0[v * uv0ByteStride]) : glm::vec3(0.0f);
						vert.uv1 = bufferTexCoordSet1 ? glm::make_vec2(&bufferTexCoordSet1[v * uv1ByteStride]) : glm::vec3(0.0f);

						vert.color = bufferColorSet0 ? glm::make_vec4(&bufferColorSet0[v * color0ByteStride]) : glm::vec4(1.0f);

						if (hasSkin)
						{
							switch (jointComponentType)
							{
							case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
							{
								const uint16_t *buf = static_cast<const uint16_t *>(bufferJoints);
								vert.joint0 = glm::vec4(glm::make_vec4(&buf[v * jointByteStride]));
								break;
							}
							case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
							{
								const uint8_t *buf = static_cast<const uint8_t *>(bufferJoints);
								vert.joint0 = glm::vec4(glm::make_vec4(&buf[v * jointByteStride]));
								break;
							}
							default:
								// Not supported by spec
								std::cerr << "Joint component type " << jointComponentType << " not supported!" << std::endl;
								break;
							}
						}
						else
						{
							vert.joint0 = glm::vec4(0.0f);
						}
						vert.weight0 = hasSkin ? glm::make_vec4(&bufferWeights[v * weightByteStride]) : glm::vec4(0.0f);
						// Fix for all zero weights
						if (glm::length(vert.weight0) == 0.0f)
						{
							vert.weight0 = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
						}
						// std::cout << "vertexPos++ loaderInfo.vertexPos=" << loaderInfo.vertexPos << std::endl;
						loaderInfo.vertexPos++;
					}
				}
				// Indices
				if (hasIndices)
				{
					const tinygltf::Accessor &accessor = model.accessors[primitive.indices > -1 ? primitive.indices : 0];
					const tinygltf::BufferView &bufferView = model.bufferViews[accessor.bufferView];
					const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];

					indexCount = static_cast<uint32_t>(accessor.count);
					const void *dataPtr = &(buffer.data[accessor.byteOffset + bufferView.byteOffset]);

					switch (accessor.componentType)
					{
					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
					{
						const uint32_t *buf = static_cast<const uint32_t *>(dataPtr);
						for (size_t index = 0; index < accessor.count; index++)
						{
							loaderInfo.indexBuffer[loaderInfo.indexPos] = buf[index] + vertexStart;
							loaderInfo.indexPos++;
						}
						break;
					}
					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
					{
						const uint16_t *buf = static_cast<const uint16_t *>(dataPtr);
						for (size_t index = 0; index < accessor.count; index++)
						{
							loaderInfo.indexBuffer[loaderInfo.indexPos] = buf[index] + vertexStart;
							loaderInfo.indexPos++;
						}
						break;
					}
					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
					{
						const uint8_t *buf = static_cast<const uint8_t *>(dataPtr);
						for (size_t index = 0; index < accessor.count; index++)
						{
							loaderInfo.indexBuffer[loaderInfo.indexPos] = buf[index] + vertexStart;
							loaderInfo.indexPos++;
						}
						break;
					}
					default:
						std::cerr << "Index component type " << accessor.componentType << " not supported!" << std::endl;
						return;
					}
				}
				Primitive *newPrimitive = new Primitive(indexStart, indexCount, vertexCount, primitive.material > -1 ? materials[primitive.material] : materials.back());
				newPrimitive->setBoundingBox(posMin, posMax);
				newMesh->primitives.push_back(newPrimitive);
			}
			// Mesh BB from BBs of primitives
			for (auto p : newMesh->primitives)
			{
				if (p->bb.valid && !newMesh->bb.valid)
				{
					newMesh->bb = p->bb;
					newMesh->bb.valid = true;
				}
				newMesh->bb.min = glm::min(newMesh->bb.min, p->bb.min);
				newMesh->bb.max = glm::max(newMesh->bb.max, p->bb.max);
			}
			newNode->mesh = newMesh;
		}
		if (parent)
		{
			parent->children.push_back(newNode);
		}
		else
		{
			nodes.push_back(newNode);
		}
		linearNodes.push_back(newNode);
	}

	void Model::getNodeProps(const tinygltf::Node &node, const tinygltf::Model &model, size_t &vertexCount, size_t &indexCount)
	{
		if (node.children.size() > 0)
		{
			for (size_t i = 0; i < node.children.size(); i++)
			{
				getNodeProps(model.nodes[node.children[i]], model, vertexCount, indexCount);
			}
		}
		if (node.mesh > -1)
		{
			const tinygltf::Mesh mesh = model.meshes[node.mesh];
			for (size_t i = 0; i < mesh.primitives.size(); i++)
			{
				auto primitive = mesh.primitives[i];
				vertexCount += model.accessors[primitive.attributes.find("POSITION")->second].count;
				if (primitive.indices > -1)
				{
					indexCount += model.accessors[primitive.indices].count;
				}
			}
		}
	}

	void Model::loadSkins(const tinygltf::Model &gltfModel)
	{
		for (tinygltf::Skin const &source : gltfModel.skins)
		{
			Skin *newSkin = new Skin{};
			newSkin->name = source.name;

			// Find skeleton root node
			if (source.skeleton > -1)
			{
				newSkin->skeletonRoot = nodeFromIndex(source.skeleton);
			}

			// Find joint nodes
			for (int jointIndex : source.joints)
			{
				Node *node = nodeFromIndex(jointIndex);
				if (node)
				{
					newSkin->joints.push_back(nodeFromIndex(jointIndex));
				}
			}

			// Get inverse bind matrices from buffer
			if (source.inverseBindMatrices > -1)
			{
				const tinygltf::Accessor &accessor = gltfModel.accessors[source.inverseBindMatrices];
				const tinygltf::BufferView &bufferView = gltfModel.bufferViews[accessor.bufferView];
				const tinygltf::Buffer &buffer = gltfModel.buffers[bufferView.buffer];
				newSkin->inverseBindMatrices.resize(accessor.count);
				memcpy(newSkin->inverseBindMatrices.data(), &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(glm::mat4));
			}

			skins.push_back(newSkin);
		}
	}

	void Model::loadTextures(const tinygltf::Model &gltfModel, oceancore::VulkanDevice *device, VkQueue transferQueue)
	{
		for (tinygltf::Texture const &tex : gltfModel.textures)
		{
			tinygltf::Image image = gltfModel.images[tex.source];
			TextureSampler textureSampler;
			if (tex.sampler == -1)
			{
				// No sampler specified, use a default one
				textureSampler.magFilter = VK_FILTER_LINEAR;
				textureSampler.minFilter = VK_FILTER_LINEAR;
				textureSampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
				textureSampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
				textureSampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			}
			else
			{
				textureSampler = textureSamplers[tex.sampler];
			}
			// GltfTexture texture;
			// texture.fromGltfImage(image, textureSampler, device, transferQueue);
			// textures.push_back(texture);
			oceantexture::Texture t = loadTextureFromGltfImage(image, textureSampler, device, transferQueue);
			textures.push_back(t);
		}
	}

	oceantexture::Texture Model::loadTextureFromGltfImage(tinygltf::Image& gltfimage, TextureSampler textureSampler,
		oceancore::VulkanDevice* device, VkQueue copyQueue)
	{
		auto start = std::chrono::high_resolution_clock::now();

		oceantexture::Texture texture;
		texture.device = device;

		std::vector<unsigned char> buffer;
		VkDeviceSize bufferSize = 0;

		if (gltfimage.component == 3) {
			// Convert RGB to RGBA
			bufferSize = gltfimage.width * gltfimage.height * 4;
			buffer.resize(bufferSize);
			unsigned char* rgba = buffer.data();
			const unsigned char* rgb = gltfimage.image.data();
			for (size_t i = 0; i < gltfimage.width * gltfimage.height; ++i) {
				std::copy(rgb, rgb + 3, rgba);
				rgba += 4;
				rgb += 3;
			}
		}
		else {
			buffer = gltfimage.image;
			bufferSize = buffer.size();
		}

		VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
		VkFormatProperties formatProperties;

		texture.width = gltfimage.width;
		texture.height = gltfimage.height;
		texture.mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texture.width, texture.height)))) + 1;

		vkGetPhysicalDeviceFormatProperties(device->physicalDevice, format, &formatProperties);
		assert(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT);
		assert(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT);

		// Create staging buffer
		VkBuffer stagingBuffer;
		VkDeviceMemory stagingMemory;
		VkBufferCreateInfo bufferCreateInfo{};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.size = bufferSize;
		bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		VK_CHECK_RESULT(vkCreateBuffer(device->logicalDevice, &bufferCreateInfo, nullptr, &stagingBuffer));

		VkMemoryRequirements memReqs;
		vkGetBufferMemoryRequirements(device->logicalDevice, stagingBuffer, &memReqs);
		VkMemoryAllocateInfo memAllocInfo{};
		memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memAllocInfo.allocationSize = memReqs.size;
		memAllocInfo.memoryTypeIndex = device->getMemoryType(memReqs.memoryTypeBits,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		VK_CHECK_RESULT(vkAllocateMemory(device->logicalDevice, &memAllocInfo, nullptr, &stagingMemory));
		VK_CHECK_RESULT(vkBindBufferMemory(device->logicalDevice, stagingBuffer, stagingMemory, 0));

		void* data;
		VK_CHECK_RESULT(vkMapMemory(device->logicalDevice, stagingMemory, 0, memReqs.size, 0, &data));
		std::memcpy(data, buffer.data(), bufferSize);
		vkUnmapMemory(device->logicalDevice, stagingMemory);

		// Create image
		VkImageCreateInfo imageCreateInfo{};
		imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
		imageCreateInfo.format = format;
		imageCreateInfo.extent = { texture.width, texture.height, 1 };
		imageCreateInfo.mipLevels = texture.mipLevels;
		imageCreateInfo.arrayLayers = 1;
		imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		VK_CHECK_RESULT(vkCreateImage(device->logicalDevice, &imageCreateInfo, nullptr, &texture.image));

		vkGetImageMemoryRequirements(device->logicalDevice, texture.image, &memReqs);
		memAllocInfo.allocationSize = memReqs.size;
		memAllocInfo.memoryTypeIndex = device->getMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		VK_CHECK_RESULT(vkAllocateMemory(device->logicalDevice, &memAllocInfo, nullptr, &texture.deviceMemory));
		VK_CHECK_RESULT(vkBindImageMemory(device->logicalDevice, texture.image, texture.deviceMemory, 0));

		VkCommandBuffer copyCmd = device->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

		// Transition image layout and copy buffer to image
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = texture.image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.layerCount = 1;

		barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		vkCmdPipelineBarrier(copyCmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
			0, 0, nullptr, 0, nullptr, 1, &barrier);

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;
		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = { texture.width, texture.height, 1 };
		vkCmdCopyBufferToImage(copyCmd, stagingBuffer, texture.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

		// Transition image layout for shader read
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		vkCmdPipelineBarrier(copyCmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			0, 0, nullptr, 0, nullptr, 1, &barrier);

		device->flushCommandBuffer(copyCmd, copyQueue, true);

		vkFreeMemory(device->logicalDevice, stagingMemory, nullptr);
		vkDestroyBuffer(device->logicalDevice, stagingBuffer, nullptr);

		// Generate mip chain
		VkCommandBuffer blitCmd = device->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
		for (size_t i = 1; i < texture.mipLevels; i++) {
			VkImageBlit blit{};
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.mipLevel = i - 1;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = 1;
			blit.srcOffsets[1] = { static_cast<int32_t>(texture.width >> (i - 1)),
								  static_cast<int32_t>(texture.height >> (i - 1)),
								  1 };
			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.mipLevel = i;
			blit.dstSubresource.baseArrayLayer = 0;
			blit.dstSubresource.layerCount = 1;
			blit.dstOffsets[1] = { static_cast<int32_t>(texture.width >> i),
								  static_cast<int32_t>(texture.height >> i),
								  1 };

			VkImageMemoryBarrier blitBarrier{};
			blitBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			blitBarrier.image = texture.image;
			blitBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blitBarrier.subresourceRange.baseMipLevel = i;
			blitBarrier.subresourceRange.levelCount = 1;
			blitBarrier.subresourceRange.layerCount = 1;

			blitBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			blitBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			blitBarrier.srcAccessMask = 0;
			blitBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			vkCmdPipelineBarrier(blitCmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
				0, 0, nullptr, 0, nullptr, 1, &blitBarrier);

			vkCmdBlitImage(blitCmd, texture.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				texture.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit, VK_FILTER_LINEAR);

			blitBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			blitBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			blitBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			blitBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			vkCmdPipelineBarrier(blitCmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
				0, 0, nullptr, 0, nullptr, 1, &blitBarrier);
		}

		VkImageSubresourceRange subresourceRange{};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = texture.mipLevels;
		subresourceRange.layerCount = 1;

		VkImageMemoryBarrier finalBarrier{};
		finalBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		finalBarrier.image = texture.image;
		finalBarrier.subresourceRange = subresourceRange;
		finalBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		finalBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		finalBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		finalBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		vkCmdPipelineBarrier(blitCmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			0, 0, nullptr, 0, nullptr, 1, &finalBarrier);

		device->flushCommandBuffer(blitCmd, copyQueue, true);

		// Create sampler
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = textureSampler.magFilter;
		samplerInfo.minFilter = textureSampler.minFilter;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.addressModeU = textureSampler.addressModeU;
		samplerInfo.addressModeV = textureSampler.addressModeV;
		samplerInfo.addressModeW = textureSampler.addressModeW;
		samplerInfo.compareOp = VK_COMPARE_OP_NEVER;
		samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
		samplerInfo.maxAnisotropy = 8.0f;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxLod = static_cast<float>(texture.mipLevels);
		VK_CHECK_RESULT(vkCreateSampler(device->logicalDevice, &samplerInfo, nullptr, &texture.sampler));

		// Create image view
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = texture.image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = texture.mipLevels;
		viewInfo.subresourceRange.layerCount = 1;
		VK_CHECK_RESULT(vkCreateImageView(device->logicalDevice, &viewInfo, nullptr, &texture.view));

		// Setup descriptor
		texture.descriptor.sampler = texture.sampler;
		texture.descriptor.imageView = texture.view;
		texture.descriptor.imageLayout = texture.imageLayout;

		// Record the end time
		auto end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> elapsed = end - start;

		// Output compact texture load info
		std::string textureName = gltfimage.name.empty() ? "unnamed" : gltfimage.name;
		ocean::info("Texture loaded: name=" + textureName +
		           ", size=" + std::to_string(texture.width) + "x" + std::to_string(texture.height) +
		           ", mipLevels=" + std::to_string(texture.mipLevels) +
		           ", time=" + std::to_string(elapsed.count()) + "s");

		return texture;
	}

	VkSamplerAddressMode Model::getVkWrapMode(int32_t wrapMode)
	{
		switch (wrapMode)
		{
		case -1:
		case 10497:
			return VK_SAMPLER_ADDRESS_MODE_REPEAT;
		case 33071:
			return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		case 33648:
			return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		}

		std::cerr << "Unknown wrap mode for getVkWrapMode: " << wrapMode << std::endl;
		return VK_SAMPLER_ADDRESS_MODE_REPEAT;
	}

	VkFilter Model::getVkFilterMode(int32_t filterMode)
	{
		switch (filterMode)
		{
		case -1:
		case 9728:
			return VK_FILTER_NEAREST;
		case 9729:
			return VK_FILTER_LINEAR;
		case 9984:
			return VK_FILTER_NEAREST;
		case 9985:
			return VK_FILTER_NEAREST;
		case 9986:
			return VK_FILTER_LINEAR;
		case 9987:
			return VK_FILTER_LINEAR;
		}

		std::cerr << "Unknown filter mode for getVkFilterMode: " << filterMode << std::endl;
		return VK_FILTER_NEAREST;
	}

	void Model::loadTextureSamplers(const tinygltf::Model &gltfModel)
	{
		for (tinygltf::Sampler smpl : gltfModel.samplers)
		{
			TextureSampler sampler{};
			sampler.minFilter = getVkFilterMode(smpl.minFilter);
			sampler.magFilter = getVkFilterMode(smpl.magFilter);
			sampler.addressModeU = getVkWrapMode(smpl.wrapS);
			sampler.addressModeV = getVkWrapMode(smpl.wrapT);
			sampler.addressModeW = sampler.addressModeV;
			textureSamplers.push_back(sampler);
		}
	}

	void Model::loadMaterials(const tinygltf::Model &gltfModel)
	{
		ocean::info("=== LOADING MATERIALS ===");
		ocean::info("Processing " + std::to_string(gltfModel.materials.size()) + " materials");
		std::cout << "VkGltfModel.loadMaterials gltfModel" << std::endl;

		size_t materialIndex = 0;
		for (tinygltf::Material const &mat : gltfModel.materials)
		{
			ocean::info("Processing material " + std::to_string(materialIndex) + ": '" + mat.name + "'");
			Material material{};
			material.doubleSided = mat.doubleSided;

			// Log material properties
			ocean::info("  Double-sided: " + std::string(mat.doubleSided ? "yes" : "no"));
			ocean::info("  Base color factor: (" + std::to_string(mat.pbrMetallicRoughness.baseColorFactor[0]) +
			           ", " + std::to_string(mat.pbrMetallicRoughness.baseColorFactor[1]) +
			           ", " + std::to_string(mat.pbrMetallicRoughness.baseColorFactor[2]) +
			           ", " + std::to_string(mat.pbrMetallicRoughness.baseColorFactor[3]) + ")");
			ocean::info("  Metallic factor: " + std::to_string(mat.pbrMetallicRoughness.metallicFactor));
			ocean::info("  Roughness factor: " + std::to_string(mat.pbrMetallicRoughness.roughnessFactor));

			/*if (mat.values.find("baseColorTexture") != mat.values.end()) {
				std::cout << "name=" << mat.name << " mat.pbrMetallicRoughness.baseColorTexture.index = "
					<< mat.pbrMetallicRoughness.baseColorTexture.index << " baseColorTexture="
					<< std::to_string(mat.values.at("baseColorTexture").TextureIndex()) << std::endl;
			}
			else {
				std::cout << "name=" << mat.name << " mat.pbrMetallicRoughness.baseColorTexture.index = "
					<< mat.pbrMetallicRoughness.baseColorTexture.index << " baseColorTexture=NULL" << std::endl;
			}*/

			if (mat.pbrMetallicRoughness.baseColorTexture.index != -1)
			{
				material.baseColorTexture = &textures[mat.pbrMetallicRoughness.baseColorTexture.index];
				material.texCoordSets.baseColor = mat.pbrMetallicRoughness.baseColorTexture.texCoord;
				ocean::info("  Base color texture: index " + std::to_string(mat.pbrMetallicRoughness.baseColorTexture.index) +
				           ", texCoord " + std::to_string(mat.pbrMetallicRoughness.baseColorTexture.texCoord));
			}
			if (mat.pbrMetallicRoughness.metallicRoughnessTexture.index != -1)
			{
				material.metallicRoughnessTexture = &textures[mat.pbrMetallicRoughness.metallicRoughnessTexture.index];
				material.texCoordSets.metallicRoughness = mat.pbrMetallicRoughness.metallicRoughnessTexture.texCoord;
				ocean::info("  Metallic-roughness texture: index " + std::to_string(mat.pbrMetallicRoughness.metallicRoughnessTexture.index) +
				           ", texCoord " + std::to_string(mat.pbrMetallicRoughness.metallicRoughnessTexture.texCoord));
			}
			// if (mat.values.find("roughnessFactor") != mat.values.end()) {
			//	material.roughnessFactor = static_cast<float>(mat.values.at("roughnessFactor").Factor());
			// }
			material.roughnessFactor = static_cast<float>(mat.pbrMetallicRoughness.roughnessFactor);
			// if (mat.values.find("metallicFactor") != mat.values.end()) {
			//	material.metallicFactor = static_cast<float>(mat.values.at("metallicFactor").Factor());
			// }
			material.metallicFactor = static_cast<float>(mat.pbrMetallicRoughness.metallicFactor);

			// if (mat.values.find("baseColorFactor") != mat.values.end()) {
			//	material.baseColorFactor = glm::make_vec4(mat.values.at("baseColorFactor").ColorFactor().data());
			// }
			material.baseColorFactor = glm::make_vec4(mat.pbrMetallicRoughness.baseColorFactor.data());

			// if (mat.additionalValues.find("normalTexture") != mat.additionalValues.end()) {
			//	material.normalTexture = &textures[mat.additionalValues.at("normalTexture").TextureIndex()];
			//	material.texCoordSets.normal = mat.additionalValues.at("normalTexture").TextureTexCoord();
			// }
			if (mat.normalTexture.index != -1)
			{
				material.normalTexture = &textures[mat.normalTexture.index];
				material.texCoordSets.normal = mat.normalTexture.texCoord;
			}
			// if (mat.additionalValues.find("emissiveTexture") != mat.additionalValues.end()) {
			//	material.emissiveTexture = &textures[mat.additionalValues.at("emissiveTexture").TextureIndex()];
			//	material.texCoordSets.emissive = mat.additionalValues.at("emissiveTexture").TextureTexCoord();
			// }
			if (mat.emissiveTexture.index != -1)
			{
				material.emissiveTexture = &textures[mat.emissiveTexture.index];
				material.texCoordSets.emissive = mat.emissiveTexture.texCoord;
			}

			// if (mat.additionalValues.find("occlusionTexture") != mat.additionalValues.end()) {
			//	material.occlusionTexture = &textures[mat.additionalValues.at("occlusionTexture").TextureIndex()];
			//	material.texCoordSets.occlusion = mat.additionalValues.at("occlusionTexture").TextureTexCoord();
			// }
			if (mat.occlusionTexture.index != -1)
			{
				material.occlusionTexture = &textures[mat.occlusionTexture.index];
				material.texCoordSets.occlusion = mat.occlusionTexture.texCoord;
			}
			if (mat.alphaMode == "BLEND")
			{
				material.alphaMode = Material::ALPHAMODE_BLEND;
			}
			if (mat.alphaMode == "MASK")
			{
				material.alphaCutoff = 0.5f;
				material.alphaMode = Material::ALPHAMODE_MASK;
			}
			/*if (mat.additionalValues.find("alphaMode") != mat.additionalValues.end()) {
				tinygltf::Parameter param = mat.additionalValues.at("alphaMode");
				if (param.string_value == "BLEND") {
					material.alphaMode = Material::ALPHAMODE_BLEND;
				}
				if (param.string_value == "MASK") {
					material.alphaCutoff = 0.5f;
					material.alphaMode = Material::ALPHAMODE_MASK;
				}
			}*/
			// if (mat.additionalValues.find("alphaCutoff") != mat.additionalValues.end()) {
			//	material.alphaCutoff = static_cast<float>(mat.additionalValues.at("alphaCutoff").Factor());
			// }
			material.alphaCutoff = static_cast<float>(mat.alphaCutoff);

			// if (mat.additionalValues.find("emissiveFactor") != mat.additionalValues.end()) {
			//	material.emissiveFactor = glm::vec4(glm::make_vec3(mat.additionalValues.at("emissiveFactor").ColorFactor().data()), 1.0);
			// }
			material.emissiveFactor = glm::vec4(glm::make_vec3(mat.emissiveFactor.data()), 1.0);

			// Extensions
			// @TODO: Find out if there is a nicer way of reading these properties with recent tinygltf headers
			if (mat.extensions.find("KHR_materials_pbrSpecularGlossiness") != mat.extensions.end())
			{
				auto ext = mat.extensions.find("KHR_materials_pbrSpecularGlossiness");
				if (ext->second.Has("specularGlossinessTexture"))
				{
					auto index = ext->second.Get("specularGlossinessTexture").Get("index");
					material.extension.specularGlossinessTexture = &textures[index.Get<int>()];
					auto texCoordSet = ext->second.Get("specularGlossinessTexture").Get("texCoord");
					material.texCoordSets.specularGlossiness = texCoordSet.Get<int>();
					material.pbrWorkflows.specularGlossiness = true;
				}
				if (ext->second.Has("diffuseTexture"))
				{
					auto index = ext->second.Get("diffuseTexture").Get("index");
					material.extension.diffuseTexture = &textures[index.Get<int>()];
				}
				if (ext->second.Has("diffuseFactor"))
				{
					auto factor = ext->second.Get("diffuseFactor");
					for (size_t i = 0; i < factor.ArrayLen(); i++)
					{
						auto val = factor.Get(i);
						material.extension.diffuseFactor[i] = val.IsNumber() ? (float)val.Get<double>() : (float)val.Get<int>();
					}
				}
				if (ext->second.Has("specularFactor"))
				{
					auto factor = ext->second.Get("specularFactor");
					for (size_t i = 0; i < factor.ArrayLen(); i++)
					{
						auto val = factor.Get(i);
						material.extension.specularFactor[i] = val.IsNumber() ? (float)val.Get<double>() : (float)val.Get<int>();
					}
				}
			}

			materials.push_back(material);
			materialIndex++;
		}
		// Push a default material at the end of the list for meshes with no material assigned
		materials.push_back(Material());
		ocean::info("Materials loading completed. Total materials: " + std::to_string(materials.size()) + " (including default material)");
	}

	void Model::loadAnimations(const tinygltf::Model &gltfModel, int defaultAnimationIndex)
	{
		for (auto node : nodes)
		{
			node->animationIndex = defaultAnimationIndex;
		}

		for (tinygltf::Animation const &anim : gltfModel.animations)
		{
			Animation animation{};
			animation.name = anim.name;
			if (anim.name.empty())
			{
				animation.name = std::to_string(animations.size());
			}

			// Samplers
			for (auto &samp : anim.samplers)
			{
				AnimationSampler sampler{};

				if (samp.interpolation == "LINEAR")
				{
					sampler.interpolation = AnimationSampler::InterpolationType::LINEAR;
				}
				if (samp.interpolation == "STEP")
				{
					sampler.interpolation = AnimationSampler::InterpolationType::STEP;
				}
				if (samp.interpolation == "CUBICSPLINE")
				{
					sampler.interpolation = AnimationSampler::InterpolationType::CUBICSPLINE;
				}

				// Read sampler input time values
				{
					const tinygltf::Accessor &accessor = gltfModel.accessors[samp.input];
					const tinygltf::BufferView &bufferView = gltfModel.bufferViews[accessor.bufferView];
					const tinygltf::Buffer &buffer = gltfModel.buffers[bufferView.buffer];

					assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

					const void *dataPtr = &buffer.data[accessor.byteOffset + bufferView.byteOffset];
					const float *buf = static_cast<const float *>(dataPtr);
					for (size_t index = 0; index < accessor.count; index++)
					{
						sampler.inputs.push_back(buf[index]);
					}

					for (auto input : sampler.inputs)
					{
						if (input < animation.start)
						{
							animation.start = input;
						};
						if (input > animation.end)
						{
							animation.end = input;
						}
					}
				}

				// Read sampler output T/R/S values
				{
					const tinygltf::Accessor &accessor = gltfModel.accessors[samp.output];
					const tinygltf::BufferView &bufferView = gltfModel.bufferViews[accessor.bufferView];
					const tinygltf::Buffer &buffer = gltfModel.buffers[bufferView.buffer];

					assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

					const void *dataPtr = &buffer.data[accessor.byteOffset + bufferView.byteOffset];

					switch (accessor.type)
					{
					case TINYGLTF_TYPE_VEC3:
					{
						const glm::vec3 *buf = static_cast<const glm::vec3 *>(dataPtr);
						for (size_t index = 0; index < accessor.count; index++)
						{
							sampler.outputsVec4.push_back(glm::vec4(buf[index], 0.0f));
						}
						break;
					}
					case TINYGLTF_TYPE_VEC4:
					{
						const glm::vec4 *buf = static_cast<const glm::vec4 *>(dataPtr);
						for (size_t index = 0; index < accessor.count; index++)
						{
							sampler.outputsVec4.push_back(buf[index]);
						}
						break;
					}
					default:
					{
						std::cout << "unknown type" << std::endl;
						break;
					}
					}
				}

				animation.samplers.push_back(sampler);
			}

			// Channels
			for (auto &source : anim.channels)
			{
				AnimationChannel channel{};

				if (source.target_path == "rotation")
				{
					channel.path = AnimationChannel::PathType::ROTATION;
				}
				if (source.target_path == "translation")
				{
					channel.path = AnimationChannel::PathType::TRANSLATION;
				}
				if (source.target_path == "scale")
				{
					channel.path = AnimationChannel::PathType::SCALE;
				}
				if (source.target_path == "weights")
				{
					std::cout << "weights not yet supported, skipping channel" << std::endl;
					continue;
				}
				channel.samplerIndex = source.sampler;
				channel.node = nodeFromIndex(source.target_node);
				if (!channel.node)
				{
					continue;
				}

				animation.channels.push_back(channel);
			}

			animations.push_back(animation);
		}
	}

	void Model::loadFromFile(std::string_view filename, oceancore::VulkanDevice *device, VkQueue transferQueue, float scale)
	{
		ocean::info("=== GLTF MODEL LOADING START ===");
		ocean::info("GLTF file: " + std::string(filename));
		ocean::info("Scale factor: " + std::to_string(scale));

		auto totalStart = std::chrono::high_resolution_clock::now();

		tinygltf::Model gltfModel;
		tinygltf::TinyGLTF gltfContext;

		std::string error;
		std::string warning;

		this->device = device;

		bool binary = false;
		size_t extpos = filename.rfind('.', filename.length());
		if (extpos != std::string::npos)
		{
			binary = (filename.substr(extpos + 1, filename.length() - extpos) == "glb");
		}

		ocean::info("GLTF format detected: " + std::string(binary ? "Binary (.glb)" : "ASCII (.gltf)"));
		ocean::info("Starting GLTF file parsing...");

		auto tStart = std::chrono::high_resolution_clock::now();
		bool fileLoaded = binary ? gltfContext.LoadBinaryFromFile(&gltfModel, &error, &warning,
			std::string(filename).c_str()) :
			gltfContext.LoadASCIIFromFile(&gltfModel, &error, &warning, std::string(filename).c_str());
		auto tEnd = std::chrono::high_resolution_clock::now();
		auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();

		ocean::info("GLTF file parsing completed in " + std::to_string(tDiff) + " ms");
		std::cout << "VkGltfModel.Model.loadFromFile took " << tDiff << std::endl;
		LoaderInfo loaderInfo{};
		size_t vertexCount = 0;
		size_t indexCount = 0;

		if (fileLoaded)
		{
			ocean::info("GLTF file loaded successfully. Processing resources...");
			ocean::info("GLTF contains: " + std::to_string(gltfModel.textures.size()) + " textures, " +
			           std::to_string(gltfModel.materials.size()) + " materials, " +
			           std::to_string(gltfModel.nodes.size()) + " nodes, " +
			           std::to_string(gltfModel.animations.size()) + " animations");

			auto stepStart = std::chrono::high_resolution_clock::now();
			loadTextureSamplers(gltfModel);
			auto stepEnd = std::chrono::high_resolution_clock::now();
			auto stepTime = std::chrono::duration<double, std::milli>(stepEnd - stepStart).count();
			ocean::info("Texture samplers loaded in " + std::to_string(stepTime) + " ms");

			stepStart = std::chrono::high_resolution_clock::now();
			loadTextures(gltfModel, device, transferQueue);
			stepEnd = std::chrono::high_resolution_clock::now();
			stepTime = std::chrono::duration<double, std::milli>(stepEnd - stepStart).count();
			ocean::info("Textures loaded in " + std::to_string(stepTime) + " ms");

			stepStart = std::chrono::high_resolution_clock::now();
			loadMaterials(gltfModel);
			stepEnd = std::chrono::high_resolution_clock::now();
			stepTime = std::chrono::duration<double, std::milli>(stepEnd - stepStart).count();
			ocean::info("Materials loaded in " + std::to_string(stepTime) + " ms");

			const tinygltf::Scene &scene = gltfModel.scenes[gltfModel.defaultScene > -1 ? gltfModel.defaultScene : 0];
			ocean::info("Processing scene with " + std::to_string(scene.nodes.size()) + " root nodes");

			// Get vertex and index buffer sizes up-front
			stepStart = std::chrono::high_resolution_clock::now();
			for (size_t i = 0; i < scene.nodes.size(); i++)
			{
				getNodeProps(gltfModel.nodes[scene.nodes[i]], gltfModel, vertexCount, indexCount);
			}
			stepEnd = std::chrono::high_resolution_clock::now();
			stepTime = std::chrono::duration<double, std::milli>(stepEnd - stepStart).count();
			ocean::info("Node properties calculated in " + std::to_string(stepTime) + " ms");
			ocean::info("Total vertices: " + std::to_string(vertexCount) + ", Total indices: " + std::to_string(indexCount));

			stepStart = std::chrono::high_resolution_clock::now();
			loaderInfo.vertexBuffer = new Vertex[vertexCount];
			loaderInfo.indexBuffer = new uint32_t[indexCount];
			stepEnd = std::chrono::high_resolution_clock::now();
			stepTime = std::chrono::duration<double, std::milli>(stepEnd - stepStart).count();
			ocean::info("Vertex and index buffers allocated in " + std::to_string(stepTime) + " ms");

			// TODO: scene handling with no default scene
			stepStart = std::chrono::high_resolution_clock::now();
			for (size_t i = 0; i < scene.nodes.size(); i++)
			{
				const tinygltf::Node node = gltfModel.nodes[scene.nodes[i]];
				loadNode(nullptr, node, scene.nodes[i], gltfModel, loaderInfo, scale);
			}
			stepEnd = std::chrono::high_resolution_clock::now();
			stepTime = std::chrono::duration<double, std::milli>(stepEnd - stepStart).count();
			ocean::info("Scene nodes loaded in " + std::to_string(stepTime) + " ms");

			if (gltfModel.animations.size() > 0)
			{
				stepStart = std::chrono::high_resolution_clock::now();
				loadAnimations(gltfModel);
				stepEnd = std::chrono::high_resolution_clock::now();
				stepTime = std::chrono::duration<double, std::milli>(stepEnd - stepStart).count();
				ocean::info("Animations loaded in " + std::to_string(stepTime) + " ms");
			}

			stepStart = std::chrono::high_resolution_clock::now();
			loadSkins(gltfModel);
			stepEnd = std::chrono::high_resolution_clock::now();
			stepTime = std::chrono::duration<double, std::milli>(stepEnd - stepStart).count();
			ocean::info("Skins loaded in " + std::to_string(stepTime) + " ms");

			for (auto node : linearNodes)
			{
				// Assign skins
				if (node->skinIndex > -1)
				{
					node->skin = skins[node->skinIndex];
				}
				// Initial pose
				if (node->mesh)
				{
					node->update();
				}
			}
		}
		else
		{
			// TODO: throw
			std::cerr << "Could not load gltf file: " << error << std::endl;
			return;
		}

		extensions = gltfModel.extensionsUsed;

		size_t vertexBufferSize = vertexCount * sizeof(Vertex);
		size_t indexBufferSize = indexCount * sizeof(uint32_t);

		assert(vertexBufferSize > 0);

		struct StagingBuffer
		{
			VkBuffer buffer;
			VkDeviceMemory memory;
		} vertexStaging, indexStaging;

		// Create staging buffers
		// Vertex data
		VK_CHECK_RESULT(device->createBuffer(
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			vertexBufferSize,
			&vertexStaging.buffer,
			&vertexStaging.memory,
			"vertexData", loaderInfo.vertexBuffer));
		// Index data
		if (indexBufferSize > 0)
		{
			VK_CHECK_RESULT(device->createBuffer(
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				indexBufferSize,
				&indexStaging.buffer,
				&indexStaging.memory,
				"indexData", loaderInfo.indexBuffer));
		}

		// Create device local buffers
		// Vertex buffer
		VK_CHECK_RESULT(device->createBuffer(
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			vertexBufferSize,
			&vertices.buffer,
			&vertices.memory, "vertexBuffer"));
		// Index buffer
		if (indexBufferSize > 0)
		{
			VK_CHECK_RESULT(device->createBuffer(
				VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				indexBufferSize,
				&indices.buffer,
				&indices.memory, "indexBuffer"));
		}

		// Copy from staging buffers
		VkCommandBuffer copyCmd = device->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

		VkBufferCopy copyRegion = {};

		copyRegion.size = vertexBufferSize;
		vkCmdCopyBuffer(copyCmd, vertexStaging.buffer, vertices.buffer, 1, &copyRegion);

		if (indexBufferSize > 0)
		{
			copyRegion.size = indexBufferSize;
			vkCmdCopyBuffer(copyCmd, indexStaging.buffer, indices.buffer, 1, &copyRegion);
		}

		device->flushCommandBuffer(copyCmd, transferQueue, true);

		vkDestroyBuffer(device->logicalDevice, vertexStaging.buffer, nullptr);
		vkFreeMemory(device->logicalDevice, vertexStaging.memory, nullptr);
		if (indexBufferSize > 0)
		{
			vkDestroyBuffer(device->logicalDevice, indexStaging.buffer, nullptr);
			vkFreeMemory(device->logicalDevice, indexStaging.memory, nullptr);
		}

		delete[] loaderInfo.vertexBuffer;
		delete[] loaderInfo.indexBuffer;

		getSceneDimensions();

		// Log total timing summary
		auto totalEnd = std::chrono::high_resolution_clock::now();
		auto totalDuration = std::chrono::duration_cast<std::chrono::microseconds>(totalEnd - totalStart);
		ocean::info("=== GLTF MODEL LOADING COMPLETE ===");
		ocean::info("Total loading time: " + std::to_string(totalDuration.count() / 1000.0) + " ms");
		ocean::info("Total vertices: " + std::to_string(vertexCount));
		ocean::info("Total indices: " + std::to_string(indexCount));
		ocean::info("Total textures: " + std::to_string(textures.size()));
		ocean::info("Total materials: " + std::to_string(materials.size()));
		ocean::info("Total nodes: " + std::to_string(nodes.size()));
		ocean::info("=======================================");
	}
	void Model::loadFromModel(uint32_t gltfId, const tinygltf::Model *gltfModel,
							  oceancore::VulkanDevice *device, VkQueue transferQueue, float scale)
	{
		if (gltfModel == nullptr || gltfModel->scenes.size() == 0)
			return;

		assert(gltfModel->nodes.size() > 0);

		this->id = gltfId;

		this->device = device;
		LoaderInfo loaderInfo{};
		// delete[] loaderInfo.vertexBuffer;
		// delete[] loaderInfo.indexBuffer;
		// loaderInfo.indexBuffer = 0;
		// loaderInfo.indexPos = 0;

		size_t vertexCount = 0;
		size_t indexCount = 0;

		loadTextureSamplers(*gltfModel);
		loadTextures(*gltfModel, device, transferQueue);
		loadMaterials(*gltfModel);

		const tinygltf::Scene &scene = gltfModel->scenes[gltfModel->defaultScene > -1 ? gltfModel->defaultScene : 0];

		// Get vertex and index buffer sizes up-front
		for (size_t i = 0; i < scene.nodes.size(); i++)
		{
			getNodeProps(gltfModel->nodes[scene.nodes[i]], *gltfModel, vertexCount, indexCount);
		}
		ocean::info("GLTF Model - id=" + std::to_string(gltfId) + ", vertexCount=" + std::to_string(vertexCount) + ", indexCount=" + std::to_string(indexCount));
		assert(vertexCount < 1000000);
		loaderInfo.vertexBuffer = new Vertex[vertexCount];
		loaderInfo.indexBuffer = new uint32_t[indexCount];

		// TODO: scene handling with no default scene
		for (size_t i = 0; i < scene.nodes.size(); i++)
		{
			const tinygltf::Node node = gltfModel->nodes[scene.nodes[i]];
			loadNode(nullptr, node, scene.nodes[i], *gltfModel, loaderInfo, scale);
		}
		if (gltfModel->animations.size() > 0)
		{
			if (gltfModel->animations.size() == 1)
			{
				loadAnimations(*gltfModel, 0);
			}
			else
			{
				loadAnimations(*gltfModel, 4);
			}
		}
		loadSkins(*gltfModel);

		for (auto node : linearNodes)
		{
			// Assign skins
			if (node->skinIndex > -1)
			{
				node->skin = skins[node->skinIndex];
			}
			// Initial pose
			if (node->mesh)
			{
				node->update();
			}
		}

		extensions = gltfModel->extensionsUsed;

		size_t vertexBufferSize = vertexCount * sizeof(Vertex);
		size_t indexBufferSize = indexCount * sizeof(uint32_t);
		ocean::info("GLTF Buffers - id=" + std::to_string(gltfId) + ", vertexBuffer=" + std::to_string(vertexBufferSize / 1024.0) + "KB, indexBuffer=" + std::to_string(indexBufferSize / 1024.0) + "KB");

		assert(vertexBufferSize > 0);

		struct StagingBuffer
		{
			VkBuffer buffer;
			VkDeviceMemory memory;
		} vertexStaging, indexStaging;

		// Create staging buffers
		// Vertex data
		VK_CHECK_RESULT(device->createBuffer(
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			vertexBufferSize,
			&vertexStaging.buffer,
			&vertexStaging.memory,
			"vertexData",
			loaderInfo.vertexBuffer));
		// Index data
		if (indexBufferSize > 0)
		{
			VK_CHECK_RESULT(device->createBuffer(
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				indexBufferSize,
				&indexStaging.buffer,
				&indexStaging.memory,
				"indexData",
				loaderInfo.indexBuffer));
		}

		// Create device local buffers
		// Vertex buffer
		VK_CHECK_RESULT(device->createBuffer(
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			vertexBufferSize,
			&vertices.buffer,
			&vertices.memory, "vertexBuffer"));
		// Index buffer
		if (indexBufferSize > 0)
		{
			VK_CHECK_RESULT(device->createBuffer(
				VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				indexBufferSize,
				&indices.buffer,
				&indices.memory, "indexBuffer"));
		}

		// Copy from staging buffers
		VkCommandBuffer copyCmd = device->createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

		VkBufferCopy copyRegion = {};

		copyRegion.size = vertexBufferSize;
		vkCmdCopyBuffer(copyCmd, vertexStaging.buffer, vertices.buffer, 1, &copyRegion);

		if (indexBufferSize > 0)
		{
			copyRegion.size = indexBufferSize;
			vkCmdCopyBuffer(copyCmd, indexStaging.buffer, indices.buffer, 1, &copyRegion);
		}

		device->flushCommandBuffer(copyCmd, transferQueue, true);

		vkDestroyBuffer(device->logicalDevice, vertexStaging.buffer, nullptr);
		vkFreeMemory(device->logicalDevice, vertexStaging.memory, nullptr);
		if (indexBufferSize > 0)
		{
			vkDestroyBuffer(device->logicalDevice, indexStaging.buffer, nullptr);
			vkFreeMemory(device->logicalDevice, indexStaging.memory, nullptr);
		}

		delete[] loaderInfo.vertexBuffer;
		delete[] loaderInfo.indexBuffer;
		// loaderInfo.indexPos = 0;
		// loaderInfo.vertexPos = 0;
		getSceneDimensions();
	}

	void Model::drawNode(Node *node, VkCommandBuffer commandBuffer)
	{
		if (node->mesh)
		{
			for (Primitive *primitive : node->mesh->primitives)
			{
				vkCmdDrawIndexed(commandBuffer, primitive->indexCount, 1, primitive->firstIndex, 0, 0);
			}
		}
		for (auto &child : node->children)
		{
			drawNode(child, commandBuffer);
		}
	}

	void Model::draw(VkCommandBuffer commandBuffer)
	{
		const VkDeviceSize offsets[1] = {0};
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertices.buffer, offsets);
		vkCmdBindIndexBuffer(commandBuffer, indices.buffer, 0, VK_INDEX_TYPE_UINT32);
		for (auto &node : nodes)
		{
			drawNode(node, commandBuffer);
		}
	}

	void Model::calculateBoundingBox(Node *node, Node *parent)
	{
		BoundingBox parentBvh = parent ? parent->bvh : BoundingBox(dimensions.min, dimensions.max);

		if (node->mesh)
		{
			if (node->mesh->bb.valid)
			{
				node->aabb = node->mesh->bb.getAABB(node->getMatrix());
				if (node->children.size() == 0)
				{
					node->bvh.min = node->aabb.min;
					node->bvh.max = node->aabb.max;
					node->bvh.valid = true;
				}
			}
		}

		parentBvh.min = glm::min(parentBvh.min, node->bvh.min);
		parentBvh.max = glm::min(parentBvh.max, node->bvh.max);

		for (auto &child : node->children)
		{
			calculateBoundingBox(child, node);
		}
	}

	void Model::getSceneDimensions()
	{
		// Calculate binary volume hierarchy for all nodes in the scene
		for (auto node : linearNodes)
		{
			calculateBoundingBox(node, nullptr);
		}

		dimensions.min = glm::vec3(FLT_MAX);
		dimensions.max = glm::vec3(-FLT_MAX);

		for (auto node : linearNodes)
		{
			if (node->bvh.valid)
			{
				dimensions.min = glm::min(dimensions.min, node->bvh.min);
				dimensions.max = glm::max(dimensions.max, node->bvh.max);
			}
		}

		// Calculate scene aabb
		aabb = glm::scale(glm::mat4(1.0f), glm::vec3(dimensions.max[0] - dimensions.min[0], dimensions.max[1] - dimensions.min[1], dimensions.max[2] - dimensions.min[2]));
		aabb[3][0] = dimensions.min[0];
		aabb[3][1] = dimensions.min[1];
		aabb[3][2] = dimensions.min[2];
	}
	void Model::updateNodes(const tinygltf::Model *model)
	{
		if (nodes.size() == 0)
			return;

		if (nodes.size() == model->nodes.size())
		{
			// Primitive Nodes without animation
			for (size_t i = 0; i < nodes.size(); i++)
			{
				if (nodes[i]->translation.length() == 3)
				{
					nodes[i]->translation.x = model->nodes[i].translation[0];
					nodes[i]->translation.y = model->nodes[i].translation[1];
					nodes[i]->translation.z = model->nodes[i].translation[2];
				}
			}

			for (auto &node : nodes)
			{
				node->update();
			}
		}
		else
		{
			// animations

			uint32_t rootNodeIndex = MAXUINT32;

			for (size_t i = 0; i < model->nodes.size(); i++)
			{
				if (model->nodes[i].name.compare("Root") == 0)
				{
					rootNodeIndex = i;
					break;
				}
			}

			if (rootNodeIndex == MAXUINT32)
				return;

			if (model->nodes[rootNodeIndex].translation.size() == 3)
			{
				nodes[0]->translation.x = model->nodes[rootNodeIndex].translation[0];
				nodes[0]->translation.y = model->nodes[rootNodeIndex].translation[1];
				nodes[0]->translation.z = model->nodes[rootNodeIndex].translation[2];
			}
		}
	}
	void Model::updateAnimations(void)
	{
		for (auto &node : nodes)
		{
			if (node->animationIndex != -1)
				updateAnimation(node->animationIndex, node->animationTimer);
		}
	}
	void Model::updateAnimation(uint32_t index, float time)
	{
		if (animations.empty())
		{
			std::cout << ".gltf does not contain animation." << std::endl;
			return;
		}
		if (index > static_cast<uint32_t>(animations.size()) - 1)
		{
			std::cout << "No animation with index " << index << std::endl;
			return;
		}
		Animation &animation = animations[index];

		bool updated = false;
		for (auto &channel : animation.channels)
		{
			AnimationSampler &sampler = animation.samplers[channel.samplerIndex];
			if (sampler.inputs.size() > sampler.outputsVec4.size())
			{
				continue;
			}

			for (size_t i = 0; i < sampler.inputs.size() - 1; i++)
			{
				if ((time >= sampler.inputs[i]) && (time <= sampler.inputs[i + 1]))
				{
					float u = std::max(0.0f, time - sampler.inputs[i]) / (sampler.inputs[i + 1] - sampler.inputs[i]);
					if (u <= 1.0f)
					{
						switch (channel.path)
						{
						case AnimationChannel::PathType::TRANSLATION:
						{
							glm::vec4 trans = glm::mix(sampler.outputsVec4[i], sampler.outputsVec4[i + 1], u);
							channel.node->translation = glm::vec3(trans);
							break;
						}
						case AnimationChannel::PathType::SCALE:
						{
							glm::vec4 trans = glm::mix(sampler.outputsVec4[i], sampler.outputsVec4[i + 1], u);
							channel.node->scale = glm::vec3(trans);
							break;
						}
						case AnimationChannel::PathType::ROTATION:
						{
							glm::quat q1;
							q1.x = sampler.outputsVec4[i].x;
							q1.y = sampler.outputsVec4[i].y;
							q1.z = sampler.outputsVec4[i].z;
							q1.w = sampler.outputsVec4[i].w;
							glm::quat q2;
							q2.x = sampler.outputsVec4[i + 1].x;
							q2.y = sampler.outputsVec4[i + 1].y;
							q2.z = sampler.outputsVec4[i + 1].z;
							q2.w = sampler.outputsVec4[i + 1].w;
							channel.node->rotation = glm::normalize(glm::slerp(q1, q2, u));
							break;
						}
						}
						updated = true;
					}
				}
			}
		}
		if (updated)
		{
			for (auto &node : nodes)
			{
				node->update();
			}
		}
	}

	Node *Model::findNode(Node *parent, uint32_t index)
	{
		Node *nodeFound = nullptr;
		if (parent->index == index)
		{
			return parent;
		}
		for (auto &child : parent->children)
		{
			nodeFound = findNode(child, index);
			if (nodeFound)
			{
				break;
			}
		}
		return nodeFound;
	}

	Node *Model::nodeFromIndex(uint32_t index)
	{
		Node *nodeFound = nullptr;
		for (auto &node : nodes)
		{
			nodeFound = findNode(node, index);
			if (nodeFound)
			{
				break;
			}
		}
		return nodeFound;
	}

}