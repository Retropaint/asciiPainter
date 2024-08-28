# Agnos(tic)

Agnos (cool name, right? Short for "agnostic") is where code that behaves differently across platforms is handled. 

This is the chosen alternative compared to something like macro checks, as that kind of thing may or may not work with your compiler. Instead, the complete separation of code by files makes it fully portable.

Platforms that are handled:
* UNIX (Linux & MacOS)
* Win32 (Windows)
