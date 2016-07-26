cmake_minimum_required (VERSION 2.6)

# Nome do projeto
project(ghp_UTILS)

add_library(ghp_UTILS STATIC
	ghp_DB.cpp ghp_DB.h
	ghp_DEFS.h
	ghp_ENCDEC.cpp ghp_ENCDEC.h
	ghp_FILE.cpp ghp_FILE.h
	ghp_LIC.cpp ghp_LIC.h
	ghp_LOG.cpp ghp_LOG.h
	ghp_MU.cpp ghp_MU.h
	ghp_PROTOCOL.h
	ghp_STRUCTURED_FILE.cpp ghp_STRUCTURED_FILE.h
	ghp_MEMORY_PROTECTION.cpp ghp_MEMORY_PROTECTION.h
	ghp_SYSTEM.cpp ghp_SYSTEM.h
	jpeg-compressor/jpge.cpp jpeg-compressor/jpge.h
	jpeg-compressor/jpgd.cpp jpeg-compressor/jpgd.h
	hiberlite/BeanLoader.cpp hiberlite/BeanUpdater.cpp hiberlite/ChildKiller.cpp hiberlite/CppModel.cpp hiberlite/Database.cpp
	hiberlite/ModelExtractor.cpp hiberlite/Registry.cpp hiberlite/shared_res.cpp hiberlite/SQLiteStmt.cpp hiberlite/Visitor.cpp hiberlite/sqlite3.c
	tinyxml2/tinyxml2.cpp tinyxml2/tinyxml2.h
	)

source_group("JPGE" FILES
	jpeg-compressor/jpge.cpp jpeg-compressor/jpge.h
	jpeg-compressor/jpgd.cpp jpeg-compressor/jpgd.h
	)
	
source_group("TinyXML2" FILES
	tinyxml2/tinyxml2.cpp tinyxml2/tinyxml2.h
	)
	
source_group("hiberlite" FILES
	hiberlite/BeanLoader.cpp hiberlite/BeanUpdater.cpp hiberlite/ChildKiller.cpp hiberlite/CppModel.cpp hiberlite/Database.cpp
	hiberlite/ModelExtractor.cpp hiberlite/Registry.cpp hiberlite/shared_res.cpp hiberlite/SQLiteStmt.cpp hiberlite/Visitor.cpp hiberlite/sqlite3.c
	)
	
#TARGET_LINK_LIBRARIES(ghp_UTILS ghp_UTILS)