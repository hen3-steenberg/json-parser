# CMakeList.txt : CMake project for JSON, include source and define
# project specific logic here.
#

# Add source to this project's executable.
file(GLOB modules "${CMAKE_CURRENT_SOURCE_DIR}/*.ixx")
set(JSON_MODULES 
	${modules} 
	PARENT_SCOPE
)
# TODO: Add tests and install targets if needed.
