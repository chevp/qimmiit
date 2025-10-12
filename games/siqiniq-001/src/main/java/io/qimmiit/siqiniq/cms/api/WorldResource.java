package io.qimmiit.siqiniq.cms.api;

import jakarta.ws.rs.*;
import jakarta.ws.rs.core.MediaType;
import jakarta.ws.rs.core.Response;
import org.eclipse.microprofile.openapi.annotations.Operation;
import org.eclipse.microprofile.openapi.annotations.tags.Tag;

import java.util.ArrayList;
import java.util.List;
import java.util.Map;

/**
 * REST API for world management
 */
@Path("/api/worlds")
@Produces(MediaType.APPLICATION_JSON)
@Consumes(MediaType.APPLICATION_JSON)
@Tag(name = "Worlds", description = "World management operations")
public class WorldResource {

    @GET
    @Operation(summary = "List all worlds")
    public Response listWorlds() {
        // TODO: Load worlds from content files
        List<Map<String, Object>> worlds = new ArrayList<>();

        worlds.add(Map.of(
            "id", "sunny_plains_001",
            "name", "Sunny Plains",
            "description", "A bright and cheerful grassland"
        ));

        return Response.ok(worlds).build();
    }

    @GET
    @Path("/{id}")
    @Operation(summary = "Get world by ID")
    public Response getWorld(@PathParam("id") String id) {
        // TODO: Load specific world
        return Response.ok(Map.of(
            "id", id,
            "name", "Sunny Plains",
            "regions", List.of()
        )).build();
    }

    @POST
    @Operation(summary = "Create new world")
    public Response createWorld(Map<String, Object> worldData) {
        // TODO: Validate and save world
        return Response.status(Response.Status.CREATED)
            .entity(Map.of("message", "World created successfully"))
            .build();
    }

    @PUT
    @Path("/{id}")
    @Operation(summary = "Update world")
    public Response updateWorld(@PathParam("id") String id, Map<String, Object> worldData) {
        // TODO: Update world
        return Response.ok(Map.of("message", "World updated successfully")).build();
    }

    @DELETE
    @Path("/{id}")
    @Operation(summary = "Delete world")
    public Response deleteWorld(@PathParam("id") String id) {
        // TODO: Delete world
        return Response.ok(Map.of("message", "World deleted successfully")).build();
    }
}
