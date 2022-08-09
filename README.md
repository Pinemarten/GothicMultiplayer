# Gothic Multiplayer (2011 version)
The source code of Gothic Multiplayer that was first released in January 2011. 
This version of the code is probably the latest one (late 2011/early 2012), and it may contain changes that were not released at that time to players.
It's worth noting that this is not the version shown later (codename "red") with scripting capabilities.

The code is ~10 years old and belongs in a museum or on a comedy show (due to its exquisite quality :-)).
I'm posting it here more as a curiosity than something usable.

# How to build
First, init submodules
```bash
git submodule update --init --recursive
```
Launch GMP.sln in Visual Studio 2022 and build all projects.

The original code was built in Visual Studio 2010, and binaries built with 2022 may probably not work with Gothic out of the box.
I did not try to run the server or the client (.dll).
If you want to run this, you need to figure it out yourself.

# Credits
G2ext headers - Paindevs and Patrick Vogel (see https://github.com/DaDummy/g2ext)
Gothic Multiplayer Server/Client/Starter - Gothic Multiplayer Team (pampi, skejt23, mecio)
