# PDCurses & PDCursesMod (for Windows)

There are 2 variants of PDCurses available:

* The original, which is unused due to not supporting window resize adjustment (https://github.com/wmcbrine/PDCurses)
* PDCursesMod, which is used because it does, indeed, support window resize adjustment (https://github.com/Bill-Gray/PDCursesMod)

The latter offers more stuff, but that's mainly what it's used for.

## Important Note

Both `.lib` files (PDCurses and PDCursesMod) were compiled as x64, and will not work if compiled in x86. If you do, MSVC will warn you about this, but you might not notice it in the sea of errors for missing PDCurses definitions.

## (Not so) Important Note 

The `pdcutil.c` file of PDCursesMod's source (wincon) was changed to not call PlaySound() as this required linking with winmm (which I do not want nor need). The full file in question is included in PDCursesMod's folder, so you can plop it right in the source and compile it to be the same lib that is currently used.

## To-Do

Include the source of both libs here, so they can be compiled alongside asciiPainter. Not a high priority since everyone's Windows is practically the same, but FOSSbros will get a kick out of it!
