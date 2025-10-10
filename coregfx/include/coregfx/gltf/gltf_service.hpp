/**
 * Copyright (C) by Patrice Chevillat
 */

#pragma once

#include <string>
#include <chrono>
#include <iostream>
#include "gltf_folder.hpp"
#include "context_engine.pb.h"
#include "context_engine.grpc.pb.h"
// REMOVED: #include "gltfEngine.pb.h" - gltfEngine.proto deleted (100% unused, superseded by gltf17.proto)
// REMOVED: #include "gltfEngine.grpc.pb.h" - gltfEngine.proto deleted
#include "cgfx.pb.h"
#include "cgfx.grpc.pb.h"
#include <grpcpp/grpcpp.h>
#include <grpcpp/impl/codegen/client_unary_call.h>
#include <google/protobuf/util/message_differencer.h>
#include <sstream>
#include <fstream>


#ifndef DEFAULT_GLTF_ERROR_PLACEHOLDER
#define DEFAULT_GLTF_ERROR_PLACEHOLDER
constexpr uint32_t DEFAULT_GLTF_ERROR_PLACEHOLDER_ID = 2798460519;
constexpr std::string_view DEFAULT_GLTF_ERROR_PLACEHOLDER_FILENAME = 
    "C:/workspaces/media-store/3d-graphics/work/_playground2/placeholder/gltf/error_placeholder1x1x1.gltf";
#endif

using google::protobuf::RepeatedPtrField;

namespace GltfEngine
{
    class GltfService
    {
    private:
        TinyGltfFolder *tinyGltfFolder = nullptr;

        tinygltf::Model *model = nullptr;

        struct GltfOffsets
        {
            uint32_t material = 0;
            uint32_t mesh = 0;
            uint32_t skin = 0;
            uint32_t animation = 0;
        };

        std::vector<uint32_t> newGltfModelIdsToBuild = std::vector<uint32_t>(); // ge�ndertes gltf

    public:
        com::context::grpc::GltfContext gltfContext;

        com::context::grpc::DatabaseContext dbContext;

        enum GltfStatus
        {
            GLTF_READY = 1,        // ready to use
            GLTF_NODE_MODIFIED = 2 // some nodes has been added or removed
        };

        std::vector<std::tuple<uint32_t, GltfStatus, tinygltf::Model>> gltfModelContext; // gltf und nodes

        GltfService() {}
        GltfService(TinyGltfFolder *_tinyGltfFolder) : tinyGltfFolder(_tinyGltfFolder)
        {
            model = new tinygltf::Model();
        }
        
        void printGltfContext()
        {
            gltfContext.PrintDebugString();
        }

        void addNode(com::context::grpc::NodeEntity *node)
        {
            std::ostringstream query;

            query << "INSERT INTO nodes (id,gltf_id,animation,px,py,pz,rx,ry,rz,rw) VALUES ("
                << node->id() << ","
                << node->gltfid() << ","
                << node->animation() << ","
                << node->px() << ","
                << node->py() << ","
                << node->pz() << ","
                << node->rx() << ","
                << node->ry() << ","
                << node->rz() << ","
                << node->rw() << ");";

            std::cout << query.str() << std::endl;

            if (existNode(node->id()))
            {
                auto m = "nodeId: " + std::to_string(node->id()) + " already inserted";
                throw std::runtime_error(&m[0]);
            }

            if (!existGltf(node->gltfid()))
            {
                auto m = "gltfId: " + std::to_string(node->gltfid()) + " does not exist";
                std::cerr << m << std::endl;
                node->set_gltfid(DEFAULT_GLTF_ERROR_PLACEHOLDER_ID);
            }

            dbContext.add_nodes()->CopyFrom(*node);

            // hinzuf�gen, wenn nicht enthalten
            if (!std::count(newGltfModelIdsToBuild.begin(), newGltfModelIdsToBuild.end(), node->gltfid()))
                newGltfModelIdsToBuild.push_back(node->gltfid());
        }
        void addGltf(com::context::grpc::GltfEntity *gltf)
        {
            std::cout << "INSERT INTO gltfs (id,uri) VALUES (" +
                           std::to_string(gltf->id()) + "," + gltf->uri() +  ")" << std::endl;

            if (existGltf(gltf->id()))
            {
                auto m = "gltfId: " + std::to_string(gltf->id()) + " already inserted";
                throw std::runtime_error(&m[0]);
            }

            dbContext.add_gltfs()->CopyFrom(*gltf);
        }
        void clear()
        {
            gltfContext.Clear();

            gltfModelContext.clear();

            dbContext.Clear();

            buildModel();
        }
        bool existGltf(uint32_t id)
        {
            for (size_t i = 0; i < static_cast<size_t>(dbContext.gltfs_size()); i++)
            {
                if (dbContext.gltfs().at(i).id() == id)
                    return true;
            }

            return false;
        }
        bool existNode(uint32_t id)
        {
            for (size_t i = 0; i < static_cast<size_t>(dbContext.nodes_size()); i++)
            {
                if (dbContext.nodes().at(i).id() == id)
                    return true;
            }

            return false;
        }
        void insertNode(const com::context::grpc::NodeEntity *node)
        {
            if (checkForDuplicateNodes(node->id()))
            {
                std::cerr << "GltfService.postNode nodeContext: { Id:" +
                    std::to_string(node->id()) + " } Already inserted" << std::endl;
                return;
            }
            auto a = gltfContext.add_nodes();

            com::context::grpc::NodeContext nc;
            com::context::grpc::Node n;

            nc.set_id(node->id());
            n.set_gltfid(node->gltfid());
            n.set_px(node->px());
            n.set_py(node->py());
            n.set_pz(node->pz());
            nc.set_allocated_node(&n);

            a->CopyFrom(nc);

            // hinzuf�gen, wenn nicht enthalten
            if (!std::count(newGltfModelIdsToBuild.begin(), newGltfModelIdsToBuild.end(), 
                node->gltfid()))
                newGltfModelIdsToBuild.push_back(node->gltfid());
        }
        void insertNode(const com::context::grpc::NodeContext *node)
        {
            if (checkForDuplicateNodes(node->id()))
            {
                std::cerr << "GltfService.postNode nodeContext: { Id:" +
                    std::to_string(node->id()) + " } Already inserted" << std::endl;
                return;
            }
            auto a = gltfContext.add_nodes();
            a->CopyFrom(*node);

            // hinzuf�gen, wenn nicht enthalten
            if (!std::count(newGltfModelIdsToBuild.begin(), newGltfModelIdsToBuild.end(), node->node().gltfid()))
                newGltfModelIdsToBuild.push_back(node->node().gltfid());
        }
        void insertLinearTranslation(const com::context::grpc::LinearTranslation *node)
        {
            if (checkForDuplicateLinearTranslations(node->nodeid()))
            {
                std::cerr << "GltfService.postLinearTranslation linearTranslation: { nodeId: " +
                    std::to_string(node->nodeid()) + " } duplicated translation for same node" << std::endl;
                return;
            }
            if (findNodeById(node->nodeid()) == SIZE_MAX)
            {
                std::cerr << "GltfService.postLinearTranslation linearTranslation: { nodeId: " +
                    std::to_string(node->nodeid()) + " } node not found" << std::endl;
                return;
            }
            auto a = gltfContext.add_lineartranslations();
            a->CopyFrom(*node);
        }
        /*
         * Bei ScriptNodes ist auch keine Lineare Bewegung erlaubt!
         */
        void postScriptContext(const com::context::grpc::ScriptContext *node)
        {
            if (checkForDuplicateScripts(node->script().nodeid()))
            {
                std::cerr << "GltfService.postScriptContext scriptContext(nodeId=" + 
                    std::to_string(node->script().nodeid()) + ") duplicated script" << std::endl;
                return;
            }
            if (checkForDuplicateLinearTranslations(node->script().nodeid()))
            {
                std::cerr << "GltfService.postScriptContext scriptContext(nodeId=" +
                                std::to_string(node->id()) + ") node already as linear translation" << std::endl;
                return;
            }
            if (findNodeById(node->script().nodeid()) == SIZE_MAX)
            {
                std::cerr << "GltfService.postScriptContext scriptContext(nodeId=" +
                    std::to_string(node->id()) + ") node not found" << std::endl;
                return;
            }
            auto a = gltfContext.add_scripts();
            a->CopyFrom(*node);
        }

    private:
        std::string getNodeString(const com::context::grpc::NodeContext *node)
        {
            return " node: { nodeId: " + std::to_string(node->id()) + ", p: [" + std::to_string(node->node().px()) +
                   "," + std::to_string(node->node().py()) + "," +
                   std::to_string(node->node().pz()) + "] }";
        }

    public:
        void updateNode(const cgfx::Struct& node)
        {
            throw std::runtime_error("updateNode not implemented");
        }
        void updateNode(const cgfx::Node& node)
        {
            std::ostringstream oss;
            oss << "GltfService.updateNode node: { id: "
                << node.id() << ", p: ["
                << node.p()[0] << "," << node.p()[1]
                << "," << node.p()[2]
                << "] }";

            std::cout << oss.str();

            auto a = findDbNodeById(node.id());
            if (a == MAXUINT32)
            {
                std::cerr << "GltfService.updateNode node: { id: " + std::to_string(node.id()) + " } NotFound" << std::endl;
                return;
            }

            auto* b = dbContext.mutable_nodes(a);

            // Retrieve p vector and its size once
            const auto& p = node.p();
            size_t pSize = node.p_size();

            if (pSize == 3) {
                b->set_px(p[0]);
                b->set_py(p[1]);
                b->set_pz(p[2]);
            }

            // Retrieve r vector and its size once
            const auto& r = node.r();
            size_t rSize = node.r_size();

            if (rSize == 4) {
                b->set_rx(r[0]);
                b->set_ry(r[1]);
                b->set_rz(r[2]);
                b->set_rw(r[3]);
            }

            if (node.a() >= 0)
            {
                b->set_animation(node.a());
            }
        }
        void updateNode(com::context::grpc::NodeEntity *node)
        {
            std::cout << "GltfService.updateNode node: { id: " + std::to_string(node->id()) + " }" << std::endl;

            auto a = findDbNodeById(node->id());
            if (a == MAXUINT32)
            {
                std::cerr << "GltfService.updateNode node: { id: " + std::to_string(node->id()) + " } NotFound" << std::endl;
                return;
            }

            auto b = dbContext.mutable_nodes(a);
            b->set_px(node->px());
            b->set_py(node->py());
            b->set_pz(node->pz());
            b->set_rx(node->rx());
            b->set_ry(node->ry());
            b->set_rz(node->rz());
            b->set_rw(node->rw());
            if (node->animation() >= 0)
            {
                b->set_animation(node->animation());
            }
        }
        void updateNode(const com::context::grpc::NodeContext *node)
        {
            // DEPRECATED
            std::cout << "GltfService.updateNode " + getNodeString(node) << std::endl;

            auto a = findNodeById(node->id());
            if (a == MAXUINT32)
            {
                std::cerr << "GltfService.updateNode nodeContext(Id=" + std::to_string(node->id()) +
                                ", node(gltfId=" + std::to_string(node->node().gltfid()) +
                                ", nodeGroupId=" + std::to_string(node->node().nodegroupid()) + ")) NotFound" << std::endl;
                return;
            }
            if (gltfContext.nodes(a).node().gltfid() != node->node().gltfid() ||
                gltfContext.nodes(a).node().nodegroupid() != node->node().nodegroupid())
            {
                std::cerr << "GltfService.updateNode nodeContext(Id=" + std::to_string(node->id()) +
                                ", node(gltfId=" + std::to_string(node->node().gltfid()) +
                                ", nodeGroupId=" + std::to_string(node->node().nodegroupid()) + ")) gltfId or nodeGroupId are not identical" << std::endl;
                return;
            }

            auto n = gltfContext.mutable_nodes(a);
            n->CopyFrom(*node);
        }
        void putNodeAndUpdate(const com::context::grpc::NodeContext *node)
        {
            updateNode(node);
            updateNodes(); // eigentlich nicht f�r alle nodes n�tig!
        }
        com::context::grpc::NodeContext *findNodeContextById(uint32_t nodeId)
        {
            auto a = findNodeById(nodeId);
            return gltfContext.mutable_nodes(a);
        }
        com::context::grpc::LinearTranslation *findLinearTranslationById(uint32_t nodeId)
        {
            auto a = findNodeById(nodeId);
            return gltfContext.mutable_lineartranslations(a);
        }
        void updateNodes()
        {
            uint32_t offset = 0;
            for (int32_t i = 0; i < gltfContext.nodes_size(); i++)
            {
                offset += updateNode(&gltfContext.nodes(i), offset);
            }
        }
        void updateNodeTranslations3()
        {
            uint32_t offset;
            for (size_t i = 0; i < gltfModelContext.size(); i++)
            {
                offset = 0;
                for (size_t s = 0; s < static_cast<size_t>(gltfContext.nodes().size()); s++)
                {
                    if (gltfContext.nodes().at(s).node().gltfid() == std::get<0>(gltfModelContext.at(i)))
                    {
                        offset += updateNode(&gltfContext.nodes(s), offset);
                    }
                }
            }
        }
        RepeatedPtrField<com::context::grpc::LinearTranslation> findAllLinearNodes()
        {
            return gltfContext.lineartranslations();
        }
        RepeatedPtrField<com::context::grpc::ScriptContext> findAllScriptNodes()
        {
            return gltfContext.scripts();
        }
        void buildModel()
        {
            std::cout << "GltfService.buildModel" << std::endl;

            clearModel();

            model = loadModel();
        }
        
        void buildModelFromDatabaseContext()
        {
            auto tStart = std::chrono::high_resolution_clock::now();

            ocean::info("buildModelFromDatabaseContext: Processing " + std::to_string(newGltfModelIdsToBuild.size()) + " models");

            for (const auto& gltfModelId : newGltfModelIdsToBuild) {
                ocean::info("buildModelFromDatabaseContext: Processing gltfModelId=" + std::to_string(gltfModelId));

                // Cache the index result and avoid recomputing multiple times
                size_t index = getModelContextIndexById(gltfModelId);
                ocean::info("buildModelFromDatabaseContext: getModelContextIndexById returned index=" + std::to_string(index) +
                           ", gltfModelContext.size()=" + std::to_string(gltfModelContext.size()));

                // Handle case when the model is not found
                if (index == MAXUINT32) {
                    ocean::info("buildModelFromDatabaseContext: Model not found in context, loading...");
                    auto a = tinyGltfFolder->getGltfModelById(gltfModelId);

                    if (!a) {
                        ocean::error("GltfService.buildModelFromDatabaseContext tinyGltfFolder { gltfId: " +
                            std::to_string(gltfModelId) + " } not found");

                        if (gltfModelId == DEFAULT_GLTF_ERROR_PLACEHOLDER_ID) {
                            tinyGltfFolder->loadGltfFromFile(DEFAULT_GLTF_ERROR_PLACEHOLDER_ID,
                                DEFAULT_GLTF_ERROR_PLACEHOLDER_FILENAME);
                        }
                        else {
                            continue; // Skip if no matching error model
                        }
                    }

                    ocean::info("buildModelFromDatabaseContext: Emplacing new model context entry");
                    // Directly emplace the tuple, minimizing allocations and copies
                    gltfModelContext.emplace_back(gltfModelId, GltfStatus::GLTF_READY, tinygltf::Model());
                    index = gltfModelContext.size() - 1;
                    ocean::info("buildModelFromDatabaseContext: New index=" + std::to_string(index));
                }

                ocean::info("buildModelFromDatabaseContext: About to call clearModelByIndex with index=" + std::to_string(index));
                // Clear the model by index and load the new one
                clearModelByIndex(index);
                ocean::info("buildModelFromDatabaseContext: clearModelByIndex completed");

                ocean::info("buildModelFromDatabaseContext: About to call loadModelFromDatabaseContext");
                // Use a smart pointer to avoid manual memory management
                std::unique_ptr<tinygltf::Model> model(loadModelFromDatabaseContext(gltfModelId));
                ocean::info("buildModelFromDatabaseContext: loadModelFromDatabaseContext completed");

                if (model) {
                    ocean::info("buildModelFromDatabaseContext: Model loaded successfully, updating context at index=" + std::to_string(index));
                    // Update the context efficiently with the new model
                    std::get<2>(gltfModelContext[index]) = std::move(*model);
                    std::get<1>(gltfModelContext[index]) = GltfStatus::GLTF_NODE_MODIFIED;
                    ocean::info("buildModelFromDatabaseContext: Context updated successfully");
                }

                // Memory managed by smart pointer, no need to manually clear model
            }

            // Clear the list of models to build
            newGltfModelIdsToBuild.clear();

            // Timing calculation
            auto tModelBuilding = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - tStart).count();
            std::cout << "GltfService.buildModelFromDatabaseContext took " << tModelBuilding << " ms" << std::endl;
        }
        void buildModel3()
        {
            std::cout << "GltfService.buildModel3" << std::endl;

            size_t index;
            for (size_t i = 0; i < newGltfModelIdsToBuild.size(); i++)
            {
                index = getModelContextIndexById(newGltfModelIdsToBuild[i]);
                if (index == MAXUINT32)
                {
                    auto a = tinyGltfFolder->getGltfModelById(newGltfModelIdsToBuild[i]);
                    if (a == nullptr)
                    {
                        std::cerr << "GltfService.buildModel3 tinyGltfFolder { gltfId: " +
                            std::to_string(newGltfModelIdsToBuild[i]) + " } not found" << std::endl;
                        continue;
                    }
                    auto b = tinygltf::Model();

                    gltfModelContext.push_back(std::make_tuple(
                        newGltfModelIdsToBuild[i], GltfStatus::GLTF_READY, tinygltf::Model()));

                    index = gltfModelContext.size() - 1;
                }
                clearModelByIndex(index);
                tinygltf::Model *model = &std::get<2>(gltfModelContext[index]);
                model = loadModel3(newGltfModelIdsToBuild[i]);
            }
            newGltfModelIdsToBuild.clear();
        }

    private:
        uint32_t getModelContextIndexById(uint32_t gltfId)
        {
            for (size_t i = 0; i < gltfModelContext.size(); i++)
            {
                if (std::get<0>(gltfModelContext[i]) == gltfId)
                    return i;
            }
            return UINT32_MAX;
        }
        void clearModelByIndex(uint32_t gltfIndex)
        {
            if (gltfIndex >= gltfModelContext.size()) {
                ocean::error("clearModelByIndex: index " + std::to_string(gltfIndex) +
                           " out of range (size: " + std::to_string(gltfModelContext.size()) + ")");
                return;
            }
            clearTinyModel(&std::get<2>(gltfModelContext[gltfIndex]));
        }
        void clearTinyModel(tinygltf::Model* model)
        {
            if (model != nullptr)
            {
                model->nodes.clear();
                model->scenes.clear();
                model->meshes.clear();
                model->materials.clear();
                model->textures.clear();
                model->bufferViews.clear();
                model->samplers.clear();
                model->skins.clear();
                model->buffers.clear();
                model->animations.clear();
                model->accessors.clear();
                model->images.clear();
            }
        }
        void clearModel()
        {
            if (model != nullptr)
            {
                clearTinyModel(model);
            }
            else
            {
                model = new tinygltf::Model();
            }

            model->defaultScene = 0;
        }
        bool checkForDuplicateScripts(uint32_t nodeId)
        {
            for (int32_t i = 0; i < gltfContext.scripts_size(); i++)
            {
                if (gltfContext.scripts(i).id() == nodeId)
                    return true;
            }
            return false;
        }
        bool checkForDuplicateLinearTranslations(uint32_t nodeId)
        {
            for (int32_t i = 0; i < gltfContext.lineartranslations_size(); i++)
            {
                if (gltfContext.lineartranslations(i).nodeid() == nodeId)
                    return true;
            }
            return false;
        }
        bool checkForDuplicateNodes(uint32_t nodeId)
        {
            for (int32_t i = 0; i < gltfContext.nodes_size(); i++)
            {
                if (gltfContext.nodes(i).id() == nodeId)
                    return true;
            }
            return false;
        }
        tinygltf::Model *loadModel()
        {
            auto distinctModelIds = findAllDistinctModelIds();
            for (size_t i = 0; i < distinctModelIds.size(); i++)
            {
                loadGltfModel(model, distinctModelIds[i]);
            }

            // to do remove duplicate buffers !!!

            for (int32_t i = 0; i < gltfContext.nodes_size(); i++)
            {
                auto offsets = GltfOffsets{};

                for (uint32_t s = 0; s < distinctModelIds.size(); s++)
                {
                    if (gltfContext.nodes(i).node().gltfid() == distinctModelIds[s])
                        break;

                    tinygltf::Model *nodeModelSrc = tinyGltfFolder->getGltfModelById(distinctModelIds[s]);
                    if (nodeModelSrc == nullptr)
                    {
                        std::cerr << "GltfService.loadModel nodeModelSrc=NULL" << std::endl;
                        return nullptr;
                    }

                    offsets.mesh += nodeModelSrc->meshes.size();
                    offsets.material += nodeModelSrc->materials.size();
                }

                // std::cout << "GltfService.loadModel gltfId=" + std::to_string(gltfContext.nodes(i).node().gltfid()) +
                //     " offsets.mesh = "+std::to_string(offsets.mesh));

                loadNodes(model, &gltfContext.nodes(i), &offsets);
            }

            // tinygltf::TinyGLTF a;

            // std::string filename("C:/resources/vkpbr1/models/context/context.gltf");

            // a.WriteGltfSceneToFile(model, filename, false, false, true, false);

            // std::string* err = nullptr;
            // std::string* warn = nullptr;

            // a.LoadASCIIFromFile(model, err, warn, filename);

            return model;
        }
        tinygltf::Model *loadModel4(uint32_t gltfId)
        {
            std::cout << "GltfService.loadModel4(gltfId=" + std::to_string(gltfId) + ")" << std::endl;

            tinygltf::Model *model = new tinygltf::Model();

            loadGltfModel(model, gltfId);

            assert(model != nullptr);

            // to do remove duplicate buffers !!!

            for (int32_t i = 0; i < gltfContext.nodes_size(); i++)
            {
                if (gltfContext.nodes().at(i).node().gltfid() == gltfId)
                {
                    loadNodes4(model, &gltfContext.nodes(i));
                }
            }
            // tinygltf::TinyGLTF a;

            // std::string filename("C:/resources/vkpbr1/models/context/context4_" + std::to_string(gltfId) + ".gltf");

            // a.WriteGltfSceneToFile(model, filename, false, false, true, false);

            std::cout << "UPDATE tiny_context_gltfs SET model='0x...' WHERE gltf_id = " + std::to_string(gltfId) + ";" << std::endl;

            return model;
        }
        tinygltf::Model *loadModelFromDatabaseContext(uint32_t gltfId)
        {
            std::cout << "GltfService.loadModelFromDatabaseContext(gltfId=" + std::to_string(gltfId) + ")" << std::endl;

            tinygltf::Model *model = new tinygltf::Model();

            loadGltfModel(model, gltfId);

            assert(model != nullptr);

            // to do remove duplicate buffers !!!

            for (int32_t i = 0; i < dbContext.nodes_size(); i++)
            {
                if (dbContext.nodes().at(i).gltfid() == gltfId)
                {
                    loadNodesFromDatabaseContext(model, &dbContext.nodes(i));
                }
            }

            // tinygltf::TinyGLTF a;

            // std::string filename("C:/resources/vkpbr1/models/context/contextFromDbContext_" + std::to_string(gltfId) + ".gltf");

            // a.WriteGltfSceneToFile(model, filename, false, false, true, false);

            std::cout << "UPDATE tiny_context_gltfs SET ... WHERE gltf_id = " + std::to_string(gltfId) + ";" << std::endl;

            return model;
        }
        tinygltf::Model *loadModel3(uint32_t gltfId)
        {
            tinygltf::Model *srcModel = tinyGltfFolder->getGltfModelById(gltfId);

            auto s = getModelContextIndexById(gltfId);

            std::get<2>(gltfModelContext[s]).nodes.clear();
            std::get<2>(gltfModelContext[s]).scenes.clear();
            std::get<2>(gltfModelContext[s]).meshes = std::vector<tinygltf::Mesh>(srcModel->meshes);
            std::get<2>(gltfModelContext[s]).materials = std::vector<tinygltf::Material>(srcModel->materials);
            std::get<2>(gltfModelContext[s]).textures = std::vector<tinygltf::Texture>(srcModel->textures);
            std::get<2>(gltfModelContext[s]).bufferViews = std::vector<tinygltf::BufferView>(srcModel->bufferViews);
            std::get<2>(gltfModelContext[s]).samplers = std::vector<tinygltf::Sampler>(srcModel->samplers);
            std::get<2>(gltfModelContext[s]).skins = std::vector<tinygltf::Skin>(srcModel->skins);
            std::get<2>(gltfModelContext[s]).buffers = std::vector<tinygltf::Buffer>(srcModel->buffers);
            std::get<2>(gltfModelContext[s]).animations = std::vector<tinygltf::Animation>(srcModel->animations);
            std::get<2>(gltfModelContext[s]).accessors = std::vector<tinygltf::Accessor>(srcModel->accessors);
            std::get<2>(gltfModelContext[s]).images = std::vector<tinygltf::Image>(srcModel->images);
            std::get<2>(gltfModelContext[s]).defaultScene = 0;

            auto offsets = GltfOffsets{};

            for (int32_t i = 0; i < gltfContext.nodes_size(); i++)
            {
                if (gltfContext.nodes(i).node().gltfid() == gltfId)
                {
                    std::cout << "GltfService.loadModel3 gltfId=" +gltfContext.nodes(i).node().gltfid() << std::endl;

                    loadNodes(&std::get<2>(gltfModelContext[s]), &gltfContext.nodes(i), &offsets);
                }
            }

            //tinygltf::TinyGLTF a;

            //std::string filename("C:/resources/vkpbr1/models/context/context_" + std::to_string(gltfId) + ".gltf");

            //a.WriteGltfSceneToFile(&std::get<2>(gltfModelContext[s]), filename, false, false, true, false);

            // std::string* err = nullptr;
            // std::string* warn = nullptr;

            // a.LoadASCIIFromFile(model, err, warn, filename);

            return &std::get<2>(gltfModelContext[s]);
        }

        uint32_t findNodeById(uint32_t nodeId)
        {
            for (int32_t i = 0; i < gltfContext.nodes_size(); i++)
            {
                if (gltfContext.nodes().at(i).id() == nodeId)
                {
                    return i;
                }
            }
            return MAXUINT32;
        }
        uint32_t findDbNodeById(uint32_t nodeId)
        {
            for (int32_t i = 0; i < dbContext.nodes_size(); i++)
            {
                if (dbContext.nodes().at(i).id() == nodeId)
                {
                    return i;
                }
            }
            return MAXUINT32;
        }
        uint32_t findLinearNodeById(uint32_t nodeId)
        {
            for (size_t i = 0; i < static_cast<size_t>(gltfContext.lineartranslations_size()); i++)
            {
                if (gltfContext.nodes().at(i).id() == nodeId)
                {
                    return i;
                }
            }
            return MAXUINT32;
        }
        bool isLoaded(uint32_t modelId)
        {
            for (int32_t i = 0; i < gltfContext.nodes_size(); i++)
            {
                if (gltfContext.nodes(i).node().gltfid() == modelId)
                {
                    return true;
                }
            }
            return false;
        }
        void loadGltfModel(tinygltf::Model *model, uint32_t modelId)
        {
            assert(model != nullptr);

            auto src = tinyGltfFolder->getGltfModelById(modelId);

            if (src == nullptr)
            {
                std::cerr << "GltfService.loadGltfModel modelId=" << modelId << " model=NULL";
                model = nullptr;
                return;
            }

            if (model->buffers.empty())
            {
                if (src->animations.size() > 0)
                {
                    model->nodes = std::vector<tinygltf::Node>(src->nodes);
                    model->scenes = std::vector<tinygltf::Scene>(src->scenes);
                }
                // model->nodes = std::vector<tinygltf::Node>(src->nodes);
                // model->scenes = std::vector<tinygltf::Scene>(src->scenes);
                model->meshes = std::vector<tinygltf::Mesh>(src->meshes);
                model->materials = std::vector<tinygltf::Material>(src->materials);
                model->textures = std::vector<tinygltf::Texture>(src->textures);
                model->bufferViews = std::vector<tinygltf::BufferView>(src->bufferViews);
                model->samplers = std::vector<tinygltf::Sampler>(src->samplers);
                model->skins = std::vector<tinygltf::Skin>(src->skins);
                model->buffers = std::vector<tinygltf::Buffer>(src->buffers);
                model->animations = std::vector<tinygltf::Animation>(src->animations);
                model->accessors = std::vector<tinygltf::Accessor>(src->accessors);
                model->images = std::vector<tinygltf::Image>(src->images);
                model->defaultScene = 0;
            }
            else
            {
                auto bufferOffset = model->buffers.size();
                auto materialOffset = model->materials.size();
                auto textureOffset = model->textures.size();
                auto bufferViewOffset = model->bufferViews.size();
                auto accessorOffset = model->accessors.size();
                auto samplerOffset = model->samplers.size();
                auto imageOffset = model->images.size();

                for (size_t i = 0; i < src->meshes.size(); i++)
                {
                    auto a = src->meshes[i];
                    // auto b = tinygltf::Mesh();
                    for (size_t p = 0; p < a.primitives.size(); p++)
                    {
                        a.primitives[p].indices += accessorOffset;

                        if (a.primitives[p].attributes.find("POSITION") != a.primitives[p].attributes.end())
                        {
                            a.primitives[p].attributes.find("POSITION")->second += accessorOffset;
                        }
                        
                        if (a.primitives[p].attributes.find("NORMAL") != a.primitives[p].attributes.end())
                        {
                            a.primitives[p].attributes.find("NORMAL")->second += accessorOffset;
                        }

                        if (a.primitives[p].attributes.find("TEXCOORD_0") != a.primitives[p].attributes.end())
                        {
                            a.primitives[p].attributes.find("TEXCOORD_0")->second += accessorOffset;
                        }

                        if (a.primitives[p].attributes.find("TANGENT") != a.primitives[p].attributes.end())
                        {
                            a.primitives[p].attributes.find("TANGENT")->second += accessorOffset;
                        }
                        a.primitives[p].material += materialOffset;
                    }
                    model->meshes.push_back(a);
                }
                for (size_t i = 0; i < src->materials.size(); i++)
                {
                    auto a = src->materials[i];

                    if (a.pbrMetallicRoughness.baseColorTexture.index != -1)
                        a.pbrMetallicRoughness.baseColorTexture.index += textureOffset;

                    if (a.emissiveTexture.index != -1)
                        a.emissiveTexture.index += textureOffset;

                    if (a.normalTexture.index != -1)
                        a.normalTexture.index += textureOffset;

                    if (a.occlusionTexture.index != -1)
                        a.occlusionTexture.index += textureOffset;

                    if (a.pbrMetallicRoughness.metallicRoughnessTexture.index != -1)
                        a.pbrMetallicRoughness.metallicRoughnessTexture.index += textureOffset;

                    model->materials.push_back(a);
                }
                for (size_t i = 0; i < src->textures.size(); i++)
                {
                    auto a = src->textures[i];
                    auto b = tinygltf::Texture();

                    if (a.sampler != -1)
                        b.sampler = a.sampler + samplerOffset;

                    if (a.source != -1)
                        b.source = a.source + imageOffset;

                    model->textures.push_back(b);
                }
                for (size_t i = 0; i < src->bufferViews.size(); i++)
                {
                    auto a = src->bufferViews[i];
                    a.buffer += bufferOffset;
                    model->bufferViews.push_back(a);
                }
                for (size_t i = 0; i < src->samplers.size(); i++)
                {
                    auto a = src->samplers[i];
                    model->samplers.push_back(a);
                }
                for (size_t i = 0; i < src->skins.size(); i++)
                {
                    auto a = src->skins[i];
                    model->skins.push_back(a);
                }

                for (size_t i = 0; i < src->buffers.size(); i++)
                {
                    auto a = src->buffers[i];
                    model->buffers.push_back(a);
                }
                for (size_t i = 0; i < src->accessors.size(); i++)
                {
                    auto a = src->accessors[i];
                    a.bufferView += bufferViewOffset;
                    model->accessors.push_back(a);
                }
                for (size_t i = 0; i < src->images.size(); i++)
                {
                    auto a = src->images[i];
                    model->images.push_back(a);
                }
            }
        }
        void loadNodes(tinygltf::Model *model, const com::context::grpc::NodeContext *node,
                       GltfOffsets *offsets)
        {
            if (model == nullptr || model->meshes.size() == 0)
                return;

            tinygltf::Model *nodeModelSrc = tinyGltfFolder->getGltfModelById(node->node().gltfid());

            if (model->scenes.size() == 0)
            {
                auto a = tinygltf::Scene();
                a.name = "Scene";
                model->scenes.push_back(a);
            }

            if (nodeModelSrc->animations.size() == 0)
            {
                // Nodes
                size_t nodeOffsetDest = model->nodes.size();
                for (size_t i = 0; i < nodeModelSrc->scenes.at(nodeModelSrc->defaultScene).nodes.size(); i++)
                {
                    // std::cout << "GltfService.loadNodes i="+std::to_string(i)+ " of nodes.size()="+
                    //     std::to_string(nodeModelSrc->scenes.at(nodeModelSrc->defaultScene).nodes.size()));

                    size_t nodeSrcIndex = nodeModelSrc->scenes.at(nodeModelSrc->defaultScene).nodes[i];

                    auto n = tinygltf::Node();

                    if (nodeModelSrc->nodes[nodeSrcIndex].translation.size() == 3)
                    {
                        n.translation.push_back(nodeModelSrc->nodes[nodeSrcIndex].translation[0] + node->node().px());
                        n.translation.push_back(nodeModelSrc->nodes[nodeSrcIndex].translation[1] + node->node().py());
                        n.translation.push_back(nodeModelSrc->nodes[nodeSrcIndex].translation[2] + node->node().pz());
                    }
                    else
                    {
                        n.translation.push_back(node->node().px());
                        n.translation.push_back(node->node().py());
                        n.translation.push_back(node->node().pz());
                    }
                    if (nodeModelSrc->nodes[nodeSrcIndex].rotation.size() == 4)
                    {
                        n.rotation.push_back(nodeModelSrc->nodes[nodeSrcIndex].rotation[0]);
                        n.rotation.push_back(nodeModelSrc->nodes[nodeSrcIndex].rotation[1]);
                        n.rotation.push_back(nodeModelSrc->nodes[nodeSrcIndex].rotation[2]);
                        n.rotation.push_back(nodeModelSrc->nodes[nodeSrcIndex].rotation[3]);
                    }

                    if (nodeModelSrc->nodes[nodeSrcIndex].scale.size() == 0)
                    {
                        nodeModelSrc->nodes[nodeSrcIndex].scale.push_back(1.0);
                        nodeModelSrc->nodes[nodeSrcIndex].scale.push_back(1.0);
                        nodeModelSrc->nodes[nodeSrcIndex].scale.push_back(1.0);
                    }
                    // if (nodeModelSrc->nodes[nodeSrcIndex].scale.size() == 3)
                    {
                        n.scale.push_back(nodeModelSrc->nodes[nodeSrcIndex].scale[0]);
                        n.scale.push_back(nodeModelSrc->nodes[nodeSrcIndex].scale[1]);
                        n.scale.push_back(nodeModelSrc->nodes[nodeSrcIndex].scale[2]);
                    }
                    n.mesh = nodeModelSrc->nodes[nodeSrcIndex].mesh + offsets->mesh;

                    // std::cout << "GltfService.loadNodes n.mesh=" + std::to_string(n.mesh) +
                    //     " gltfId=" + std::to_string(node->node().gltfid()));

                    if (nodeModelSrc->nodes[nodeSrcIndex].skin != -1)
                        n.skin = nodeModelSrc->nodes[nodeSrcIndex].skin + offsets->skin;

                    model->nodes.push_back(n);
                    model->scenes.at(model->defaultScene).nodes.push_back(i + nodeOffsetDest);

                    // loadChildNode(nodeModelSrc, model->nodes, nodeModelSrc->nodes[nodeSrcIndex].children);
                }
            }
            else
            {
                model->nodes = std::vector<tinygltf::Node>(nodeModelSrc->nodes);
                model->scenes.at(model->defaultScene).nodes.push_back(nodeModelSrc->scenes[0].nodes[0]);
            }
        }
        void loadNodesFromDatabaseContext(tinygltf::Model *model, const com::context::grpc::NodeEntity *node)
        {
            std::cout << "GltfService.loadNodesFromDatabaseContext node.gltfid=" + std::to_string(node->gltfid()) << std::endl;
            auto last = std::chrono::high_resolution_clock::now();
            if (model == nullptr || model->meshes.size() == 0)
                return;

            tinygltf::Model *nodeModelSrc = tinyGltfFolder->getGltfModelById(node->gltfid());

            std::cout << "GltfService.loadNodesFromDatabaseContext mesh.count=" + 
                std::to_string(nodeModelSrc->meshes.size()) + " gltfId=" + std::to_string(node->gltfid()) << std::endl;

            if (model->scenes.size() == 0)
            {
                auto a = tinygltf::Scene();
                a.name = "Scene";
                model->scenes.push_back(a);
            }

            // Nodes
            size_t nodeOffsetDest = model->nodes.size();
            for (size_t i = 0; i < nodeModelSrc->scenes.at(nodeModelSrc->defaultScene).nodes.size(); i++)
            {
                // std::cout << "GltfService.loadNodes i="+std::to_string(i)+ " of nodes.size()="+
                //     std::to_string(nodeModelSrc->scenes.at(nodeModelSrc->defaultScene).nodes.size()));

                size_t nodeSrcIndex = nodeModelSrc->scenes.at(nodeModelSrc->defaultScene).nodes[i];

                auto n = tinygltf::Node();

                n.id = node->id();
                n.shader = node->shader();

                if (nodeModelSrc->nodes[nodeSrcIndex].translation.size() == 3)
                {
                    n.translation.push_back(nodeModelSrc->nodes[nodeSrcIndex].translation[0] + node->px());
                    n.translation.push_back(nodeModelSrc->nodes[nodeSrcIndex].translation[1] + node->py());
                    n.translation.push_back(nodeModelSrc->nodes[nodeSrcIndex].translation[2] + node->pz());
                }
                else
                {
                    n.translation.push_back(node->px());
                    n.translation.push_back(node->py());
                    n.translation.push_back(node->pz());
                }
                if (nodeModelSrc->nodes[nodeSrcIndex].rotation.size() == 4)
                {
                    n.rotation.push_back(nodeModelSrc->nodes[nodeSrcIndex].rotation[0]);
                    n.rotation.push_back(nodeModelSrc->nodes[nodeSrcIndex].rotation[1]);
                    n.rotation.push_back(nodeModelSrc->nodes[nodeSrcIndex].rotation[2]);
                    n.rotation.push_back(nodeModelSrc->nodes[nodeSrcIndex].rotation[3]);
                }

                if (nodeModelSrc->nodes[nodeSrcIndex].scale.size() == 0)
                {
                    nodeModelSrc->nodes[nodeSrcIndex].scale.push_back(1.0);
                    nodeModelSrc->nodes[nodeSrcIndex].scale.push_back(1.0);
                    nodeModelSrc->nodes[nodeSrcIndex].scale.push_back(1.0);
                }
                // if (nodeModelSrc->nodes[nodeSrcIndex].scale.size() == 3)
                {
                    n.scale.push_back(nodeModelSrc->nodes[nodeSrcIndex].scale[0]);
                    n.scale.push_back(nodeModelSrc->nodes[nodeSrcIndex].scale[1]);
                    n.scale.push_back(nodeModelSrc->nodes[nodeSrcIndex].scale[2]);
                }
                n.mesh = nodeModelSrc->nodes[nodeSrcIndex].mesh;

                // std::cout << "GltfService.loadNodes n.mesh=" + std::to_string(n.mesh) +
                //     " gltfId=" + std::to_string(node->node().gltfid()));

                n.skin = nodeModelSrc->nodes[nodeSrcIndex].skin;

                model->nodes.push_back(n);
                model->scenes.at(model->defaultScene).nodes.push_back(i + nodeOffsetDest);

                // loadChildNode(nodeModelSrc, model->nodes, nodeModelSrc->nodes[nodeSrcIndex].children);
            }
            auto duration = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - last).count();
            std::cout << "GltfService.loadNodesFromDatabaseContext node.gltfid=" + 
                std::to_string(node->gltfid()) + " END after "+std::to_string(duration) + " ms" << std::endl;
        }
        void loadNodes4(tinygltf::Model *model, const com::context::grpc::NodeContext *node)
        {
            // std::cout << "GltfService.loadNodes4 " + node->DebugString());
            auto start = std::chrono::high_resolution_clock::now();

            if (model == nullptr || model->meshes.size() == 0)
                return;

            tinygltf::Model *nodeModelSrc = tinyGltfFolder->getGltfModelById(node->node().gltfid());

            std::cout << "GltfService.loadNodes4 mesh.count=" + std::to_string(nodeModelSrc->meshes.size()) +
                " gltfId=" + std::to_string(node->node().gltfid()) << std::endl;

            if (model->scenes.size() == 0)
            {
                auto a = tinygltf::Scene();
                a.name = "Scene";
                model->scenes.push_back(a);
            }

            // Nodes
            size_t nodeOffsetDest = model->nodes.size();
            for (size_t i = 0; i < nodeModelSrc->scenes.at(nodeModelSrc->defaultScene).nodes.size(); i++)
            {
                // std::cout << "GltfService.loadNodes i="+std::to_string(i)+ " of nodes.size()="+
                //     std::to_string(nodeModelSrc->scenes.at(nodeModelSrc->defaultScene).nodes.size()));

                size_t nodeSrcIndex = nodeModelSrc->scenes.at(nodeModelSrc->defaultScene).nodes[i];

                auto n = tinygltf::Node();

                n.id = node->id();

                if (nodeModelSrc->nodes[nodeSrcIndex].translation.size() == 3)
                {
                    n.translation.push_back(nodeModelSrc->nodes[nodeSrcIndex].translation[0] + node->node().px());
                    n.translation.push_back(nodeModelSrc->nodes[nodeSrcIndex].translation[1] + node->node().py());
                    n.translation.push_back(nodeModelSrc->nodes[nodeSrcIndex].translation[2] + node->node().pz());
                }
                else
                {
                    n.translation.push_back(node->node().px());
                    n.translation.push_back(node->node().py());
                    n.translation.push_back(node->node().pz());
                }
                if (nodeModelSrc->nodes[nodeSrcIndex].rotation.size() == 4)
                {
                    n.rotation.push_back(nodeModelSrc->nodes[nodeSrcIndex].rotation[0]);
                    n.rotation.push_back(nodeModelSrc->nodes[nodeSrcIndex].rotation[1]);
                    n.rotation.push_back(nodeModelSrc->nodes[nodeSrcIndex].rotation[2]);
                    n.rotation.push_back(nodeModelSrc->nodes[nodeSrcIndex].rotation[3]);
                }

                if (nodeModelSrc->nodes[nodeSrcIndex].scale.size() == 0)
                {
                    nodeModelSrc->nodes[nodeSrcIndex].scale.push_back(1.0);
                    nodeModelSrc->nodes[nodeSrcIndex].scale.push_back(1.0);
                    nodeModelSrc->nodes[nodeSrcIndex].scale.push_back(1.0);
                }
                // if (nodeModelSrc->nodes[nodeSrcIndex].scale.size() == 3)
                {
                    n.scale.push_back(nodeModelSrc->nodes[nodeSrcIndex].scale[0]);
                    n.scale.push_back(nodeModelSrc->nodes[nodeSrcIndex].scale[1]);
                    n.scale.push_back(nodeModelSrc->nodes[nodeSrcIndex].scale[2]);
                }
                n.mesh = nodeModelSrc->nodes[nodeSrcIndex].mesh;

                // std::cout << "GltfService.loadNodes n.mesh=" + std::to_string(n.mesh) +
                //     " gltfId=" + std::to_string(node->node().gltfid()));

                n.skin = nodeModelSrc->nodes[nodeSrcIndex].skin;

                model->nodes.push_back(n);
                model->scenes.at(model->defaultScene).nodes.push_back(i + nodeOffsetDest);

                // loadChildNode(nodeModelSrc, model->nodes, nodeModelSrc->nodes[nodeSrcIndex].children);
            }

            auto duration = std::chrono::duration<double, std::milli>(
                                std::chrono::high_resolution_clock::now() - start)
                                .count();

            std::cout << "GltfService.loadNodes4 gltfId=" + std::to_string(node->node().gltfid()) + " loaded in " + std::to_string(duration) + " ms" << std::endl;
        }

        void loadChildNode(tinygltf::Model *nodeModelSrc, std::vector<tinygltf::Node> &nodes,
                           std::vector<int> &children)
        {
            for (size_t i = 0; i < children.size(); i++)
            {
                auto n = tinygltf::Node();

                if (nodeModelSrc->nodes[children[i]].translation.size() == 3)
                {
                    n.translation.push_back(nodeModelSrc->nodes[children[i]].translation[0]);
                    n.translation.push_back(nodeModelSrc->nodes[children[i]].translation[1]);
                    n.translation.push_back(nodeModelSrc->nodes[children[i]].translation[2]);
                }
                n.mesh = nodeModelSrc->nodes[children[i]].mesh;
                n.skin = nodeModelSrc->nodes[children[i]].skin;
                n.name = nodeModelSrc->nodes[children[i]].name;
                nodes.push_back(n);

                if (nodeModelSrc->nodes[children[i]].children.size() > 0)
                    loadChildNode(nodeModelSrc, nodes, nodeModelSrc->nodes[children[i]].children);
            }
        }
        uint32_t updateNode(const com::context::grpc::NodeContext *node, uint32_t nodeIndexOffset)
        {
            // FEHLERHAFT
            // es sollte nicht tinygltf verwendet werden!!!

            tinygltf::Model *nodeModelSrc = tinyGltfFolder->getGltfModelById(node->node().gltfid());

            if (nodeModelSrc == nullptr)
                return nodeIndexOffset;

            size_t nodeSrcIndex;
            // Nodes
            for (size_t i = 0; i < nodeModelSrc->scenes.at(nodeModelSrc->defaultScene).nodes.size(); i++)
            {
                nodeSrcIndex = nodeModelSrc->scenes.at(nodeModelSrc->defaultScene).nodes[i];

                auto index = getModelContextIndexById(node->node().gltfid());

                // std::get<1>(gltfModelContext[index]) = GltfStatus::GLTF_NODE_UPDATED;

                auto n = &std::get<2>(gltfModelContext[index]).nodes[nodeIndexOffset + i];

                if (nodeModelSrc->nodes[nodeSrcIndex].scale.size() == 0)
                {
                    nodeModelSrc->nodes[nodeSrcIndex].scale.push_back(1.0);
                    nodeModelSrc->nodes[nodeSrcIndex].scale.push_back(1.0);
                    nodeModelSrc->nodes[nodeSrcIndex].scale.push_back(1.0);
                }

                if (n->scale.size() == 0)
                {
                    n->scale.push_back(1.0);
                    n->scale.push_back(1.0);
                    n->scale.push_back(1.0);
                }

                if (nodeModelSrc->nodes[nodeSrcIndex].translation.size() == 3)
                {
                    n->translation[0] = nodeModelSrc->nodes[nodeSrcIndex].translation[0] + node->node().px();
                    n->translation[1] = nodeModelSrc->nodes[nodeSrcIndex].translation[1] + node->node().py();
                    n->translation[2] = nodeModelSrc->nodes[nodeSrcIndex].translation[2] + node->node().pz();
                }
                else if (n->translation.size() == 3)
                {
                    n->translation[0] = node->node().px();
                    n->translation[1] = node->node().py();
                    n->translation[2] = node->node().pz();
                }

                if (nodeModelSrc->nodes[nodeSrcIndex].rotation.size() == 4)
                {
                    if (n->rotation.size() == 0)
                    {
                        n->rotation.push_back(1.0);
                        n->rotation.push_back(0.0);
                        n->rotation.push_back(0.0);
                        n->rotation.push_back(1.0);
                    }
                    n->rotation[0] = nodeModelSrc->nodes[nodeSrcIndex].rotation[0];
                    n->rotation[1] = nodeModelSrc->nodes[nodeSrcIndex].rotation[1];
                    n->rotation[2] = nodeModelSrc->nodes[nodeSrcIndex].rotation[2];
                    n->rotation[3] = nodeModelSrc->nodes[nodeSrcIndex].rotation[3];
                }
                if (nodeModelSrc->nodes[nodeSrcIndex].scale.size() == 3)
                {
                    n->scale[0] = nodeModelSrc->nodes[nodeSrcIndex].scale[0];
                    n->scale[1] = nodeModelSrc->nodes[nodeSrcIndex].scale[1];
                    n->scale[2] = nodeModelSrc->nodes[nodeSrcIndex].scale[2];
                }
                // n->mesh = nodeModelSrc->nodes[nodeSrcIndex].mesh;
                // n->skin = nodeModelSrc->nodes[nodeSrcIndex].skin;
            }
            return nodeModelSrc->scenes.at(nodeModelSrc->defaultScene).nodes.size();
        }
        std::vector<uint32_t> findAllDistinctModelIds()
        {
            auto d = std::vector<uint32_t>();
            uint32_t nid;
            for (int32_t i = 0; i < gltfContext.nodes_size(); i++)
            {
                nid = gltfContext.nodes().at(i).node().gltfid();
                if (std::find(d.begin(), d.end(), nid) == d.end())
                {
                    d.push_back(nid);
                }
            }
            return d;
        }
    };
}