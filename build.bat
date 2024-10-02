@echo off

REM cl /LD /nologo /Od /Z7 /OUT:game.dll code/game.cpp code/game_math.cpp code/memory.cpp
REM cl /LD /nologo /Od /Z7 /FC /Fewin32_platform.exe code/platform.cpp code/renderer.cpp code/memory.cpp
REM cl /LD /nologo /Od /Z7 /FC test.cpp

msbuild /nologo /v:q build/engine.sln
