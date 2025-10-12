# Render Targets / Output Control

**Kategorie 11**: Zielorte für Renderings

## Zweck
- Offscreen Render Targets
- Multiview / VR Support
- Dynamische Auflösung
- Mipmap-Generierung
- Framebuffer-Management

## Geplante Proto-Dateien
- `cryo_render_target.proto` - Render target definitions
- `cryo_framebuffer.proto` - Framebuffer layouts
- `cryo_multiview.proto` - VR/multiview support
- `cryo_output_control.proto` - Output resolution and format control

## Semantische Verben
- `CreateRenderTarget`
- `SetOutputResolution`
- `BindRenderTarget`
- `GenerateMipmaps`
- `SetMultiviewMode`