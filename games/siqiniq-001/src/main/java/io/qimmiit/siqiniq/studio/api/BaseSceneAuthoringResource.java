package io.qimmiit.siqiniq.studio.api;

import com.google.protobuf.util.JsonFormat;
import cryo.services.CryoSceneAuthoringServiceOuterClass.SceneFormat;
import io.qimmiit.siqiniq.studio.service.CryoSceneAuthoringService;
import jakarta.inject.Inject;
import jakarta.ws.rs.Consumes;
import jakarta.ws.rs.Produces;
import jakarta.ws.rs.core.MediaType;
import org.jboss.logging.Logger;

/**
 * Base class for Scene Authoring REST resources
 * Provides shared configuration and utilities
 */
@Produces(MediaType.APPLICATION_JSON)
@Consumes(MediaType.APPLICATION_JSON)
public abstract class BaseSceneAuthoringResource {

    protected static final Logger LOG = Logger.getLogger(BaseSceneAuthoringResource.class);

    @Inject
    protected CryoSceneAuthoringService sceneService;

    protected final JsonFormat.Printer jsonPrinter = JsonFormat.printer()
            .includingDefaultValueFields()
            .preservingProtoFieldNames();

    protected final JsonFormat.Parser jsonParser = JsonFormat.parser()
            .ignoringUnknownFields();

    protected String getExtension(SceneFormat format) {
        return switch (format) {
            case PBTXT -> ".pbtxt";
            case JSON -> ".json";
            case BINARY -> ".bin";
            case YAML -> ".yaml";
            default -> ".dat";
        };
    }
}