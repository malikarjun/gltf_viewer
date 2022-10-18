# glTF viewer

Viewer to visualize glTF files using OpenGL. 

## Dependencies
- glad
- glfw
- tinygltf
- glm

## Getting started
Tested on MacBook (M1) running MacOS 12.6. 

To build the project
```
mkdir build
cd build
cmake ..
make
```

To test a sample scene
```
./gltf_viewer ../scene/separate/assets.gltf
```

Once the windows is open, `w`, `s`, `a`, `d` keys and cursor can be used to navigate the scene. `o` can be pressed to
take a screenshot of the window. The screenshot is saved as out.png in the `build` folder. 

