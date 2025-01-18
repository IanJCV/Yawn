@echo off
python ../convert-to-h.py --shader ../Player/shaders/errorshader.hlsl ./errorshader.h

python ../convert-to-h.py ../Player/models/error.obj ./errormodel.h