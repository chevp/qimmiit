package io.qimmiit.siqiniq.studio.api;

import com.google.protobuf.util.JsonFormat;
import cryo.services.CryoTemplateServiceOuterClass.*;
import io.qimmiit.siqiniq.studio.service.CryoTemplateService;
import jakarta.inject.Inject;
import jakarta.ws.rs.*;
import jakarta.ws.rs.core.MediaType;
import jakarta.ws.rs.core.Response;
import org.eclipse.microprofile.openapi.annotations.Operation;
import org.eclipse.microprofile.openapi.annotations.tags.Tag;
import org.jboss.logging.Logger;

import java.util.Map;

/**
 * REST API for Cryo Protocol Template CRUD operations
 * Provides HTTP 1.1 endpoints for studio development tools
 */
@Path("/api/studio/templates")
@Produces(MediaType.APPLICATION_JSON)
@Consumes(MediaType.APPLICATION_JSON)
@Tag(name = "Cryo Templates", description = "CRUD operations for Cryo Protocol templates")
public class CryoTemplateRestResource {

    private static final Logger LOG = Logger.getLogger(CryoTemplateRestResource.class);

    @Inject
    CryoTemplateService templateService;

    private final JsonFormat.Printer jsonPrinter = JsonFormat.printer()
            .includingDefaultValueFields()
            .preservingProtoFieldNames();

    private final JsonFormat.Parser jsonParser = JsonFormat.parser()
            .ignoringUnknownFields();

    // ========================================================================
    // CREATE
    // ========================================================================

    @POST
    @Operation(summary = "Create a new template", description = "Creates a new Cryo Protocol template")
    public Response createTemplate(Map<String, Object> request) {
        try {
            String name = (String) request.get("name");
            String description = (String) request.get("description");
            String typeStr = (String) request.get("type");
            String content = (String) request.get("content");
            String createdBy = (String) request.getOrDefault("created_by", "studio");

            TemplateType type = TemplateType.valueOf(typeStr);

            CreateTemplateRequest.Builder reqBuilder = CreateTemplateRequest.newBuilder()
                    .setName(name)
                    .setDescription(description)
                    .setType(type)
                    .setContent(content)
                    .setCreatedBy(createdBy);

            @SuppressWarnings("unchecked")
            Map<String, String> tags = (Map<String, String>) request.get("tags");
            if (tags != null) {
                reqBuilder.putAllTags(tags);
            }

            TemplateResponse response = templateService.createTemplate(reqBuilder.build());
            String json = jsonPrinter.print(response);

            return Response.status(Response.Status.CREATED)
                    .entity(json)
                    .build();

        } catch (Exception e) {
            LOG.error("Failed to create template", e);
            return Response.status(Response.Status.BAD_REQUEST)
                    .entity(Map.of("error", e.getMessage()))
                    .build();
        }
    }

    // ========================================================================
    // READ
    // ========================================================================

    @GET
    @Path("/{id}")
    @Operation(summary = "Get template by ID", description = "Retrieves a template by its ID")
    public Response getTemplate(@PathParam("id") String id,
                                 @QueryParam("includeContent") @DefaultValue("true") boolean includeContent) {
        try {
            GetTemplateRequest request = GetTemplateRequest.newBuilder()
                    .setId(id)
                    .setIncludeContent(includeContent)
                    .build();

            TemplateResponse response = templateService.getTemplate(request);
            String json = jsonPrinter.print(response);

            return Response.ok(json).build();

        } catch (Exception e) {
            LOG.error("Failed to get template: " + id, e);
            return Response.status(Response.Status.NOT_FOUND)
                    .entity(Map.of("error", e.getMessage()))
                    .build();
        }
    }

    @GET
    @Operation(summary = "List all templates", description = "Lists templates with pagination")
    public Response listTemplates(@QueryParam("page") @DefaultValue("0") int page,
                                   @QueryParam("pageSize") @DefaultValue("20") int pageSize,
                                   @QueryParam("search") String search) {
        try {
            ListTemplatesRequest.Builder reqBuilder = ListTemplatesRequest.newBuilder()
                    .setPage(page)
                    .setPageSize(pageSize);

            if (search != null && !search.isEmpty()) {
                reqBuilder.setSearch(search);
            }

            ListTemplatesResponse response = templateService.listTemplates(reqBuilder.build());
            String json = jsonPrinter.print(response);

            return Response.ok(json).build();

        } catch (Exception e) {
            LOG.error("Failed to list templates", e);
            return Response.status(Response.Status.INTERNAL_SERVER_ERROR)
                    .entity(Map.of("error", e.getMessage()))
                    .build();
        }
    }

    @GET
    @Path("/by-type/{type}")
    @Operation(summary = "List templates by type", description = "Lists templates filtered by type")
    public Response listTemplatesByType(@PathParam("type") String typeStr,
                                         @QueryParam("page") @DefaultValue("0") int page,
                                         @QueryParam("pageSize") @DefaultValue("20") int pageSize) {
        try {
            TemplateType type = TemplateType.valueOf(typeStr);

            ListTemplatesByTypeRequest request = ListTemplatesByTypeRequest.newBuilder()
                    .setType(type)
                    .setPage(page)
                    .setPageSize(pageSize)
                    .build();

            ListTemplatesResponse response = templateService.listTemplatesByType(request);
            String json = jsonPrinter.print(response);

            return Response.ok(json).build();

        } catch (Exception e) {
            LOG.error("Failed to list templates by type: " + typeStr, e);
            return Response.status(Response.Status.BAD_REQUEST)
                    .entity(Map.of("error", e.getMessage()))
                    .build();
        }
    }

    // ========================================================================
    // UPDATE
    // ========================================================================

    @PUT
    @Path("/{id}")
    @Operation(summary = "Update template", description = "Updates an existing template")
    public Response updateTemplate(@PathParam("id") String id, Map<String, Object> request) {
        try {
            UpdateTemplateRequest.Builder reqBuilder = UpdateTemplateRequest.newBuilder()
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

            @SuppressWarnings("unchecked")
            Map<String, String> tags = (Map<String, String>) request.get("tags");
            if (tags != null) {
                reqBuilder.putAllTags(tags);
            }

            TemplateResponse response = templateService.updateTemplate(reqBuilder.build());
            String json = jsonPrinter.print(response);

            return Response.ok(json).build();

        } catch (Exception e) {
            LOG.error("Failed to update template: " + id, e);
            return Response.status(Response.Status.BAD_REQUEST)
                    .entity(Map.of("error", e.getMessage()))
                    .build();
        }
    }

    // ========================================================================
    // DELETE
    // ========================================================================

    @DELETE
    @Path("/{id}")
    @Operation(summary = "Delete template", description = "Deletes a template by ID")
    public Response deleteTemplate(@PathParam("id") String id) {
        try {
            DeleteTemplateRequest request = DeleteTemplateRequest.newBuilder()
                    .setId(id)
                    .build();

            templateService.deleteTemplate(request);

            return Response.noContent().build();

        } catch (Exception e) {
            LOG.error("Failed to delete template: " + id, e);
            return Response.status(Response.Status.NOT_FOUND)
                    .entity(Map.of("error", e.getMessage()))
                    .build();
        }
    }

    // ========================================================================
    // SPECIAL OPERATIONS
    // ========================================================================

    @POST
    @Path("/{id}/load")
    @Operation(summary = "Load template as runtime", description = "Loads a template into a runtime instance")
    public Response loadTemplateAsRuntime(@PathParam("id") String id, Map<String, Object> request) {
        try {
            LoadTemplateRequest.Builder reqBuilder = LoadTemplateRequest.newBuilder()
                    .setTemplateId(id);

            @SuppressWarnings("unchecked")
            Map<String, String> overrides = (Map<String, String>) request.get("overrides");
            if (overrides != null) {
                reqBuilder.putAllOverrides(overrides);
            }

            LoadTemplateResponse response = templateService.loadTemplateAsRuntime(reqBuilder.build());
            String json = jsonPrinter.print(response);

            return Response.ok(json).build();

        } catch (Exception e) {
            LOG.error("Failed to load template as runtime: " + id, e);
            return Response.status(Response.Status.INTERNAL_SERVER_ERROR)
                    .entity(Map.of("error", e.getMessage()))
                    .build();
        }
    }

    @POST
    @Path("/validate")
    @Operation(summary = "Validate template", description = "Validates template syntax without saving")
    public Response validateTemplate(Map<String, Object> request) {
        try {
            String content = (String) request.get("content");
            String typeStr = (String) request.get("type");
            TemplateType type = TemplateType.valueOf(typeStr);

            ValidateTemplateRequest validationRequest = ValidateTemplateRequest.newBuilder()
                    .setContent(content)
                    .setType(type)
                    .build();

            ValidateTemplateResponse response = templateService.validateTemplate(validationRequest);
            String json = jsonPrinter.print(response);

            return Response.ok(json).build();

        } catch (Exception e) {
            LOG.error("Failed to validate template", e);
            return Response.status(Response.Status.BAD_REQUEST)
                    .entity(Map.of("error", e.getMessage()))
                    .build();
        }
    }
}
