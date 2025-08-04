# D++ CMake bot, Guiding Light

This is a Discord bot written in C++ using the [D++](https://dpp.dev) library. It's a JTC voice channels manager. It assumes that D++, along with [fmtlib](https://fmt.dev) are already installed.

## Dependencies
* D++ 10.1.0+
* fmtlib (tested on 9.1.0)

## Compilation

    mkdir build
    cd build
    cmake ..
    make -j

If DPP is installed in a different location you can specify the root directory to look in while running cmake 

    cmake .. -DDPP_ROOT_DIR=<your-path>

If you have the same situation with fmtlib then good luck cuz I'm not a cmake genius

## Running the bot

Create a config.json in `Guiding_Light_Config`, located above the `build` directory (I have a private GitHub repository containing only the config file called `Guiding_Light_Config`, probably not the best way to do this but that's irrelevant):

```json
{
    "BOT_TOKEN": "Bot token here",
    "BOT_TOKEN_DEV" : "Test bot token here (use --dev when starting to test new changes without affecting anything used in production)",
    "BOT_DM_LOGS_ID" : The ID of the channel for DM logs,
    "MY_ID" : Your account's ID,
    "TOPGG_WEBHOOK_CHANNEL_ID" : The ID of the channel for top.gg vote webhook messages,
    "MY_GUILD_ID" : The ID of your guild. Not used anymore,
    "MY_PRIVATE_GUILD_ID" : The ID of your guild. Used to create commands only administrators (which I know can only be me) can use
    "TICKETS_GUILD_ID" : The ID of your guild which contains tickets. Can be the same as MY_PRIVATE_GUILD_ID if you like
}
```

## Setup

If you don't have the log files already, run the `create_files.sh` script. It'll create all the log files. If you don't have the database files already, run `init_db.sh`. If you've used the version of this bot which used text files as a database, run `conv_db.sh`. If you've used the bot before, chances are, your database still remembers the "infinite" value of the VC user limit as 100. Run `100_to_0.sh` to fix this as otherwise the bot will try to create a VC with the limit set to 100 and fail. If this is the case, you'll also have underscores instead of spaces. This isn't needed anymore so the bot won't replace underscore with spaces when creating a temporary VC. Convert them with `us_to_sp.sh`.

Start the bot:

    cd build
    ./guidingLight

## Monitoring

If you want to see what's going on with the bot, you can peek at the logs in `logging/<mode>/<logs type>_logs.log` or issue the `/logs` slashcommand. You can also use the `select.sh` script which `SELECT`s everything from a table. It takes two parameters: bot running mode (`dev` or `release`) and the table name. For example, you can do:

    ./select.sh dev jtc_vcs

This will create a file in `database/select/dev`, called `jtc_vcs.md`. This is what is run to generate a file when an administrator runs `/select` (that, along with `/logging` is a guild command, so only one guild gets it, you don't have to worry about that).

## Extending the bot

You can add as many header files and .cpp files into the src and include folders as you wish. All .cpp files in the src directory will be linked together into the bot's executable.

## Renaming the bot

To rename the bot, search and replace "guidingLight" in the `CMakeLists.txt` with your new bot's name and then rename the guidingLight folder in include. Rerun `cmake ..` from the `build` directory and rebuild. You might need to re-create the build directory.
