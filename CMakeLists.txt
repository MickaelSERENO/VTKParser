#Minimum cmake version 3.0
cmake_minimum_required(VERSION 3.0)

#Define the project
project(SerenoVTKParser CXX)

#Define Outputs (bin, lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_EXPORT_COMPILE_COMMANDS "ON")
set(CMAKE_CXX_STANDARD 14)

set(INSTALL_LIB_DIR       "${CMAKE_INSTALL_PREFIX}/lib"             CACHE PATH "Installation directory for libraries")
set(INSTALL_INC_DIR       "${CMAKE_INSTALL_PREFIX}/include"         CACHE PATH "Installation directory for headers")
set(INSTALL_PKGCONFIG_DIR "${CMAKE_INSTALL_PREFIX}/share/pkgconfig" CACHE PATH "Installation directory for pkgconfig (.pc) files")
set(COMPILE_TEST FALSE CACHE BOOL "Should we compile the test program ?")

set(VERSION 1.0)

#Workaround for MSVC not creating DEBUG and RELEASE folder
foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
    string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
    set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/bin)
    set( CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/lib)
    set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_BINARY_DIR}/lib)
endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )

#Debug version
set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CONFIGURATION_TYPES "Debug" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /w14244")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")
endif()

#Set the sources and the headers
file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
set(PC ${CMAKE_BINARY_DIR}/serenoVTKParser.pc)

list(REMOVE_ITEM SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)

#Configure the executable : sources, compile options (CFLAGS) and link options (LDFLAGS)
add_library(serenoVTKParser SHARED ${SOURCES} ${HEADERS})

#Add include directory
target_include_directories(serenoVTKParser PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

#Configure .pc
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/serenoVTKParser.pc.in
               ${CMAKE_CURRENT_BINARY_DIR}/serenoVTKParser.pc @ONLY)

#Test
if(COMPILE_TEST)
    add_executable(serenoVTKParserTest ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)
    target_link_libraries(serenoVTKParserTest PUBLIC serenoVTKParser)
endif()

#Installation
if(NOT SKIP_INSTALL_LIBRARIES AND NOT SKIP_INSTALL_ALL )
    install(TARGETS serenoVTKParser
        RUNTIME DESTINATION "${INSTALL_BIN_DIR}"
        ARCHIVE DESTINATION "${INSTALL_LIB_DIR}"
        LIBRARY DESTINATION "${INSTALL_LIB_DIR}" )
endif()

if(NOT SKIP_INSTALL_HEADERS AND NOT SKIP_INSTALL_ALL )
    file(GLOB DEPLOY_FILES_AND_DIRS "${PROJECT_SOURCE_DIR}/include/*")
    foreach(ITEM ${DEPLOY_FILES_AND_DIRS})
       IF( IS_DIRECTORY "${ITEM}" )
          LIST( APPEND DIRS_TO_DEPLOY "${ITEM}" )
       ELSE()
          LIST( APPEND FILES_TO_DEPLOY "${ITEM}" )
       ENDIF()
    endforeach()
    install(FILES ${FILES_TO_DEPLOY} DESTINATION ${INSTALL_INC_DIR})
    install(DIRECTORY ${DIRS_TO_DEPLOY} DESTINATION ${INSTALL_INC_DIR})
endif()

if(NOT SKIP_INSTALL_FILES AND NOT SKIP_INSTALL_ALL )
    install(FILES ${PC} DESTINATION "${INSTALL_PKGCONFIG_DIR}")
endif()


