## Brick Blaster - Build System
## License: MIT License (c) 2026 ISSALIG
## ----------------------------------------------------
LANGS := ES EN FR GR
.PHONY: all_languages $(LANGS)

all_languages: $(LANGS)

$(LANGS):
	$(MAKE) LANG=$@
##-----------------------------LICENSE NOTICE------------------------------------
##  This file is part of CPCtelera: An Amstrad CPC Game Engine 
##  Copyright (C) 2015 ronaldo / Fremos / Cheesetea / ByteRealms (@FranGallegoBR)
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

###########################################################################
##                          CPCTELERA ENGINE                             ##
##                  Main Building Makefile for Projects                  ##
##-----------------------------------------------------------------------##
## This file contains the rules for building a CPCTelera project. These  ##
## These rules work generically for every CPCTelera project.             ##
## Usually, this file should be left unchanged:                          ##
##  * Project's build configuration is to be found in build_config.mk    ##
##  * Global paths and tool configuration is located at $(CPCT_PATH)/cfg/##
###########################################################################

##
## PROJECT CONFIGURATION (you may change things there to setup this project)
##
## Language configuration
ifeq ($(LANG), EN)
    LANG_SUFFIX := en
    LANG_MACRO   := -DLANG_EN
else ifeq ($(LANG), FR)
    LANG_SUFFIX := fr
    LANG_MACRO   := -DLANG_FR
else ifeq ($(LANG), GR)
    LANG_SUFFIX := gr
    LANG_MACRO   := -DLANG_GR
else
    # Default to Spanish
    LANG_SUFFIX := es
    LANG_MACRO   := -DLANG_ES
endif

PROJNAME := brickblaster_$(LANG_SUFFIX)

# Language-specific build detection to force rebuilds when LANG changes
LANG_CONFIG := obj/lang.config
.PHONY: force_lang_check
$(LANG_CONFIG): force_lang_check
	@mkdir -p obj
	@echo "$(LANG)" > $@.tmp
	@if [ ! -f $@ ] || [ "`cat $@`" != "`cat $@.tmp`" ]; then \
		cp $@.tmp $@; \
		echo "Language changed to $(LANG). Forcing rebuild of all modules."; \
	fi
	@rm $@.tmp

include cfg/build_config.mk

# Force rebuild of ALL modules when language changes
$(OBJFILES) $(GENOBJFILES): $(LANG_CONFIG)

# Ensure output directories exist
$(DSK) $(CDT): | $(DISTDIR) $(OBJDIR)/$(DISTDIR)
$(OBJDSKINCSDIR)/loading.scr.$(DSKINC_EXT) $(OBJDSKINCSDIR)/DISC.BAS.$(DSKINC_EXT): | $(OBJDIR)/$(DISTDIR)

$(DISTDIR) $(OBJDIR)/$(DISTDIR):
	@mkdir -p $@

Z80CCFLAGS += $(LANG_MACRO)

##
## CUSTOM RULES
##
src/assets/sprites.h: tools/gen_sprites.py
	python3 tools/gen_sprites.py

src/assets/boss.h: tools/sprite2h.py assets/final_enemy.png tools/png2sprite.py
	python3 tools/png2sprite.py assets/final_enemy.png --out boss_tmp
	python3 tools/sprite2h.py boss_tmp_sprite.txt src/assets/boss.h boss_new
	rm boss_tmp*

## Make sure the compiled object depends on the generated headers
obj/main.rel: src/assets/sprites.h src/assets/boss.h

# Handle loading screen generation
dsk_files/loading.scr: assets/loading.png tools/img2scr.py
	python3 tools/img2scr.py assets/loading.png dsk_files/loading.scr --basic dsk_files/DISC.BAS

# Ensure the generated file is seen by the engine's inclusion logic
DSKINCOBJFILES += $(OBJDSKINCSDIR)/loading.scr.$(DSKINC_EXT)

# Exclude files with custom rules from standard inclusion to avoid warnings
DSKINCSRCFILES := $(filter-out dsk_files/DISC.BAS dsk_files/loading.scr,$(DSKINCSRCFILES))

# # Convert BASIC to tokenized binary (without AMSDOS header)
# dsk_files/DISC.bin: dsk_files/DISC.BAS tools/bas2bin.py
# 	@python3 tools/bas2bin.py dsk_files/DISC.BAS dsk_files/DISC.bin

# # Create CDT loader with tokenized binary
# $(CDT): dsk_files/DISC.bin dsk_files/loading.scr $(BINFILE)
# 	@$(call PRINT,$(CDT),"Creating Multi-file Cassette File $@")
# 	# -n -F 0  -r "CAS.BAS" CAS.BAS brickblaster.cdt
# 	@$(2CDT) -n -F 0 -r "DISC" dsk_files/DISC.bin $@
# 	@$(2CDT) -F 2 -L 0xC000 -r "loading.scr" dsk_files/loading.scr $@
# 	@$(2CDT) -F 2 -L 0x0500 -X 0x1C00 -r "brickbla.bin" $(BINFILE) $@

# Custom rules for dsk_files that need special flags (Shadowing standard rules)
# We use engine macros to avoid explicit iDSK calls where possible
$(OBJDSKINCSDIR)/loading.scr.$(DSKINC_EXT): dsk_files/loading.scr $(DSK)
	@$(call ADDCODEFILETODSK,$(DSK),$<,0xC000,0x0000,$@)

$(OBJDSKINCSDIR)/DISC.BAS.$(DSKINC_EXT): dsk_files/DISC.BAS $(DSK)
	@$(IDSK) $(DSK) -i $< -t 0 -f &> /dev/null
	@touch $@
	@$(call PRINT,$(DSK),"Added '$<' as BASIC")
	@cp $(DSK) web/assets/brickblaster_$(LANG_SUFFIX).dsk

# Generate Base64 version of the disk for the web portal
$(WEB_DISK_JS): $(DSK)
	@$(call PRINT,$(DSK),"Generating $@ (Base64)")
	@echo -n "window.diskData = '" > $@
	@base64 -w 0 $(DSK) >> $@
	@echo "';" >> $@

##
## USE GLOBAL MAKEFILE (general rules for building CPCtelera projects)
##
include $(CPCT_PATH)/cfg/global_main_makefile.mk
