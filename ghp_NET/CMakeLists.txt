cmake_minimum_required (VERSION 2.6)

# Nome do projeto
project(ghp_NET)

add_library(ghp_NET STATIC
	mongoose.c mongoose.h
	ghp_HTTPCLIENT.cpp ghp_HTTPCLIENT.h
	)
	
#TARGET_LINK_LIBRARIES(ghp_NET ghp_NET)