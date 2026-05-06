# CMakeList.txt : CMake project for CodeGeneratorCore, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable (CodeGeneratorCore 
	main.cpp
	Parser.h
	Parser.cpp
	Structs.h
	IR.h
	IR.cpp
	Helpers.h
	Helpers.cpp
	CodeGenerator.h
	CodeGenerator.cpp
	TRS.h
	TRS.cpp
	TermParser.h
	TermParser.cpp
)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET CodeGeneratorCore PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
