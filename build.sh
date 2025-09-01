clear
windres src/app.rc -O coff -o build/app.res
g++ src/index.cpp build/app.res -lraylib -lopengl32 -lgdi32 -lwinmm -ldsound -mwindows -o build/NamelessGame.exe