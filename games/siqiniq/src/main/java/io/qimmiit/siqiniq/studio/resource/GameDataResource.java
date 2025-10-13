package io.qimmiit.siqiniq.studio.resource;

import jakarta.ws.rs.*;
import jakarta.ws.rs.core.MediaType;
import jakarta.ws.rs.core.Response;
import org.eclipse.microprofile.openapi.annotations.Operation;
import org.eclipse.microprofile.openapi.annotations.tags.Tag;
import org.jboss.logging.Logger;

import java.io.IOException;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.List;
import java.util.stream.Stream;

/**
 * REST endpoint for serving game data files (templates, worlds, items, frames)
 *
 * All files are served from: /game-data/{category}/{path}
 *
 * Examples:
 * - GET /game-data/templates/npc_merchant_template.pbtxt
 * - GET /game-data/worlds/sunny_plains.pbtxt
 * - GET /game-data/items/weapons/swords.pbtxt
 * - GET /game-data/frames/game_server_spawn_with_overrides.pbtxt
 */
@Path("/game-data")
@Tag(name = "Game Data", description = "Access game templates, worlds, items, and frame examples")
public class GameDataResource {

    private static final Logger LOG = Logger.getLogger(GameDataResource.class);
    private static final String BASE_PATH = "/META-INF/resources/game-data";

    @GET
    @Path("/{category}/{filename}")
    @Produces(MediaType.TEXT_PLAIN)
    @Operation(summary = "Get a game data file", description = "Retrieve a specific game data file by category and filename")
    public Response getFile(
            @PathParam("category") String category,
            @PathParam("filename") String filename) {

        String resourcePath = BASE_PATH + "/" + category + "/" + filename;
        LOG.infof("Fetching game data: %s", resourcePath);

        try (InputStream is = getClass().getResourceAsStream(resourcePath)) {
            if (is == null) {
                LOG.warnf("File not found: %s", resourcePath);
                return Response.status(Response.Status.NOT_FOUND)
                        .entity("File not found: " + category + "/" + filename)
                        .build();
            }

            String content = new String(is.readAllBytes(), StandardCharsets.UTF_8);
            return Response.ok(content)
                    .header("Content-Type", getContentType(filename))
                    .build();

        } catch (IOException e) {
            LOG.errorf(e, "Error reading file: %s", resourcePath);
            return Response.status(Response.Status.INTERNAL_SERVER_ERROR)
                    .entity("Error reading file: " + e.getMessage())
                    .build();
        }
    }

    @GET
    @Path("/{category}/{subpath}/{filename}")
    @Produces(MediaType.TEXT_PLAIN)
    @Operation(summary = "Get a game data file with subpath", description = "Retrieve a game data file with nested path (e.g., items/weapons/swords.pbtxt)")
    public Response getFileWithSubpath(
            @PathParam("category") String category,
            @PathParam("subpath") String subpath,
            @PathParam("filename") String filename) {

        String resourcePath = BASE_PATH + "/" + category + "/" + subpath + "/" + filename;
        LOG.infof("Fetching game data: %s", resourcePath);

        try (InputStream is = getClass().getResourceAsStream(resourcePath)) {
            if (is == null) {
                LOG.warnf("File not found: %s", resourcePath);
                return Response.status(Response.Status.NOT_FOUND)
                        .entity("File not found: " + category + "/" + subpath + "/" + filename)
                        .build();
            }

            String content = new String(is.readAllBytes(), StandardCharsets.UTF_8);
            return Response.ok(content)
                    .header("Content-Type", getContentType(filename))
                    .build();

        } catch (IOException e) {
            LOG.errorf(e, "Error reading file: %s", resourcePath);
            return Response.status(Response.Status.INTERNAL_SERVER_ERROR)
                    .entity("Error reading file: " + e.getMessage())
                    .build();
        }
    }

    @GET
    @Path("/{category}")
    @Produces(MediaType.APPLICATION_JSON)
    @Operation(summary = "List files in a category", description = "List all available files in a specific category")
    public Response listFiles(@PathParam("category") String category) {
        LOG.infof("Listing files in category: %s", category);

        try {
            List<String> files = new ArrayList<>();
            String resourcePath = BASE_PATH + "/" + category;

            // For packaged resources, we'll need to list them differently
            // This is a simplified version - you may want to enhance this
            String[] knownFiles = getKnownFiles(category);
            for (String file : knownFiles) {
                files.add(file);
            }

            return Response.ok(files).build();

        } catch (Exception e) {
            LOG.errorf(e, "Error listing files in category: %s", category);
            return Response.status(Response.Status.INTERNAL_SERVER_ERROR)
                    .entity("Error listing files: " + e.getMessage())
                    .build();
        }
    }

    private String getContentType(String filename) {
        if (filename.endsWith(".pbtxt")) {
            return "text/plain; charset=utf-8";
        } else if (filename.endsWith(".proto")) {
            return "text/plain; charset=utf-8";
        } else if (filename.endsWith(".json")) {
            return "application/json";
        } else if (filename.endsWith(".md")) {
            return "text/markdown; charset=utf-8";
        }
        return "text/plain; charset=utf-8";
    }

    private String[] getKnownFiles(String category) {
        // This is a static list - ideally you'd scan the classpath resources
        return switch (category) {
            case "templates" -> new String[]{
                "destructible_crate_template.pbtxt",
                "door_standard_template.pbtxt",
                "lever_switch_template.pbtxt",
                "npc_merchant_template.pbtxt",
                "pickup_item_template.pbtxt",
                "pressure_plate_template.pbtxt",
                "proximity_sensor_template.pbtxt",
                "teleporter_template.pbtxt",
                "torch_light_template.pbtxt",
                "treasure_chest_template.pbtxt"
            };
            case "frames" -> new String[]{
                "game_server_minimal_delta.pbtxt",
                "game_server_minimal_spawn.pbtxt",
                "game_server_spawn_with_overrides.pbtxt"
            };
            case "worlds" -> new String[]{
                "sunny_plains.pbtxt"
            };
            case "items" -> new String[]{
                "weapons/swords.pbtxt"
            };
            default -> new String[]{};
        };
    }
}
