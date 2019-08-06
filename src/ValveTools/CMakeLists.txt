add_library(ValveTools STATIC Matrices.cpp Matrices.h Vectors.h pathtools.cpp pathtools.h compat.h strtools.cpp strtools.h)
target_include_directories(ValveTools PUBLIC ./)
