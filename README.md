# D++ CMake bot, Guiding Light

This is a Discord bot written in C++ using the [D++](https://dpp.dev) library. It's a JTC voice channels manager. It assumes that D++, along with its dependencies, as well as [fmtlib](https://fmt.dev) are already installed.

## Dependencies
* D++ 10.1.0+
* fmtlib (tested on 9.1.0)

### Compiler
The bot is tested in-dev with g++-13 and hosted with clang++-18. It is advised to use g++ with the version of at least 13 as it uses coroutines which are unstable on g++-12 and earlier. I'm unaware of the coroutines situation with clang++.

## Compilation
The program assumes it's placed inside a directory below the project's root directory. So make one:

    mkdir build
    cd build

Then, generate a Makefile with cmake:

    cmake ..

You need to run the command above very time a change is made to `CMakeLists.txt`.

Now you're ready to compile:

    make -j

If DPP is installed in a different location you can specify the root directory to look in while running cmake 

    cmake .. -DCMAKE_PREFIX_PATH=<your/path>

If you have the same situation with fmtlib then good luck cuz I'm not a cmake genius and the above line actually comes from templatebot, the repository this one is a fork of.

If you wish to use a non-default compiler, you can specify it while running cmake

    cmake .. -DCMAKE_CXX_COMPILER=<your_compiler>


## Running the bot

Create a config.json in `Guiding_Light_Config`, located above the `build` directory (I have a private GitHub repository containing only the config file called `Guiding_Light_Config`, probably not the best way to do this but that's irrelevant):

```json
{
    "BOT_TOKEN": "Bot token here",
    "BOT_TOKEN_DEV" : "Test bot token here (use --dev when starting to test new changes without affecting anything used in production)",
    "BOT_DM_LOGS_ID" : "(integer) The ID of the channel for DM logs",
    "MY_ID" : "(integer) Your account's ID",
    "TOPGG_WEBHOOK_CHANNEL_ID" : "(integer) The ID of the channel for top.gg vote webhook messages",
    "MY_GUILD_ID" : "(integer) The ID of your guild. Not used anymore",
    "MY_PRIVATE_GUILD_ID" : "(integer) The ID of your guild. Used to create commands only administrators (which I know can only be me) can use",
    "TICKETS_GUILD_ID" : "(integer) The ID of your guild which contains tickets. Can be the same as MY_PRIVATE_GUILD_ID if you like"
}
```

## Setup

If you don't have the log files already, they will be created automatically as soon as the program tries to open them. Whenever it does, the files are cleared so back them up if you want to. Any other setup steps involve you [launching the CLI version of the program](#cli-mode). Once done, you need to run commands inside of it. You can get assistance from the `help` command. If you don't have the database files already, run init_db. If you've used the version of this bot which used text files as a database, run conv_db. Here is what it does step-by-step:

* Read the text files and add the values to the database.
* Convert the "infinite" value of the VC user limit from 100 to 0.
* Convert the default name underscores to spaces.

To create the slashcommands that are currently non-existent run `globalcreate ...` and `guildcreate ...`. This creates every single command defined. Do `list slashcommands` to see which commands there are if you only want to create some of them. If you want to delete a command, do `cdelete <command>`. Simply `cdelete` will delete every single command.

__***NOTE: because the commands are created in bulk, the unspecified ones will be deleted if they have already been created.***__

Once you're done setting up, you can start the bot.

### Start the bot:

    cd build
    ./guidingLight

### Start the dev version of the bot:

    cd build
    ./guidingLight --dev

### Have the bot return instead of running forever after starting:

    cd build
    ./guidingLight --return

`--dev` and `--return` can be combined.

### Start the bot in CLI mode:

    cd build
    ./guidingLight --cli

`--dev` and `--cli` can be combined, although the `switch` command makes it unnecessary. `--return`, although not discarded, has no effect in this mode.

## CLI mode

The CLI mode is built into the rest of the program. By compiling the bot, you compile the CLI alongside it. In this mode the bot doesn't handle events, neither does it start. This mode is used to set the bot up and create/delete slashcommands. It supports history, but it doesn't have autocomplete because I couldn't figure it out.
When you enter the CLI mode, you're logged into a pseudo-user - `guidingLight`, the release mode user and `curiousLight`, the development mode user. The input line looks something like this:

    guidingLight>

The username is coloured depending on the mode. The `>` sign is coloured depending on the bot running status: red if it's not running, yellow if it's currently launching and green if it's running. This status is independent of anything but the CLI. This means that if you launch the bot and then enter the CLI mode, the status is considered to be "not running". To fix this, do `launch` when in the mode. 

## Monitoring

If you want to see what's going on with the bot, you can peek at the logs in `logging/<interface>/<mode>/<logs type>_logs.log` or issue the `/logs` slashcommand. You can also use the `select` CLI command which `SELECT`s everything from a table. It takes one parameter: the table name. For example, you can do:

    select jtc_vcs

This will create a file in `database/select/<mode>`, called `jtc_vcs.md`. This is what is done to generate a file when an administrator runs `/select` (that, along with `/logging` is a guild command, so only one guild gets it, you don't have to worry about that).

## Extending the bot

You can add as many header files and .cpp files into the src and include folders as you wish. All .cpp files in the src directory will be linked together into the bot's executable.

## Renaming the bot

To rename the bot, search and replace "guidingLight" in the `CMakeLists.txt` with your new bot's name and then rename the guidingLight folder in include. Rerun `cmake ..` from the `build` directory and rebuild. You might need to re-create the build directory.

## Curious Light? Guiding Light? What do the names have to do with the bot's function?

Back in late 2023 this bot used to be a simple, tip-providing Discord bot about Doors the Roblox (#freeschlep) game, hence the name, Guiding Light. Later I decided to expand it to join-to-create voice channels as there was no bot that notified about new channels being created which was quite annoying as I really liked talking to the members of our friend Discord server, Central City. So, at first, the bot simply had extra functionality in Central City, and then it was expanded as a full-blown, public, join-to-create Discord bot. The name stayed the same and Curious Light became a nod to another Doors entity (taken from Rooms). **In this program's case, Curious Light is the in-development version of Guiding Light.** That's also why when in the CLI mode, the colour of `guidingLight` is cyan while that of `curiousLight` is yellow.
