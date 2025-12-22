# Mana Graphics Engine - TODO

## Current Status
- [x] GLFW window creation
- [x] OpenGL context initialization (GLAD)
- [x] Shader compilation and linking
- [x] VAO/VBO creation
- [x] Basic triangle rendering
- [x] Per-vertex colors

---

## Core Graphics Features

### Geometry Helpers
- [ ] `mana_createQuadVBO()` - Create a textured quad (useful for sprites, UI)
- [ ] `mana_createCubeVBO()` - Create a 3D cube with normals
- [ ] Generic mesh loading from vertex arrays
- [ ] Index buffer support (`glDrawElements`)
- [ ] Line and point rendering modes

### Texture Support
- [ ] `mana_glGenTexture()` - Generate texture ID
- [ ] `mana_glBindTexture()` - Bind texture
- [ ] `mana_glTexImage2D()` - Upload texture data
- [ ] `mana_glTexParameteri()` - Set texture parameters
- [ ] Image loading helper (PNG/JPG via stb_image)
- [ ] Texture atlas support
- [ ] Framebuffer objects (render-to-texture)

### Matrix/Transform Uniforms
- [ ] `mana_glUniformMatrix4fv()` - Upload 4x4 matrices
- [ ] Basic math library (Vec2, Vec3, Vec4, Mat4)
- [ ] Perspective/orthographic projection matrices
- [ ] View matrix (camera)
- [ ] Model matrix (position, rotation, scale)
- [ ] MVP matrix composition

### Input Handling
- [ ] `mana_glfwGetKey()` - Keyboard state polling
- [ ] `mana_glfwGetMouseButton()` - Mouse button state
- [ ] `mana_glfwGetCursorPos()` - Mouse position
- [ ] `mana_glfwSetInputMode()` - Cursor lock/hide
- [ ] Key callback system (event-based input)
- [ ] Mouse scroll callback
- [ ] Gamepad/controller support

---

## Advanced Features

### Rendering Pipeline
- [ ] Depth testing (`glEnable(GL_DEPTH_TEST)`)
- [ ] Blending/transparency (`glEnable(GL_BLEND)`)
- [ ] Face culling (`glEnable(GL_CULL_FACE)`)
- [ ] Wireframe mode
- [ ] Multiple render passes
- [ ] Post-processing effects

### Lighting
- [ ] Directional light
- [ ] Point lights
- [ ] Spot lights
- [ ] Ambient/diffuse/specular materials
- [ ] Normal mapping
- [ ] Shadow mapping

### 2D Rendering
- [ ] Sprite batch renderer
- [ ] Text rendering (bitmap fonts)
- [ ] UI system (buttons, panels)
- [ ] Particle system

### 3D Features
- [ ] Model loading (OBJ, glTF)
- [ ] Skeletal animation
- [ ] Instanced rendering
- [ ] Level-of-detail (LOD)

### Audio (via OpenAL or miniaudio)
- [ ] Sound loading (WAV, OGG)
- [ ] Sound playback
- [ ] 3D positional audio
- [ ] Music streaming

---

## Compiler Integration

### Auto Graphics Build
- [ ] Detect `extern fn mana_*` graphics functions
- [ ] Auto-include `mana_graphics.h`
- [ ] Auto-link GLFW/GLAD/OpenGL
- [ ] `mana build --graphics` flag

### Standard Library
- [ ] `std.graphics` module with all FFI bindings
- [ ] `std.math` module for vectors/matrices
- [ ] `std.input` module for keyboard/mouse

---

## Example Projects

### Demos to Create
- [ ] Spinning textured cube
- [ ] 2D sprite renderer
- [ ] Simple platformer game
- [ ] 3D first-person camera
- [ ] Particle effects demo
- [ ] UI/menu system

---

## Notes

### Building Graphics Projects
```cmd
cd examples/graphics
build.bat <your_file>.mana
build\Release\<your_file>.exe
```

### FFI Pattern
All graphics functions use the `mana_` prefix to avoid conflicts with C/OpenGL macros:
```mana
extern fn mana_glClearColor(r: f32, g: f32, b: f32, a: f32) -> void
```

### Pointer Handles
Use `i64` for opaque pointers (window handles, etc.):
```mana
let window: i64 = mana_glfwCreateWindow(800, 600, "Title", 0, 0)
```
