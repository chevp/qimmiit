package io.qimmiit.siqiniq.studio.service.scene;

import com.google.protobuf.Empty;
import cryo.services.CryoSceneAuthoringServiceOuterClass.*;
import jakarta.enterprise.context.ApplicationScoped;
import jakarta.inject.Inject;
import org.jboss.logging.Logger;

import java.util.List;
import java.util.UUID;

/**
 * Service handling Entity operations within scenes
 */
@ApplicationScoped
public class EntityOperationsService {

    private static final Logger LOG = Logger.getLogger(EntityOperationsService.class);

    @Inject
    SceneDataRepository repository;

    public void addEntity(AddEntityRequest request,
                          io.grpc.stub.StreamObserver<EntityResponse> responseObserver) {
        try {
            SceneDataRepository.SceneData scene = repository.findById(request.getSceneId());
            if (scene == null) {
                responseObserver.onError(new RuntimeException("Scene not found: " + request.getSceneId()));
                return;
            }

            String entityId = UUID.randomUUID().toString();

            SceneDataRepository.EntityData entity = new SceneDataRepository.EntityData();
            entity.setInfo(EntityInfo.newBuilder()
                    .setId(entityId)
                    .setName("Entity_" + entityId.substring(0, 8))
                    .setType("Entity")
                    .setEnabled(true)
                    .setChildCount(0)
                    .build());
            entity.setContent(request.getEntityContent());
            entity.setParentId(request.hasParentId() ? request.getParentId() : null);

            scene.getEntities().put(entityId, entity);

            // Update entity count
            scene.setMetadata(scene.getMetadata().toBuilder()
                    .setEntityCount(scene.getEntities().size())
                    .build());

            EntityResponse response = EntityResponse.newBuilder()
                    .setInfo(entity.getInfo())
                    .setContent(entity.getContent())
                    .build();

            LOG.infof("Added entity %s to scene %s", entityId, request.getSceneId());
            responseObserver.onNext(response);
            responseObserver.onCompleted();

        } catch (Exception e) {
            LOG.error("Failed to add entity", e);
            responseObserver.onError(e);
        }
    }

    public void updateEntity(UpdateEntityRequest request,
                             io.grpc.stub.StreamObserver<EntityResponse> responseObserver) {
        try {
            SceneDataRepository.SceneData scene = repository.findById(request.getSceneId());
            if (scene == null) {
                responseObserver.onError(new RuntimeException("Scene not found: " + request.getSceneId()));
                return;
            }

            SceneDataRepository.EntityData entity = scene.getEntities().get(request.getEntityId());
            if (entity == null) {
                responseObserver.onError(new RuntimeException("Entity not found: " + request.getEntityId()));
                return;
            }

            entity.setContent(request.getEntityContent());

            EntityResponse response = EntityResponse.newBuilder()
                    .setInfo(entity.getInfo())
                    .setContent(entity.getContent())
                    .build();

            responseObserver.onNext(response);
            responseObserver.onCompleted();

        } catch (Exception e) {
            LOG.error("Failed to update entity", e);
            responseObserver.onError(e);
        }
    }

    public void removeEntity(RemoveEntityRequest request,
                             io.grpc.stub.StreamObserver<Empty> responseObserver) {
        try {
            SceneDataRepository.SceneData scene = repository.findById(request.getSceneId());
            if (scene == null) {
                responseObserver.onError(new RuntimeException("Scene not found: " + request.getSceneId()));
                return;
            }

            SceneDataRepository.EntityData removed = scene.getEntities().remove(request.getEntityId());
            if (removed == null) {
                responseObserver.onError(new RuntimeException("Entity not found: " + request.getEntityId()));
                return;
            }

            // Update entity count
            scene.setMetadata(scene.getMetadata().toBuilder()
                    .setEntityCount(scene.getEntities().size())
                    .build());

            LOG.infof("Removed entity %s from scene %s", request.getEntityId(), request.getSceneId());
            responseObserver.onNext(Empty.getDefaultInstance());
            responseObserver.onCompleted();

        } catch (Exception e) {
            LOG.error("Failed to remove entity", e);
            responseObserver.onError(e);
        }
    }

    public void listEntities(ListEntitiesRequest request,
                             io.grpc.stub.StreamObserver<ListEntitiesResponse> responseObserver) {
        try {
            SceneDataRepository.SceneData scene = repository.findById(request.getSceneId());
            if (scene == null) {
                responseObserver.onError(new RuntimeException("Scene not found: " + request.getSceneId()));
                return;
            }

            List<EntityInfo> entities = scene.getEntities().values().stream()
                    .map(SceneDataRepository.EntityData::getInfo)
                    .toList();

            ListEntitiesResponse response = ListEntitiesResponse.newBuilder()
                    .addAllEntities(entities)
                    .setTotalCount(entities.size())
                    .build();

            responseObserver.onNext(response);
            responseObserver.onCompleted();

        } catch (Exception e) {
            LOG.error("Failed to list entities", e);
            responseObserver.onError(e);
        }
    }
}