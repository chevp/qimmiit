package io.qimmiit.siqiniq.studio.service.scene;

import com.google.protobuf.Empty;
import cryo.services.CryoSceneAuthoringServiceOuterClass.*;
import jakarta.enterprise.context.ApplicationScoped;
import jakarta.inject.Inject;
import org.jboss.logging.Logger;

import java.util.ArrayList;
import java.util.List;
import java.util.UUID;

/**
 * Service handling Scene CRUD operations
 */
@ApplicationScoped
public class SceneCrudService {

    private static final Logger LOG = Logger.getLogger(SceneCrudService.class);

    @Inject
    SceneDataRepository repository;

    public void createScene(CreateSceneRequest request,
                            io.grpc.stub.StreamObserver<SceneResponse> responseObserver) {
        try {
            String id = UUID.randomUUID().toString();

            SceneMetadata metadata = SceneMetadata.newBuilder()
                    .setId(id)
                    .setName(request.getName())
                    .setDescription(request.getDescription())
                    .setProjectId(request.getProjectId())
                    .setCreatedBy(request.getCreatedBy())
                    .addAllTags(request.getTagsList())
                    .setStatus(SceneStatus.DRAFT)
                    .setEntityCount(0)
                    .build();

            SceneDataRepository.SceneData data = new SceneDataRepository.SceneData();
            data.setMetadata(metadata);
            data.setContent(request.getInitialContent());

            repository.save(id, data);

            SceneResponse response = SceneResponse.newBuilder()
                    .setMetadata(metadata)
                    .setContent(data.getContent())
                    .setFormat(SceneFormat.PBTXT)
                    .build();

            LOG.infof("Created scene: %s (%s)", metadata.getName(), id);
            responseObserver.onNext(response);
            responseObserver.onCompleted();

        } catch (Exception e) {
            LOG.error("Failed to create scene", e);
            responseObserver.onError(e);
        }
    }

    public void getScene(GetSceneRequest request,
                         io.grpc.stub.StreamObserver<SceneResponse> responseObserver) {
        try {
            SceneDataRepository.SceneData data = repository.findById(request.getId());
            if (data == null) {
                responseObserver.onError(new RuntimeException("Scene not found: " + request.getId()));
                return;
            }

            String content = convertSceneFormat(data.getContent(), request.getFormat());

            SceneResponse response = SceneResponse.newBuilder()
                    .setMetadata(data.getMetadata())
                    .setContent(content)
                    .setFormat(request.getFormat())
                    .build();

            responseObserver.onNext(response);
            responseObserver.onCompleted();

        } catch (Exception e) {
            LOG.error("Failed to get scene", e);
            responseObserver.onError(e);
        }
    }

    public void updateScene(UpdateSceneRequest request,
                            io.grpc.stub.StreamObserver<SceneResponse> responseObserver) {
        try {
            SceneDataRepository.SceneData data = repository.findById(request.getId());
            if (data == null) {
                responseObserver.onError(new RuntimeException("Scene not found: " + request.getId()));
                return;
            }

            SceneMetadata.Builder metadataBuilder = data.getMetadata().toBuilder();

            if (request.hasName()) {
                metadataBuilder.setName(request.getName());
            }
            if (request.hasDescription()) {
                metadataBuilder.setDescription(request.getDescription());
            }
            if (!request.getTagsList().isEmpty()) {
                metadataBuilder.clearTags().addAllTags(request.getTagsList());
            }
            if (request.hasStatus()) {
                metadataBuilder.setStatus(request.getStatus());
            }
            metadataBuilder.setUpdatedBy(request.getUpdatedBy());

            data.setMetadata(metadataBuilder.build());

            if (request.hasContent()) {
                data.setContent(request.getContent());
            }

            SceneResponse response = SceneResponse.newBuilder()
                    .setMetadata(data.getMetadata())
                    .setContent(data.getContent())
                    .setFormat(SceneFormat.PBTXT)
                    .build();

            LOG.infof("Updated scene: %s", request.getId());
            responseObserver.onNext(response);
            responseObserver.onCompleted();

        } catch (Exception e) {
            LOG.error("Failed to update scene", e);
            responseObserver.onError(e);
        }
    }

    public void deleteScene(DeleteSceneRequest request,
                            io.grpc.stub.StreamObserver<Empty> responseObserver) {
        try {
            boolean removed = repository.delete(request.getId());
            if (!removed) {
                responseObserver.onError(new RuntimeException("Scene not found: " + request.getId()));
                return;
            }

            LOG.infof("Deleted scene: %s", request.getId());
            responseObserver.onNext(Empty.getDefaultInstance());
            responseObserver.onCompleted();

        } catch (Exception e) {
            LOG.error("Failed to delete scene", e);
            responseObserver.onError(e);
        }
    }

    public void listScenes(ListScenesRequest request,
                           io.grpc.stub.StreamObserver<ListScenesResponse> responseObserver) {
        try {
            List<SceneMetadata> filteredScenes = new ArrayList<>();

            for (SceneDataRepository.SceneData data : repository.findAll()) {
                // Filter by project
                if (!data.getMetadata().getProjectId().equals(request.getProjectId())) {
                    continue;
                }

                // Filter by status
                if (request.hasStatus() && data.getMetadata().getStatus() != request.getStatus()) {
                    continue;
                }

                // Search filter
                if (request.getSearch() != null && !request.getSearch().isEmpty()) {
                    if (!data.getMetadata().getName().toLowerCase().contains(request.getSearch().toLowerCase())) {
                        continue;
                    }
                }

                filteredScenes.add(data.getMetadata());
            }

            // Pagination
            int page = request.getPage();
            int pageSize = request.getPageSize() > 0 ? request.getPageSize() : 20;
            int start = page * pageSize;
            int end = Math.min(start + pageSize, filteredScenes.size());

            List<SceneMetadata> pageScenes = filteredScenes.subList(
                    Math.min(start, filteredScenes.size()),
                    Math.min(end, filteredScenes.size())
            );

            ListScenesResponse response = ListScenesResponse.newBuilder()
                    .addAllScenes(pageScenes)
                    .setTotalCount(filteredScenes.size())
                    .setPage(page)
                    .setPageSize(pageSize)
                    .build();

            responseObserver.onNext(response);
            responseObserver.onCompleted();

        } catch (Exception e) {
            LOG.error("Failed to list scenes", e);
            responseObserver.onError(e);
        }
    }

    private String convertSceneFormat(String content, SceneFormat format) {
        // TODO: Implement actual format conversion
        // For now, just return as-is
        return content;
    }
}