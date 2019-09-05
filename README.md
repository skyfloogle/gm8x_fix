Joyfix is a patcher that fixes certain issues in games made with GameMaker 8.x.

# How do I use it?
Drag the executable file for your game onto joyfix.exe. Or, if you're a
commandline nerd, you can run it with:
```bash
joyfix [-s] FILE
```
The -s option removes commandline output. It will still exit with code 1 on
failure though, so check for that if you're automating.

For building, you can just build the C file with your favourite compiler, it
doesn't have any dependencies.

# The issues this fixes
Early versions of GameMaker have a *terrible* implementation of joystick
support, which can cause games to slow down if joysticks are installed but not
plugged in. This slowdown exists on games that don't use the joystick
functions, but is much worse on games that do. This occurs because every frame,
it polls the first two joysticks even if they aren't connected, and polling
joysticks that aren't connected causes a massive search through the registry,
which takes up a lot of time.

Some games also run into issues on some newer computers where the game crashes
on startup with the message "Unexpected error occured when running the game."

# How this fixes them
This patch replaces literally every call to the joystick API with some code
that pretends there are no joysticks connected. Basically, it completely
disables joystick support.

The latter issue can, on some games, be fixed by enabling the flag in the
header that tells the OS the program can understand addresses bigger than 2GB.
Shoutouts to [these guys](https://iwannacommunity.com/forum/index.php@topic=2308.msg16505.html)
for finding that.

# So what's the catch?
If the game used the builtin joystick functions, it will no longer work with
joysticks. Sorry. Some games use an external library like [joydll](http://gmc.yoyogames.com/index.php?showtopic=495788)
for joystick support. Those will work fine.