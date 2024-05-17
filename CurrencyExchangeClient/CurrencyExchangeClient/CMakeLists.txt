# CMakeList.txt: проект CMake для CurrencyExchangeClient2; включите исходный код и определения,
# укажите здесь логику для конкретного проекта.
#

# Добавьте источник в исполняемый файл этого проекта.
add_executable (CurrencyExchangeClient2 "CurrencyExchangeClient2.cpp" )

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET CurrencyExchangeClient2 PROPERTY CXX_STANDARD 20)
endif()

# TODO: Добавьте тесты и целевые объекты, если это необходимо.
