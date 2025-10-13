package io.qimmiit.siqiniq.studio.service.scene;

import cryo.services.CryoSceneAuthoringServiceOuterClass.*;
import jakarta.enterprise.context.ApplicationScoped;

import java.util.*;
import java.util.concurrent.ConcurrentHashMap;

/**
 * Repository for scene data storage
 * In production, this should be replaced with actual database access
 */
@ApplicationScoped
public class SceneDataRepository {

    private final Map<String, SceneData> scenes = new ConcurrentHashMap<>();

    public static class SceneData {
        private SceneMetadata metadata;
        private String content;
        private Map<String, EntityData> entities;
        private HierarchyNode rootHierarchy;

        public SceneData() {
            entities = new ConcurrentHashMap<>();
        }

        public SceneMetadata getMetadata() {
            return metadata;
        }

        public void setMetadata(SceneMetadata metadata) {
            this.metadata = metadata;
        }

        public String getContent() {
            return content;
        }

        public void setContent(String content) {
            this.content = content;
        }

        public Map<String, EntityData> getEntities() {
            return entities;
        }

        public HierarchyNode getRootHierarchy() {
            return rootHierarchy;
        }

        public void setRootHierarchy(HierarchyNode rootHierarchy) {
            this.rootHierarchy = rootHierarchy;
        }
    }

    public static class EntityData {
        private EntityInfo info;
        private String content;
        private String parentId;
        private List<String> childIds;

        public EntityData() {
            childIds = new ArrayList<>();
        }

        public EntityInfo getInfo() {
            return info;
        }

        public void setInfo(EntityInfo info) {
            this.info = info;
        }

        public String getContent() {
            return content;
        }

        public void setContent(String content) {
            this.content = content;
        }

        public String getParentId() {
            return parentId;
        }

        public void setParentId(String parentId) {
            this.parentId = parentId;
        }

        public List<String> getChildIds() {
            return childIds;
        }
    }

    public void save(String id, SceneData data) {
        scenes.put(id, data);
    }

    public SceneData findById(String id) {
        return scenes.get(id);
    }

    public boolean delete(String id) {
        return scenes.remove(id) != null;
    }

    public Collection<SceneData> findAll() {
        return scenes.values();
    }

    public boolean exists(String id) {
        return scenes.containsKey(id);
    }
}