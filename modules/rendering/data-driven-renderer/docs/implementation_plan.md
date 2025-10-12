Implementation Plan
Phase 1: Core Integration (Foundation)
1.1 Create DataDrivenRenderer Class
File: apps/foundation/data-driven-renderer/src/DataDrivenRenderer.hpp/cpp Purpose: Main renderer class that wraps OceanPbrApp, similar to CoregfxApiModel in elyrion.coregfx.renderer Key responsibilities:
Initialize OceanPbrApp (Vulkan, window, pipelines)
Manage window lifecycle (Win32 HWND)
Bridge between DataDrivenShaderManager and OceanPbrApp
Handle render loop with OceanPbrApp::renderFrame()
API:
class DataDrivenRenderer {
public:
    DataDrivenRenderer(const AppSettings& settings);
    ~DataDrivenRenderer();
    
    bool initialize(HINSTANCE hInstance);
    void renderFrame();
    bool isInitialized() const;
    void shutdown();
    
    // Shader instance management
    void updateShaderInstance(const rendering::ShaderUpdate& update);
    void updateCamera(const glm::vec3& pos, const glm::vec3& rot);
    
    OceanPbrApp* getPbrApp() { return pbrApp_; }
    
private:
    OceanPbrApp* pbrApp_ = nullptr;
    HWND hwnd_ = nullptr;
    AppSettings settings_;
    DataDrivenShaderManager shaderManager_;
};
1.2 Update main.cpp to Use DataDrivenRenderer
File: apps/foundation/data-driven-renderer/src/main.cpp Changes:
Replace placeholder TODOs with actual DataDrivenRenderer initialization
Create Win32 window (or skip for headless)
Call renderer.renderFrame() in loop
Proper shutdown and cleanup
Phase 2: Shader Pipeline Integration
2.1 Extend DataDrivenShaderManager
File: apps/foundation/data-driven-renderer/src/DataDrivenShaderManager.cpp Implement missing methods:
createPipelineFromTemplate() - Create Vulkan pipeline from shader template
updateDescriptorSet() - Update uniform buffers and textures from gRPC messages
Load shader SPIR-V files from template paths
Integration points with coregfx:
Use OceanPbrApp::pipelines for pipeline storage
Use OceanPbrApp::descriptorSets for descriptor management
Leverage existing PBR shader infrastructure
2.2 Map Shader Instances to Geometry
Challenge: data-driven.index.xml defines shader instances but doesn't specify which geometry to render Solution options:
Load GLTF models from <geometry> tags in XML
Create simple procedural geometry (quad, cube) for testing
Reuse elyrion's entity system but drive it with shader instances
Recommendation: Start with option 2 (procedural geometry) for MVP, then add GLTF support
Phase 3: gRPC Real-Time Updates
3.1 Connect GrpcRendererClient to DataDrivenRenderer
File: apps/foundation/data-driven-renderer/src/main.cpp Wire up callbacks:
grpcClient->setShaderUpdateCallback([&renderer](const rendering::ShaderUpdate& update) {
    renderer.updateShaderInstance(update);
});
3.2 Apply Updates to GPU
File: apps/foundation/data-driven-renderer/src/DataDrivenRenderer.cpp Flow:
Receive gRPC message → ShaderUpdate protobuf
Parse uniforms → DataDrivenShaderManager::updateShaderInstance()
Update Vulkan descriptor set → vkUpdateDescriptorSets()
Next renderFrame() uses updated parameters
Phase 4: Rendering Loop
4.1 Implement Render Loop
File: apps/foundation/data-driven-renderer/src/main.cpp Replace simulation loop (lines 132-149) with:
while (!renderer.shouldClose()) {
    // Process Win32 messages (if not headless)
    if (!settings.headless) {
        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }
    
    // Render frame with current shader instances
    renderer.renderFrame();
    
    // Send telemetry to gRPC server
    if (grpcClient && grpcClient->isConnected()) {
        grpcClient->sendFrameRendered(frameNumber++, deltaTime);
    }
}
4.2 Headless Mode Support
Requirement: Must work without window creation Implementation:
Skip Win32 window creation if --headless
Use Vulkan without swapchain (offscreen rendering)
Render to framebuffer, optionally save to file
Phase 5: Configuration Integration
5.1 Load arctic.config.xml
File: apps/foundation/data-driven-renderer/src/ConfigLoader.hpp/cpp Parse XML for:
Logging settings
Camera defaults
gRPC client settings (reconnect, heartbeat)
Headless mode, studio mode flags
Apply to:
AppSettings structure
oceancore::Settings for OceanPbrApp initialization
5.2 Merge Two Config Files
Challenge: Two separate XMLs (arctic.config.xml + data-driven.index.xml) Solution:
Load arctic.config.xml first → application settings
Load data-driven.index.xml second → scene/shader data
Merge camera settings (index.xml overrides config.xml if present)
Phase 6: Testing & Validation
6.1 MVP Test Case
Goal: Render a single colored cube with PBR material Steps:
Create cube geometry procedurally
Load PBR shader template
Create shader instance with metallic/roughness values
Render 60 FPS in window
6.2 gRPC Update Test
Goal: Change cube material color via gRPC while running Steps:
Start renderer with default red material
Send ShaderUpdate via gRPC changing base_color_factor to blue
Verify cube turns blue without restart
Implementation Order (Recommended)
Week 1: Core Setup
✅ Create DataDrivenRenderer class skeleton
✅ Initialize OceanPbrApp from main.cpp
✅ Create window (Win32) or headless mode
✅ Render first frame (even if empty/black screen)
Week 2: Shader Integration
✅ Implement DataDrivenShaderManager::createPipelineFromTemplate()
✅ Load PBR shader SPIR-V files
✅ Create simple cube geometry
✅ Render cube with hardcoded PBR material
Week 3: gRPC Integration
✅ Connect GrpcRendererClient callbacks to renderer
✅ Implement DataDrivenShaderManager::updateShaderInstance() for descriptor updates
✅ Test real-time shader updates via gRPC
✅ Send telemetry back to server
Week 4: Polish & Testing
✅ Load GLTF models from <geometry> tags
✅ Load arctic.config.xml and apply settings
✅ Headless mode testing
✅ Performance optimization (minimize descriptor updates)
Key Files to Create/Modify
New Files
apps/foundation/data-driven-renderer/src/DataDrivenRenderer.hpp
apps/foundation/data-driven-renderer/src/DataDrivenRenderer.cpp
apps/foundation/data-driven-renderer/src/ConfigLoader.hpp
apps/foundation/data-driven-renderer/src/ConfigLoader.cpp
apps/foundation/data-driven-renderer/src/GeometryFactory.hpp (procedural geometry)
Modified Files
apps/foundation/data-driven-renderer/src/main.cpp (remove TODOs, add real rendering)
apps/foundation/data-driven-renderer/src/DataDrivenShaderManager.cpp (implement pipeline/descriptor methods)
apps/foundation/data-driven-renderer/CMakeLists.txt (add new source files)
Dependencies on elyrion.coregfx.renderer
Reuse patterns from:
Application.hpp → Window creation, initialization flow
CoregfxApiModel → Bridge between API and OceanPbrApp
CoregfxManager → Vulkan lifecycle management
PBR pipeline setup in OceanPbrApp::init()
Key differences:
❌ No HTTP REST API (data-driven uses gRPC)
❌ No entity system (data-driven uses shader instances directly)
✅ Similar Vulkan/window initialization
✅ Similar render loop structure