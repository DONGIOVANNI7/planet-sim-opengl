# OpenGL Planetary System Simulation 🪐

A 3D graphics simulation built with C++ and Legacy OpenGL (GLUT). This project models a heliocentric-style system where a 
central planet travels in a circular orbit while acting as a dynamic point-light source for six orbiting satellites (cubes).

## Technical Features
- **Custom OBJ Loader:** Parses `.obj` files from scratch (Vertices, Normals, UVs) without relying on heavy external libraries like Assimp.
- **Hierarchical Transformations:** Uses the OpenGL Matrix Stack to calculate complex compound motions (cubes orbit the moving planet while rotating on their own axes).
- **Dynamic Lighting:** Implements the Blinn-Phong lighting model. The `GL_LIGHT0` position is updated every frame to match the planet, creating realistic shadows on the dark sides of the cubes.
- **Texture Mapping:** Integrates the lightweight `stb_image.h` library to map 2D images onto 3D geometry without distortion.
- **Orbit Camera:** Features a spherical coordinate camera system for smooth 3D navigation.

##  Prerequisites (Linux / Ubuntu)
To compile and run this project, you need the GCC compiler and the FreeGLUT development libraries.
```bash
sudo apt update
sudo apt install build-essential freeglut3-dev
