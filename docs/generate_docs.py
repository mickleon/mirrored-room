#!/usr/bin/env python3
"""
Генератор документации в формате typst для C++ заголовочных файлов
Использование: python generate_docs.py <путь_к_заголовочным_файлам>
"""

import re
import sys
from pathlib import Path
from typing import Dict, List


class CppParser:
    """Парсер C++ заголовочных файлов"""

    def __init__(self, filepath: str):
        self.filepath = filepath
        self.content = self._read_file()
        self.classes = []

    def _read_file(self) -> str:
        """Читает содержимое файла"""
        with open(self.filepath, 'r', encoding='utf-8') as f:
            return f.read()

    def _remove_comments(self, text: str) -> str:
        """Удаляет комментарии из кода"""
        # Удаляем многострочные комментарии
        text = re.sub(r'/\*.*?\*/', '', text, flags=re.DOTALL)
        # Удаляем однострочные комментарии
        text = re.sub(r'//.*?$', '', text, flags=re.MULTILINE)
        return text

    def _parse_inheritance(self, inheritance_str: str) -> List[Dict[str, str]]:
        """Парсит строку наследования и возвращает список родительских классов"""
        parents = []
        if not inheritance_str:
            return parents

        # Разбиваем по запятым (множественное наследование)
        parts = inheritance_str.split(',')

        for part in parts:
            part = part.strip()
            # Паттерн: (public|protected|private)? ClassName (поддержка :: для namespace)
            match = re.match(r'(public|protected|private)?\s*([\w:]+)', part)
            if match:
                access = match.group(1) if match.group(1) else 'private'
                class_name = match.group(2)
                parents.append({'name': class_name, 'access': access})

        return parents

    def parse(self):
        """Парсит заголовочный файл"""
        # Не удаляем комментарии, они нам нужны для документации
        content = self.content

        # Ищем все классы с возможным наследованием
        class_pattern = r'class\s+(\w+)\s*(?::\s*(.*?))?\s*\{'
        class_matches = re.finditer(class_pattern, content)

        for match in class_matches:
            class_name = match.group(1)
            inheritance_str = match.group(2)
            class_start = match.start()

            # Находим конец класса (считаем фигурные скобки)
            class_end = self._find_class_end(content, class_start)
            class_body = content[class_start:class_end]

            # Ищем комментарий перед классом
            class_comment = self._extract_comment_before(content, class_start)

            class_info = {
                'name': class_name,
                'comment': class_comment,
                'parents': self._parse_inheritance(inheritance_str),
                'private_fields': [],
                'public_fields': [],
                'protected_fields': [],
                'private_methods': [],
                'public_methods': [],
                'protected_methods': [],
                'nested_classes': [],
            }

            self._parse_class_body(class_body, class_info)
            self.classes.append(class_info)

    def _find_class_end(self, text: str, start: int) -> int:
        """Находит конец определения класса"""
        brace_count = 0
        in_class = False

        for i in range(start, len(text)):
            if text[i] == '{':
                brace_count += 1
                in_class = True
            elif text[i] == '}':
                brace_count -= 1
                if in_class and brace_count == 0:
                    return i + 1

        return len(text)

    def _extract_comment_before(self, text: str, pos: int) -> str:
        """Извлекает комментарий перед позицией"""
        # Ищем комментарий перед классом (однострочный //)
        lines_before = text[:pos].split('\n')

        # Идем назад и собираем комментарии
        comment_lines = []
        for line in reversed(lines_before):
            stripped = line.strip()
            if stripped.startswith('//'):
                # Убираем // и пробелы
                comment_text = stripped[2:].strip()
                comment_lines.insert(0, comment_text)
            elif stripped == '':
                # Пустая строка - продолжаем
                continue
            else:
                # Не комментарий - прекращаем
                break

        return ' '.join(comment_lines) if comment_lines else ''

    def _extract_inline_comment(self, line: str) -> tuple[str, str]:
        """Извлекает inline комментарий из строки, возвращает (код, комментарий)"""
        # Ищем // комментарий
        comment_pos = line.find('//')
        if comment_pos != -1:
            code = line[:comment_pos].strip()
            comment = line[comment_pos + 2 :].strip()
            return code, comment
        return line.strip(), ''

    def _parse_class_body(self, body: str, class_info: Dict):
        """Парсит тело класса"""
        current_access = 'private'  # По умолчанию в классах
        class_info['constructors'] = []  # Добавляем список конструкторов

        lines = body.split('\n')
        i = 0

        while i < len(lines):
            original_line = lines[i]
            line, inline_comment = self._extract_inline_comment(original_line)

            # Проверяем изменение уровня доступа
            if line.startswith('private:'):
                current_access = 'private'
                i += 1
                continue
            elif line.startswith('public:'):
                current_access = 'public'
                i += 1
                continue
            elif line.startswith('protected:'):
                current_access = 'protected'
                i += 1
                continue

            # Проверяем вложенные классы/структуры
            if re.match(r'(class|struct)\s+\w+', line):
                nested_match = re.search(r'(class|struct)\s+(\w+)', line)
                if nested_match:
                    class_info['nested_classes'].append(
                        {
                            'name': nested_match.group(2),
                            'access': current_access,
                            'comment': inline_comment,
                        }
                    )

            # Парсим методы и поля
            if line and not line.startswith('/*'):
                # Конструктор (включая деструктор и конструктор копирования)
                # Обычный конструктор: ClassName(params)
                # Конструктор копирования: ClassName(const ClassName&)
                # Деструктор: ~ClassName()
                constructor_match = re.match(
                    r'(virtual\s+)?~?(\w+)\s*\((.*?)\)\s*(=\s*(default|delete))?\s*(noexcept)?\s*;?',
                    line,
                )

                if constructor_match:
                    class_name = constructor_match.group(2)
                    # Проверяем, что это конструктор/деструктор текущего класса
                    if class_name == class_info[
                        'name'
                    ] or constructor_match.group(1):
                        params = constructor_match.group(3).strip()
                        is_destructor = line.startswith('~')
                        specifier = constructor_match.group(4) or ''

                        if is_destructor:
                            signature = f'~{class_name}()'
                        else:
                            signature = f'{class_name}({params})'

                        if specifier:
                            signature += f' {specifier}'

                        if constructor_match.group(5):
                            signature += f' noexcept'

                        class_info['constructors'].append(
                            {
                                'signature': signature,
                                'comment': inline_comment,
                                'access': current_access,
                            }
                        )

                        i += 1
                        continue

                # Метод (кроме конструкторов)
                method_match = re.match(
                    r'(virtual\s+)?(static\s+)?([\w\s\*&<>:,\[\]\:]+?)\s+([\w\*&]+)\s*\((.*?)\)\s*(const)?\s*(noexcept)?\s*;?',
                    line,
                )

                if method_match:
                    method_name = method_match.group(4)
                    return_type = method_match.group(3).strip()
                    params = method_match.group(5).strip()
                    is_const = method_match.group(6) is not None

                    # Пропускаем конструкторы (они уже обработаны выше)
                    if method_name == class_info['name']:
                        i += 1
                        continue

                    method_str = f'{return_type} {method_name}({params})'
                    if is_const:
                        method_str += ' const'

                    class_info[f'{current_access}_methods'].append(
                        {'signature': method_str, 'comment': inline_comment}
                    )
                # Поле (с поддержкой инициализации при определении)
                elif ';' in line and '(' not in line:
                    # Используем более гибкое регулярное выражение для полей
                    field_match = re.match(
                        r'([\w\s\*&<>:,\[\]\:]+?)\s+([\w\*&]+)\s*(?:=\s*[^;]+)?\s*;',
                        line,
                    )
                    if field_match:
                        field_type = field_match.group(1).strip()
                        field_name = field_match.group(2).strip()
                        # Пропускаем объявления дружественных функций
                        if field_type == 'friend':
                            i += 1
                            continue
                        class_info[f'{current_access}_fields'].append(
                            {
                                'type': field_type,
                                'name': field_name,
                                'comment': inline_comment,
                            }
                        )

            i += 1


class TypstGenerator:
    """Генератор typst документации"""

    def __init__(self, parser: CppParser):
        self.parser = parser
        self.output = []

    def generate(self) -> str:
        """Генерирует typst документацию"""
        self.output = []

        # Заголовок файла
        filename = Path(self.parser.filepath).name
        self.output.append(f'= `{filename}`\n')

        # Документируем каждый класс
        for class_info in self.parser.classes:
            self._generate_class_doc(class_info)

        return '\n'.join(self.output)

    def _generate_class_doc(self, class_info: Dict):
        """Генерирует документацию для класса"""
        # Формируем заголовок с наследованием
        header = f'{class_info["name"]}'
        if class_info['parents']:
            inheritance_parts = []
            for parent in class_info['parents']:
                inheritance_parts.append(
                    f'{parent["access"]} {parent["name"]}'
                )
            header += ': ' + ', '.join(inheritance_parts)

        self.output.append(f'== Класс `{header}`\n')

        # Комментарий к классу
        if class_info['comment']:
            self.output.append(f'{class_info["comment"]}\n')

        # Вложенные классы
        if len(class_info['nested_classes']) > 1:
            self.output.append('*Вложенные классы*\n')
            for nested in class_info['nested_classes'][1:]:
                if nested.get('comment'):
                    self.output.append(
                        f'- `{nested["name"]}` #linebreak() {nested["comment"]}.'
                    )
                else:
                    self.output.append(f'- `{nested["name"]}`')
            self.output.append('')

        # Конструкторы и деструкторы
        if class_info['constructors']:
            self.output.append('*Конструкторы/деструктор*\n')
            for constructor in class_info['constructors']:
                if constructor.get('comment'):
                    self.output.append(
                        f'- `{constructor["signature"]}` #linebreak() {constructor["comment"]}.'
                    )
                else:
                    self.output.append(f'- `{constructor["signature"]}`')
            self.output.append('')

        if (
            class_info['private_fields']
            or class_info['protected_fields']
            or class_info['public_fields']
        ):
            self.output.append('*Поля*\n')

            if class_info['private_fields']:
                self.output.append('private:\n')
                for field in class_info['private_fields']:
                    if field.get('comment'):
                        self.output.append(
                            f'- `{field["type"]} {field["name"]}` #linebreak() {field["comment"]}.'
                        )
                    else:
                        self.output.append(
                            f'- `{field["type"]} {field["name"]}`'
                        )
                self.output.append('')

            if class_info['protected_fields']:
                self.output.append('protected:\n')
                for field in class_info['protected_fields']:
                    if field.get('comment'):
                        self.output.append(
                            f'- `{field["type"]} {field["name"]}` #linebreak() {field["comment"]}.'
                        )
                    else:
                        self.output.append(
                            f'- `{field["type"]} {field["name"]}`'
                        )
                self.output.append('')

            if class_info['public_fields']:
                self.output.append('public:\n')
                for field in class_info['public_fields']:
                    if field.get('comment'):
                        self.output.append(
                            f'- `{field["type"]} {field["name"]}` #linebreak() {field["comment"]}.'
                        )
                    else:
                        self.output.append(
                            f'- `{field["type"]} {field["name"]}`'
                        )
                self.output.append('')

        if (
            class_info['private_methods']
            or class_info['protected_methods']
            or class_info['public_methods']
        ):
            self.output.append('*Методы*\n')

            if class_info['private_methods']:
                self.output.append('private:\n')
                for method in class_info['private_methods']:
                    if method.get('comment'):
                        self.output.append(
                            f'- `{method["signature"]}` #linebreak() {method["comment"]}.'
                        )
                    else:
                        self.output.append(f'- `{method["signature"]}`')
                self.output.append('')

            if class_info['protected_methods']:
                self.output.append('protected:\n')
                for method in class_info['protected_methods']:
                    if method.get('comment'):
                        self.output.append(
                            f'- `{method["signature"]}` #linebreak() {method["comment"]}.'
                        )
                    else:
                        self.output.append(f'- `{method["signature"]}`')
                self.output.append('')

            if class_info['public_methods']:
                self.output.append('public:\n')
                for method in class_info['public_methods']:
                    if method.get('comment'):
                        self.output.append(
                            f'- `{method["signature"]}` #linebreak() {method["comment"]}.'
                        )
                    else:
                        self.output.append(f'- `{method["signature"]}`')
                self.output.append('')


def main():
    if len(sys.argv) < 2:
        print('Использование: python generate_docs.py <файл1.h> [файл2.h] ...')
        print('Или: python generate_docs.py <папка_с_заголовками>')
        sys.exit(1)

    input_path = Path(sys.argv[1])

    # Определяем список файлов для обработки
    if input_path.is_dir():
        header_files = list(input_path.glob('*.h')) + list(
            input_path.glob('*.hpp')
        )
    else:
        header_files = [Path(arg) for arg in sys.argv[1:]]

    if not header_files:
        print('Не найдено заголовочных файлов!')
        sys.exit(1)

    # Генерируем документацию
    all_docs = []

    for header_file in header_files:
        print(f'Обработка {header_file}...')
        parser = CppParser(str(header_file))
        parser.parse()

        generator = TypstGenerator(parser)
        doc = generator.generate()
        all_docs.append(doc)

    # Сохраняем результат
    output_file = 'docs.typ'
    with open(output_file, 'w', encoding='utf-8') as f:
        f.write('\n'.join(all_docs))

    print(f'\n✓ Документация сохранена в {output_file}')


if __name__ == '__main__':
    main()
