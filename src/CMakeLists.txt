project(cdslow C)
cmake_minimum_required(VERSION 3.0)

add_library(cdslow STATIC
cdlist.c
cdshell.c
cdslow.c
cmdline.c
configdata.c
dlgabout.c
dlgbar.c
dlgconf.c
dlgdrive.c
dlginfo.c
dlgnotify.c
dlgpass.c
dlgsave.c
hotkeys.c
menu.c
messages.c
osinfo.c
regfunc.c
version.c
w6notify.c
)
