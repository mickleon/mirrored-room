#!/bin/sh

debug() {
    set -o xtrace
    cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=True \
        -DBUILD_SHARED_LIBS=False \
        -S . -B ./build \
        -D CMAKE_BUILD_TYPE=Debug &&
        cmake --build ./build --parallel
}

release() {
    set -o xtrace
    cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=True \
        -DBUILD_SHARED_LIBS=False \
        -S . -B ./build \
        -D CMAKE_BUILD_TYPE=Release &&
        cmake --build ./build --parallel
}

clean() {
    set -o xtrace
    rm -rf ./build
}

help() {
    echo "Использование:"
    echo "./maker.sh release -> Собрать проект с помощью CMake и make -C ./build/"
    echo "./maker.sh debug -> Собрать проект с сохранением информации для отладки"
    echo "./maker.sh clean -> Удалить содержимое директории ./build/"
}

case "$1" in
debug) debug ;;
release) release ;;
winbuild) winbuild ;;
clean) clean ;;
*) help ;;
esac
