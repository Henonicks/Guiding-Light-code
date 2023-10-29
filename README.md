# D++ CMake bot, Guiding Light

This is a Discord bot written in C++ using the [D++](https://dpp.dev) library. It's a JTC voice channels manager. It assumes that fmtlib and D++ are already installed.

## Compilation

    mkdir build
    cd build
    cmake ..
    make -j

If DPP is installed in a different location you can specify the root directory to look in while running cmake 

    cmake .. -DDPP_ROOT_DIR=<your-path>

## Running the template bot

Create a config.json in the directory above the build directory:

```json
{
    "BOT_TOKEN": "Bot token here",
    "BOT_LOGS_ID" : "ID of the logs channel",
    "BOT_DM_LOGS_ID" : "ID of the channel for DM logs",
    "MY_ID" : "Creator ID",
    "MY_GUILD_ID" : "?",
    "ELGATO" : "Elgato emoji string",
}
```

Start the bot:

    cd build
    ./guidingLight

## Extending the bot

You can add as many header files and .cpp files into the src and include folders as you wish. All .cpp files in the src directory will be linked together into the bot's executable.

## Renaming the bot

To rename the bot, search and replace "guidingLight" in the `CMakeLists.txt` with your new bots name and then rename the guidingLight folder in include. Rerun `cmake ..` from the `build` directory and rebuild. You might need to re-create the build directory.
