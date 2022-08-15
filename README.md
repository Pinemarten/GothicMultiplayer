# Gothic Multiplayer (2011 version)
The source code of Gothic Multiplayer that was first released in January 2011. 
This version of the code is probably the latest one (late 2011/early 2012), and it may contain changes that were not released at that time to players.
It's worth noting that this is not the version shown later (codename "red") with scripting capabilities.

The code is ~10 years old and belongs in a museum or on a comedy show (due to its exquisite quality :-)).
I'm posting it here more as a curiosity than something usable.

# How to build
## Prerequsities
* vcpkg - Gothic Multiplayer build system uses vcpkg dependency manager, follow the instructions from https://vcpkg.io/en/getting-started.html to install vcpkg.
* git bash - If you're on Windows, git bash is recommended to execute the commands from below.
* cmake (https://cmake.org/)
## Instructions
```bash
git clone https://github.com/Pinemarten/GothicMultiplayer2011.git
cd GothicMultiplayer2011
mkdir build
cd build
cmake .. -A WIN32
cmake --build . --config Release
```


If you want to run this, you need to figure it out yourself.

# Credits
G2ext headers - Paindevs and Patrick Vogel (see https://github.com/DaDummy/g2ext)
Gothic Multiplayer Server/Client/Starter - Gothic Multiplayer Team (pampi, skejt23, mecio)
