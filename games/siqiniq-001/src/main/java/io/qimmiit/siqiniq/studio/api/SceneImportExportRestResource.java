package io.qimmiit.siqiniq.studio.api;

import cryo.services.CryoSceneAuthoringServiceOuterClass.*;
import jakarta.ws.rs.*;
import jakarta.ws.rs.core.MediaType;
import jakarta.ws.rs.core.Response;
import org.eclipse.microprofile.openapi.annotations.Operation;
import org.eclipse.microprofile.openapi.annotations.tags.Tag;

import java.util.Map;

/**
 * REST API for Scene Import/Export operations
 * Handles exporting scenes to various formats and importing scenes from files
 */
@Path("/api/studio/scenes")
@Tag(name = "Scene Import/Export", description = "Operations for importing and exporting scenes")
public class SceneImportExportRestResource extends BaseSceneAuthoringResource {

    @POST
    @Path("/{sceneId}/export")
    @Operation(summary = "Export scene", description = "Exports scene in specified format")
    @Produces(MediaType.APPLICATION_OCTET_STREAM)
    public Response exportScene(@PathParam("sceneId") String sceneId, Map<String, Object> request) {
        try {
            String formatStr = (String) request.getOrDefault("format", "PBTXT");
            SceneFormat format = SceneFormat.valueOf(formatStr);

            ExportOptions.Builder optionsBuilder = ExportOptions.newBuilder();
            if (request.containsKey("options")) {
                @SuppressWarnings("unchecked")
                Map<String, Boolean> options = (Map<String, Boolean>) request.get("options");
                optionsBuilder.setIncludeAssets(options.getOrDefault("include_assets", true));
                optionsBuilder.setIncludeMetadata(options.getOrDefault("include_metadata", true));
                optionsBuilder.setPrettyPrint(options.getOrDefault("pretty_print", true));
            }

            ExportSceneRequest exportRequest = ExportSceneRequest.newBuilder()
                    .setSceneId(sceneId)
                    .setFormat(format)
                    .setOptions(optionsBuilder.build())
                    .build();

            ExportSceneResponse response = sceneService.exportScene(exportRequest);

            return Response.ok(response.getData().toByteArray())
                    .header("Content-Disposition", "attachment; filename=\"scene_" + sceneId + getExtension(format) + "\"")
                    .header("X-Checksum", response.getChecksum())
                    .build();

        } catch (Exception e) {
            LOG.error("Failed to export scene: " + sceneId, e);
            return Response.status(Response.Status.INTERNAL_SERVER_ERROR)
                    .entity(Map.of("error", e.getMessage()))
                    .type(MediaType.APPLICATION_JSON)
                    .build();
        }
    }

    @POST
    @Path("/import")
    @Operation(summary = "Import scene", description = "Imports a scene from file")
    @Consumes(MediaType.APPLICATION_OCTET_STREAM)
    public Response importScene(@QueryParam("name") String name,
                                 @QueryParam("projectId") String projectId,
                                 @QueryParam("format") @DefaultValue("PBTXT") String formatStr,
                                 @QueryParam("createdBy") @DefaultValue("studio") String createdBy,
                                 byte[] data) {
        try {
            SceneFormat format = SceneFormat.valueOf(formatStr);

            ImportSceneRequest request = ImportSceneRequest.newBuilder()
                    .setName(name)
                    .setProjectId(projectId)
                    .setData(com.google.protobuf.ByteString.copyFrom(data))
                    .setFormat(format)
                    .setCreatedBy(createdBy)
                    .build();

            SceneResponse response = sceneService.importScene(request);
            String json = jsonPrinter.print(response);

            return Response.status(Response.Status.CREATED)
                    .entity(json)
                    .type(MediaType.APPLICATION_JSON)
                    .build();

        } catch (Exception e) {
            LOG.error("Failed to import scene", e);
            return Response.status(Response.Status.BAD_REQUEST)
                    .entity(Map.of("error", e.getMessage()))
                    .type(MediaType.APPLICATION_JSON)
                    .build();
        }
    }
}