/**
 * Copyright (C) by Patrice Chevillat
 */

#pragma once
#include <gltf17.pb.h>
#include <tinygltf/tiny_gltf.h>
#include <grpcpp/grpcpp.h>
#include <google/protobuf/message.h>
#include <iostream>
#include <string>
#include <google/protobuf/util/json_util.h>

#ifndef GLTF_MODIFIER_HPP_
#define GLTF_MODIFIER_HPP_

inline void addNodeToGltfModel(cgfx::GltfModel& model, const cgfx::GNode &node) {
    // Add the new node to the repeated 'nodes' field in the model
    cgfx::GNode* newNode = model.add_nodes();

    // Copy the data from the provided node to the new node in the model
    *newNode = node;
}

inline std::string postProcessJsonToAddSamplers(const std::string& json) {
    // Parse the JSON string into a JSON object
    nlohmann::json jsonObject;
    try {
        jsonObject = nlohmann::json::parse(json);
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to parse JSON: " << e.what() << std::endl;
        return "";
    }

    // Ensure all textures have the 'sampler' field
    if (jsonObject.contains("textures")) {
        for (auto& texture : jsonObject["textures"]) {
            if (!texture.contains("sampler")) {
                texture["sampler"] = 0; // Add sampler field with default value 0
            }
        }
    }

    // Ensure all channels in animations have the 'sampler' field
    if (jsonObject.contains("animations")) {
        for (auto& animation : jsonObject["animations"]) {
            if (animation.contains("channels")) {
                for (auto& channel : animation["channels"]) {
                    if (!channel.contains("sampler")) {
                        channel["sampler"] = 0; // Add sampler field with default value 0
                    }
                }
            }
        }
    }

    // Convert the modified JSON object back to a string
    std::string processedJson;
    try {
        processedJson = jsonObject.dump(4); // Pretty-print with 4 spaces indentation
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to convert JSON object to string: " << e.what() << std::endl;
        return "";
    }

    return processedJson;
}

// Function to convert GltfModel to JSON string
inline std::string convertGltfModelToJson(const cgfx::GltfModel& model) {
    std::string json_string;
    google::protobuf::util::JsonOptions options;

    // Convert the Protobuf message to JSON
    google::protobuf::util::MessageToJsonString(model, &json_string, options);

    std::string processed_json = postProcessJsonToAddSamplers(json_string);

    return processed_json;
}

inline void addParentNodeToTinyGltfModel(tinygltf::Model& model, 
    const cgfx::GltfModel& protoModel, const cgfx::GNode& parentNode) {
    // Create a new tinygltf::Node for the parent
    tinygltf::Node newTinyGltfNode;

    // Copy the parent node name (only if it has been set in the GNode)
    if (!parentNode.name().empty()) {
        newTinyGltfNode.name = parentNode.name();
    }

    // Set the mesh index if it's non-zero (default value in proto3 is 0)
    if (parentNode.mesh() != 0) {
        newTinyGltfNode.mesh = parentNode.mesh();
    }

    // Set the skin index if it's non-zero
    if (parentNode.skin() != 0) {
        newTinyGltfNode.skin = parentNode.skin();
    }

    // Set the transform (scale, translation, rotation, or matrix) if present
    if (parentNode.scale_size() == 3) {
        newTinyGltfNode.scale = { parentNode.scale(0), parentNode.scale(1), parentNode.scale(2) };
    }
    if (parentNode.translation_size() == 3) {
        newTinyGltfNode.translation = { parentNode.translation(0), parentNode.translation(1), parentNode.translation(2) };
    }
    if (parentNode.rotation_size() == 4) {
        newTinyGltfNode.rotation = { parentNode.rotation(0), parentNode.rotation(1), parentNode.rotation(2), parentNode.rotation(3) };
    }
    if (parentNode.matrix_size() == 16) {
        newTinyGltfNode.matrix.assign(parentNode.matrix().begin(), parentNode.matrix().end());
    }

    // Now add the parent node to the tinygltf::Model node list and get its index
    int parentNodeIndex = static_cast<int>(model.nodes.size());
    model.nodes.push_back(newTinyGltfNode);

    // Check if this node has children
    if (parentNode.children_size() > 0) {
        // For each child, add its corresponding GNode from the protoModel to tinygltf::Model
        for (int i = 0; i < parentNode.children_size(); ++i) {
            uint32_t childIndex = parentNode.children(i);

            // Make sure the child index exists in the protoModel
            if (childIndex < protoModel.nodes_size()) {
                const cgfx::GNode& childNode = protoModel.nodes(childIndex);

                // Recursively add child nodes
                addParentNodeToTinyGltfModel(model, protoModel, childNode);

                // Add the child node index to the parent node's children in tinygltf
                model.nodes[parentNodeIndex].children.push_back(static_cast<int>(model.nodes.size() - 1));
            }
        }
    }

    // Optionally, add this node to the first scene
    if (model.scenes.size() > 0) {
        model.scenes[0].nodes.push_back(parentNodeIndex);  // Add the parent node index to the scene
    }
}

inline void addNodeToTinyGltfModel(tinygltf::Model& model, const cgfx::GNode& node) {
    // Create a new tinygltf::Node
    tinygltf::Node newTinyGltfNode;

    // Copy the name
    if (!node.name().empty()) {
        newTinyGltfNode.name = node.name();
    }
 
    // Set the mesh index if it exists
    if (node.mesh() != 0) {
        newTinyGltfNode.mesh = node.mesh();
    }

    // Set the skin index if it exists
    if (node.skin() != 0) {
        newTinyGltfNode.skin = node.skin();
    }

    // Set the scale (if provided, should have exactly 3 elements)
    if (node.scale_size() == 3) {
        newTinyGltfNode.scale = { node.scale(0), node.scale(1), node.scale(2) };
    }

    // Set the translation (if provided, should have exactly 3 elements)
    if (node.translation_size() == 3) {
        newTinyGltfNode.translation = { node.translation(0), node.translation(1), node.translation(2) };
    }

    // Set the rotation (if provided, should have exactly 4 elements)
    if (node.rotation_size() == 4) {
        newTinyGltfNode.rotation = { node.rotation(0), node.rotation(1), node.rotation(2), node.rotation(3) };
    }

    // Set the matrix (if provided, should have exactly 16 elements)
    if (node.matrix_size() == 16) {
        newTinyGltfNode.matrix.assign(node.matrix().begin(), node.matrix().end());
    }

    // Set the children nodes if they exist
    for (int i = 0; i < node.children_size(); ++i) {
        newTinyGltfNode.children.push_back(node.children(i));
    }

    // Add the new node to the tinygltf::Model's node list and get its index
    int newNodeIndex = static_cast<int>(model.nodes.size());
    model.nodes.push_back(newTinyGltfNode);

    // Add this node index to the scene (assuming you're adding it to the default scene)
    if (model.scenes.size() > 0) {
        model.scenes[0].nodes.push_back(newNodeIndex);  // Assuming you want to add it to the first scene
    }
}

inline void convertRiggedAnimationToSkin(const cgfx::GltfModel& riggedModel, cgfx::GltfModel& skinModel) {
    // Step 1: Transfer all relevant nodes
    std::unordered_map<uint32_t, uint32_t> nodeMapping; // Maps rigged node IDs to skinModel node IDs
    for (int i = 0; i < riggedModel.nodes_size(); ++i) {
        const auto& riggedNode = riggedModel.nodes(i);
        auto* newNode = skinModel.add_nodes();
        newNode->CopyFrom(riggedNode); // Copy the node directly
        nodeMapping[i] = skinModel.nodes_size() - 1; // Map old node ID to new node ID
    }

    // Step 2: Create skins from rigged model nodes (assuming skinning information is provided)
    for (int i = 0; i < riggedModel.skins_size(); ++i) {
        const auto& riggedSkin = riggedModel.skins(i);
        auto* newSkin = skinModel.add_skins();
        newSkin->CopyFrom(riggedSkin); // Copy the skin information

        // Fix joint node mappings in the new skin (joints might need remapping)
        for (int j = 0; j < newSkin->joints_size(); ++j) {
            uint32_t jointNodeId = newSkin->joints(j);
            newSkin->set_joints(j, nodeMapping[jointNodeId]); // Remap to new node IDs in skinModel
        }
    }

    // Step 3: Transfer the inverse bind matrices (if available)
    for (int i = 0; i < riggedModel.skins_size(); ++i) {
        const auto& riggedSkin = riggedModel.skins(i);
        auto* newSkin = skinModel.mutable_skins(i);
        for (int j = 0; j < riggedSkin.inversebindmatrices_size(); ++j) {
            newSkin->add_inversebindmatrices(riggedSkin.inversebindmatrices(j));
        }
    }

    // Step 4: Convert and transfer animations from riggedModel to skinModel
    for (int i = 0; i < riggedModel.animations_size(); ++i) {
        const auto& riggedAnimation = riggedModel.animations(i);
        auto* newAnimation = skinModel.add_animations();
        newAnimation->set_name(riggedAnimation.name());

        // Transfer all channels from the rigged animation
        for (int j = 0; j < riggedAnimation.channels_size(); ++j) {
            const auto& riggedChannel = riggedAnimation.channels(j);
            auto* newChannel = newAnimation->add_channels();
            newChannel->CopyFrom(riggedChannel);

            // Remap target node IDs to the new node IDs in skinModel
            const auto& riggedTarget = riggedChannel.target();
            newChannel->mutable_target()->set_node(nodeMapping[riggedTarget.node()]);
            newChannel->mutable_target()->set_path(riggedTarget.path()); // Assuming the path is correct
        }

        // Transfer the animation samplers
        for (int j = 0; j < riggedAnimation.samplers_size(); ++j) {
            const auto& riggedSampler = riggedAnimation.samplers(j);
            auto* newSampler = newAnimation->add_samplers();
            newSampler->CopyFrom(riggedSampler);
        }
    }

    // Step 5: Transfer the remaining textures, materials, etc., if necessary
    skinModel.mutable_textures()->CopyFrom(riggedModel.textures());
    skinModel.mutable_materials()->CopyFrom(riggedModel.materials());
    skinModel.mutable_images()->CopyFrom(riggedModel.images());
    skinModel.mutable_buffers()->CopyFrom(riggedModel.buffers());
    skinModel.mutable_bufferviews()->CopyFrom(riggedModel.bufferviews());
    skinModel.mutable_accessors()->CopyFrom(riggedModel.accessors());
    skinModel.mutable_asset()->CopyFrom(riggedModel.asset());

    // If the riggedModel has a default scene, transfer it as well
    skinModel.set_scene(riggedModel.scene());
}

// Function to parse a GltfModel from a JSON string
inline bool parseGltfModelFromJson(const std::string& json_string, cgfx::GltfModel& model) {
    // Clear any existing data in the model
    model.Clear();

    // Create a JsonParseOptions object for optional settings
    google::protobuf::util::JsonParseOptions options;
    options.ignore_unknown_fields = true; // Optionally ignore unknown fields if needed

    // Parse the JSON string into the Protobuf message
    google::protobuf::util::Status status = google::protobuf::util::JsonStringToMessage(json_string, &model, options);

    if (status.ok()) {
        // Successfully parsed
        return true;
    }
    else {
        // Parsing failed
        std::cerr << "Failed to parse JSON to GltfModel: " << status.ToString() << std::endl;
        return false;
    }
}

inline bool parseGltfModelFromSerializedString(const std::string& input, cgfx::GltfModel& model) {
    // Clear any existing data in the model
    model.Clear();

    // Create a C++ string stream from the input string
    google::protobuf::io::ArrayInputStream input_stream(input.data(), input.size());

    // Create a C++ CodedInputStream from the input stream
    google::protobuf::io::CodedInputStream coded_input(&input_stream);

    // Parse the serialized data into the model
    if (model.ParseFromCodedStream(&coded_input)) {
        // Successfully parsed
        return true;
    }
    else {
        // Parsing failed
        std::cerr << "Failed to parse GltfModel from string." << std::endl;
        return false;
    }
}

#endif