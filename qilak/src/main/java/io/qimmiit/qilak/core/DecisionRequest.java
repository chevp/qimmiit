package io.qimmiit.qilak.core;

import java.util.Map;

/**
 * Decision request from Nanook to Qilak
 *
 * Represents a request for AI-based decision making
 */
public class DecisionRequest {

    private DecisionType type;
    private Map<String, Object> parameters;
    private String requestId;
    private long timestamp;

    public enum DecisionType {
        OPTIMIZE_ASSET,
        ANALYZE_PERFORMANCE,
        RECOMMEND_LOD,
        GENERATE_SHADER_VARIANT,
        COMPUTE_LIGHTING,
        UNKNOWN
    }

    public DecisionRequest() {
    }

    public DecisionRequest(DecisionType type, Map<String, Object> parameters) {
        this.type = type;
        this.parameters = parameters;
        this.timestamp = System.currentTimeMillis();
    }

    // Getters and Setters

    public DecisionType getType() {
        return type;
    }

    public void setType(DecisionType type) {
        this.type = type;
    }

    public Map<String, Object> getParameters() {
        return parameters;
    }

    public void setParameters(Map<String, Object> parameters) {
        this.parameters = parameters;
    }

    public String getRequestId() {
        return requestId;
    }

    public void setRequestId(String requestId) {
        this.requestId = requestId;
    }

    public long getTimestamp() {
        return timestamp;
    }

    public void setTimestamp(long timestamp) {
        this.timestamp = timestamp;
    }

    public Object getParameter(String key) {
        return parameters != null ? parameters.get(key) : null;
    }

    @Override
    public String toString() {
        return "DecisionRequest{" +
                "type=" + type +
                ", requestId='" + requestId + '\'' +
                ", timestamp=" + timestamp +
                ", parameters=" + parameters +
                '}';
    }
}
