package io.qimmiit.siqiniq.studio.service.template;

import com.google.protobuf.Empty;
import cryo.services.CryoTemplateServiceOuterClass.*;
import jakarta.enterprise.context.ApplicationScoped;
import jakarta.inject.Inject;
import org.jboss.logging.Logger;

import java.time.Instant;
import java.util.UUID;

/**
 * Service handling Template CRUD operations
 */
@ApplicationScoped
public class TemplateCrudService {

    private static final Logger LOG = Logger.getLogger(TemplateCrudService.class);

    @Inject
    TemplateDataRepository repository;

    public void createTemplate(CreateTemplateRequest request,
                                io.grpc.stub.StreamObserver<TemplateResponse> responseObserver) {
        try {
            String id = UUID.randomUUID().toString();
            long now = Instant.now().toEpochMilli();

            TemplateMetadata metadata = TemplateMetadata.newBuilder()
                    .setId(id)
                    .setName(request.getName())
                    .setDescription(request.getDescription())
                    .setType(request.getType())
                    .setVersion("1.0.0")
                    .setCreatedBy(request.getCreatedBy())
                    .putAllTags(request.getTagsMap())
                    .build();

            TemplateDataRepository.TemplateData data = new TemplateDataRepository.TemplateData();
            data.setMetadata(metadata);
            data.setContent(request.getContent());

            repository.save(id, data);

            TemplateResponse response = TemplateResponse.newBuilder()
                    .setMetadata(metadata)
                    .setContent(data.getContent())
                    .build();

            LOG.infof("Created template: %s (%s)", metadata.getName(), id);
            responseObserver.onNext(response);
            responseObserver.onCompleted();

        } catch (Exception e) {
            LOG.error("Failed to create template", e);
            responseObserver.onError(e);
        }
    }

    public void getTemplate(GetTemplateRequest request,
                            io.grpc.stub.StreamObserver<TemplateResponse> responseObserver) {
        try {
            TemplateDataRepository.TemplateData data = repository.findById(request.getId());
            if (data == null) {
                responseObserver.onError(new RuntimeException("Template not found: " + request.getId()));
                return;
            }

            TemplateResponse.Builder responseBuilder = TemplateResponse.newBuilder()
                    .setMetadata(data.getMetadata());

            if (request.getIncludeContent()) {
                responseBuilder.setContent(data.getContent());
            }

            responseObserver.onNext(responseBuilder.build());
            responseObserver.onCompleted();

        } catch (Exception e) {
            LOG.error("Failed to get template", e);
            responseObserver.onError(e);
        }
    }

    public void updateTemplate(UpdateTemplateRequest request,
                                io.grpc.stub.StreamObserver<TemplateResponse> responseObserver) {
        try {
            TemplateDataRepository.TemplateData data = repository.findById(request.getId());
            if (data == null) {
                responseObserver.onError(new RuntimeException("Template not found: " + request.getId()));
                return;
            }

            TemplateMetadata.Builder metadataBuilder = data.getMetadata().toBuilder();

            if (request.hasName()) {
                metadataBuilder.setName(request.getName());
            }
            if (request.hasDescription()) {
                metadataBuilder.setDescription(request.getDescription());
            }
            if (!request.getTagsMap().isEmpty()) {
                metadataBuilder.putAllTags(request.getTagsMap());
            }
            metadataBuilder.setUpdatedBy(request.getUpdatedBy());

            data.setMetadata(metadataBuilder.build());

            if (request.hasContent()) {
                data.setContent(request.getContent());
            }

            TemplateResponse response = TemplateResponse.newBuilder()
                    .setMetadata(data.getMetadata())
                    .setContent(data.getContent())
                    .build();

            LOG.infof("Updated template: %s", request.getId());
            responseObserver.onNext(response);
            responseObserver.onCompleted();

        } catch (Exception e) {
            LOG.error("Failed to update template", e);
            responseObserver.onError(e);
        }
    }

    public void deleteTemplate(DeleteTemplateRequest request,
                                io.grpc.stub.StreamObserver<Empty> responseObserver) {
        try {
            boolean removed = repository.delete(request.getId());
            if (!removed) {
                responseObserver.onError(new RuntimeException("Template not found: " + request.getId()));
                return;
            }

            LOG.infof("Deleted template: %s", request.getId());
            responseObserver.onNext(Empty.getDefaultInstance());
            responseObserver.onCompleted();

        } catch (Exception e) {
            LOG.error("Failed to delete template", e);
            responseObserver.onError(e);
        }
    }
}
