package io.qimmiit.siqiniq.studio.service.scene;

import cryo.services.CryoSceneAuthoringServiceOuterClass.*;
import jakarta.enterprise.context.ApplicationScoped;
import jakarta.inject.Inject;
import org.jboss.logging.Logger;

/**
 * Service handling Scene hierarchy operations
 */
@ApplicationScoped
public class SceneHierarchyService {

    private static final Logger LOG = Logger.getLogger(SceneHierarchyService.class);

    @Inject
    SceneDataRepository repository;

    public void getHierarchy(GetHierarchyRequest request,
                             io.grpc.stub.StreamObserver<HierarchyResponse> responseObserver) {
        try {
            SceneDataRepository.SceneData scene = repository.findById(request.getSceneId());
            if (scene == null) {
                responseObserver.onError(new RuntimeException("Scene not found: " + request.getSceneId()));
                return;
            }

            // TODO: Build actual hierarchy tree from entities
            HierarchyResponse response = HierarchyResponse.newBuilder()
                    .build();

            responseObserver.onNext(response);
            responseObserver.onCompleted();

        } catch (Exception e) {
            LOG.error("Failed to get hierarchy", e);
            responseObserver.onError(e);
        }
    }

    public void updateHierarchy(UpdateHierarchyRequest request,
                                io.grpc.stub.StreamObserver<HierarchyResponse> responseObserver) {
        // TODO: Implement hierarchy update logic
        responseObserver.onError(new RuntimeException("Not implemented yet"));
    }
}