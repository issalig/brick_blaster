## Brick Blaster - Build System
## License: MIT License (c) 2026 ISSALIG
## ----------------------------------------------------
LANGS := ES EN FR GR VA PT
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
else ifeq ($(LANG), VA)
    LANG_SUFFIX := va
    LANG_MACRO   := -DLANG_VA
else ifeq ($(LANG), PT)
    LANG_SUFFIX := pt
    LANG_MACRO   := -DLANG_PT
else
    # Default to Spanish
    LANG_SUFFIX := es
    LANG_MACRO   := -DLANG_ES
endif

PROJNAME := brickb

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
$(DSK) $(CDT) $(SNA) $(WEB_DISK_JS): | $(DISTDIR) $(OBJDIR)/$(DISTDIR) web/assets/disks

$(DISTDIR) $(OBJDIR)/$(DISTDIR) web/assets/disks:
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

# Generate SNA snapshot (Version 1 - Default)
SNA := $(DISTDIR)/$(PROJNAME)_$(LANG_SUFFIX).sna
$(SNA): $(BINFILE) tools/bin2sna.py obj/brickb.map
	@MAIN_ADDR=$$(grep -w "_main" obj/brickb.map | awk '{print "0x"$$1}' | head -n 1); \
	python3 tools/bin2sna.py $(BINFILE) $(SNA) 0x0500 $$MAIN_ADDR 0xF000 1; \
	echo "[$$SNA] Generated SNA snapshot V1 (PC=$$MAIN_ADDR)"; \
	cp $(SNA) web/assets/disks/

# Generate SNA snapshot (Version 2 - Optional)
SNA_V2 := $(DISTDIR)/$(PROJNAME)_$(LANG_SUFFIX)_v2.sna
$(SNA_V2): $(BINFILE) tools/bin2sna.py obj/brickb.map
	@MAIN_ADDR=$$(grep -w "_main" obj/brickb.map | awk '{print "0x"$$1}' | head -n 1); \
	python3 tools/bin2sna.py $(BINFILE) $(SNA_V2) 0x0500 $$MAIN_ADDR 0xC000 2; \
	echo "[$$SNA_V2] Generated SNA snapshot V2 (PC=$$MAIN_ADDR)"; \
	cp $(SNA_V2) web/assets/disks/

# Ensure the generated file is seen by the engine's inclusion logic
DSKINCOBJFILES += $(OBJDSKINCSDIR)/loading.scr.$(DSKINC_EXT)

# Exclude files with custom rules from standard inclusion to avoid warnings
DSKINCSRCFILES := $(filter-out dsk_files/DISC.BAS dsk_files/loading.scr,$(DSKINCSRCFILES))

# Custom rules for dsk_files that need special flags (Shadowing standard rules)
# We use engine macros to avoid explicit iDSK calls where possible
$(OBJDSKINCSDIR)/loading.scr.$(DSKINC_EXT): dsk_files/loading.scr $(DSK)
	@$(call ADDCODEFILETODSK,$(DSK),$<,0xC000,0x0000,$@)

$(OBJDSKINCSDIR)/DISC.BAS.$(DSKINC_EXT): dsk_files/DISC.BAS $(DSK)
	@$(IDSK) $(DSK) -i $< -t 0 -f &> /dev/null
	@touch $@
	@$(call PRINT,$(DSK),"Added '$<' as BASIC")

# Ensure the web DSK is only copied after all files are included
WEB_DSK_FINAL := web/assets/disks/brickb_$(LANG_SUFFIX).dsk
$(WEB_DSK_FINAL): $(DSKINC)
	@cp $(DSK) $@
	@$(call PRINT,$(DSK),"Final web DSK synchronized: $@")

# Generate Base64 version of the disk for the web portal
$(WEB_DISK_JS): $(WEB_DSK_FINAL)
	@$(call PRINT,$(DSK),"Generating $@ (Base64)")
	@echo -n "window.diskData = '" > $@
	@base64 -w 0 $< >> $@
	@echo "';" >> $@

all_sna:
	@for lang in $(LANGS); do $(MAKE) LANG=$$lang sna; done

all_sna_v2:
	@for lang in $(LANGS); do $(MAKE) LANG=$$lang sna_v2; done

.PHONY: sna sna_v2 all_sna all_sna_v2
sna: $(SNA)
sna_v2: $(SNA_V2)

##
## USE GLOBAL MAKEFILE (general rules for building CPCtelera projects)
##
include $(CPCT_PATH)/cfg/global_main_makefile.mk
