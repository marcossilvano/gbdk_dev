Tile Creator for the Sega Master System & Sega Game Gear
---------------------------------------------------------

This program uses stb_image as png loader and tinyfiledialogs as the dialog manager, so i dont have
to worry about irritating operating systems and can concentrate on what matters. :)

To use open a console window at the location of the exe, paste in your tilesheet and do the following...

** NOTE you will need to have the images in the same location as the TCS8.exe
**NOTE DONT INCLUDE [] on the command line, these are just to show parameters clearly

* Select console to build for -sms for Master System or -gg for GameGear
* Tile widths must be divisible by 8
* Tile heights must be divisible by 8
* You must put the palettes you use from top left of the image going across
* The top row of the image is expected as a palette store - see example images for reference
* Also palettes are expected to be in the correct format for the gameboy/gameboy colour
* That means RGB values must also be divisible by 8 to convert to 16bit BGR format of gameboy
* PNG images must be 32bit, which means an alpha channel is expected, although is not used
* -tgd paramter is for sprites, when 8x16 format, as the tiles are arranged from top to bottom, then left to right
* -tid paramter will split tileblocks with comment //block x - where x is the id of the block
* palette values will also be exported in the output textfile

* You may have issues if your palettes share too many of the same colours, so be warned

TCS8 [-sms for Master system] or [-gg for gamegear] [filename.png] [width in pixels of your tiles] [height in pixels of your tiles] [tile count] [optional params] [-tgd] [-tid]

TCS8 -sms smsCoinTile1.png 16 16 2 -tid

TCS8 -gg ggT1.png 8 8 1

** Note when converting images as sprites - if sprites are to be 8x16 in gbdk, then set block size parameters to 8x16 to match sprite sizes and also block count will be amount of those block sizes present in the image now, to convert.
** example windows TCS8.exe -gg ggBugT1.png 8 16 64
** example other ./TCS8 -gg ggBugT1.png 8 16 64

Hopefully this will be useful

By BriG 2024 of - Retro C Game Programming channel
