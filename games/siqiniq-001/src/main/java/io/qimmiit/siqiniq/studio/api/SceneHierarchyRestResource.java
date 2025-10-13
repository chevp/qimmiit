package io.qimmiit.siqiniq.studio.api;

import cryo.services.CryoSceneAuthoringServiceOuterClass.*;
import jakarta.ws.rs.*;
import jakarta.ws.rs.core.Response;
import org.eclipse.microprofile.openapi.annotations.Operation;
import org.eclipse.microprofile.openapi.annotations.tags.Tag;

import java.util.Map;

/**
 * REST API for Scene Hierarchy operations
 * Handles viewing and managing the entity hierarchy tree
 */
@Path("/api/studio/scenes/{sceneId}/hierarchy")
@Tag(name = "Scene Hierarchy", description = "Operations for managing scene entity hierarchies")
public class SceneHierarchyRestResource extends BaseSceneAuthoringResource {

    @GET
    @Operation(summary = "Get scene hierarchy", description = "Gets the entity hierarchy tree")
    public Response getHierarchy(@PathParam("sceneId") String sceneId,
                                  @QueryParam("rootEntityId") String rootEntityId,
                                  @QueryParam("maxDepth") @DefaultValue("0") int maxDepth) {
        try {
            GetHierarchyRequest.Builder reqBuilder = GetHierarchyRequest.newBuilder()
                    .setSceneId(sceneId)
                    .setMaxDepth(maxDepth);

            if (rootEntityId != null) {
                reqBuilder.setRootEntityId(rootEntityId);
            }

            HierarchyResponse response = sceneService.getHierarchy(reqBuilder.build());
            String json = jsonPrinter.print(response);

            return Response.ok(json).build();

        } catch (Exception e) {
            LOG.error("Failed to get hierarchy for scene: " + sceneId, e);
            return Response.status(Response.Status.INTERNAL_SERVER_ERROR)
                    .entity(Map.of("error", e.getMessage()))
                    .build();
        }
    }
}