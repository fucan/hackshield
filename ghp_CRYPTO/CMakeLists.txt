cmake_minimum_required (VERSION 2.6)

# Nome do projeto
project(ghp_CRYPTO)

add_library(ghp_CRYPTO STATIC
	ghp_CRYPTO.cpp ghp_CRYPTO.h
	)

#TARGET_LINK_LIBRARIES(ghp_CRYPTO ghp_CRYPTO)