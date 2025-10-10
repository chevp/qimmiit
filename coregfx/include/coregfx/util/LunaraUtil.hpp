#pragma once
#include <coregfx/protobuf_wrapper.hpp>

namespace lunara_utils {

    template <typename T>
    T* getMutableComponent(nyx::Entity* entity) {
        static_assert(sizeof(T) == 0, "getMutableComponent<T>() must be specialized for this component type.");
        return nullptr;
    }

    // Generic template (will cause link error if used for unsupported type)
    template<typename T>
    const T* getComponent(const nyx::Entity& entity);

    template <>
    inline nyx::TransformComponent* getMutableComponent(nyx::Entity* entity) {
        for (int i = 0; i < entity->components_size(); ++i) {
            nyx::Component* component = entity->mutable_components(i);
            if (component->has_transform()) {
                return component->mutable_transform();
            }
        }
        return nullptr;
    }

    // TransformComponent specialization
    template<>
    inline const nyx::TransformComponent* getComponent(const nyx::Entity& entity) {
        for (const auto& component : entity.components()) {
            if (component.has_transform()) {
                return &component.transform();
            }
        }
        return nullptr;
    }

    // VelocityComponent specialization
    template<>
    inline const nyx::VelocityComponent* getComponent(const nyx::Entity& entity) {
        for (const auto& component : entity.components()) {
            if (component.has_velocity()) {
                return &component.velocity();
            }
        }
        return nullptr;
    }

    // CameraComponent specialization
    template<>
    inline const nyx::CameraComponent* getComponent(const nyx::Entity& entity) {
        for (const auto& component : entity.components()) {
            if (component.has_camera()) {
                return &component.camera();
            }
        }
        return nullptr;
    }

    // ModelComponent specialization
    template<>
    inline const nyx::ModelComponent* getComponent(const nyx::Entity& entity) {
        for (const auto& component : entity.components()) {
            if (component.has_model()) {
                return &component.model();
            }
        }
        return nullptr;
    }

    // AnimationComponent specialization
    template<>
    inline const nyx::AnimationComponent* getComponent(const nyx::Entity& entity) {
        for (const auto& component : entity.components()) {
            if (component.has_animation()) {
                return &component.animation();
            }
        }
        return nullptr;
    }

    // RenderComponent specialization
    template<>
    inline const nyx::RenderComponent* getComponent(const nyx::Entity& entity) {
        for (const auto& component : entity.components()) {
            if (component.has_render()) {
                return &component.render();
            }
        }
        return nullptr;
    }

    // Check existence of component
    template<typename T>
    inline bool hasComponent(const nyx::Entity& entity) {
        return getComponent<T>(entity) != nullptr;
    }

    inline com::context::grpc::NodeEntity toNodeEntity(const nyx::Entity& entity) {
        com::context::grpc::NodeEntity node;

        // Convert entity ID from string to int
        try {
            node.set_id(std::stoi(entity.id()));
        }
        catch (const std::exception& e) {
            ocean::error("Invalid Entity ID '{}': {}", e.what());
            node.set_id(0); // Fallback or handle more gracefully
        }

        // Get TransformComponent
        const auto* transform = getComponent<nyx::TransformComponent>(entity);
        if (transform) {
            node.set_px(transform->pos().x());
            node.set_py(transform->pos().y());
            node.set_pz(transform->pos().z());

            if (transform->has_rot()) {
                node.set_rx(transform->rot().x());
                node.set_ry(transform->rot().y());
                node.set_rz(transform->rot().z());
                node.set_rw(transform->rot().w());
            }
            else {
                // Default rotation: identity quaternion
                node.set_rx(0);
                node.set_ry(0);
                node.set_rz(0);
                node.set_rw(1);
            }
        }
        else {
            ocean::warn("Entity '{}' has no TransformComponent", entity.id());
        }

        // Get ModelComponent
        const auto* model = getComponent<nyx::ModelComponent>(entity);
        if (model) {
            try {
                node.set_gltfid(std::stoi(model->model_id()));
            }
            catch (const std::exception& e) {
                ocean::error("Invalid model_id '{}': {}", e.what());
                node.set_gltfid(0); // Fallback or handle more gracefully
            }
        }
        else {
            ocean::warn("Entity '{}' has no ModelComponent", entity.id());
        }

        return node;
    }

} // namespace lunara_utils
