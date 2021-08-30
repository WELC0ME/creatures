# Creatures

## Description
This program is designed to experiment with genetic algorithms. It
is based on a YouTube video, but it is not a copy. It is an
independent implementation in C++.

## Assembly
I build project with this command:
```
C:\MinGW\bin\g++.exe src\main.cpp -lsfml-graphics -lsfml-window -lsfml-system -IC:\libs\SFML\include -LC:\libs\SFML\lib -o build\main.exe
```
You can use it, however you must change the path to the folder with the SFML
library and compiler for a successful build. Of course, you can use a
ready-made assembly.

## Launch
The main file is main.exe in the build folder. To run the program
correctly, do not change the contents of the build folder.

## Control
Use the keyboard to control the simulation:
 - Q - exit
 - S - one simulation step
 - E - start / stop simulation 
