##-----------------------------LICENSE NOTICE------------------------------------
##  This file is part of CPCtelera: An Amstrad CPC Game Engine 
##  Copyright (C) 2016 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
##
##  This program is free software: you can redistribute it and/or modify
##  it under the terms of the GNU Lesser General Public License as published by
##  the Free Software Foundation, either version 3 of the License, or
##  (at your option) any later version.
##
##  This program is distributed in the hope that it will be useful,
##  but WITHOUT ANY WARRANTY; without even the implied warranty of
##  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
##  GNU Lesser General Public License for more details.
##
##  You should have received a copy of the GNU Lesser General Public License
##  along with this program.  If not, see <http://www.gnu.org/licenses/>.
##------------------------------------------------------------------------------

############################################################################
##                        CPCTELERA ENGINE                                ##
##                 Automatic image conversion file                        ##
##------------------------------------------------------------------------##
## This file is intended for users to automate music conversion from      ##
## original files (like Arkos Tracker .aks) into data arrays.             ##
##                                                                        ##
## Macro used for conversion is AKS2C, which has up to 5 parameters:      ##
##  (1): AKS file to be converted to data array                           ##
##  (2): C identifier for the generated data array (will have underscore  ##
##       in front in ASM)                                                 ##
##  (3): Output folder for .s and .h files generated (Default same folder)##
##  (4): Memory address where music data will be loaded                   ##
##  (5): Aditional options (you can use this to pass aditional modifiers  ##
##       to cpct_aks2c)                                                   ##
##                                                                        ##
## Macro is used in this way (one line for each image to be converted):   ##
##  $(eval $(call AKS2C,(1),(2),(3),(4),(5))                              ##
##                                                                        ##
## Important:                                                             ##
##  * Do NOT separate macro parameters with spaces, blanks or other chars.##
##    ANY character you put into a macro parameter will be passed to the  ##
##    macro. Therefore ...,src/music,... will represent "src/music"       ##
##    folder, whereas ...,  src/music,... means "  src/sprites" folder.   ##
##  * You can omit parameters by leaving them empty.                      ##
##  * Parameter  (5) (Aditional options) is  optional and  generally not  ##
##    required.                                                           ##
############################################################################

## === HOW TO CHANGE THE MUSIC IN THE FUTURE ===
## 1. Place your new song (.aks format from Arkos Tracker 1.0 or 2.0 exported as 1.0) into the project's music/ folder.
## 2. Change the filename 'music.aks' in the macro below to your new filename (e.g. 'my_song.aks').
## 3. Change the array name 'music_song' below to match your new name (e.g. 'my_song_data').
## 4. Run 'make clean && make' to recompile.
## 5. In src/main.c, update the #include to your new .h file and use the new array name in cpct_akp_musicInit.

## Address 0x0500: song fits 0x0500-0x1ABF (5567 bytes), reclaimed from BASIC area.
## The CPC loads it into RAM (under ROM), exposed after cpct_disableFirmware().
$(eval $(call AKS2C,music/music.aks,music_song,src/music/,0x0500))