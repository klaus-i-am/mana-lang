# Creating Graphics with Mana and OpenGL

This guide will teach you how to create graphics (like video games or visual applications) using the Mana programming language.

---

## Table of Contents

1. [What You Will Learn](#what-you-will-learn)
2. [What You Need Before Starting](#what-you-need-before-starting)
3. [Setting Up Your Computer](#setting-up-your-computer)
4. [Your First Graphics Program](#your-first-graphics-program)
5. [Understanding the Code](#understanding-the-code)
6. [Compiling and Running](#compiling-and-running)
7. [Common Problems and Solutions](#common-problems-and-solutions)
8. [Available Functions Reference](#available-functions-reference)
9. [Next Steps](#next-steps)

---

## What You Will Learn

By the end of this guide, you will be able to:
- Create a window on your screen
- Draw shapes (like triangles)
- Handle keyboard input (like pressing ESC to close)
- Understand the basics of how graphics work

**What is OpenGL?**

OpenGL is a tool that lets your program talk to your computer's graphics card. It's what video games use to draw things on screen. Mana includes built-in support for OpenGL, making it easier to create graphics.

---

## What You Need Before Starting

Before you can create graphics with Mana, you need to install some software on your computer.

### Required Software

| Software | What It Does | Where to Get It |
|----------|--------------|-----------------|
| Mana Compiler | Turns your Mana code into a program | Already included in this project |
| C++ Compiler | Builds the final program | See instructions below |
| GLFW | Creates windows and handles input | See instructions below |
| GLAD | Loads OpenGL functions | See instructions below |

---

## Setting Up Your Computer

Follow the instructions for your operating system (Windows, Mac, or Linux).

### Windows Setup

#### Step 1: Install Visual Studio

Visual Studio is a free program that includes a C++ compiler.

1. Go to https://visualstudio.microsoft.com/downloads/
2. Download "Visual Studio Community" (it's free)
3. Run the installer
4. When asked what to install, check the box that says **"Desktop development with C++"**
5. Click Install and wait for it to finish

#### Step 2: Download GLFW

GLFW is a library that creates windows and handles keyboard/mouse input.

1. Go to https://www.glfw.org/download.html
2. Click on "64-bit Windows binaries"
3. Extract the downloaded ZIP file
4. Inside, find the folder `lib-vc2022` (or similar)
5. Copy `glfw3.lib` to your project folder
6. Find the `include` folder and copy the `GLFW` folder to your project

#### Step 3: Download GLAD

GLAD loads OpenGL functions so your program can use them.

1. Go to https://glad.dav1d.de/
2. Set these options:
   - Language: **C/C++**
   - Specification: **OpenGL**
   - API gl: **Version 3.3** (or higher)
   - Profile: **Core**
3. Click "Generate"
4. Download the ZIP file
5. Extract it and copy:
   - `glad.c` to your project folder
   - The `glad` and `KHR` folders (from `include`) to your project

#### Your Project Folder Should Look Like This:

```
your-project/
    glad.c
    glad/
        glad.h
    KHR/
        khrplatform.h
    GLFW/
        glfw3.h
    glfw3.lib
    mana_runtime.h
    your_program.mana
```

---

### Linux Setup (Ubuntu/Debian)

Open a terminal and run these commands:

```bash
# Update your package list
sudo apt update

# Install the C++ compiler
sudo apt install build-essential

# Install GLFW
sudo apt install libglfw3-dev

# Install OpenGL development files
sudo apt install libgl1-mesa-dev
```

For GLAD, follow the same download steps as Windows (Step 3 above).

---

### Mac Setup

1. Install Xcode from the App Store (this includes a C++ compiler)
2. Open Terminal and install Homebrew if you don't have it:
   ```bash
   /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
   ```
3. Install GLFW:
   ```bash
   brew install glfw
   ```
4. Download GLAD (same as Windows Step 3)

---

## Your First Graphics Program

Let's create a simple program that shows a colored triangle on the screen.

### Step 1: Create the Mana File

Create a new file called `triangle.mana` and copy this code into it:

```mana
// My First OpenGL Program in Mana
// This program draws an orange triangle on a teal background

fn main() -> i32 {
    // Step 1: Start up the graphics system
    if !gl_init() {
        println("Error: Could not start graphics system");
        return -1;
    }

    // Step 2: Tell the system what version of OpenGL we want
    gl_window_hint(GLFW_CONTEXT_VERSION_MAJOR(), 3);
    gl_window_hint(GLFW_CONTEXT_VERSION_MINOR(), 3);
    gl_window_hint(GLFW_OPENGL_PROFILE(), GLFW_OPENGL_CORE_PROFILE());

    // Step 3: Create a window (800 pixels wide, 600 pixels tall)
    let window = gl_create_window(800, 600, "My First Triangle");
    gl_make_context_current(window);

    // Step 4: Load OpenGL functions
    if !gl_load_gl() {
        println("Error: Could not load OpenGL");
        return -1;
    }

    // Step 5: Set the drawing area size
    gl_viewport(0, 0, 800, 600);

    // Step 6: Create the shader program (tells GPU how to draw)
    let shader: u32 = create_simple_shader();

    // Step 7: Define our triangle's corners (called vertices)
    // Each corner has 3 numbers: x position, y position, z position
    // x: -1 is left edge, +1 is right edge, 0 is center
    // y: -1 is bottom, +1 is top, 0 is center
    // z: we use 0 for 2D graphics
    let vertices: Vec<f32> = [
        -0.5, -0.5, 0.0,    // Bottom-left corner
         0.5, -0.5, 0.0,    // Bottom-right corner
         0.0,  0.5, 0.0     // Top-center corner
    ];

    // Step 8: Send the triangle data to the graphics card
    let vao: u32 = gl_gen_vertex_array();
    let vbo: u32 = gl_gen_buffer();

    gl_bind_vertex_array(vao);
    gl_bind_buffer(GL_ARRAY_BUFFER(), vbo);
    gl_buffer_data_f32(GL_ARRAY_BUFFER(), vertices, GL_STATIC_DRAW());
    gl_vertex_attrib_pointer(0, 3, 3, 0);
    gl_enable_vertex_attrib_array(0);

    println("Window opened! Press ESC to close.");

    // Step 9: The main loop - runs until you close the window
    while !gl_window_should_close(window) {

        // Check if ESC key is pressed
        if gl_get_key(window, GLFW_KEY_ESCAPE()) == GLFW_PRESS() {
            gl_set_window_should_close(window, true);
        }

        // Clear the screen with a teal color (red=0.2, green=0.3, blue=0.3)
        gl_clear_color(0.2, 0.3, 0.3, 1.0);
        gl_clear(GL_COLOR_BUFFER_BIT());

        // Draw the triangle
        gl_use_program(shader);
        gl_bind_vertex_array(vao);
        gl_draw_arrays(GL_TRIANGLES(), 0, 3);

        // Show what we drew on screen
        gl_swap_buffers(window);

        // Check for keyboard/mouse events
        gl_poll_events();
    }

    // Step 10: Clean up when done
    gl_delete_vertex_array(vao);
    gl_delete_buffer(vbo);
    gl_delete_program(shader);
    gl_terminate();

    println("Goodbye!");
    0
}

// This function creates a shader program
// Shaders are small programs that run on your graphics card
fn create_simple_shader() -> u32 {
    // Vertex shader: decides WHERE to draw each corner
    let vertex_code: string = "#version 330 core
layout (location = 0) in vec3 aPos;
void main() {
    gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
}";

    // Fragment shader: decides WHAT COLOR to draw
    let fragment_code: string = "#version 330 core
out vec4 FragColor;
void main() {
    FragColor = vec4(1.0, 0.5, 0.2, 1.0);
}";

    // Compile the vertex shader
    let vertex_shader: u32 = gl_create_shader(GL_VERTEX_SHADER());
    gl_shader_source(vertex_shader, vertex_code);
    gl_compile_shader(vertex_shader);

    // Check for errors
    if !gl_get_shader_compile_status(vertex_shader) {
        println(f"Vertex shader error: {gl_get_shader_info_log(vertex_shader)}");
    }

    // Compile the fragment shader
    let fragment_shader: u32 = gl_create_shader(GL_FRAGMENT_SHADER());
    gl_shader_source(fragment_shader, fragment_code);
    gl_compile_shader(fragment_shader);

    // Check for errors
    if !gl_get_shader_compile_status(fragment_shader) {
        println(f"Fragment shader error: {gl_get_shader_info_log(fragment_shader)}");
    }

    // Link both shaders into one program
    let program: u32 = gl_create_program();
    gl_attach_shader(program, vertex_shader);
    gl_attach_shader(program, fragment_shader);
    gl_link_program(program);

    // Check for errors
    if !gl_get_program_link_status(program) {
        println(f"Shader program error: {gl_get_program_info_log(program)}");
    }

    // Clean up individual shaders (we don't need them after linking)
    gl_delete_shader(vertex_shader);
    gl_delete_shader(fragment_shader);

    program
}
```

---

## Understanding the Code

Let's break down what each part does:

### The Main Loop

Every graphics program has a "main loop" that runs over and over until you close the window:

```
┌─────────────────────────────────────────┐
│           START PROGRAM                 │
└────────────────┬────────────────────────┘
                 │
                 ▼
┌─────────────────────────────────────────┐
│  Set up window, shaders, triangle data  │
└────────────────┬────────────────────────┘
                 │
                 ▼
        ┌────────────────┐
        │  MAIN LOOP     │◄──────────────┐
        └───────┬────────┘               │
                │                        │
                ▼                        │
┌─────────────────────────────────────┐  │
│  1. Check for keyboard input        │  │
│  2. Clear the screen                │  │
│  3. Draw the triangle               │  │
│  4. Show the result                 │  │
│  5. Check for window events         │  │
└───────────────┬─────────────────────┘  │
                │                        │
                ▼                        │
        ┌───────────────┐                │
        │ Window open?  │────YES─────────┘
        └───────┬───────┘
                │ NO
                ▼
┌─────────────────────────────────────────┐
│           Clean up and exit             │
└─────────────────────────────────────────┘
```

### Coordinate System

OpenGL uses a coordinate system where:
- **X axis**: -1.0 (left edge) to +1.0 (right edge)
- **Y axis**: -1.0 (bottom) to +1.0 (top)
- **Center of screen**: (0, 0)

```
                    +1.0 (top)
                       │
                       │
    -1.0 (left) ───────┼─────── +1.0 (right)
                       │
                       │
                    -1.0 (bottom)
```

### Colors

Colors are specified with 4 numbers (RGBA):
- **R** (Red): 0.0 to 1.0
- **G** (Green): 0.0 to 1.0
- **B** (Blue): 0.0 to 1.0
- **A** (Alpha/Transparency): 0.0 (invisible) to 1.0 (solid)

Examples:
- Red: `(1.0, 0.0, 0.0, 1.0)`
- Green: `(0.0, 1.0, 0.0, 1.0)`
- Blue: `(0.0, 0.0, 1.0, 1.0)`
- White: `(1.0, 1.0, 1.0, 1.0)`
- Black: `(0.0, 0.0, 0.0, 1.0)`
- Orange: `(1.0, 0.5, 0.2, 1.0)`

---

## Compiling and Running

### Step 1: Compile Your Mana Code

Open a terminal/command prompt in your project folder and run:

```bash
mana-compiler triangle.mana
```

This creates a file called `generated.cpp`.

### Step 2: Compile the C++ Code

#### On Windows (Command Prompt or PowerShell):

```cmd
cl /DMANA_OPENGL_ENABLED /EHsc generated.cpp glad.c /I. glfw3.lib opengl32.lib user32.lib gdi32.lib shell32.lib /Fe:triangle.exe
```

#### On Linux:

```bash
g++ -DMANA_OPENGL_ENABLED generated.cpp glad.c -o triangle -lglfw -lGL -ldl -lm
```

#### On Mac:

```bash
g++ -DMANA_OPENGL_ENABLED generated.cpp glad.c -o triangle -lglfw -framework OpenGL
```

### Step 3: Run Your Program

#### On Windows:
```cmd
triangle.exe
```

#### On Linux/Mac:
```bash
./triangle
```

You should see a window with an orange triangle on a teal background!

---

## Common Problems and Solutions

### "Cannot find glfw3.h" or "Cannot find glad.h"

**Problem**: The compiler cannot find the required files.

**Solution**: Make sure the header files are in the right place:
- `GLFW/glfw3.h` should exist in your project folder
- `glad/glad.h` should exist in your project folder

### "Cannot find glfw3.lib" or "undefined reference to glfwInit"

**Problem**: The linker cannot find the GLFW library.

**Solution**:
- Windows: Make sure `glfw3.lib` is in your project folder
- Linux: Run `sudo apt install libglfw3-dev`
- Mac: Run `brew install glfw`

### "GLFW_CONTEXT_VERSION_MAJOR not found" or similar

**Problem**: You forgot to enable OpenGL in the compiler command.

**Solution**: Make sure you include `-DMANA_OPENGL_ENABLED` in your compile command.

### Black screen (no triangle)

**Problem**: The shaders might have errors.

**Solution**: Check the console output for shader error messages. Make sure the shader code is exactly as shown above.

### Window opens then immediately closes

**Problem**: An error occurred during setup.

**Solution**: Look at the console output for error messages like "Could not start graphics system" or "Could not load OpenGL".

---

## Available Functions Reference

Here is a list of all OpenGL functions available in Mana:

### Window Functions

| Function | What It Does |
|----------|--------------|
| `gl_init()` | Starts up the graphics system. Returns `true` if successful. |
| `gl_terminate()` | Shuts down the graphics system. Call this before your program ends. |
| `gl_create_window(width, height, title)` | Creates a window with the given size and title. |
| `gl_window_should_close(window)` | Returns `true` if the user wants to close the window. |
| `gl_set_window_should_close(window, value)` | Set to `true` to close the window. |
| `gl_swap_buffers(window)` | Shows what you've drawn on screen. |
| `gl_poll_events()` | Checks for keyboard and mouse input. |
| `gl_window_hint(hint, value)` | Sets options before creating a window. |
| `gl_make_context_current(window)` | Makes the window ready for drawing. |
| `gl_load_gl()` | Loads OpenGL functions. Returns `true` if successful. |

### Input Functions

| Function | What It Does |
|----------|--------------|
| `gl_get_key(window, key)` | Checks if a key is pressed. Returns `GLFW_PRESS()` or `GLFW_RELEASE()`. |
| `gl_get_time()` | Returns the time since the program started (in seconds). |

### Drawing Setup Functions

| Function | What It Does |
|----------|--------------|
| `gl_viewport(x, y, width, height)` | Sets the area of the window to draw in. |
| `gl_clear_color(r, g, b, a)` | Sets the background color (0.0 to 1.0 for each). |
| `gl_clear(mask)` | Clears the screen. Use `GL_COLOR_BUFFER_BIT()`. |
| `gl_enable(feature)` | Turns on a feature (like depth testing). |
| `gl_disable(feature)` | Turns off a feature. |

### Shader Functions

| Function | What It Does |
|----------|--------------|
| `gl_create_shader(type)` | Creates a new shader. Type is `GL_VERTEX_SHADER()` or `GL_FRAGMENT_SHADER()`. |
| `gl_shader_source(shader, code)` | Sets the shader's code. |
| `gl_compile_shader(shader)` | Compiles the shader. |
| `gl_get_shader_compile_status(shader)` | Returns `true` if compilation succeeded. |
| `gl_get_shader_info_log(shader)` | Gets error messages if compilation failed. |
| `gl_delete_shader(shader)` | Deletes a shader you no longer need. |
| `gl_create_program()` | Creates a new shader program. |
| `gl_attach_shader(program, shader)` | Attaches a shader to a program. |
| `gl_link_program(program)` | Links all attached shaders together. |
| `gl_get_program_link_status(program)` | Returns `true` if linking succeeded. |
| `gl_get_program_info_log(program)` | Gets error messages if linking failed. |
| `gl_use_program(program)` | Activates a shader program for drawing. |
| `gl_delete_program(program)` | Deletes a program you no longer need. |

### Buffer Functions

| Function | What It Does |
|----------|--------------|
| `gl_gen_vertex_array()` | Creates a VAO (stores vertex setup). |
| `gl_gen_buffer()` | Creates a VBO (stores vertex data). |
| `gl_bind_vertex_array(vao)` | Activates a VAO. |
| `gl_bind_buffer(target, buffer)` | Activates a buffer. Use `GL_ARRAY_BUFFER()` for vertices. |
| `gl_buffer_data_f32(target, data, usage)` | Sends float data to the buffer. |
| `gl_vertex_attrib_pointer(index, size, stride, offset)` | Describes the vertex data format. |
| `gl_enable_vertex_attrib_array(index)` | Enables a vertex attribute. |
| `gl_delete_vertex_array(vao)` | Deletes a VAO. |
| `gl_delete_buffer(vbo)` | Deletes a buffer. |

### Drawing Functions

| Function | What It Does |
|----------|--------------|
| `gl_draw_arrays(mode, first, count)` | Draws shapes. Mode is usually `GL_TRIANGLES()`. |
| `gl_draw_elements(mode, count, type, offset)` | Draws indexed shapes. |

### Keyboard Constants

| Constant | Key |
|----------|-----|
| `GLFW_KEY_ESCAPE()` | ESC key |
| `GLFW_KEY_SPACE()` | Spacebar |
| `GLFW_KEY_W()` | W key |
| `GLFW_KEY_A()` | A key |
| `GLFW_KEY_S()` | S key |
| `GLFW_KEY_D()` | D key |
| `GLFW_KEY_UP()` | Up arrow |
| `GLFW_KEY_DOWN()` | Down arrow |
| `GLFW_KEY_LEFT()` | Left arrow |
| `GLFW_KEY_RIGHT()` | Right arrow |
| `GLFW_PRESS()` | Key is currently pressed |
| `GLFW_RELEASE()` | Key is not pressed |

---

## Next Steps

Now that you can draw a triangle, here are some things to try:

### 1. Change the Triangle Color

In the `create_simple_shader` function, find this line in the fragment shader:
```
FragColor = vec4(1.0, 0.5, 0.2, 1.0);
```
Change the numbers to create different colors!

### 2. Change the Triangle Size

In the `vertices` array, change the numbers to make the triangle bigger or smaller:
```mana
let vertices: Vec<f32> = [
    -0.8, -0.8, 0.0,    // Make these bigger for a larger triangle
     0.8, -0.8, 0.0,
     0.0,  0.8, 0.0
];
```

### 3. Draw a Square (Two Triangles)

A square is made of two triangles:
```mana
let vertices: Vec<f32> = [
    // First triangle
    -0.5, -0.5, 0.0,
     0.5, -0.5, 0.0,
     0.5,  0.5, 0.0,
    // Second triangle
    -0.5, -0.5, 0.0,
     0.5,  0.5, 0.0,
    -0.5,  0.5, 0.0
];
```
Don't forget to change `gl_draw_arrays(GL_TRIANGLES(), 0, 3)` to `gl_draw_arrays(GL_TRIANGLES(), 0, 6)` since you now have 6 vertices!

### 4. Add Keyboard Controls

Make something happen when you press a key:
```mana
if gl_get_key(window, GLFW_KEY_SPACE()) == GLFW_PRESS() {
    // Change background color when space is pressed
    gl_clear_color(1.0, 0.0, 0.0, 1.0);  // Red background
}
```

---

## Getting Help

If you run into problems:

1. Check the [Common Problems](#common-problems-and-solutions) section above
2. Make sure all files are in the correct locations
3. Read any error messages carefully - they often tell you exactly what's wrong
4. Visit https://learnopengl.com for more OpenGL tutorials

Happy coding!
