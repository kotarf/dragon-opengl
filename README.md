# Dragon OpenGL

Renders variants of the Stanford Dragon (1996) using up-to-date OpenGL code. [GLFW](https://www.glfw.org/) is used as the window manager.
[glad](https://github.com/Dav1dde/glad) is used as the context loader.

This is a university project and is not productionized, but
could serve as a fair "starter" project since many concepts and examples from various OpenGL tutorials are used.

## Credits
### Meshes
The included meshes and textures were made by talented researchers and artists.

**Original Stanford Dragon**

This is the origin of the dragon model and the bunny. Credit to Stanford University for creating
the bunny and dragon models from scans in 1993 and 1996, respectively.

[Original Stanford Dragon](http://graphics.stanford.edu/data/3Dscanrep/)

**Updated Stanford Dragon**

Morgan McGuire reconstructed a model in 2011 from a rescan graciously provided by Georgia Tech. It contains more triangles than the original model.

Morgan McGuire, Computer Graphics Archive, July 2017 (https://casual-effects.com/data)

**Baked Stanford Dragon**

Prefix Studios in 2022 adapted the reconstructed Stanford Dragon by Morgan McGuire to be PBR baked with an accompanying texture and normal mapping. It is a very nice looking model and is the primary mesh used in this project.
The model is licensed under CC 4.0 and I converted it to wavefront via Blender.

[Stanford Dragon PBR on Sketchfab by Prefix Studios](https://sketchfab.com/3d-models/stanford-dragon-pbr-5d610f842a4542ccb21613d41bbd7ea1)

[Prefix Studios](https://prefixstudios.com/hackmans/)

[Creative Commons 4.0 License](https://creativecommons.org/licenses/by/4.0/)

### Libraries
#### libigl
A geometry and graphics processing library [libigl](https://github.com/libigl/libigl) is used to facilitate loading 3D mesh files.
#### stb_image
The [stb_image](https://github.com/nothings/stb/blob/master/stb_image.h) library is used for saving output to an image.

### References
Several tutorials were critical for my productivity.

Fleischhacker, Markus. “GitHub.” https://github.com/mfl28/opengl-cpp-starter.

Overvoorde, Alexander. “open.gl.” https://open.gl/.

Vries, Joey de. “Learn OpenGL.” https://learnopengl.com/.

## Build
The included CMake scripts in the cmake subdirectory should fetch all the requirements necessary to run the code on Linux,
provided the latest OpenGL profile is supported.

### CachyOS
I tested on CachyOS, an Arch-based distro. It should have everything necessary out of the box, otherwise
this [wiki](https://wiki.archlinux.org/title/OpenGL) is worth checking out.

### Ubuntu
The package *xorg-dev* may be necessary.

### Windows
Only tested with bundled mingw in CLion.

```bash
mkdir build
cd build/
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```
### Meshes
The meshes must be extracted into the *data* directory to run.

### Run
The first argument is the model, one of:
* dragon
* dragon_off
* bunny

The second argument is extra functionality, and one of:
* image
* flat
* wireframe

If *image*, the application will dump the framebuffer and exit.

Flat and wireframe are additional rendering modes.

If no arguments are provided, the textured dragon will be rendered.

Examples:
```bash
dragon-opengl dragon_off
```

```bash
dragon-opengl bunny flat
```

**Controls**

There are some very basic controls implemented.

* Scrolling (via the mouse wheel) zooms in and out.
* Arrow keys perform a rotation of the model.
