This is an HTTP server that uses D++. It listens to requests and if they come from top.gg, handles them as a vote, otherwise tries to pull the requested resource from `resources/`.

### Running

#### Compiling

Simply create a build directory from the directory the readme is in:

    mkdir build
    cd build

With CMake, generate the compilation files and build with make:

    cmake ..
    make -j

This assumes that Henifig has already been built by Guiding Light and FMT exists in a searchable path.

#### Launching

From the build directory, simply run:

    ./topgg_listener

### Serving

You can serve anything in the `resources` directory.

#### NOTE: The lookup is case-insensitive, so if you have something like `resources/ReSoUrCe` and `resources/resource`, the one found first will be served.

If no resource is found, 404 is returned.
