# mirrored-room

## Windows

Для генерация sln-файла желательно запустить winregen.bat, что можно сделать
прямо из проводника двойным кликом (или из cmd.exe).

После выполнения скрипта создастся директория build в корне проекта, в которой
лежит файл решения .sln. Его можно открыть в Visual Studio и начать
работать с проектом.

## Linux/Mac

Установка зависимостей:

- Ubuntu/Debian:

  ```
  sudo apt-get install cmake make g++ libasound2-dev libx11-dev libxrandr-dev libxi-dev libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev libxinerama-dev libwayland-dev libxkbcommon-dev libgtk-3-dev
  ```

- ALT Linux:

  ```
  apt-get install cmake make gcc-c++ wayland-devel libwayland-client-devel libwayland-cursor-devel libwayland-egl-devel libxkbcommon-devel libX11-devel libXrandr-devel libXinerama-devel libXcursor-devel libXi-devel libgtk+3-devel libGL-devel
  ```

Для сборки можно использовать скрипт maker.sh (это делать нужно из корня проекта):

```sh
./maker.sh debug && ./build/MirroredRoom
```
