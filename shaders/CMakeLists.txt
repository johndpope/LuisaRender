file(GLOB SHADER_SOURCE_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/*.metal)
file(GLOB SHADER_IR_FILES_OLD RELATIVE ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_BINARY_DIR}/*.air)

add_custom_target(shaders ALL SOURCES ${SHADER_SOURCE_FILES})

foreach (IR_FILE ${SHADER_IR_FILES_OLD})
    add_custom_command(TARGET shaders PRE_BUILD
                       COMMAND rm ${CMAKE_CURRENT_BINARY_DIR}/${IR_FILE}
                       COMMENT "Cleaning old IR file: ${CMAKE_CURRENT_BINARY_DIR}/${IR_FILE}")
endforeach ()

foreach (SOURCE_FILE ${SHADER_SOURCE_FILES})
    add_custom_command(TARGET shaders
                       DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE}
                       COMMAND xcrun -sdk macosx metal -c ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE} -o ${CMAKE_CURRENT_BINARY_DIR}/${SOURCE_FILE}.air
                       COMMENT "Generating Metal IR file: ${CMAKE_CURRENT_BINARY_DIR}/${SOURCE_FILE}.air")
endforeach ()

add_custom_command(TARGET shaders
                   COMMAND xcrun -sdk macosx metallib ${CMAKE_CURRENT_BINARY_DIR}/*.air -o ${RESOURCE_DIR}/shaders.metallib
                   COMMENT "Generating Metal Library: ${RESOURCE_DIR}/shaders.metallib")
