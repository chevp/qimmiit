/**
 * Copyright (C) by Patrice Chevillat
 */

#pragma once
#include <string>
#include <vector>
#include <glm/fwd.hpp>

namespace rscpocean {

    struct Head {
        std::string title = "";
        std::string mode = "";
        std::string author = "";
        std::string base = "";
    };

    struct Node {
        std::string id = "";
        std::string gltfId = "";
        float px;
        float py;
        float pz;
        float scale = 1.0f;
    };

    struct NodeGroup {
        std::string id = "";
        std::string gltfId = "";
        std::string gltfUri = "";
    };

    struct Gltf {
        std::string id = "";
        std::string uri = "";
    };

    struct Model {
        std::string id = "";
        Head head;
        std::vector<Node> nodes;
        std::vector<NodeGroup> nodeGroups;
        std::vector<Gltf> gltfs;
    };
}
