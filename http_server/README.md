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

#### Configuring

Before listening, you need to create a configuration file near this README, called `config.hfg`. Create one and put this in:

```hfg
/TOPGG_WEBHOOK_LISTEN_IP\ | "(string) The IP to listen on, 0.0.0.0 binds to every local IP, works for me."
/TOPGG_WEBHOOK_LISTEN_PORT\ | "(integer) The port to listen on."
/TOPGG_BOT_WEBHOOK_SECRET\    | "(string) Your bot's webhook secret on top.gg, starting with whs_"
/TOPGG_SERVER_WEBHOOK_SECRET\ | "(string) Your server's webhook secret on top.gg, starting with whs_"
/TOPGG_WEBHOOK_LINK\ | "(string) The Discord webhook link to send messages about each vote to."
```

#### Launching

From the build directory, simply run:

    ./topgg_listener

### Serving

You can serve anything in the `resources` directory.

#### NOTE: The lookup is case-insensitive, so if you have something like `resources/ReSoUrCe` and `resources/resource`, the one found first will be served.

You can also redirect requests. If someone requests, for example, `dpptgg`, which happens to be a directory, inside it, you need to put a `.redirect.hfg` file and put this in:

```hfg
/Location\ | "(string) The location to redirect to."
```

You can put either a URL or a relative to `resources` location. For example, this:

```hfg
/Location\ | "dpptgg/dpptgg.png"
```

...will redirect to `<domain>/dpptgg/dpptgg.png`, and `<domain>` can be replaced with the path to `resources`. Again, this is case-insensitive.

If no resource is found, 404 is returned.
