# Mana Game Engine - Feature Roadmap

## Completed Features

### Rendering
- [x] OpenGL 3.3 Core Profile
- [x] Shader compilation and linking
- [x] Vertex buffers and VAOs
- [x] Texture loading (BMP, TGA, PPM)
- [x] Texture parameters and mipmaps
- [x] Blending and depth testing

### Math Library
- [x] Vec2, Vec3, Vec4
- [x] Mat3, Mat4
- [x] Quaternions
- [x] Transform matrices (translate, rotate, scale)
- [x] Projection matrices (perspective, ortho)
- [x] View matrices (look_at, fps_view)
- [x] Color utilities (HSV, hex)

### Input
- [x] Keyboard input
- [x] Mouse position and buttons
- [x] Mouse delta tracking (FPS camera)
- [x] Cursor modes (normal, hidden, disabled)
- [x] Scroll wheel
- [x] Raw mouse motion
- [x] Gamepad/controller support
- [x] Gamepad buttons and axes
- [x] Deadzone handling

### Audio
- [x] WAV file loading
- [x] Sound effects (play, stop, loop)
- [x] Music playback (play, pause, resume, seek)
- [x] Volume control (master, sound, music)
- [x] Stereo panning
- [x] 3D positional audio
- [x] Procedural sound generation (tones, noise)

### Networking
- [x] UDP sockets

### Threading
- [x] Thread spawning
- [x] Mutexes
- [x] Channels
- [x] Thread pools

### Crypto
- [x] SHA-256
- [x] MD5

---

## TODO - Future Features

### Physics
- [ ] AABB collision detection
- [ ] Circle collision detection
- [ ] Polygon collision detection
- [ ] Collision response
- [ ] Rigid body dynamics
- [ ] Raycasting

### Sprite/2D Rendering
- [ ] Sprite struct (texture, UV, size)
- [ ] Sprite batching for performance
- [ ] Sprite sheets / texture atlases
- [ ] Animation frames
- [ ] Sprite sorting (z-order)
- [ ] 2D camera

### Text/Font Rendering
- [ ] Bitmap font loading
- [ ] Text rendering
- [ ] TTF/OTF font support (via stb_truetype)
- [ ] Text alignment
- [ ] Word wrapping

### UI/GUI System
- [ ] UI canvas / coordinate system
- [ ] Buttons (hover, click, disabled states)
- [ ] Labels / text display
- [ ] Panels / containers
- [ ] Text input fields
- [ ] Sliders
- [ ] Checkboxes
- [ ] Layout system (horizontal, vertical, grid)
- [ ] UI theming / styling

### Particle System
- [ ] Particle emitter
- [ ] Particle properties (lifetime, velocity, color, size)
- [ ] Emission shapes (point, circle, rectangle)
- [ ] Particle physics (gravity, drag)
- [ ] Color/size over lifetime
- [ ] Texture particles
- [ ] Particle pooling

### Entity/Component System
- [ ] Entity creation/destruction
- [ ] Component attachment
- [ ] System iteration
- [ ] Entity queries
- [ ] Parent-child hierarchies

### Scene Management
- [ ] Scene struct
- [ ] Scene loading/unloading
- [ ] Scene transitions (fade, etc.)
- [ ] Scene stack (push/pop)

### Animation
- [ ] Tween functions (ease in/out/both)
- [ ] Easing curves (quad, cubic, elastic, bounce)
- [ ] Property animation
- [ ] Skeletal animation
- [ ] Animation blending
- [ ] Animation events

### Pathfinding
- [ ] A* algorithm
- [ ] Grid-based pathfinding
- [ ] Navigation mesh
- [ ] Path smoothing
- [ ] Dynamic obstacles

### State Machine
- [ ] State struct
- [ ] State transitions
- [ ] Enter/exit/update callbacks
- [ ] Hierarchical states
- [ ] AI behavior trees

### Utilities
- [ ] Timer (countdown, interval)
- [ ] Delta time helpers
- [ ] FPS counter
- [ ] Input action mapping (bind keys to named actions)
- [ ] Logging system with levels (debug, info, warn, error)
- [ ] Performance profiling
- [ ] Debug drawing (lines, shapes, text)

### Serialization
- [ ] JSON parsing
- [ ] JSON writing
- [ ] Binary serialization
- [ ] Save/load game state
- [ ] Asset manifest

### Asset Management
- [ ] Asset loading queue
- [ ] Async asset loading
- [ ] Asset caching
- [ ] Hot reloading

---

## Examples Created

- `opengl_triangle.mana` - Basic triangle rendering
- `opengl_cube.mana` - 3D rotating cube with MVP matrices
- `opengl_camera.mana` - FPS camera with keyboard controls
- `opengl_lighting.mana` - Phong lighting demo
- `opengl_texture.mana` - Texture loading and rendering
- `opengl_mouse_camera.mana` - Mouse look FPS camera
- `opengl_gamepad.mana` - Gamepad controller input
- `audio_demo.mana` - Audio playback demonstration

---

## Notes

### Conditional Compilation
- `MANA_OPENGL_ENABLED` - Enables OpenGL/GLFW support
- `MANA_AUDIO_ENABLED` - Enables audio system

### Dependencies
- GLFW 3.x (window/input)
- OpenGL 3.3+ (rendering)
- Windows: winmm.lib (audio)

### Platform Support
- Windows (primary, fully tested)
- Linux/macOS (OpenGL works, audio needs platform backends)
