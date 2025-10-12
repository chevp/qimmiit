# Qimmiit Studio Prime

## Überblick

**Qimmiit Studio Prime** ist eine integrierte C++-basierte Studio-Anwendung für die Entwicklung und Verwaltung von 3D-Rendering-Projekten. Die Anwendung kombiniert eine leistungsstarke C++-Rendering-Engine mit modernen Web-basierten Studio-Tools.

## Architektur

Qimmiit Studio Prime besteht aus drei Hauptkomponenten:

### 1. Electron Desktop App
- Benutzeroberfläche für Studio-Funktionalitäten
- Java-basierter HTTP-Server für Event-Kommunikation mit ImGui Layer
- Koordiniert verschiedene Electron-basierte Tools
- **Electron-Tools verwenden Cryo Tooling HTTP Server via HTTP-Requests**

### 2. CoreGFX Renderer (C++)
- **Arctic Foundation Layer**: Basis-Framework für Rendering
  - `coregfx`: Vulkan-basierte Rendering-Engine
  - Vulkan Context Management
  - Resource Management

- **Arctic Domain Layer**: Domänenspezifische Logik
  - Scene Management
  - Asset Pipeline
  - Material System

- **Arctic gRPC Client**: Kommunikation mit Java gRPC Server (Qilak AI Agent)
  - C++ gRPC Client-Bibliothek
  - Verbindung zum Java-basierten Qilak AI Agent Server
  - AI-gestützte Rendering-Entscheidungen
  - Protobuf-basierter Nachrichtenaustausch
  - Asset-Optimierung und Performance-Analyse

### 3. C++ Studio Domain Layer
- **ImGui Integration Layer**:
  - HTTP-Client für Kommunikation mit Java-Server
  - Empfängt Events vom Java-Server
  - Kann Electron-Tools per Event öffnen/steuern

- **Cryo Tooling HTTP Server**:
  - Stellt Studio-Funktionalität für Electron-Tools bereit
  - **Empfängt HTTP-Requests von Electron-Tools**
  - Integration zwischen C++-Engine und Web-Tools
  - Bietet REST-API für Studio-Operationen

## Kommunikationsfluss

```
┌───────────────────────────────────────────────────┐
│                  Electron App                     │
│  ┌─────────────────┐    ┌──────────────────┐      │
│  │ Electron Tools  │    │ Java HTTP Server │      │
│  │                 │    │                  │      │
│  └────────┬────────┘    └────────┬─────────┘      │
└───────────┼──────────────────────┼────────────────┘
            │                      │
            │ HTTP Requests        │ HTTP Events
            │                      │
            │                      ▼
            │            ┌─────────────────────────┐
            │            │   ImGui Layer           │
            │            │   (HTTP Client)         │
            │            └─────────┬───────────────┘
            │                      │
            │                      ▼
            │            ┌─────────────────────────┐
            │            │  CoreGFX Renderer       │
            │            │  (C++ Vulkan)           │
            │            │         │               │
            │            │         ├─► Arctic      │
            │            │         │   gRPC Client │
            │            └─────────┬───────────────┘
            │                      │         │
            │                      │         │ gRPC
            │                      │         ▼
            │                      │    ┌──────────────────┐
            │           ┌──────────▼──  │ Java gRPC Server │
            └───────────│ Cryo Tooling  │ (Qilak AI Agent) │
                        │ (HTTP Server) │                  │
                        └─────────────┤ │                  │
                                        └──────────────────┘
```

### Kommunikationsarten:
- **Java HTTP Server → ImGui Layer**: Event-basierte Kommunikation
- **Electron Tools → Cryo Tooling**: HTTP REST Requests für Studio-Funktionalität
- **Arctic gRPC Client → Java gRPC Server**: gRPC für AI-gestützte Rendering-Entscheidungen (Qilak AI Agent)

## Technologie-Stack

### C++ Layer
- **Rendering**: Vulkan via CoreGFX
- **UI**: ImGui
- **Networking**:
  - gRPC (Arctic gRPC Client)
  - HTTP Client/Server (Cryo Tooling)
- **Build System**: CMake

### Frontend Layer
- **Desktop Framework**: Electron
- **Server**: Java HTTP Server
- **Tools**: Electron-basierte Studio-Tools

## Features

- **Echtzeit-Rendering**: Vulkan-basiertes High-Performance Rendering
- **Studio-Integration**: Nahtlose Integration zwischen C++-Engine und Web-Tools
- **Event-System**: Bidirektionale Kommunikation zwischen Engine und Studio
- **Modulare Architektur**: Klare Trennung zwischen Foundation, Domain und Studio Layer
- **Tool-Erweiterbarkeit**: Electron-Tools können dynamisch geladen werden

## Abhängigkeiten

### C++ Komponenten
- `coregfx` (Arctic Foundation)
- `arctic-domain` Layer
- `arctic-grpc-client` (gRPC Client für Java Server Kommunikation)
  - Verwendet `qilak.proto` für Protobuf-Definitionen
  - Kommuniziert mit Qilak AI Agent (Java gRPC Server)
  - Integriert in CoreGFX Renderer
- `cryo-tooling`
- ImGui
- Vulkan SDK

### Java Komponenten
- Qilak AI Agent (Java gRPC Server)
- Java HTTP Server (Electron-ImGui Kommunikation)

### Studio Komponenten
- Electron
- Java Runtime
- Node.js

## Entwicklung

Details zur Entwicklung und Build-Prozess folgen in separaten Dokumenten.

## Lizenz

[Lizenzinformationen hier einfügen]