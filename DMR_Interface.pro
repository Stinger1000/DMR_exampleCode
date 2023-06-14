TEMPLATE = subdirs

SUBDIRS += \
    Gui \
    DMRlib

Gui.depends += DMRlib
