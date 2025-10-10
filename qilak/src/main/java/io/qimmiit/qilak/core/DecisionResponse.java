package io.qimmiit.qilak.core;

import java.util.HashMap;
import java.util.Map;

/**
 * Decision response from Qilak to Nanook
 *
 * Contains AI-based recommendations and metadata
 */
public class DecisionResponse {

    private String status;
    private String recommendation;
    private double confidence;
    private Map<String, Object> metadata;
    private long timestamp;

    public DecisionResponse() {
        this.timestamp = System.currentTimeMillis();
        this.metadata = new HashMap<>();
    }

    // Builder pattern
    public static Builder builder() {
        return new Builder();
    }

    public static DecisionResponse unknown() {
        return builder()
                .status("unknown")
                .recommendation("No recommendation available")
                .confidence(0.0)
                .build();
    }

    // Getters and Setters

    public String getStatus() {
        return status;
    }

    public void setStatus(String status) {
        this.status = status;
    }

    public String getRecommendation() {
        return recommendation;
    }

    public void setRecommendation(String recommendation) {
        this.recommendation = recommendation;
    }

    public double getConfidence() {
        return confidence;
    }

    public void setConfidence(double confidence) {
        this.confidence = confidence;
    }

    public Map<String, Object> getMetadata() {
        return metadata;
    }

    public void setMetadata(Map<String, Object> metadata) {
        this.metadata = metadata;
    }

    public long getTimestamp() {
        return timestamp;
    }

    public void setTimestamp(long timestamp) {
        this.timestamp = timestamp;
    }

    public void addMetadata(String key, Object value) {
        this.metadata.put(key, value);
    }

    @Override
    public String toString() {
        return "DecisionResponse{" +
                "status='" + status + '\'' +
                ", recommendation='" + recommendation + '\'' +
                ", confidence=" + confidence +
                ", timestamp=" + timestamp +
                ", metadata=" + metadata +
                '}';
    }

    // Builder
    public static class Builder {
        private final DecisionResponse response = new DecisionResponse();

        public Builder status(String status) {
            response.status = status;
            return this;
        }

        public Builder recommendation(String recommendation) {
            response.recommendation = recommendation;
            return this;
        }

        public Builder confidence(double confidence) {
            response.confidence = confidence;
            return this;
        }

        public Builder metadata(String key, Object value) {
            response.metadata.put(key, value);
            return this;
        }

        public DecisionResponse build() {
            return response;
        }
    }
}
