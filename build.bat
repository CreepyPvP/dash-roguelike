@echo off
cl /LD code/game.cpp code/game_math.cpp code/memory.cpp /OUT:game.dll /Od /Zi 
