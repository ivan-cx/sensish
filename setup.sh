#!/bin/bash

# 1. install prerequisites - requires sudo:
apt install libglfw3-dev build-essential

# 2. download submodules
git submodule init
git submodule update
