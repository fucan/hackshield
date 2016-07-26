cmake_minimum_required (VERSION 2.6)

# Nome do projeto
project(LicGen)

add_executable(LicGen main.cpp)

if(R_WARZ)
set_target_properties(LicGen PROPERTIES PREFIX "WZ_")
endif(R_WARZ)