# Magnum Game

Template game project for mosra/magnum

## Building for web on MacOS

To set up the build environemnt, run once on the machine:

    build-emscripten-depenencies.sh

To then build the project

    build-release.sh

It will output to ~/www folder, which you can then serve with a simple web server:

    ( cd ~/www && python -m SimpleHTTPServer 8000 )

Then open a browser to [http://localhost:8000/namepicker](http://localhost:8000/namepicker)


## Developing

Make sure you've run the:

    build-emscripten-depenencies.sh

Open the project in CLion, and copy the cmake options from [build-release.sh](build-release.sh) into the CMake options in the CLion settings (except CMAKE_BUILD_TYPE).


## TO DO

* Strip the old name picker code
* Load environment model
* Load animated player model
* Movement controls
* Shadow caster/receiver drawables
