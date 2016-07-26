cmake_minimum_required (VERSION 2.6)

# Nome do projeto
project(GHP)

# Gera o RC
configure_file (
  "${PROJECT_SOURCE_DIR}/resource.rc.in"
  "${PROJECT_SOURCE_DIR}/resource.rc"
  )

add_library(GHP SHARED
	ghp_MAIN.cpp
	ghp_CORE.cpp ghp_CORE.h
	ghp_NETBOX.cpp ghp_NETBOX.h
	ghp_HOOK.cpp ghp_HOOK.h
	ghp_SCAN.cpp ghp_SCAN.h
	ghp_SPLASH.cpp ghp_SPLASH.h
	ghp_UPDATER.cpp ghp_UPDATER.h
	resource.rc
	Detours/creatwth.cpp Detours/detours.cpp Detours/disasm.cpp Detours/image.cpp Detours/modules.cpp
	)

source_group("Detours" FILES Detours/creatwth.cpp Detours/detours.cpp Detours/disasm.cpp Detours/image.cpp Detours/modules.cpp)

set_target_properties(GHP PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")

if(R_WARZ)
set_target_properties(GHP PROPERTIES PREFIX "WZ_")
endif(R_WARZ)