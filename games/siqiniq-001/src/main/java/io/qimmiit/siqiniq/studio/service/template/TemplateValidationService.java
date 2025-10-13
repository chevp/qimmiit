package io.qimmiit.siqiniq.studio.service.template;

import cryo.services.CryoTemplateServiceOuterClass.*;
import jakarta.enterprise.context.ApplicationScoped;
import jakarta.inject.Inject;
import org.jboss.logging.Logger;

import java.util.UUID;

/**
 * Service handling Template validation and loading operations
 */
@ApplicationScoped
public class TemplateValidationService {

    private static final Logger LOG = Logger.getLogger(TemplateValidationService.class);

    @Inject
    TemplateDataRepository repository;

    public void loadTemplateAsRuntime(LoadTemplateRequest request,
                                       io.grpc.stub.StreamObserver<LoadTemplateResponse> responseObserver) {
        try {
            TemplateDataRepository.TemplateData data = repository.findById(request.getTemplateId());
            if (data == null) {
                responseObserver.onError(new RuntimeException("Template not found: " + request.getTemplateId()));
                return;
            }

            // TODO: Implement actual template loading logic
            // This should parse the pbtxt, apply overrides, and create runtime instance

            String runtimeId = UUID.randomUUID().toString();

            LoadTemplateResponse response = LoadTemplateResponse.newBuilder()
                    .setRuntimeId(runtimeId)
                    .setSuccess(true)
                    .setMessage("Template loaded successfully")
                    .build();

            LOG.infof("Loaded template %s as runtime %s", request.getTemplateId(), runtimeId);
            responseObserver.onNext(response);
            responseObserver.onCompleted();

        } catch (Exception e) {
            LOG.error("Failed to load template as runtime", e);
            responseObserver.onError(e);
        }
    }

    public void validateTemplate(ValidateTemplateRequest request,
                                  io.grpc.stub.StreamObserver<ValidateTemplateResponse> responseObserver) {
        try {
            // TODO: Implement actual template validation logic
            // This should parse the pbtxt and check for syntax/semantic errors

            ValidateTemplateResponse response = ValidateTemplateResponse.newBuilder()
                    .setValid(true)
                    .build();

            responseObserver.onNext(response);
            responseObserver.onCompleted();

        } catch (Exception e) {
            LOG.error("Failed to validate template", e);
            responseObserver.onError(e);
        }
    }
}
