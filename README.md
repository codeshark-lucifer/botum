- OpenGL ( Rendering API ) ✓
- Win32 ( Window & Input ) ✓
- GLAD ( Load OpenGL functions ) ✓
- Utils ( Math, Vectors, Matrices ) ✓
- stb_image ( Texture Loading ) ✓
- ImGui ( Debugging UI ) * Optionsl 

+ Compiler ( MinGW ) ✓
+ CMakeLists for configure ✓

+ Structure Project 
- Core ✓
- Renderer
- Math ✓
- ECS
- Resources
- Game

- utils.h ( for customize helper datatype )

* Core Systems:
- Load Window  ✓
- Load Glad  ✓
- Create Context OpenGL ✓
- Game Loop ✓
- Game Machanic

* Rendering System (2D)
- Vertex Buffer
- Index Buffer 
- Vertex Array Object
- Shader Class

** Entity Component System
- Entity: transform, sprite

** Resource Manager