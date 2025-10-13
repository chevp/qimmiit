package io.qimmiit.siqiniq.studio.service;

import com.google.protobuf.Empty;
import cryo.services.CryoTemplateServiceOuterClass.*;
import io.qimmiit.siqiniq.studio.service.template.*;
import io.quarkus.grpc.GrpcService;
import jakarta.enterprise.context.ApplicationScoped;
import jakarta.inject.Inject;
import org.jboss.logging.Logger;

/**
 * Cryo Template Service Implementation
 * Handles CRUD operations for Cryo Protocol templates
 *
 * This is a facade service that delegates to specialized service classes
 */
@ApplicationScoped
@GrpcService
public class CryoTemplateService implements cryo.services.CryoTemplateServiceGrpc.CryoTemplateServiceImplBase {

    private static final Logger LOG = Logger.getLogger(CryoTemplateService.class);

    @Inject
    TemplateCrudService templateCrudService;

    @Inject
    TemplateQueryService templateQueryService;

    @Inject
    TemplateValidationService templateValidationService;

    // ========================================================================
    // CREATE - Delegated to TemplateCrudService
    // ========================================================================

    @Override
    public void createTemplate(CreateTemplateRequest request,
                                io.grpc.stub.StreamObserver<TemplateResponse> responseObserver) {
        templateCrudService.createTemplate(request, responseObserver);
    }

    // ========================================================================
    // READ - Delegated to TemplateCrudService and TemplateQueryService
    // ========================================================================

    @Override
    public void getTemplate(GetTemplateRequest request,
                            io.grpc.stub.StreamObserver<TemplateResponse> responseObserver) {
        templateCrudService.getTemplate(request, responseObserver);
    }

    @Override
    public void listTemplates(ListTemplatesRequest request,
                              io.grpc.stub.StreamObserver<ListTemplatesResponse> responseObserver) {
        templateQueryService.listTemplates(request, responseObserver);
    }

    @Override
    public void listTemplatesByType(ListTemplatesByTypeRequest request,
                                     io.grpc.stub.StreamObserver<ListTemplatesResponse> responseObserver) {
        templateQueryService.listTemplatesByType(request, responseObserver);
    }

    // ========================================================================
    // UPDATE - Delegated to TemplateCrudService
    // ========================================================================

    @Override
    public void updateTemplate(UpdateTemplateRequest request,
                                io.grpc.stub.StreamObserver<TemplateResponse> responseObserver) {
        templateCrudService.updateTemplate(request, responseObserver);
    }

    // ========================================================================
    // DELETE - Delegated to TemplateCrudService
    // ========================================================================

    @Override
    public void deleteTemplate(DeleteTemplateRequest request,
                                io.grpc.stub.StreamObserver<Empty> responseObserver) {
        templateCrudService.deleteTemplate(request, responseObserver);
    }

    // ========================================================================
    // SPECIAL OPERATIONS - Delegated to TemplateValidationService
    // ========================================================================

    @Override
    public void loadTemplateAsRuntime(LoadTemplateRequest request,
                                       io.grpc.stub.StreamObserver<LoadTemplateResponse> responseObserver) {
        templateValidationService.loadTemplateAsRuntime(request, responseObserver);
    }

    @Override
    public void validateTemplate(ValidateTemplateRequest request,
                                  io.grpc.stub.StreamObserver<ValidateTemplateResponse> responseObserver) {
        templateValidationService.validateTemplate(request, responseObserver);
    }
}
