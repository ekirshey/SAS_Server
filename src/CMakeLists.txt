include_directories(
	${PROJECT_SOURCE_DIR}/src
	${LIBMONGOCXX_INCLUDE_DIRS}
	${Boost_INCLUDE_DIRS}
	${PROJECT_SOURCE_DIR}/src/third_party/asio
)
	
file( GLOB SRC 
	#INCLUDE
	${PROJECT_SOURCE_DIR}/src/*.h

	#SOURCE
	${PROJECT_SOURCE_DIR}/src/*.cpp
)

file(GLOB SRC_THIRD_PARTY
	#INCLUDE
	${PROJECT_SOURCE_DIR}/src/third_party/*.h
	${PROJECT_SOURCE_DIR}/src/third_party/*.hpp
	
	#SOURCE
	${PROJECT_SOURCE_DIR}/src/third_party/*.cpp
	${PROJECT_SOURCE_DIR}/src/third_party/*.c
)

add_executable(SAS_Server
	${SRC}
	${SRC_THIRD_PARTY}
)

source_group("third_party"			FILES ${SRC_THIRD_PARTY})

target_link_libraries(SAS_Server
	PRIVATE ${LIBMONGOCXX_LIBRARIES}
)

target_compile_definitions(SAS_Server
	PRIVATE ${LIBMONGOCXX_DEFINITIONS}
	PRIVATE ASIO_STANDALONE
	PRIVATE ASIO_HAS_MOVE
	PRIVATE ASIO_HAS_STD_SYSTEM_ERROR
)

# Sanity-check that static library macros are not set when building against the shared library.
# Users don't need to include this section in their projects.
list(FIND LIBMONGOCXX_DEFINITIONS "BSONCXX_STATIC" LIST_IDX)
if (${LIST_IDX} GREATER -1)
    message(FATAL_ERROR "Expected BSONCXX_STATIC to not be defined")
endif()
list(FIND LIBMONGOCXX_DEFINITIONS "MONGOCXX_STATIC" LIST_IDX)
if (${LIST_IDX} GREATER -1)
    message(FATAL_ERROR "Expected MONGOCXX_STATIC to not be defined")
endif()