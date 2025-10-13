package io.qimmiit.siqiniq.studio.service;

import com.google.protobuf.Empty;
import cryo.services.CryoSceneAuthoringServiceOuterClass.*;
import io.qimmiit.siqiniq.studio.service.scene.*;
import io.quarkus.grpc.GrpcService;
import jakarta.enterprise.context.ApplicationScoped;
import jakarta.inject.Inject;
import org.jboss.logging.Logger;

/**
 * Cryo Scene Authoring Service Implementation
 * Handles full scene manipulation for studio tools
 *
 * This is a facade service that delegates to specialized service classes
 */
@ApplicationScoped
@GrpcService
public class CryoSceneAuthoringService implements cryo.services.CryoSceneAuthoringServiceGrpc.CryoSceneAuthoringServiceImplBase {

    private static final Logger LOG = Logger.getLogger(CryoSceneAuthoringService.class);

    @Inject
    SceneCrudService sceneCrudService;

    @Inject
    EntityOperationsService entityOperationsService;

    @Inject
    SceneHierarchyService sceneHierarchyService;

    @Inject
    SceneExportImportService sceneExportImportService;

    // ========================================================================
    // SCENE CRUD - Delegated to SceneCrudService
    // ========================================================================

    @Override
    public void createScene(CreateSceneRequest request,
                            io.grpc.stub.StreamObserver<SceneResponse> responseObserver) {
        sceneCrudService.createScene(request, responseObserver);
    }

    @Override
    public void getScene(GetSceneRequest request,
                         io.grpc.stub.StreamObserver<SceneResponse> responseObserver) {
        sceneCrudService.getScene(request, responseObserver);
    }

    @Override
    public void updateScene(UpdateSceneRequest request,
                            io.grpc.stub.StreamObserver<SceneResponse> responseObserver) {
        sceneCrudService.updateScene(request, responseObserver);
    }

    @Override
    public void deleteScene(DeleteSceneRequest request,
                            io.grpc.stub.StreamObserver<Empty> responseObserver) {
        sceneCrudService.deleteScene(request, responseObserver);
    }

    @Override
    public void listScenes(ListScenesRequest request,
                           io.grpc.stub.StreamObserver<ListScenesResponse> responseObserver) {
        sceneCrudService.listScenes(request, responseObserver);
    }

    // ========================================================================
    // ENTITY OPERATIONS - Delegated to EntityOperationsService
    // ========================================================================

    @Override
    public void addEntity(AddEntityRequest request,
                          io.grpc.stub.StreamObserver<EntityResponse> responseObserver) {
        entityOperationsService.addEntity(request, responseObserver);
    }

    @Override
    public void updateEntity(UpdateEntityRequest request,
                             io.grpc.stub.StreamObserver<EntityResponse> responseObserver) {
        entityOperationsService.updateEntity(request, responseObserver);
    }

    @Override
    public void removeEntity(RemoveEntityRequest request,
                             io.grpc.stub.StreamObserver<Empty> responseObserver) {
        entityOperationsService.removeEntity(request, responseObserver);
    }

    @Override
    public void listEntities(ListEntitiesRequest request,
                             io.grpc.stub.StreamObserver<ListEntitiesResponse> responseObserver) {
        entityOperationsService.listEntities(request, responseObserver);
    }

    // ========================================================================
    // HIERARCHY OPERATIONS - Delegated to SceneHierarchyService
    // ========================================================================

    @Override
    public void getHierarchy(GetHierarchyRequest request,
                             io.grpc.stub.StreamObserver<HierarchyResponse> responseObserver) {
        sceneHierarchyService.getHierarchy(request, responseObserver);
    }

    @Override
    public void updateHierarchy(UpdateHierarchyRequest request,
                                io.grpc.stub.StreamObserver<HierarchyResponse> responseObserver) {
        sceneHierarchyService.updateHierarchy(request, responseObserver);
    }

    // ========================================================================
    // EXPORT/IMPORT - Delegated to SceneExportImportService
    // ========================================================================

    @Override
    public void exportScene(ExportSceneRequest request,
                            io.grpc.stub.StreamObserver<ExportSceneResponse> responseObserver) {
        sceneExportImportService.exportScene(request, responseObserver);
    }

    @Override
    public void importScene(ImportSceneRequest request,
                            io.grpc.stub.StreamObserver<SceneResponse> responseObserver) {
        sceneExportImportService.importScene(request, responseObserver);
    }

    // ========================================================================
    // STREAMING - Delegated to SceneExportImportService
    // ========================================================================

    @Override
    public io.grpc.stub.StreamObserver<SceneUpdate> applySceneUpdate(
            io.grpc.stub.StreamObserver<Empty> responseObserver) {
        return sceneExportImportService.applySceneUpdate(responseObserver);
    }

    @Override
    public void streamSceneUpdates(StreamSceneUpdatesRequest request,
                                    io.grpc.stub.StreamObserver<SceneUpdate> responseObserver) {
        sceneExportImportService.streamSceneUpdates(request, responseObserver);
    }
}
