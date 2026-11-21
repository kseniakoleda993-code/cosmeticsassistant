execute_process(COMMAND "${CMAKE_COMMAND}" -E env --unset=TELEGRAM_BOT_TOKEN "${BOT}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    RESULT_VARIABLE result
    ERROR_VARIABLE error
)
if(NOT result STREQUAL "1" OR NOT error MATCHES "Не задан TELEGRAM_BOT_TOKEN")
    message(FATAL_ERROR "Ожидалась ошибка отсутствующего токена, получено ${result}: ${error}")
endif()
