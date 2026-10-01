project(md5_crypt C)
cmake_minimum_required(VERSION 3.0)

add_library(md5_crypt
md5.c
md5_crypt.c
)
