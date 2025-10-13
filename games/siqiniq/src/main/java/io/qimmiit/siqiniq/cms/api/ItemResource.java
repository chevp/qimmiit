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
 * REST API for item management
 */
@Path("/api/items")
@Produces(MediaType.APPLICATION_JSON)
@Consumes(MediaType.APPLICATION_JSON)
@Tag(name = "Items", description = "Item management operations")
public class ItemResource {

    @GET
    @Operation(summary = "List all items")
    public Response listItems(@QueryParam("type") String type) {
        // TODO: Load items from content files
        List<Map<String, Object>> items = new ArrayList<>();

        items.add(Map.of(
            "id", "sun_blade",
            "name", "Sun Blade",
            "type", "WEAPON",
            "value", 250
        ));

        return Response.ok(items).build();
    }

    @GET
    @Path("/{id}")
    @Operation(summary = "Get item by ID")
    public Response getItem(@PathParam("id") String id) {
        // TODO: Load specific item
        return Response.ok(Map.of(
            "id", id,
            "name", "Sun Blade",
            "type", "WEAPON"
        )).build();
    }

    @POST
    @Operation(summary = "Create new item")
    public Response createItem(Map<String, Object> itemData) {
        // TODO: Validate and save item
        return Response.status(Response.Status.CREATED)
            .entity(Map.of("message", "Item created successfully"))
            .build();
    }
}
