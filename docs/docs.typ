== `Room.h`

=== Класс `Point`

Класс точки, являющейся началом или концом зеркальной стены.

*Конструкторы/деструктор*:

- `Point(const Vector2 &coord)` #h(1em) Конструктор точки с координатами `coord`.
- `Point(const json &j)` #h(1em) Конструктор из JSON.

*Поля*:

private:

- `Vector2 coord` #h(1em) Координаты точки.
- `vector<Wall *> walls` #h(1em) Связанные с точкой стены.

*Методы*:

public:

- `void setCoord(const Vector2 &coord)`
- `Vector2 getCoord()`
- `float getX()`
- `float getY()`
- `void updateWalls()` #h(1em) Обновление параметров связанных стен.
- `void addWall(Wall *wall)` #h(1em) Добавить в связанные стену `wall`.
- `json toJson()` #h(1em) Возвращает JSON-объект.
- `void draw()` #h(1em) Отрисовывает точку на экране приложения.
- `void clear(Wall *wall)` #h(1em) Удаляет из связанных стену `wall`.

=== Класс `Wall`

Абстрактный класс зеркальной стены.

*Конструкторы/деструктор*:

- `Wall(Point *start, Point *end, Room *room)` #h(1em) Конструктор стены с началом в `start`, с концом в `end` в комнате `room`.
- `~Wall()` #h(1em) Удаляет стену из связанных для точек `start` и `end`

*Поля*:

protected:

- `Point *start` #h(1em) Начальная точка.
- `Point *end` #h(1em) Конечная точка.

public:

- `Room *room`

*Методы*:

public:

- `void updateParams()` #h(1em) Обновление параметров стены.
- `void draw()` #h(1em) Отрисовывает стену на экране приложения.
- `json toJson()` #h(1em) Возвращает JSON-объект.
- `Point *getStart()`
- `Point *getEnd()`
- `Vector2 getNormal(const Vector2 &point)` #h(1em) Возвращает вектор нормали к стене в точке `point`.
- `Vector2 closestPoint(const Vector2 &point)` #h(1em) Возвращает ближайшую точку на стене к `point`.
- `float distanceToWall(const Vector2 &point)` #h(1em) Возвращает расстояние до `point`.
- `float getTByPoint(const Vector2 &point, float presicion = 0.1f)` #h(1em) Получить параметр $t in [0, 1]$ по точке на стене.
- `Vector2 getPointByT(float t)` #h(1em) Получить точку на стене по параметру  $t in [0, 1]$.

==== Класс `WallLine`

Класс стены-плоского зеркала. Наследуется публично от `Wall`.

==== Класс `WallRound`

Класс стены-сферического зеркала, представляющей собой дугу.  Наследуется публично от `Wall`.

*Вложенные классы*:

- `InvalidRadiusCoef: public std::exception` #h(1em) Исключение, выбрасывается, когда неверный `radiusCoef`.

*Конструкторы/деструктор*:

- `WallRound(Point *start, Point *end, Room *room, float radiusCoef, bool orient)` #h(1em) Конструктор стены-сферического зеркала с коэффициентом радиуса `radiusCoef` и с флагом выпуклости/вогнутости `orient`.

*Поля*:

private:

- `float startAngle` #h(1em) Угол начала относительно горизонтального луча, с вершиной в центре круга, направленного вправо.
- `float endAngle` #h(1em) Угол конца относительно горизонтального луча, с вершиной в центре круга, направленного вправо.
- `float radiusCoef` #h(1em) Коэффициент $k in (0, 100)$ для вычисления радиуса.
- `float chord` #h(1em) Длина хорды между `start` и `end` из конструктора.
- `float radius` #h(1em) Радиус дуги.
- `Vector2 center` #h(1em) Центр дуги.
- `bool isBig` #h(1em) `true` --- бóльшая, `false` --- меньшая дуга.
- `bool orient` #h(1em) `true` --- вогнутое (при рисовании выпуклой фигуры по часовой стрелке), `false` --- выпуклое.

*Методы*:

private:

- `void updateAngles()` #h(1em) Обновление углов.
- `bool crossesZeroDeg()` #h(1em) Вспомогательная функция, определяющая пересекает ли дуга $0 degree$.
- `float getAngularLength()` #h(1em) Вспомогательная функция, возвращает угловую ширину дуги.
- `float getAngleByT(float t)` #h(1em) Возвращает значение угла по параметру $t in [0, 1]$.
- `float getTByAngle(float angleDeg)` #h(1em) Возвращает значение параметра $t in [0, 1]$ по углу.

public:

- `void toggleOrient()` #h(1em) Делает противоположным флаг вогнутости/выпуклости дуги.
- `float getRadiusCoef()`
- `void setRadiusCoef(float radiusCoef)`
- `Vector2 closestPoint(const Vector2 &point)`
- `Vector2 getNormal(const Vector2 &point)`
- `Vector2 getPointByT(float t)`
- `float getTByPoint(const Vector2 &point, float precision = 0.1f)`
- `bool isAngleInArc(float angleDeg, float precision = 0.0f)` #h(1em) `true` --- если `angleDeg` попадает в дугу, иначе --- `false`.
- `bool isPointOnArc(const Vector2 &point, float precision = 0.1f)` #h(1em) `true` --- если `point` находится на дуге.
- `Vector2 getCenter()`
- `float getRadius()`
- `float getStartAngle()`
- `float getEndAngle()`

*Примечание*: для классов `Point` и `Wall` реализован паттерн "наблюдатель" c целью того, чтобы при перемещении любой точки, изменялись параметры связанных стен (это больше относится к объектам класса `WallRound`).

=== Класс `Room`

Комната, представляющая собой многоугольник.

*Вложенные классы*:

- `RoomException: public std::exception` #h(1em) Неизвестное исключение при работе c оптическим экспериментом.
- `PointsAreTooClose: public Room::RoomException` #h(1em) Исключение, выбрасывается, когда 2 точки слишком близки.
- `TooManyPoints: public Room::RoomException` #h(1em) Исключение, выбрасывается, когда точек больше установленного количества.
- `TooFewPoints: public Room::RoomException` #h(1em) Исключение, выбрасывается, когда точек слишком мало.

*Конструкторы/деструктор*:

- `Room()` #h(1em) Конструктор пустой комнаты.
- `Room(const json &j)` #h(1em) Конструктор из JSON.
- `~Room()` #h(1em) Удаляет все элементы комнаты.

*Поля*:

private:

- `vector<Point> points` #h(1em) Вершины многоугольника.
- `vector<Wall *> walls` #h(1em) Стены, ограничивающие комнату.

public:

- `RayStart *rayStart` #h(1em) Объект луча, если не установлен~--- `nullptr`.
- `float defaultRayAngle` #h(1em) Угол по отношению к стене, с которым луч создается по умолчанию.
- `AimArea *aim` #h(1em) Объект целевой зоны попадания луча, если не установлен --- `nullptr`.
- `const int static minimalDistance` #h(1em) Минимальное расстояние, на котором могут располагаться две точки.
- `const int static maximumPoints` #h(1em) Максимальное число точек в комнате.
- `const int static minimumPoints` #h(1em) Минимальное число точек в комнате.
- `const int static maximumRayDepth` #h(1em) Максимальное число переотражений.

*Методы*:

public:

- `void addAim(const Vector2 &center, float radius = 20.0f)` #h(1em) Добавить цель радиусом `radius` и c центром в точке `center`.
- `void moveAim(const Vector2 &newCenter)` #h(1em) Изменить центр цели.
- `bool isRayInAim(const Vector2 &rayStart, const Vector2 &rayEnd, Vector2 &intersectionPoint)` #h(1em) Возвращает `true` и изменяет `intersectionPoint`, если луч из `rayStart` до `rayEnd` пересекает цель, иначе --- возвращает `false`.
- `Wall *closestWall(const Vector2 &point)` #h(1em) Возвращает ближайшую стену к `point`, если она находится в зоне досягаемости мыши.
- `RayStart *closestRay(const Vector2 &point)` #h(1em) Возвращает луч, если `point` в зоне досягаемости до вершины луча.
- `bool isClosed()` #h(1em) Замкнутая ли комната.
- `WallLine *addWallLine(const Vector2 &coord)` #h(1em) Добавить в конец ломаной прямую стену c началом в предыдущей добавленной точке и концом в `point`. Если это первая точка, то добавляется только `point`.
- `WallRound *addWallRound(const Vector2 &coord, float radiusCoef = 50, bool orient = false)` #h(1em) Добавить в конец ломаной cферическую стену c началом в предыдущей добавленной точке и концом в `point`. Если это первая точка, то добавляется только `point`.
- `Wall *changeWallType(Wall *wall)` #h(1em) Изменяет тип стены `wall`.
- `void movePoint(Point &p, const Vector2 &coord)` #h(1em) Переместить точку `p` на заданные координаты.
- `void draw()` #h(1em) Отрисовывать все объекты эксперимента на экране приложения.
- `json toJson()` #h(1em) Возвращает JSON-объект.
- `void addRay(const Vector2 &point, bool inverted = false)` #h(1em) Добавляет объект луча в ближайшую точку к `point`, которая находится на какой-либо стене, если она находится в зоне досягаемости мыши.
- `vector<Wall *> &getWalls()`
- `void clear()` #h(1em) Очистка комнаты.

*Примечание*: статические поля класса `Room` инициализируются в `main.cpp`, например:

#figure(caption: "main.cpp")[
  ```cpp
  // --snip--
  const int Room::minimalDistance = 20;
  const int Room::maximumPoints = 9;
  const int Room::minimumPoints = 4;
  const int Room::maximumRayDepth = 10;
  // --snip--
  ```
]

== `Ray.h`

=== Класс `RaySegment`

Класс сегмента луча.

*Конструкторы/деструктор*:

- `RaySegment(const Vector2 &start, const Vector2 &end, int depth == 1)` #h(1em) Конструктор сегмента луча с заданными началом и концом и аргументом числа переотражений (чтобы остановиться, если число переотражений превысило ограничение).
- `~RaySegment()` #h(1em) рекурсивно вызывает деструктор для `next`.

*Поля*:

private:

- `Vector2 start` #h(1em) Точка начала.
- `Vector2 end` #h(1em) Точка конца.
- `bool hasHit` #h(1em) Было ли столкновение со стеной.
- `Vector2 hitPoint` #h(1em) Точка столкновения (если есть).
- `Wall *hitWall` #h(1em) Стена, с которой произошло столкновение.
- `RaySegment *next` #h(1em) Следующий сегмент луча.
- `int depth` #h(1em) Число переторажений.

*Методы*:

private:

- `Vector2 intersectionWithWallLine(WallLine *wall)` #h(1em) Вычисляет пересечение луча с WallLine, если пересечения нет, возвращает `{NAN, NAN}`.
- `Vector2 intersectionWithWallRound(WallRound *wall)` #h(1em) Вычисляет пересечение луча с WallRound, если пересечения нет, возвращает `{NAN, NAN}`.

public:

- `void updateParameters(Room *room)` #h(1em) Обновляет параметры луча, в процессе пересчета рекурсивно вызывает кострукторы `RaySegment`.
- `void draw() const` #h(1em) Отрисывовывает сегмент луча в окне приложения.

=== Класс `RayStart`

Класс вершины луча.

*Вложенные классы*:

- `InvalidAngle: public std::exception` #h(1em) Исключение, выбрасывается, когда неверный угол.
- `CantStartInCorner: public std::exception` #h(1em) Исключение, выбрасывается при попытке разместить начало луча в углу комнаты.

*Конструкторы/деструктор*:

- `RayStart(const Vector2 &point, Wall *wall, float angle, bool inverted = false)` #h(1em) Конструктор луча.
- `~RayStart()` #h(1em) Удаляет все элементы луча.

*Поля*:

private:

- `Vector2 start` #h(1em) Точка начала луча.
- `float angle` #h(1em) Угол в градусах $alpha in [1 degree, 179 degree]$ относительно родительской стены.
- `Wall *wall` #h(1em) Родительская стена.
- `float t` #h(1em) Параметр $t in [0, 1]$, задающий положение точки начала луча на родительской стене.
- `RaySegment *ray` #h(1em) Следующий сегмент луча.
- `bool inverted` #h(1em) Использовать ли инвертированный вектор нормали к стене (например, чтобы луч был направлен не внутрь комнаты, а из нее).

*Методы*:

public:

- `float getAngle()`
- `Vector2 getStart()`
- `Wall *getWall()`
- `void setAngle(float angle)`
- `void setWall(Wall *wall)`
- `void inverseT()` #h(1em) Инвертирует параметр $t$.
- `void inverseDirection()` #h(1em) Инвертирует направление луча (внутрь комнаты или из неё).
- `void updateRaySegments()` #h(1em) Обновляет параметры сегментов луча.
- `void updateParams()` #h(1em) Обновляет параметры.
- `json toJson()` #h(1em) Возвращает JSON-объект.
- `void draw()` #h(1em) Отрисовывает луч в окне приложения.

=== Класс `AimArea`

Класс целевой зоны попадания луча (круг).

*Конструкторы/деструктор*:

- `AimArea(const Vector2 &center, float radius = 20.0f)`

*Поля*:

private:

- `Vector2 center` #h(1em) Центр круга.
- `float radius` #h(1em) Радиус кругa.

*Методы*:

public:

- `Vector2 getCenter()`
- `float getRadius()`
- `void setCenter(const Vector2 &center)`
- `bool intersectsWithRay(const Vector2 &rayStart, const Vector2 &rayEnd, Vector2 &intersectionPoint)` #h(1em) Возвращает `true` и изменяет `intersectionPoint`, если луч от `rayStart` до `rayEnd` попадает в зону. Иначе --- возвращает `false`.
- `bool containsPoint(const Vector2 &point)` #h(1em) Проверка, находится ли точка внутри области.
- `json toJson()` #h(1em) Возвращает JSON-объект.
- `void draw()` #h(1em) Отрисовывает зону в окне приложения.

#figure(
  caption: "Схема, показывающая отношения между классами объектов оптического эксперимента. Пунктирными стрелками показаны взаимотношения классов через поля. Голубыми стрелками показано наследование",
)[
  #image("src/room_graph.jpg", width: 25em)
]

== `MyUI.h`

=== Класс `Button`

Объект кнопки.

*Поля*:

public:

- `Rectangle rect` #h(1em) Положение кнопки.
- `const char *text` #h(1em) Текст на кнопке (для отображения иконок в `raygui` в начале строки можно добавить подстроку вида `#01#`, где `01` --- номер иконки, подробнее в документации @raygui).

*Методы*:

public:

- `bool draw()` #h(1em) Отрисовать кнопку. Возвращает `true`, если кнопка была нажата, иначе --- `false`.
- `bool draw(bool isActive)` #h(1em) Перегрузка метода `draw()` с возможностью выделить кнопку как активную, если `isActive == true`.

=== Класс `MyUI`

Класс-оберктка для вызова функций `raygui`.

*Вложенные классы*:

`enum UIMode { UI_NORMAL, UI_ADD_LINE, UI_ADD_ROUND, UI_ADD_RAY, UI_ADD_AIM, UI_EDIT_LINE, UI_EDIT_ROUND, UI_EDIT_RAY, UI_IMPORT, UI_EXPORT, UI_CLEAR };` #h(1em) Режим интерфейса. Переключается пользователем нажатием соответствующих кнопок в меню.

*Конструкторы/деструктор*:

- `MyUI(const char *fontPath, const char *iconsPath)` #h(1em) --- создает объект интерфейса приложения с ttf-шрифтом, путь которого указан в `fontPath`, и с набором иконок (файл с расширением rgi), путь которого указан в `iconsPath`

*Поля*:

private:

- `Font font` #h(1em) Шрифт интерфейса.
- `float fontSize` #h(1em) Размер шрифта в пикселях.
- `float rightPanelWidth` #h(1em) Ширина правой панели.
- `std::string currentHint` #h(1em) Тект подсказки.
- `float hintTimer` #h(1em) Время, прошедшее с начала отображения подсказки.
- `float hintDuration` #h(1em) Продолжительность показа подсказки.
- `bool hintActive` #h(1em) Является ли подсказка активной в данный момент.
- `Vector2 screen` #h(1em) Размеры экрана.
- `Rectangle canvas` #h(1em) Размеры холста.
- `Rectangle panel` #h(1em) Размеры правой панели.
- `Wall *wall` #h(1em) Стена, которая в данный момент редактируется в правой панели. Если нет --- `nullptr`.
- `RayStart *rayStart` #h(1em) Луч, который редактируется в правой панели в данный момент. Если нет --- `nullptr`.
- `Vector2 hintPosition` #h(1em) Положение подсказки.
- `Rectangle hintBar` #h(1em) Размеры подсказки.
- `Button importButton` #h(1em) кнопка открытия диалогового окна выбора файла и др. кнопки, при нажатии на которых режим `mode` переключается на соответствующий.
- `Button exportButton`
- `Button normalButton`
- `Button addLineButton`
- `Button addRoundButton`
- `Button addRayButton`
- `Button addAimButton`
- `Button clearButton`
- `MyUI::UIMode mode` #h(1em) Режим редактирования, в котором находится интерфейс.

public:

- `FileDialog fileDialog` #h(1em) Объект диалогового окна открытия/сохранения файла.

*Методы*:

private:

- `Font initFont(const char *fontPath, float fontSize)` #h(1em) Функция инициализация ttf-шрифта с нужными codepoints (включают кириллицу).

public:

- `UIMode getMode()`
- `void setMode(UIMode newMode)` #h(1em) Обрабатывает смену режима `mode` на `newMode`.
- `void updateSize()` #h(1em) Обновляет положения элементов интерфейса (при изменении размеров экрана).
- `void updateHint()` #h(1em) Обновляет подсказку.
- `Rectangle getCanvas()`
- `void saveFile(Room *room)` #h(1em) Вызывает диалоговое окно сохранения файла.
- `Room *openFIle(Room *room)` #h(1em) Вызывает диалоговое окно открытия файла. Возвращает импортированный экземпляр эксперимента.
- `void showHint(const char *message)` #h(1em) Вызывает подсказку с сообщением `message`.
- `void showPanel(Wall *wall, RayStart *rayStart)` #h(1em) Открывает в правой панели редактирование объекта: если `wall != nullptr` открывает `wall`, иначе, если `rayStart != nullptr`, открывает `rayStart`.
- `void drawPanel()` #h(1em) Отрисовывает правую панель.
- `void handleButtons(bool isClosed)` #h(1em) Обрабатывает нажатие кнопок в меню. Если `isClosed == true`, то комната замкнута, добавлять стены нельзя.

== `FileDialog.h`

=== Класс `FileDialog`

Объект диалогового окна открытия/сохранения файла.

*Вложенные классы*:

- `enum Mode { FILE_DIALOG_OPEN, FILE_DIALOG_SAVE };` #h(1em) режим открытие/сохранение файла.

*Конструкторы/деструктор*:

- `FileDialog()`
- `~FileDialog()`

*Поля*:

private:

- `bool windowActive` #h(1em) Является ли окно активным.
- `Rectangle windowBounds` #h(1em) Прямоугольник, описывающий приложение окна.
- `Vector2 panOffset`
- `bool dragMode`
- `bool supportDrag`
- `bool dirPathEditMode`
- `int filesListScrollIndex`
- `bool filesListEditMode`
- `int filesListActive`
- `bool fileNameEditMode`
- `bool SelectFilePressed`
- `bool CancelFilePressed`
- `int itemFocused`
- `FilePathList dirFiles`
- `std::string filterExt`
- `fs::path dirPathTextCopy`
- `fs::path fileNameTextCopy`
- `int prevFilesListActive`
- `Mode dialogMode` #h(1em) Режим окна (сохранение/открытие).
- `std::vector<std::string> dirFilesIcon`

*Методы*:

private:

- `void ReloadDirectoryFiles()` #h(1em) Обновляет список содержимого текущей директории.

public:

- `void update()` #h(1em) Отрисовывет в окне приложения диалоговое окно, если оно активно.
- `bool isActive() const`
- `bool isFileSelected() const`
- `fs::path filePath()` #h(1em) Возвращает выбранный путь в файловой системе.
- `void show(Mode mode)` #h(1em) Вызывает окно с режимом `mode`.

#figure(
  caption: "Схема, показывающая отношения между классами объектов интерфейса. Пунктирными стрелками показаны взаимотношения классов через поля.",
)[
  #image("src/myui_graph.jpg", width: 22em)
]

== Пример обращения к классам в `main.cpp`

#figure(
  caption: "Схема, показывающая включение заголовчных файлов C++",
)[
  #image("src/main_incl.jpg")
]

#figure(caption: "Примерная структрура main.cpp")[
  ```cpp
  // --snip--
  // Установка static полей класса Room из листинга 1
  // --snip--
  int main() {
      MyUI ui = MyUI("assets/fonts/AdwaitaSans-Regular.ttf", "assets/iconset.rgi");
      Room *room = new Room();

      while (!WindowShouldClose()) { // mainloop, каждая итерация - один кадр
          ui.updateSize();
          if (ui.fileDialog.isFileSelected()) {
            // Открытие/создание файла через ui.openFIle(room)/ui.saveFile(room)
          }
          if (ui.getMode() == MyUI::UI_CLEAR) {
            // Очистка экрана через room.cler()
          }

          BeginDrawing();
          // Отрисовка фона
          // --snip--
          ui.updateHint();
          if (ui.fileDialog.isActive()) {
              GuiLock();
          }
          ui.handleButtons(room->isClosed());
          // Область для рисования
          BeginScissorMode(
              ui.getCanvas().x, ui.getCanvas().y, ui.getCanvas().width,
              ui.getCanvas().height
          );
          // Рисование линий
          if (ui.getMode() == MyUI::UI_ADD_LINE) {
             // --snip--
          }
          // Аналогично --- рисование дуг, добавление цели и луча
          // --snip--
          room->draw();
          EndScissorMode();

          // Если в нужном режиме кликнули на объект, отобразить его через ui.showPanel(...)
          ui.drawPanel();

          GuiUnlock();
          ui.fileDialog.update();

          EndDrawing();
      }
      CloseWindow();
      delete room;
      return 0;
  }
  ```
]
