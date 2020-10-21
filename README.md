# gm8x_fix
gm8x_fix is a patcher that fixes certain issues in games made with
GameMaker 8.x. You can download the latest release from the
[Releases](https://github.com/skyfloogle/gm8x_fix/releases/latest) tab.

# How do I use it?
Drag the executable file for your game onto gm8x_fix.exe. Or, if you're a
commandline nerd, you can run it with:
```bash
gm8x_fix [-s] FILE
```
The -s option removes commandline output. It will still exit with code 1 on
failure though, so check for that if you're automating.

For building, you can just build the C file with your favourite compiler, it
doesn't have any dependencies.

# The issues this fixes, and how it fixes them
## The joystick patch
Early versions of GameMaker have a *terrible* implementation of joystick
support, which can cause games to slow down if joysticks are installed but not
plugged in. This slowdown exists on games that don't use the joystick
functions, but is much worse on games that do. This occurs because every frame,
it polls the first two joysticks even if they aren't connected, and polling
joysticks that aren't connected causes a massive search through the registry,
which takes up a lot of time.<br/>
This patch replaces literally every call to the joystick API with some code
that pretends there are no joysticks connected. Basically, it completely
disables joystick support.

## The scheduler patch
Between frames, GameMaker calls
[Sleep](https://docs.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-sleep)
in order to limit the framerate. Unfortunately, this function isn't very precise,
which means games can sleep for longer than they were intended to, which may lead
to sluggish or inconsistent framerates on some computers. There is a function called
[timeBeginPeriod](https://docs.microsoft.com/en-us/windows/win32/api/timeapi/nf-timeapi-timebeginperiod)
which allows us to make the function more precise, but GameMaker doesn't use it
out of the box. This patch invokes that function at game start and improves
the precision. The patch overwrites the import for one of the joystick functions,
so using the scheduler patch without the joystick patch may cause issues.

[Here's](https://www.youtube.com/watch?v=oGg06HMPASg) a YouTube video with a more
detailed explanation.

This fix also exists as a DLL hack for GameMaker 8.1 and up (no 8.0, sorry).
This includes GameMaker:Studio.
[Click here for more information on gms_scheduler_fix.](https://github.com/omicronrex/gms_scheduler_fix)

## The memory patch
Some games also run into issues on some newer computers where the game crashes
on startup with the message "Unexpected error occured when running the game" or
"Out of memory".<br/>
This can, on some games, be fixed by enabling the flag in the header that tells
the OS the program can understand addresses bigger than 2GB. Shoutouts to
[these guys](https://iwannacommunity.com/forum/index.php@topic=2308.msg16505.html)
for finding that.

## The DirectPlay patch
GameMaker games use DirectPlay for networking, but loading the DirectPlay DLL
on newer versions of Windows brings up a prompt because DirectPlay is
deprecated. In fact, on newer versions of Windows, it just doesn't work at all.<br/>
The patch replaces the attempt to load DirectPlay with an attempt to load
literally nothing. This somehow doesn't break anything unless you're using the
networking functions.

# So what's the catch?
If the game used the builtin joystick functions, it will no longer work with
joysticks if you apply the joystick patch. Some games use an external library
like [joydll](http://web.archive.org/web/20191214124845/https://gmc.yoyogames.com/index.php?showtopic=495788)
for joystick support. Those will work fine.

The DirectPlay patch will break networking. Any calls to the networking
functions will probably trigger an access violation. If this causes problems
outside of multiplayer modes in any games, open an issue and I'll make a less
janky solution.

The scheduler patch overwrites some debug logging, but that shouldn't cause any
issues.
