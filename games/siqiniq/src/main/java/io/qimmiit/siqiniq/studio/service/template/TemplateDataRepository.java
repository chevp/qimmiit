package io.qimmiit.siqiniq.studio.service.template;

import cryo.services.CryoTemplateServiceOuterClass.TemplateMetadata;
import jakarta.enterprise.context.ApplicationScoped;

import java.util.Collection;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

/**
 * Repository for template data storage
 * In production, this should be replaced with actual database access
 */
@ApplicationScoped
public class TemplateDataRepository {

    private final Map<String, TemplateData> templates = new ConcurrentHashMap<>();

    public static class TemplateData {
        private TemplateMetadata metadata;
        private String content;

        public TemplateMetadata getMetadata() {
            return metadata;
        }

        public void setMetadata(TemplateMetadata metadata) {
            this.metadata = metadata;
        }

        public String getContent() {
            return content;
        }

        public void setContent(String content) {
            this.content = content;
        }
    }

    public void save(String id, TemplateData data) {
        templates.put(id, data);
    }

    public TemplateData findById(String id) {
        return templates.get(id);
    }

    public boolean delete(String id) {
        return templates.remove(id) != null;
    }

    public Collection<TemplateData> findAll() {
        return templates.values();
    }

    public boolean exists(String id) {
        return templates.containsKey(id);
    }
}
