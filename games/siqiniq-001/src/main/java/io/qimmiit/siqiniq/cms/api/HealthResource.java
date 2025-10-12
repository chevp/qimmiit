package io.qimmiit.siqiniq.cms.api;

import io.qimmiit.siqiniq.server.GameServer;
import io.qimmiit.siqiniq.server.network.GameWebSocket;
import jakarta.inject.Inject;
import jakarta.ws.rs.GET;
import jakarta.ws.rs.Path;
import jakarta.ws.rs.Produces;
import jakarta.ws.rs.core.MediaType;
import jakarta.ws.rs.core.Response;

import java.util.Map;

/**
 * Game-specific health and status endpoint
 */
@Path("/api/status")
@Produces(MediaType.APPLICATION_JSON)
public class HealthResource {

    @Inject
    GameServer gameServer;

    @Inject
    GameWebSocket gameWebSocket;

    @GET
    public Response getStatus() {
        return Response.ok(Map.of(
            "game", "siqiniq-001",
            "version", "1.0.0",
            "running", gameServer.isRunning(),
            "tick", gameServer.getCurrentTick(),
            "players", gameWebSocket.getPlayerCount()
        )).build();
    }
}
