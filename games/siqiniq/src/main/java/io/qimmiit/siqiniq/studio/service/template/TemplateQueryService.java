package io.qimmiit.siqiniq.studio.service.template;

import cryo.services.CryoTemplateServiceOuterClass.*;
import jakarta.enterprise.context.ApplicationScoped;
import jakarta.inject.Inject;
import org.jboss.logging.Logger;

import java.util.ArrayList;
import java.util.List;

/**
 * Service handling Template query and search operations
 */
@ApplicationScoped
public class TemplateQueryService {

    private static final Logger LOG = Logger.getLogger(TemplateQueryService.class);

    @Inject
    TemplateDataRepository repository;

    public void listTemplates(ListTemplatesRequest request,
                              io.grpc.stub.StreamObserver<ListTemplatesResponse> responseObserver) {
        try {
            List<TemplateMetadata> allTemplates = new ArrayList<>();

            for (TemplateDataRepository.TemplateData data : repository.findAll()) {
                // Simple search filter
                if (request.getSearch() != null && !request.getSearch().isEmpty()) {
                    if (!data.getMetadata().getName().toLowerCase().contains(request.getSearch().toLowerCase()) &&
                        !data.getMetadata().getDescription().toLowerCase().contains(request.getSearch().toLowerCase())) {
                        continue;
                    }
                }
                allTemplates.add(data.getMetadata());
            }

            // Simple pagination
            int page = request.getPage();
            int pageSize = request.getPageSize() > 0 ? request.getPageSize() : 20;
            int start = page * pageSize;
            int end = Math.min(start + pageSize, allTemplates.size());

            List<TemplateMetadata> pageTemplates = allTemplates.subList(
                    Math.min(start, allTemplates.size()),
                    Math.min(end, allTemplates.size())
            );

            ListTemplatesResponse response = ListTemplatesResponse.newBuilder()
                    .addAllTemplates(pageTemplates)
                    .setTotalCount(allTemplates.size())
                    .setPage(page)
                    .setPageSize(pageSize)
                    .build();

            responseObserver.onNext(response);
            responseObserver.onCompleted();

        } catch (Exception e) {
            LOG.error("Failed to list templates", e);
            responseObserver.onError(e);
        }
    }

    public void listTemplatesByType(ListTemplatesByTypeRequest request,
                                     io.grpc.stub.StreamObserver<ListTemplatesResponse> responseObserver) {
        try {
            List<TemplateMetadata> filteredTemplates = new ArrayList<>();

            for (TemplateDataRepository.TemplateData data : repository.findAll()) {
                if (data.getMetadata().getType() == request.getType()) {
                    filteredTemplates.add(data.getMetadata());
                }
            }

            // Pagination
            int page = request.getPage();
            int pageSize = request.getPageSize() > 0 ? request.getPageSize() : 20;
            int start = page * pageSize;
            int end = Math.min(start + pageSize, filteredTemplates.size());

            List<TemplateMetadata> pageTemplates = filteredTemplates.subList(
                    Math.min(start, filteredTemplates.size()),
                    Math.min(end, filteredTemplates.size())
            );

            ListTemplatesResponse response = ListTemplatesResponse.newBuilder()
                    .addAllTemplates(pageTemplates)
                    .setTotalCount(filteredTemplates.size())
                    .setPage(page)
                    .setPageSize(pageSize)
                    .build();

            responseObserver.onNext(response);
            responseObserver.onCompleted();

        } catch (Exception e) {
            LOG.error("Failed to list templates by type", e);
            responseObserver.onError(e);
        }
    }
}
