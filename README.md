# Magnum Game

Template game project for [mosra/magnum](/mosra/magnum)

## Building for web on MacOS

To set up the build environment, run once on the machine:

    build-emscripten-depenencies.sh

To then build the project

    build-release.sh

It will output to ~/www folder, which you can then serve with a simple web server:

    ( cd ~/www && python -m SimpleHTTPServer 8000 )

Then open a browser to [http://localhost:8000/magnum-game](http://localhost:8000/magnum-game)


## Developing

Make sure you've run the:

    build-emscripten-dependencies.sh

Open the project in CLion, and copy the cmake options from [build-release.sh](build-release.sh) into the CMake options in the CLion settings (except CMAKE_BUILD_TYPE).


## How To...

### Editing the level

Using Blender, there is the [models/levels-src/]level-*.blend files.

#### Tips:

 * Name objects -collider to use their mesh as a convex hull collider
 * Use 'Duplicate Linked' action to avoid duplicating meshes
 * Use an inactive collection if you want a palette of pieces 

#### Export

Use glTF 2.0 exporter with .glb format for most optimal performance.

 * Include Cameras to include a default camera position
 * Disable the Palette collection and only export Active Collection


## TO DO

* Desktop/Web - Add mouse capture within game
* Add sky box
* Try faster Freetype lib where available
* Platform support - Mobile? 
* Mobile - Add virtual joystick

## Credits

* https://www.kenney.nl/assets/platformer-kit
* https://www.kenney.nl/assets/mini-characters-1