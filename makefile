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

include $(shell cedev-config --makefile)
