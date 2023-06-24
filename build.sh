#!/bin/sh

# Windows
make re TARGET=windows
make TARGET=windows clean

# MacOS
make re TARGET=macos
make TARGET=macos clean

# Unix
make re TARGET=unix
make TARGET=unix clean
