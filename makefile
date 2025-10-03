# ----------------------------
# Makefile Options
# ----------------------------

NAME = HELPMII
ICON = icon.png
DESCRIPTION = "HelpMii for TI-84 Plus CE"
COMPRESSED = NO
ARCHIVED = NO

CFLAGS = -Wall -Wextra -Oz
CXXFLAGS = -Wall -Wextra -Oz

# ----------------------------

GUIDE ?= 0

ifeq ($(GUIDE), 1)
	CFLAGS += -DGETTING_STARTED_GUIDE
endif



include $(shell cedev-config --makefile)
