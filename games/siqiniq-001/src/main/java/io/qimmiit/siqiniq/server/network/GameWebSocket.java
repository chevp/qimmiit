package io.qimmiit.siqiniq.server.network;

import io.quarkus.logging.Log;
import io.qimmiit.siqiniq.proto.ClientPacket;
import io.qimmiit.siqiniq.proto.ServerPacket;
import io.qimmiit.siqiniq.proto.ConnectResponse;
import jakarta.enterprise.context.ApplicationScoped;
import jakarta.websocket.*;
import jakarta.websocket.server.ServerEndpoint;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;
import java.util.UUID;

/**
 * WebSocket endpoint for game multiplayer connections
 */
@ServerEndpoint("/game/ws")
@ApplicationScoped
public class GameWebSocket {

    // Active player sessions
    private static final Map<String, Session> sessions = new ConcurrentHashMap<>();
    private static final Map<Session, String> sessionToPlayer = new ConcurrentHashMap<>();

    @OnOpen
    public void onOpen(Session session) {
        String playerId = UUID.randomUUID().toString();
        sessions.put(playerId, session);
        sessionToPlayer.put(session, playerId);

        Log.info("Player connected: " + playerId + " | Total players: " + sessions.size());
    }

    @OnClose
    public void onClose(Session session) {
        String playerId = sessionToPlayer.remove(session);
        if (playerId != null) {
            sessions.remove(playerId);
            Log.info("Player disconnected: " + playerId + " | Total players: " + sessions.size());
        }
    }

    @OnError
    public void onError(Session session, Throwable throwable) {
        String playerId = sessionToPlayer.get(session);
        Log.error("WebSocket error for player: " + playerId, throwable);
    }

    @OnMessage
    public void onMessage(ByteBuffer message, Session session) {
        String playerId = sessionToPlayer.get(session);

        try {
            // Parse client packet
            ClientPacket clientPacket = ClientPacket.parseFrom(message.array());

            // Handle packet
            handlePacket(playerId, session, clientPacket);

        } catch (Exception e) {
            Log.error("Error processing message from " + playerId, e);
            sendError(session, "PACKET_ERROR", "Failed to process message");
        }
    }

    private void handlePacket(String playerId, Session session, ClientPacket packet) {
        if (packet.hasConnect()) {
            handleConnect(playerId, session, packet.getConnect());
        } else if (packet.hasMove()) {
            handleMove(playerId, packet.getMove());
        } else if (packet.hasAction()) {
            handleAction(playerId, packet.getAction());
        } else if (packet.hasChat()) {
            handleChat(playerId, packet.getChat());
        }
    }

    private void handleConnect(String playerId, Session session,
                               io.qimmiit.siqiniq.proto.ConnectRequest request) {
        Log.info("Player connecting: " + request.getPlayerName() + " (ID: " + playerId + ")");

        // Send connect response
        ConnectResponse response = ConnectResponse.newBuilder()
            .setSuccess(true)
            .setPlayerId(playerId)
            .setMessage("Welcome to Siqiniq-001, " + request.getPlayerName() + "!")
            .build();

        ServerPacket serverPacket = ServerPacket.newBuilder()
            .setConnectResponse(response)
            .build();

        sendToSession(session, serverPacket);
    }

    private void handleMove(String playerId, io.qimmiit.siqiniq.proto.PlayerMoveRequest move) {
        Log.debug("Player " + playerId + " moved to: " + move.getX() + ", " + move.getY() + ", " + move.getZ());
        // TODO: Update player position and broadcast to nearby players
    }

    private void handleAction(String playerId, io.qimmiit.siqiniq.proto.PlayerActionRequest action) {
        Log.debug("Player " + playerId + " action: " + action.getActionType());
        // TODO: Handle player actions
    }

    private void handleChat(String playerId, io.qimmiit.siqiniq.proto.ChatMessageRequest chat) {
        Log.info("Chat from " + playerId + ": " + chat.getMessage());
        // TODO: Broadcast chat message to appropriate channel
    }

    // Utility methods
    private void sendToSession(Session session, ServerPacket packet) {
        try {
            ByteBuffer buffer = ByteBuffer.wrap(packet.toByteArray());
            session.getBasicRemote().sendBinary(buffer);
        } catch (IOException e) {
            Log.error("Failed to send packet to session", e);
        }
    }

    private void sendError(Session session, String code, String message) {
        io.qimmiit.siqiniq.proto.ErrorMessage error = io.qimmiit.siqiniq.proto.ErrorMessage.newBuilder()
            .setErrorCode(code)
            .setMessage(message)
            .build();

        ServerPacket packet = ServerPacket.newBuilder()
            .setError(error)
            .build();

        sendToSession(session, packet);
    }

    public void broadcast(ServerPacket packet) {
        ByteBuffer buffer = ByteBuffer.wrap(packet.toByteArray());

        sessions.values().forEach(session -> {
            try {
                session.getBasicRemote().sendBinary(buffer.duplicate());
            } catch (IOException e) {
                Log.error("Failed to broadcast to session", e);
            }
        });
    }

    public int getPlayerCount() {
        return sessions.size();
    }
}
