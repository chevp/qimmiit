package io.qimmiit.siqiniq.studio.api;

import cryo.services.CryoSceneAuthoringServiceOuterClass.*;
import jakarta.ws.rs.*;
import jakarta.ws.rs.core.Response;
import org.eclipse.microprofile.openapi.annotations.Operation;
import org.eclipse.microprofile.openapi.annotations.tags.Tag;

import java.util.Map;

/**
 * REST API for Entity operations
 * Handles adding, updating, removing, and listing entities in scenes
 */
@Path("/api/studio/scenes/{sceneId}/entities")
@Tag(name = "Entity Operations", description = "Operations for managing entities in scenes")
public class EntityOperationsRestResource extends BaseSceneAuthoringResource {

    @POST
    @Operation(summary = "Add entity to scene", description = "Adds a new entity to the scene")
    public Response addEntity(@PathParam("sceneId") String sceneId, Map<String, Object> request) {
        try {
            String entityContent = (String) request.get("entity_content");
            String parentId = (String) request.get("parent_id");

            AddEntityRequest.Builder reqBuilder = AddEntityRequest.newBuilder()
                    .setSceneId(sceneId)
                    .setEntityContent(entityContent);

            if (parentId != null) {
                reqBuilder.setParentId(parentId);
            }

            EntityResponse response = sceneService.addEntity(reqBuilder.build());
            String json = jsonPrinter.print(response);

            return Response.status(Response.Status.CREATED)
                    .entity(json)
                    .build();

        } catch (Exception e) {
            LOG.error("Failed to add entity to scene: " + sceneId, e);
            return Response.status(Response.Status.BAD_REQUEST)
                    .entity(Map.of("error", e.getMessage()))
                    .build();
        }
    }

    @PUT
    @Path("/{entityId}")
    @Operation(summary = "Update entity", description = "Updates an entity in the scene")
    public Response updateEntity(@PathParam("sceneId") String sceneId,
                                  @PathParam("entityId") String entityId,
                                  Map<String, Object> request) {
        try {
            String entityContent = (String) request.get("entity_content");

            UpdateEntityRequest updateRequest = UpdateEntityRequest.newBuilder()
                    .setSceneId(sceneId)
                    .setEntityId(entityId)
                    .setEntityContent(entityContent)
                    .build();

            EntityResponse response = sceneService.updateEntity(updateRequest);
            String json = jsonPrinter.print(response);

            return Response.ok(json).build();

        } catch (Exception e) {
            LOG.error("Failed to update entity: " + entityId, e);
            return Response.status(Response.Status.BAD_REQUEST)
                    .entity(Map.of("error", e.getMessage()))
                    .build();
        }
    }

    @DELETE
    @Path("/{entityId}")
    @Operation(summary = "Remove entity", description = "Removes an entity from the scene")
    public Response removeEntity(@PathParam("sceneId") String sceneId,
                                  @PathParam("entityId") String entityId,
                                  @QueryParam("removeChildren") @DefaultValue("false") boolean removeChildren) {
        try {
            RemoveEntityRequest request = RemoveEntityRequest.newBuilder()
                    .setSceneId(sceneId)
                    .setEntityId(entityId)
                    .setRemoveChildren(removeChildren)
                    .build();

            sceneService.removeEntity(request);

            return Response.noContent().build();

        } catch (Exception e) {
            LOG.error("Failed to remove entity: " + entityId, e);
            return Response.status(Response.Status.NOT_FOUND)
                    .entity(Map.of("error", e.getMessage()))
                    .build();
        }
    }

    @GET
    @Operation(summary = "List entities", description = "Lists entities in the scene")
    public Response listEntities(@PathParam("sceneId") String sceneId,
                                  @QueryParam("parentId") String parentId,
                                  @QueryParam("page") @DefaultValue("0") int page,
                                  @QueryParam("pageSize") @DefaultValue("50") int pageSize) {
        try {
            ListEntitiesRequest.Builder reqBuilder = ListEntitiesRequest.newBuilder()
                    .setSceneId(sceneId)
                    .setPage(page)
                    .setPageSize(pageSize);

            if (parentId != null) {
                reqBuilder.setParentId(parentId);
            }

            ListEntitiesResponse response = sceneService.listEntities(reqBuilder.build());
            String json = jsonPrinter.print(response);

            return Response.ok(json).build();

        } catch (Exception e) {
            LOG.error("Failed to list entities for scene: " + sceneId, e);
            return Response.status(Response.Status.INTERNAL_SERVER_ERROR)
                    .entity(Map.of("error", e.getMessage()))
                    .build();
        }
    }
}