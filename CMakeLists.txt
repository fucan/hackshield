cmake_minimum_required (VERSION 2.6)

# Solução
project(GHP5)

# Definições
set(CMAKE_MFC_FLAG 1)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT /MP")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd /MP")

add_definitions(-DUNICODE -D_UNICODE)
add_definitions(-DCURL_STATICLIB)

set(GHP_CORE_VERSION_MAJOR 5)
set(GHP_CORE_VERSION_MINOR 12)
set(GHP_CORE_RELEASE_MAJOR 0)
set(GHP_SRV_VERSION_MAJOR 5)
set(GHP_SRV_VERSION_MINOR 12)
set(GHP_SRV_RELEASE_MAJOR 0)

# Opções
option (R_WARZ "Compila as ferramentas para usar no WarZ" OFF)
option (R_MUONLINE "Compila as ferramentas para usar no MuOnline" OFF)
option (D_INJECT_DEBUG_LOGIN "Injeta o login Shiu como debug no sistema de USER_ID" OFF)

# Gera o H
configure_file (
  "${PROJECT_SOURCE_DIR}/include.h.in"
  "${PROJECT_SOURCE_DIR}/include.h"
  )

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

# Diretórios do H
include_directories(${PROJECT_SOURCE_DIR}/3rd/include)

# Diretórios de LIBS
link_directories(${PROJECT_SOURCE_DIR}/3rd/lib)
link_directories(${PROJECT_BINARY_DIR}/lib)

# Projetos da solução
add_subdirectory(ghp_CRYPTO)
add_subdirectory(ghp_NET)
add_subdirectory(ghp_UTILS)

add_subdirectory(Hackblocker)
add_subdirectory(MasterServer)
add_subdirectory(DBGen)
add_subdirectory(LicGen)
add_subdirectory(GHPUpdater)
add_subdirectory(GHPServer)
add_subdirectory(GHP)