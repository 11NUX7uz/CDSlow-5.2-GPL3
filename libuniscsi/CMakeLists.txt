project(uniscsi C)
cmake_minimum_required(VERSION 3.0)

add_library(uniscsi
log_scsi.c
scsiaspi.c
scsint.c
uniscsi.c
)
