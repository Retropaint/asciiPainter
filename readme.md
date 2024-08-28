# asciiPainter
That's right, with a lowercase a!

asciiPainter is a simple CLI tool for creating and editing ASCII, with support for colors.

## Dependencies
Higher versions will work unless stated otherwise. Lower versions were not tested (but accepting issues for doing so).
* `CMake v3.29.1`
* Linux/MacOS:
  * `ncurses v6.5`
* Windows: 
  * `PDCurses v3.9` or `PDCursesMod v4.4.0` (both included in source)

## Building
Clone this repo (either with `git clone` or the the big green button above).

Then, in that folder:

```
mkdir b
cd b
cmake ..
cmake --build .
```

asciiPainter accepts a filename as the first parameter, but it must end in `.txt`. A sample file is included (assets/bewd.txt).

### CMake Parameters

* `OG_PDCURSES:BOOL:[ON/OFF]` - Set to ON to use original PDCurses instead of PDCursesMod

## ASCII data
As seen in the sample file, art is saved by separating the ASCII content (top) and the colorcoords (bottom).
Colorcoords are saved as numbers 0-8 (only default terminal colors are supported for now).

## Potential TODOs
Not an exhaustive list, nor promises (hence potential)
* Proper vim-like insert (current works more like replace)
* Support more than the default colors
* Proper config setup
