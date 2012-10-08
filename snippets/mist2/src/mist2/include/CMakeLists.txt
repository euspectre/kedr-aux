set(INCLUDE_FILES_INSTALLED "mist.hh")

foreach(include_file ${INCLUDE_FILES_INSTALLED})
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/mist2/${include_file}"
            DESTINATION "${CMAKE_INSTALL_PREFIX}/include/mist2")
endforeach(include_file ${INCLUDE_FILES_INSTALLED})
