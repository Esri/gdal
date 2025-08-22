-- Calls both of the gdal library types. The gdal_runtime.lua file is the default gdal library that all of runtime will
-- use with the library name being gdal. The gdal_ge.lua is gdal without kakadu support with the library name gdal_ge.
-- This is because game-engine cannot use kakadu due to licensing issues.
dofile("gdal_runtime.lua")
dofile("gdal_ge.lua")
