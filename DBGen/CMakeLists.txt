cmake_minimum_required (VERSION 2.6)

# Nome do projeto
project(DBGen)

add_executable(DBGen main.cpp)

if(R_WARZ)
set_target_properties(DBGen PROPERTIES PREFIX "WZ_")
endif(R_WARZ)