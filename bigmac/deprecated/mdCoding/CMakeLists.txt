set(MDcode mdCoding.cpp)
add_library(mdcode ${MDcode})

add_executable(mdTest unittest.cpp)

if(UNIX)
	target_link_libraries(
		mdTest
        mdcode
		${Boost_LIBRARIES}
		pthread
		)
elseif(WIN32)
elseif(APPLE)
endif(UNIX)
