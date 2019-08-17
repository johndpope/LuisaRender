file(GLOB GUI_SOURCES *.mm *.h *.cpp)
add_definitions(-fobjc-arc)
add_library(gui ${GUI_SOURCES})