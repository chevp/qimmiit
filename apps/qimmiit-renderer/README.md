# Qimmiit Renderer

## Übersicht

Der **qimmiit-renderer** ist die zentrale Rendering-Engine für das Qimmiit-System. Diese native C++-Anwendung stellt die vollständige Vulkan-Visualisierung bereit und fungiert als Rendering-Canvas für 3D-Inhalte.

## Architektur

### Foundation-Komponenten

- **coregfx**: Vollständiges Vulkan-Rendering-Framework mit Swap Chain, Pipeline-Management und Texture-Handling
- **arctic-grpc-client**: gRPC-Client für die Kommunikation mit Backend-Services

### Design-Prinzipien

**Wichtig**: Diese Anwendung implementiert **KEIN ImGui-Overlay in Vulkan**.

Der qimmiit-renderer fokussiert sich auf:
- Vollständige Vulkan-Rendering-Pipeline für 3D-Visualisierung
- Canvas-ähnliche Darstellung von Rendering-Inhalten
- Reine C++-Implementierung ohne UI-Overlay-Komponenten
- Keine ImGui-Integration (kein Debug-UI im Vulkan-Context)
- High-Performance Rendering mit coregfx

## Verwendungszweck

Der Renderer dient als:
- Zentrale Rendering-Engine mit vollständiger Vulkan-Visualisierung
- Canvas für 3D-Szenen und Grafik-Inhalte
- Rendering-Backend mit gRPC-Steuerung
- Foundation für spezialisierte Rendering-Anwendungen

## Build

```bash
cmake -B build
cmake --build build
```

## Abhängigkeiten

- coregfx
- arctic-grpc-client
- gRPC/Protobuf (über arctic-grpc-client)

## Unterschied zu anderen Qimmiit-Apps

- **qimmiit-renderer**: Vollständige Vulkan-Visualisierung (Canvas), KEIN ImGui-Overlay
- **qimmiit-studio-prime**: Vollständiges Studio mit UI-Framework und erweiterten Features

Der qimmiit-renderer stellt die reine Rendering-Engine dar - vergleichbar mit einem Canvas-Element, das 3D-Inhalte darstellt, aber keine eigenen UI-Elemente (wie Debug-Overlays) enthält.