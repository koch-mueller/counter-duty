# Third-party notices

This project contains or depends on the following third-party software and assets. Each item remains subject to its own license.

## Software

| Component | Use | License | Source |
| --- | --- | --- | --- |
| ReactPhysics3D | Rigid-body physics | zlib License | [reactphysics3d.com](https://www.reactphysics3d.com/) |
| miniaudio 0.11.25 by David Reid | Audio playback | Public Domain or MIT No Attribution | [miniaud.io](https://miniaud.io/) |
| GLFW | Window and input handling | zlib/libpng License | [glfw.org](https://www.glfw.org/) |
| GLEW | OpenGL extension loading | Modified BSD, MIT, and Khronos licenses | [glew.sourceforge.net](https://glew.sourceforge.net/) |
| Assimp | 3D model import | BSD 3-Clause License | [assimp.org](https://www.assimp.org/) |
| FreeImage | Image loading | FreeImage Public License 1.0 | [freeimage.sourceforge.io](https://freeimage.sourceforge.io/) |

The FreeImage library is used in this project. FreeImage is an open-source project maintained by the FreeImage development team. See the project website for copyright information and the full FreeImage Public License.

Dependencies other than the vendored miniaudio source are restored through the checked-in vcpkg manifest rather than redistributed as compiled libraries in this repository.

## 3D models and textures

### Counter

- **Counter** by [alixbaur](https://sketchfab.com/alixbaur)
- Source: [Sketchfab](https://sketchfab.com/3d-models/counter-1bb0a8db17b44074b8e498bfde3874cc)
- License: [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/)
- Used as the checkout counter environment model.

### Grocery products

- **Groceries Pack! Low poly assets for your games!** by [tulex_art / Cassio Fernandes](https://sketchfab.com/CassioFernandes)
- Source: [Sketchfab](https://sketchfab.com/3d-models/groceries-pack-low-poly-assets-for-your-games-46d7eb7049ac4becae4258518f86818d)
- License: [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/)
- Selected product models are used for the supermarket inventory.

### Mop and bucket

- **Mop & Bucket** by J-Toastie
- Source: [Poly Pizza](https://poly.pizza/m/kWobe0q7tf)
- License: Creative Commons Attribution
- Adapted for the in-game cleaning station and mop.

### Surface materials

- **Interior Tiles**, **Beige Wall 001**, and **Ceiling Interior** from [Poly Haven](https://polyhaven.com/)
- License: [CC0](https://creativecommons.org/publicdomain/zero/1.0/)
- Used for the floor, walls, and ceiling.

### Spill textures

- Puddle/decal source textures from [TextureCan](https://www.texturecan.com/)
- License: CC0
- Adapted into the spill material and opacity mask.

## Redistributable fallback assets

The following replacements were created specifically for this repository:

- Code-generated shelf geometry
- Code-generated order terminal
- Code-generated ceiling-light fixtures
- Procedurally generated background music, intro chime, and scanner beep
- Procedurally generated barcode texture

The code can optionally load the project's original terminal, shelf, ceiling-light, and audio files from `assets/local`. That directory is excluded from version control, and those raw files are not distributed by this repository.
