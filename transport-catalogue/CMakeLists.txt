cmake_minimum_required(VERSION 3.10)

project(TransportCatalogue CXX)
set(CMAKE_CXX_STANDARD 17)

# Эта команда найдёт собранный нами пакет Protobuf.
# REQUIRED означает, что библиотека обязательна.
# Путь для поиска укажем в параметрах команды cmake.
find_package(Protobuf REQUIRED)
# Помимо Protobuf, понадобится библиотека Threads
find_package(Threads REQUIRED)

set(proto 
	transport_catalogue.proto
	map_renderer.proto
	svg.proto
	graph.proto
	transport_router.proto
)

set(cpp_file 
	domain.cpp 
	geo.cpp 
	json.cpp
	json_builder.cpp 
	json_reader.cpp 
	main.cpp
	map_renderer.cpp 
	request_handler.cpp
	serialization.cpp 
	svg.cpp
	transport_catalogue.cpp 
	transport_router.cpp
)

set(h_file 
	domain.h 
	geo.h 
	graph.h 
	json.h
	json_builder.h 
	json_reader.h 
	map_renderer.h 
	ranges.h 
	request_handler.h 
	router.h 
	serialization.h 
	svg.h 
	transport_catalogue.h
	transport_router.h
)

# Команда вызова protoc. 
# Ей переданы названия переменных, в которые будут сохранены 
# списки сгенерированных файлов, а также сам proto-файл.
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${proto})

# добавляем цель - transport catalogue
add_executable(transport_catalogue ${PROTO_SRCS} ${PROTO_HDRS} ${proto} ${cpp_file} ${h_file})

# find_package определила переменную Protobuf_INCLUDE_DIRS,
# которую нужно использовать как include-путь.
# Также нужно добавить как include-путь директорию, куда
# protoc положит сгенерированные файлы.
target_include_directories(transport_catalogue PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(transport_catalogue PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

string(REPLACE "protobuf.lib" "protobufd.lib" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
string(REPLACE "protobuf.a" "protobufd.a" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")

# Также find_package определила Protobuf_LIBRARY.
# Protobuf зависит от библиотеки Threads. Добавим и её при компоновке.
target_link_libraries(transport_catalogue "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY}>" Threads::Threads)