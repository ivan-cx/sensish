#!/bin/bash

# Cleanup:
rm -f *.o
rm -f sensish

# Initialization: 


# Common code:
check_error() {
  retVal=$1
  if [ $retVal -ne 0 ]; then
    echo "Failed $2"
    exit $retVal
  fi
}

# Compilation:

# Release:
optimization_flags="-O3"

# Debug:
# optimization_flags="-Og -g3 -fno-inline"

# Could be useful:
# -fstack-protector-all -fstrict-aliasing -fsanitize=undefined
# -fcf-protection=full -fsanitize=address #-fsanitize=leak #-fsanitize=thread 

syntax_flags="-Wall -Wstrict-aliasing -Werror -Wformat -Wpedantic -std=c++17"
include_files=" -Isrc -Ilibs/gl3w -Ilibs/stb -Ilibs/imgui -Ilibs/imgui/examples"

g++ -c src/main.cpp $include_files $definitions $syntax_flags $optimization_flags
check_error $? "to compile src/main.cpp"

g++ -c src/open_file.cpp $include_files $definitions $syntax_flags $optimization_flags
check_error $? "to compile src/open_file.cpp"

g++ -c src/texture.cpp $include_files $definitions $syntax_flags $optimization_flags
check_error $? "to compile src/texture.cpp"

g++ -c src/text.cpp $include_files $definitions $syntax_flags $optimization_flags
check_error $? "to compile src/text.cpp"

g++ -c src/stb.cpp $include_files $definitions $optimization_flags -w
check_error $? "to compile src/stb.cpp"

gcc -c libs/gl3w/GL/gl3w.c $include_files $definitions $optimization_flags
check_error $? "to compile libs/gl3w/GL/gl3w.c"

g++ -c libs/imgui/imgui.cpp $include_files $definitions $optimization_flags
check_error $? "to compile libs/imgui/imgui.cpp"

g++ -c libs/imgui/imgui_draw.cpp $include_files $definitions $optimization_flags
check_error $? "to compile libs/imgui/imgui_draw.cpp"

g++ -c libs/imgui/imgui_widgets.cpp $include_files $definitions $optimization_flags
check_error $? "to compile libs/imgui/imgui_widgets.cpp"

g++ -c libs/imgui/examples/imgui_impl_glfw.cpp $include_files $definitions $optimization_flags
check_error $? "to compile libs/imgui/examples/imgui_impl_glfw.cpp"

g++ -c libs/imgui/examples/imgui_impl_opengl3.cpp $include_files $definitions $optimization_flags
check_error $? "to compile libs/imgui/examples/imgui_impl_opengl3.cpp"


# Linking:

libs=" -ldl -pthread -lGL -lm -lglfw "

g++ *.o $definitions $syntax_flags $libs $optimization_flags -o sensish
check_error $? "to link sensish"

# Run: 
./sensish
