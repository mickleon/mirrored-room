#import "conf.typ": conclusion, conf, intro
#import "@preview/treet:1.0.0": *
#import "@preview/zebraw:0.5.5": zebraw

#show: conf.with(
  title: [
    #par()[#text(size: 20pt, weight: "bold")[ПРИЛОЖЕНИЕ "ЗЕРКАЛЬНАЯ КОМНАТА"]]
    #par()[#text(size: 15pt)[Руководство программиста]]
  ],
  type: "referat",
  info: (
    author: (
      name: [Леoнтьева Михаил Анатольевича],
      faculty: [КНиИТ],
      group: "251",
      sex: "male",
    ),
    inspector: (
      degree: "",
      name: "",
    ),
  ),
  settings: (
    title_page: (
      enabled: true,
    ),
    contents_page: (
      enabled: true,
    ),
  ),
)

#show link: body => raw(body.dest)

#let icon = path => {
  box(baseline: 30%)[
    #image(path, height: 1.5em)
  ]
}

#let attention = body => {
  rect()[*ВНИМАНИЕ!* #body]
}

#intro

== Назначение

Данное руководство посвящено программированию приложения "Зеркальная комната" на языке C++. В руководстве описывается структура классов, для каждого класса описываются поля, методы и их назначение.

== Целевая аудитория

Руководство предназначено для программистов, имеющих навык разработки на языке C++.

= РАБОТА С ПРОЕКТОМ

#figure(caption: [Структура проекта приложения "Зеркальная комната"], kind: image)[
  #set par(first-line-indent: (amount: 0pt), leading: 0.33em)
  #set text(font: "JetBrainsMono NF")
  #set align(left)

  `MirroredRoom/`
  #linebreak()
  #text(size: 11pt)[
    #tree-list()[
      - assets/
        - ...
      - libraries/
        - json/
          - ...
        - raygui/
          - ...
        - raylib/
          - ...
        - CMakeLists.txt
      - CMakeLists.txt
      - FileDialog.cpp
      - FileDialog.h
      - MyUI.cpp
      - MyUI.h
      - Ray.cpp
      - Ray.h
      - Room.cpp
      - Room.h
      - main.cpp
      - maker.sh
      - winregen.bat
    ]
  ]
]<tree-list>

== Зависимости

В проекте используются следующие C++ библиотеки:

- `nlohmann/json` --- для работы с JSON-файлами @json
- `raylib` --- для работы с графикой @raylib;
- `raygui` --- вспомогательная библиотека для `raylib`, используемая для отображения элементов графического пользовательского интерфейса @raygui.

Устанавливать эти библиотеки для работы с данным проектом не требуется. Исходный код этих библиотек расположен в соответствующих поддиректориях директории `libraries`.

== Windows

Для создания sln-файла проекта Visual Studio необходимо запустить скрипт из файла `winregen.bat`, что можно сделать прямо из проводника двойным кликом (или из `cmd.exe`).

После выполнения скрипта создастся директория `build` в корне проекта, в которой лежит файл решения .sln. Его можно открыть в Visual Studio и начать
работать с проектом.

== macOS/Linux

Перед началом работы может потребоваться установить нужные библиотеки.

- Ubuntu/Debian:

  `sudo apt-get install cmake make g++ libasound2-dev libx11-dev libxrandr-dev libxi-dev libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev libxinerama-dev libwayland-dev libxkbcommon-dev libgtk-3-dev`

- ALT Linux:

  `apt-get install cmake make gcc-c++ wayland-devel libwayland-client-devel libwayland-cursor-devel libwayland-egl-devel libxkbcommon-devel libX11-devel libXrandr-devel libXinerama-devel libXcursor-devel libXi-devel libgtk+3-devel libGL-devel`

В проекте есть нужные файлы для CMake, и сборка проекта производится стандартными командами для этого сборщика.

Также для сборки в директорию `build` можно запустить из корня проекта скрипт из файла `maker.sh`:

- в режиме отладки

  `./maker.sh debug`

- в режиме выпуска

  `./maker.sh release`

= ОБРАЩЕНИЕ К ПРОГРАММЕ

Данный раздел описывает структуру и поведение классов, код которых располагается в заголовочных файлах C++ и сопутствующим им cpp-файлам. В данном разделе мы рассмотрим все заголовочные файлы и классы, которые в них находятся.

#include "docs.typ"

= ВХОДНЫЕ И ВЫХОДНЫЕ ДАННЫЕ

Класс `MyUI` имеет следующие методы работы с файлами:

- `Room *openFIle(Room *room)` --- открывает файл, выбранный в `fileDialog`, и заменяет объект `room` новым, сконструированным из JSON-объекта, записанным в выбранном файле;
- `void saveFile(Room *room)` --- производит запись в файл, выбранный в `fileDialog`, JSON-объекта экземпляра эксперимента `room` при помощи метода `toJson()` класса `Room`.

Файл на входе и выходе должен быть валидным JSON-файлом структуры вида:

#figure(caption: "Образец входного/выходного JSON-файла.")[
  ```json
  {
    "aim": {
      "center": { "x": 242.0, "y": 169.0 },
      "radius": 20.0
    },
    "points": [
      { "x": 173.0, "y": 324.0},
      { "x": 173.0, "y": 176.0},
      {"x": 401.0, "y": 155.0},
      { "x": 416.0,"y": 316.0}
    ],
    "rayStart": {
      "angle": 1.57,
      "inverted": false,
      "start": { "x": 235.42, "y": 86.68}
    },
    "walls": [
      { "type": "line" },
      {
        "orient": false,
        "radiusCoef": 96.29629516601563,
        "type": "round"
      },
      {
        "orient": true,
        "radiusCoef": 50.0,
        "type": "round"
      },
      { "type": "line" }
    ]
  }
  ```
]

Поля JSON-объекта соответствуют полям класса `Room`. Поля `aim` и `rayStart` не являются обязательными. Обязательными являются поля `points` и `walls`, причем они оба должны являться списками.

Список `points` обозначает массив `points` в классе `Room`.

Список `walls` обозначает массив `walls` в классе `Room`, элементами которого являются объекты класса `Wall`. Причем, если элемент является объектом класса `WallLine`, то он должен содержать единственное поле `type` со значением `line`. Если же элемент является объектом класса `WallRound`, то он должен содержать поля: `type` со значением `round`, `orient` и `radiusCoef` обозначают то же, что в конструкторе класса `WallRound`.

Поля объектов `aim` и `rayStart` обозначают то же, что и поля в конструкторах классов `Aim` и `RayStart` соответственно.

При этом, объекты, обозначающие параметры типа `Vector2` или `Point` (поле `center` объекта `aim`, элементы списка `points` и поле `start` объекта `rayStart`) должны иметь поля `x` и `y` численного типа.

#bibliography("thesis.bib", style: bytes(read("gost-7-1-2003.csl")))

