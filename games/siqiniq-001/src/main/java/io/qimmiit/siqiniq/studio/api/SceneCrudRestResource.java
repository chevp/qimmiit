package io.qimmiit.siqiniq.studio.api;

import cryo.services.CryoSceneAuthoringServiceOuterClass.*;
import jakarta.ws.rs.*;
import jakarta.ws.rs.core.MediaType;
import jakarta.ws.rs.core.Response;
import org.eclipse.microprofile.openapi.annotations.Operation;
import org.eclipse.microprofile.openapi.annotations.tags.Tag;

import java.util.List;
import java.util.Map;

/**
 * REST API for Scene CRUD operations
 * Handles creating, reading, updating, deleting, and listing scenes
 */
@Path("/api/studio/scenes")
@Tag(name = "Scene CRUD", description = "Scene create, read, update, delete operations")
public class SceneCrudRestResource extends BaseSceneAuthoringResource {

    @POST
    @Operation(summary = "Create a new scene", description = "Creates a new authored scene")
    public Response createScene(Map<String, Object> request) {
        try {
            String name = (String) request.get("name");
            String description = (String) request.get("description");
            String projectId = (String) request.get("project_id");
            String createdBy = (String) request.getOrDefault("created_by", "studio");

            CreateSceneRequest.Builder reqBuilder = CreateSceneRequest.newBuilder()
                    .setName(name)
                    .setDescription(description)
                    .setProjectId(projectId)
                    .setCreatedBy(createdBy);

            if (request.containsKey("tags")) {
                @SuppressWarnings("unchecked")
                List<String> tags = (List<String>) request.get("tags");
                reqBuilder.addAllTags(tags);
            }

            if (request.containsKey("initial_content")) {
                reqBuilder.setInitialContent((String) request.get("initial_content"));
            }

            SceneResponse response = sceneService.createScene(reqBuilder.build());
            String json = jsonPrinter.print(response);

            return Response.status(Response.Status.CREATED)
                    .entity(json)
                    .build();

        } catch (Exception e) {
            LOG.error("Failed to create scene", e);
            return Response.status(Response.Status.BAD_REQUEST)
                    .entity(Map.of("error", e.getMessage()))
                    .build();
        }
    }

    @GET
    @Path("/{id}")
    @Operation(summary = "Get scene by ID", description = "Retrieves a full authored scene")
    public Response getScene(@PathParam("id") String id,
                             @QueryParam("format") @DefaultValue("PBTXT") String formatStr,
                             @QueryParam("includeEntities") @DefaultValue("true") boolean includeEntities) {
        try {
            SceneFormat format = SceneFormat.valueOf(formatStr);

            GetSceneRequest request = GetSceneRequest.newBuilder()
                    .setId(id)
                    .setFormat(format)
                    .setIncludeEntities(includeEntities)
                    .build();

            SceneResponse response = sceneService.getScene(request);
            String json = jsonPrinter.print(response);

            return Response.ok(json).build();

        } catch (Exception e) {
            LOG.error("Failed to get scene: " + id, e);
            return Response.status(Response.Status.NOT_FOUND)
                    .entity(Map.of("error", e.getMessage()))
                    .build();
        }
    }

    @PUT
    @Path("/{id}")
    @Operation(summary = "Update scene", description = "Updates an existing scene")
    public Response updateScene(@PathParam("id") String id, Map<String, Object> request) {
        try {
            UpdateSceneRequest.Builder reqBuilder = UpdateSceneRequest.newBuilder()
                    .setId(id);

            if (request.containsKey("name")) {
                reqBuilder.setName((String) request.get("name"));
            }
            if (request.containsKey("description")) {
                reqBuilder.setDescription((String) request.get("description"));
            }
            if (request.containsKey("content")) {
                reqBuilder.setContent((String) request.get("content"));
            }
            if (request.containsKey("updated_by")) {
                reqBuilder.setUpdatedBy((String) request.get("updated_by"));
            }
            if (request.containsKey("status")) {
                reqBuilder.setStatus(SceneStatus.valueOf((String) request.get("status")));
            }
            if (request.containsKey("tags")) {
                @SuppressWarnings("unchecked")
                List<String> tags = (List<String>) request.get("tags");
                reqBuilder.addAllTags(tags);
            }

            SceneResponse response = sceneService.updateScene(reqBuilder.build());
            String json = jsonPrinter.print(response);

            return Response.ok(json).build();

        } catch (Exception e) {
            LOG.error("Failed to update scene: " + id, e);
            return Response.status(Response.Status.BAD_REQUEST)
                    .entity(Map.of("error", e.getMessage()))
                    .build();
        }
    }

    @DELETE
    @Path("/{id}")
    @Operation(summary = "Delete scene", description = "Deletes a scene by ID")
    public Response deleteScene(@PathParam("id") String id) {
        try {
            DeleteSceneRequest request = DeleteSceneRequest.newBuilder()
                    .setId(id)
                    .build();

            sceneService.deleteScene(request);

            return Response.noContent().build();

        } catch (Exception e) {
            LOG.error("Failed to delete scene: " + id, e);
            return Response.status(Response.Status.NOT_FOUND)
                    .entity(Map.of("error", e.getMessage()))
                    .build();
        }
    }

    @GET
    @Operation(summary = "List scenes", description = "Lists scenes with pagination")
    public Response listScenes(@QueryParam("project_id") String projectId,
                                @QueryParam("page") @DefaultValue("0") int page,
                                @QueryParam("pageSize") @DefaultValue("20") int pageSize,
                                @QueryParam("search") String search,
                                @QueryParam("status") String statusStr) {
        try {
            ListScenesRequest.Builder reqBuilder = ListScenesRequest.newBuilder()
                    .setProjectId(projectId)
                    .setPage(page)
                    .setPageSize(pageSize);

            if (search != null && !search.isEmpty()) {
                reqBuilder.setSearch(search);
            }
            if (statusStr != null && !statusStr.isEmpty()) {
                reqBuilder.setStatus(SceneStatus.valueOf(statusStr));
            }

            ListScenesResponse response = sceneService.listScenes(reqBuilder.build());
            String json = jsonPrinter.print(response);

            return Response.ok(json).build();

        } catch (Exception e) {
            LOG.error("Failed to list scenes", e);
            return Response.status(Response.Status.INTERNAL_SERVER_ERROR)
                    .entity(Map.of("error", e.getMessage()))
                    .build();
        }
    }
}