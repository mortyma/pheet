add_library(msp-generator Generator.cpp)
target_link_libraries(msp-generator msp-graph)

add_executable(gengraph main.cpp
	../../../Util.cpp
)
target_link_libraries(gengraph
	${LIBCGRAPH_LIBRARY}
	msp-generator
	msp-graph
)
