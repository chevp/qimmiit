// frost_jni.cpp - JNI wrapper for Frost C++ Library
// Provides Java Native Interface bindings to frost.hpp

#include <frost/frost.hpp>
#include <jni.h>
#include <string>
#include <memory>

using namespace coregfx;

// Global renderer instance
static std::unique_ptr<FrostRenderer> g_renderer = nullptr;
static std::string g_lastError;

// Helper to convert jstring to std::string
std::string jstringToString(JNIEnv* env, jstring jstr) {
    if (!jstr) return "";
    const char* chars = env->GetStringUTFChars(jstr, nullptr);
    std::string result(chars);
    env->ReleaseStringUTFChars(jstr, chars);
    return result;
}

// Helper to convert std::string to jstring
jstring stringToJstring(JNIEnv* env, const std::string& str) {
    return env->NewStringUTF(str.c_str());
}

extern "C" {

// ============================================================================
// INITIALIZATION
// ============================================================================

JNIEXPORT jint JNICALL Java_com_arctic_jni_FrostJNI_initialize(
    JNIEnv* env, jclass,
    jint width, jint height,
    jboolean headless, jboolean enableValidation,
    jstring windowTitle
) {
    try {
        g_renderer = std::make_unique<FrostRenderer>();

        FrostInitParams params;
        params.width = width;
        params.height = height;
        params.headless = headless;
        params.enableValidation = enableValidation;
        params.windowTitle = jstringToString(env, windowTitle);

        auto result = g_renderer->Initialize(params);
        if (!result.success) {
            g_lastError = result.message;
            return result.error_code;
        }

        return 0; // SUCCESS
    } catch (const std::exception& e) {
        g_lastError = std::string("Exception in initialize: ") + e.what();
        return 1; // UNKNOWN_ERROR
    }
}

JNIEXPORT void JNICALL Java_com_arctic_jni_FrostJNI_shutdown(JNIEnv*, jclass) {
    try {
        if (g_renderer) {
            g_renderer->Shutdown();
            g_renderer.reset();
        }
    } catch (const std::exception& e) {
        g_lastError = std::string("Exception in shutdown: ") + e.what();
    }
}

// ============================================================================
// ASSET MANAGEMENT
// ============================================================================

JNIEXPORT jint JNICALL Java_com_arctic_jni_FrostJNI_loadGLTF(
    JNIEnv* env, jclass,
    jstring jGltfId, jstring jFilePath
) {
    try {
        if (!g_renderer) {
            g_lastError = "Renderer not initialized";
            return 10; // INITIALIZATION_FAILED
        }

        std::string gltfId = jstringToString(env, jGltfId);
        std::string filePath = jstringToString(env, jFilePath);

        auto result = g_renderer->LoadGLTF(gltfId, filePath);
        if (!result.success) {
            g_lastError = result.message;
            return result.error_code;
        }

        return 0; // SUCCESS
    } catch (const std::exception& e) {
        g_lastError = std::string("Exception in loadGLTF: ") + e.what();
        return 20; // SCENE_LOAD_FAILED
    }
}

JNIEXPORT jint JNICALL Java_com_arctic_jni_FrostJNI_unloadGLTF(
    JNIEnv* env, jclass,
    jstring jGltfId
) {
    try {
        if (!g_renderer) {
            g_lastError = "Renderer not initialized";
            return 10; // INITIALIZATION_FAILED
        }

        std::string gltfId = jstringToString(env, jGltfId);
        auto result = g_renderer->UnloadGLTF(gltfId);

        if (!result.success) {
            g_lastError = result.message;
            return result.error_code;
        }

        return 0; // SUCCESS
    } catch (const std::exception& e) {
        g_lastError = std::string("Exception in unloadGLTF: ") + e.what();
        return 1; // UNKNOWN_ERROR
    }
}

// ============================================================================
// ENTITY MANAGEMENT
// ============================================================================

JNIEXPORT jint JNICALL Java_com_arctic_jni_FrostJNI_addEntity(
    JNIEnv* env, jclass,
    jstring jEntityId, jstring jGltfId,
    jfloat posX, jfloat posY, jfloat posZ,
    jfloat rotX, jfloat rotY, jfloat rotZ,
    jfloat scaleX, jfloat scaleY, jfloat scaleZ
) {
    try {
        if (!g_renderer) {
            g_lastError = "Renderer not initialized";
            return 10; // INITIALIZATION_FAILED
        }

        std::string entityId = jstringToString(env, jEntityId);
        std::string gltfId = jstringToString(env, jGltfId);

        Transform transform;
        transform.position = { posX, posY, posZ };
        transform.rotation = { rotX, rotY, rotZ };
        transform.scale = { scaleX, scaleY, scaleZ };

        auto result = g_renderer->AddEntity(entityId, gltfId, transform);
        if (!result.success) {
            g_lastError = result.message;
            return result.error_code;
        }

        return 0; // SUCCESS
    } catch (const std::exception& e) {
        g_lastError = std::string("Exception in addEntity: ") + e.what();
        return 1; // UNKNOWN_ERROR
    }
}

JNIEXPORT jint JNICALL Java_com_arctic_jni_FrostJNI_removeEntity(
    JNIEnv* env, jclass,
    jstring jEntityId
) {
    try {
        if (!g_renderer) {
            g_lastError = "Renderer not initialized";
            return 10; // INITIALIZATION_FAILED
        }

        std::string entityId = jstringToString(env, jEntityId);
        auto result = g_renderer->RemoveEntity(entityId);

        if (!result.success) {
            g_lastError = result.message;
            return result.error_code;
        }

        return 0; // SUCCESS
    } catch (const std::exception& e) {
        g_lastError = std::string("Exception in removeEntity: ") + e.what();
        return 1; // UNKNOWN_ERROR
    }
}

JNIEXPORT jint JNICALL Java_com_arctic_jni_FrostJNI_updateEntityTransform(
    JNIEnv* env, jclass,
    jstring jEntityId,
    jfloat posX, jfloat posY, jfloat posZ,
    jfloat rotX, jfloat rotY, jfloat rotZ,
    jfloat scaleX, jfloat scaleY, jfloat scaleZ
) {
    try {
        if (!g_renderer) {
            g_lastError = "Renderer not initialized";
            return 10; // INITIALIZATION_FAILED
        }

        std::string entityId = jstringToString(env, jEntityId);

        Transform transform;
        transform.position = { posX, posY, posZ };
        transform.rotation = { rotX, rotY, rotZ };
        transform.scale = { scaleX, scaleY, scaleZ };

        auto result = g_renderer->UpdateEntityTransform(entityId, transform);
        if (!result.success) {
            g_lastError = result.message;
            return result.error_code;
        }

        return 0; // SUCCESS
    } catch (const std::exception& e) {
        g_lastError = std::string("Exception in updateEntityTransform: ") + e.what();
        return 1; // UNKNOWN_ERROR
    }
}

// ============================================================================
// CAMERA CONTROL
// ============================================================================

JNIEXPORT jint JNICALL Java_com_arctic_jni_FrostJNI_setCameraPosition(
    JNIEnv*, jclass,
    jfloat x, jfloat y, jfloat z
) {
    try {
        if (!g_renderer) {
            g_lastError = "Renderer not initialized";
            return 10; // INITIALIZATION_FAILED
        }

        Vec3 position = { x, y, z };
        auto result = g_renderer->SetCameraPosition(position);

        if (!result.success) {
            g_lastError = result.message;
            return result.error_code;
        }

        return 0; // SUCCESS
    } catch (const std::exception& e) {
        g_lastError = std::string("Exception in setCameraPosition: ") + e.what();
        return 1; // UNKNOWN_ERROR
    }
}

JNIEXPORT jint JNICALL Java_com_arctic_jni_FrostJNI_setCameraRotation(
    JNIEnv*, jclass,
    jfloat pitch, jfloat yaw, jfloat roll
) {
    try {
        if (!g_renderer) {
            g_lastError = "Renderer not initialized";
            return 10; // INITIALIZATION_FAILED
        }

        Vec3 rotation = { pitch, yaw, roll };
        auto result = g_renderer->SetCameraRotation(rotation);

        if (!result.success) {
            g_lastError = result.message;
            return result.error_code;
        }

        return 0; // SUCCESS
    } catch (const std::exception& e) {
        g_lastError = std::string("Exception in setCameraRotation: ") + e.what();
        return 1; // UNKNOWN_ERROR
    }
}

// ============================================================================
// RENDERING
// ============================================================================

JNIEXPORT jint JNICALL Java_com_arctic_jni_FrostJNI_renderFrame(JNIEnv*, jclass) {
    try {
        if (!g_renderer) {
            g_lastError = "Renderer not initialized";
            return 10; // INITIALIZATION_FAILED
        }

        auto result = g_renderer->RenderFrame();
        if (!result.success) {
            g_lastError = result.message;
            return result.error_code;
        }

        return 0; // SUCCESS
    } catch (const std::exception& e) {
        g_lastError = std::string("Exception in renderFrame: ") + e.what();
        return 1; // UNKNOWN_ERROR
    }
}

JNIEXPORT jint JNICALL Java_com_arctic_jni_FrostJNI_captureScreenshot(
    JNIEnv* env, jclass,
    jstring jFilename
) {
    try {
        if (!g_renderer) {
            g_lastError = "Renderer not initialized";
            return 10; // INITIALIZATION_FAILED
        }

        std::string filename = jstringToString(env, jFilename);
        auto result = g_renderer->CaptureScreenshot(filename);

        if (!result.success) {
            g_lastError = result.message;
            return result.error_code;
        }

        return 0; // SUCCESS
    } catch (const std::exception& e) {
        g_lastError = std::string("Exception in captureScreenshot: ") + e.what();
        return 1; // UNKNOWN_ERROR
    }
}

// ============================================================================
// SCENE MANAGEMENT
// ============================================================================

JNIEXPORT jint JNICALL Java_com_arctic_jni_FrostJNI_clearScene(JNIEnv*, jclass) {
    try {
        if (!g_renderer) {
            g_lastError = "Renderer not initialized";
            return 10; // INITIALIZATION_FAILED
        }

        auto result = g_renderer->ClearScene();
        if (!result.success) {
            g_lastError = result.message;
            return result.error_code;
        }

        return 0; // SUCCESS
    } catch (const std::exception& e) {
        g_lastError = std::string("Exception in clearScene: ") + e.what();
        return 1; // UNKNOWN_ERROR
    }
}

JNIEXPORT jstring JNICALL Java_com_arctic_jni_FrostJNI_getLastError(JNIEnv* env, jclass) {
    return stringToJstring(env, g_lastError);
}

} // extern "C"
