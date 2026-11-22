cmake_minimum_required(VERSION 3.10)
project(analizator_pcap C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(CAIRO REQUIRED cairo)
pkg_check_modules(PCAP REQUIRED libpcap)

add_executable(analizator_pcap
    src/main.c
    src/parser.c
    src/report.c
    src/charts.c
)

target_include_directories(analizator_pcap PRIVATE ${CAIRO_INCLUDE_DIRS} ${PCAP_INCLUDE_DIRS} src)
target_link_libraries(analizator_pcap PRIVATE ${CAIRO_LIBRARIES} ${PCAP_LIBRARIES} m)