# OpenGL Quick Start

Get a triangle on screen in 5 minutes.

---

## 1. Install Dependencies

### Windows
Download and install:
- [Visual Studio Community](https://visualstudio.microsoft.com/) (select "Desktop C++ development")
- [GLFW](https://www.glfw.org/download.html) (64-bit Windows binaries)
- [GLAD](https://glad.dav1d.de/) (OpenGL 3.3, Core profile, C/C++)

### Linux
```bash
sudo apt install build-essential libglfw3-dev libgl1-mesa-dev
```
Then download GLAD from https://glad.dav1d.de/

### Mac
```bash
brew install glfw
```
Then download GLAD from https://glad.dav1d.de/

---

## 2. Set Up Project Folder

Put these files in one folder:
```
my-project/
    glad.c           (from GLAD download)
    glad/glad.h      (from GLAD download)
    KHR/khrplatform.h (from GLAD download)
    GLFW/glfw3.h     (from GLFW download)
    glfw3.lib        (Windows only, from GLFW download)
    mana_runtime.h   (from mana-lang/backend-cpp/)
```

---

## 3. Create triangle.mana

```mana
fn main() -> i32 {
    if !gl_init() { return -1; }

    gl_window_hint(GLFW_CONTEXT_VERSION_MAJOR(), 3);
    gl_window_hint(GLFW_CONTEXT_VERSION_MINOR(), 3);
    gl_window_hint(GLFW_OPENGL_PROFILE(), GLFW_OPENGL_CORE_PROFILE());

    let window = gl_create_window(800, 600, "Triangle");
    gl_make_context_current(window);
    if !gl_load_gl() { return -1; }
    gl_viewport(0, 0, 800, 600);

    let vs: u32 = gl_create_shader(GL_VERTEX_SHADER());
    gl_shader_source(vs, "#version 330 core
layout(location=0) in vec3 p;
void main(){gl_Position=vec4(p,1);}");
    gl_compile_shader(vs);

    let fs: u32 = gl_create_shader(GL_FRAGMENT_SHADER());
    gl_shader_source(fs, "#version 330 core
out vec4 c;
void main(){c=vec4(1,0.5,0.2,1);}");
    gl_compile_shader(fs);

    let prog: u32 = gl_create_program();
    gl_attach_shader(prog, vs);
    gl_attach_shader(prog, fs);
    gl_link_program(prog);
    gl_delete_shader(vs);
    gl_delete_shader(fs);

    let verts: Vec<f32> = [-0.5,-0.5,0.0, 0.5,-0.5,0.0, 0.0,0.5,0.0];
    let vao: u32 = gl_gen_vertex_array();
    let vbo: u32 = gl_gen_buffer();
    gl_bind_vertex_array(vao);
    gl_bind_buffer(GL_ARRAY_BUFFER(), vbo);
    gl_buffer_data_f32(GL_ARRAY_BUFFER(), verts, GL_STATIC_DRAW());
    gl_vertex_attrib_pointer(0, 3, 3, 0);
    gl_enable_vertex_attrib_array(0);

    while !gl_window_should_close(window) {
        if gl_get_key(window, GLFW_KEY_ESCAPE()) == GLFW_PRESS() {
            gl_set_window_should_close(window, true);
        }
        gl_clear_color(0.2, 0.3, 0.3, 1.0);
        gl_clear(GL_COLOR_BUFFER_BIT());
        gl_use_program(prog);
        gl_bind_vertex_array(vao);
        gl_draw_arrays(GL_TRIANGLES(), 0, 3);
        gl_swap_buffers(window);
        gl_poll_events();
    }

    gl_terminate();
    0
}
```

---

## 4. Compile and Run

### Step A: Compile Mana to C++
```bash
mana-compiler triangle.mana
```

### Step B: Compile C++ to executable

**Windows:**
```cmd
cl /DMANA_OPENGL_ENABLED /EHsc generated.cpp glad.c glfw3.lib opengl32.lib user32.lib gdi32.lib shell32.lib /Fe:triangle.exe
```

**Linux:**
```bash
g++ -DMANA_OPENGL_ENABLED generated.cpp glad.c -o triangle -lglfw -lGL -ldl
```

**Mac:**
```bash
g++ -DMANA_OPENGL_ENABLED generated.cpp glad.c -o triangle -lglfw -framework OpenGL
```

### Step C: Run
```bash
./triangle      # Linux/Mac
triangle.exe    # Windows
```

---

## Result

You should see:

```
┌────────────────────────────────┐
│                                │
│           ▲                    │
│          ╱ ╲                   │
│         ╱   ╲                  │
│        ╱     ╲                 │
│       ╱ ORANGE╲                │
│      ╱         ╲               │
│     ▔▔▔▔▔▔▔▔▔▔▔▔▔              │
│                                │
│      (teal background)         │
│                                │
└────────────────────────────────┘
```

Press **ESC** to close.

---

## Troubleshooting

| Problem | Solution |
|---------|----------|
| "Cannot find glad.h" | Put `glad/` folder in project directory |
| "Cannot find glfw3.h" | Put `GLFW/` folder in project directory |
| "Undefined reference to glfwInit" | Linux: `sudo apt install libglfw3-dev` |
| "Cannot find glfw3.lib" | Windows: Put `glfw3.lib` in project directory |
| Black screen | Check shader compile errors in console |

---

## What's Next?

See [opengl-guide.md](opengl-guide.md) for detailed explanations and more examples.
