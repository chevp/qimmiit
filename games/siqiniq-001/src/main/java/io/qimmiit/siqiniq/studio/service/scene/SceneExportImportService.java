package io.qimmiit.siqiniq.studio.service.scene;

import com.google.protobuf.Empty;
import cryo.services.CryoSceneAuthoringServiceOuterClass.*;
import jakarta.enterprise.context.ApplicationScoped;
import jakarta.inject.Inject;
import org.jboss.logging.Logger;

import java.util.UUID;

/**
 * Service handling Scene export and import operations
 */
@ApplicationScoped
public class SceneExportImportService {

    private static final Logger LOG = Logger.getLogger(SceneExportImportService.class);

    @Inject
    SceneDataRepository repository;

    public void exportScene(ExportSceneRequest request,
                            io.grpc.stub.StreamObserver<ExportSceneResponse> responseObserver) {
        try {
            SceneDataRepository.SceneData scene = repository.findById(request.getSceneId());
            if (scene == null) {
                responseObserver.onError(new RuntimeException("Scene not found: " + request.getSceneId()));
                return;
            }

            String content = convertSceneFormat(scene.getContent(), request.getFormat());
            byte[] data = content.getBytes();

            ExportSceneResponse response = ExportSceneResponse.newBuilder()
                    .setData(com.google.protobuf.ByteString.copyFrom(data))
                    .setFormat(request.getFormat())
                    .setSizeBytes(data.length)
                    .setChecksum(calculateChecksum(data))
                    .build();

            responseObserver.onNext(response);
            responseObserver.onCompleted();

        } catch (Exception e) {
            LOG.error("Failed to export scene", e);
            responseObserver.onError(e);
        }
    }

    public void importScene(ImportSceneRequest request,
                            io.grpc.stub.StreamObserver<SceneResponse> responseObserver) {
        try {
            String id = UUID.randomUUID().toString();
            String content = new String(request.getData().toByteArray());

            SceneMetadata metadata = SceneMetadata.newBuilder()
                    .setId(id)
                    .setName(request.getName())
                    .setProjectId(request.getProjectId())
                    .setCreatedBy(request.getCreatedBy())
                    .setStatus(SceneStatus.DRAFT)
                    .setEntityCount(0)
                    .build();

            SceneDataRepository.SceneData data = new SceneDataRepository.SceneData();
            data.setMetadata(metadata);
            data.setContent(content);

            repository.save(id, data);

            SceneResponse response = SceneResponse.newBuilder()
                    .setMetadata(metadata)
                    .setContent(content)
                    .setFormat(request.getFormat())
                    .build();

            LOG.infof("Imported scene: %s (%s)", metadata.getName(), id);
            responseObserver.onNext(response);
            responseObserver.onCompleted();

        } catch (Exception e) {
            LOG.error("Failed to import scene", e);
            responseObserver.onError(e);
        }
    }

    // Streaming operations placeholder
    public io.grpc.stub.StreamObserver<SceneUpdate> applySceneUpdate(
            io.grpc.stub.StreamObserver<Empty> responseObserver) {
        // TODO: Implement real-time collaboration
        throw new UnsupportedOperationException("Not implemented yet");
    }

    public void streamSceneUpdates(StreamSceneUpdatesRequest request,
                                    io.grpc.stub.StreamObserver<SceneUpdate> responseObserver) {
        // TODO: Implement real-time collaboration streaming
        throw new UnsupportedOperationException("Not implemented yet");
    }

    private String convertSceneFormat(String content, SceneFormat format) {
        // TODO: Implement actual format conversion
        // For now, just return as-is
        return content;
    }

    private String calculateChecksum(byte[] data) {
        // TODO: Implement actual SHA256 checksum
        return "checksum-placeholder";
    }
}