# name of the application's main executable
set (APP_NAME ${PROJECT_NAME})

set (BS_MAILER_SOURCES
    main.cpp
    Common.cpp
    ValueLoader.cpp
    BSMailer.cpp
)

add_executable (${APP_NAME} ${BS_MAILER_SOURCES})

# libraries to link with
set (EXTRA_LIBS vmime)
target_link_libraries (${APP_NAME} ${EXTRA_LIBS})

set_target_properties (${APP_NAME} PROPERTIES
    INSTALL_RPATH "${VMIME_LIB_DIR}"
)

install (TARGETS ${APP_NAME} DESTINATION bin)
