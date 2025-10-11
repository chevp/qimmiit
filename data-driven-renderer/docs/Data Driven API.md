Perfect! Even simpler - just two data transfer objects:
// Local Java Layer - Minimal Data-Driven Renderer Bridge
class DataDrivenRendererBridge {
    
    // Single method to send data to renderer
    native ResponseData send(ReceiveData data);
    
    // Or if async/streaming:
    native void receive(ReceiveData data);
    native ResponseData getResponse();
}
Data Classes:
// Everything the renderer receives from server
class ReceiveData {
    // Command type
    CommandType command;  // LOAD_SCENE, UPDATE_SHADER_INSTANCE, UPDATE_CAMERA, RENDER_FRAME, etc.
    
    // Scene loading
    String sceneXmlPath;
    
    // Shader instance operations
    String instanceId;
    String templateId;
    Map<String, UniformValue> uniforms;
    Map<String, String> textureBindings;
    Geometry geometry;
    Transform transform;
    
    // Camera
    CameraState camera;
    
    // Lights
    String lightId;
    DirectionalLight directionalLight;
    PointLight pointLight;
    
    // Environment
    AmbientLight ambientLight;
    
    // Frame control
    boolean renderFrame;
}

// Everything the renderer sends back to server
class ResponseData {
    // Render stats
    long frameTimeMs;
    int drawCalls;
    int shaderInstanceCount;
    
    // User interaction events (if applicable)
    String eventType;
    String instanceId;
    
    // Error/status
    boolean success;
    String errorMessage;
}
Or even simpler - match XML structure directly:
class ReceiveData {
    CommandType command;
    String xmlFragment;  // Just send XML fragments!
}

class ResponseData {
    boolean success;
    Map<String, Object> telemetry;
    String errorMessage;
}
Ultra-minimal bridge:
native ResponseData execute(ReceiveData request);
Central Server generates XML → Local Java wraps in ReceiveData → C++ parses XML and executes → Returns ResponseData. Is this the simplicity you're looking for?