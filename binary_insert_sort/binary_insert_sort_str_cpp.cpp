#include <algorithm>
#include <fstream>
#include <iostream>
#include <limits>
#include <random>
#include <string>
#include <vector>

const int MIN_LEN = 14;

struct TableItem {
    std::string key;
    std::string data;
};

bool isValidField(const std::string& value) {
    return !value.empty();
}

int countLinesInFile(const std::string& filename, int& count) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Ошибка: не удалось открыть файл.\n";
        return 1;
    }

    std::string line;
    int local_count = 0;

    while (std::getline(file, line)) {
        if (!line.empty()) {
            local_count++;
        }
    }

    count = local_count;
    return 0;
}

int parseLine(const std::string& line, TableItem& item) {
    std::size_t separator = line.find(':');
    if (separator == std::string::npos) {
        std::cerr << "Ошибка: строка не содержит разделитель ':'.\n";
        return 1;
    }

    std::string key = line.substr(0, separator);
    std::string data = line.substr(separator + 1);

    if (!isValidField(key)) {
        std::cerr << "Ошибка: ключ не должен быть пустым.\n";
        return 1;
    }

    if (!isValidField(data)) {
        std::cerr << "Ошибка: значение не должно быть пустым.\n";
        return 1;
    }

    item.key = key;
    item.data = data;
    return 0;
}

int loadTableFromFile(const std::string& filename, std::vector<TableItem>& table) {
    int count = 0;
    if (countLinesInFile(filename, count) != 0) {
        return 1;
    }

    if (count < MIN_LEN) {
        std::cerr << "Ошибка: в файле недостаточно строк.\n";
        return 1;
    }

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Ошибка: не удалось открыть файл.\n";
        return 1;
    }

    table.clear();
    table.reserve(count);

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) {
            continue;
        }

        TableItem item;
        if (parseLine(line, item) != 0) {
            std::cerr << "Ошибка: не удалось обработать строку файла.\n";
            table.clear();
            return 1;
        }

        table.push_back(item);
    }

    return 0;
}

int loadTableFromConsole(std::vector<TableItem>& table, int count) {
    if (count < MIN_LEN) {
        std::cerr << "Ошибка: количество элементов слишком мало.\n";
        return 1;
    }

    table.clear();
    table.reserve(count);

    for (int i = 0; i < count; i++) {
        TableItem item;

        std::cout << "Введите ключ " << i + 1 << ": ";
        if (!std::getline(std::cin, item.key)) {
            std::cerr << "Ошибка: не удалось прочитать ключ.\n";
            table.clear();
            return 1;
        }

        if (!isValidField(item.key)) {
            std::cerr << "Ошибка: ключ не должен быть пустым.\n";
            table.clear();
            return 1;
        }

        std::cout << "Введите данные " << i + 1 << ": ";
        if (!std::getline(std::cin, item.data)) {
            std::cerr << "Ошибка: не удалось прочитать данные.\n";
            table.clear();
            return 1;
        }

        if (!isValidField(item.data)) {
            std::cerr << "Ошибка: значение не должно быть пустым.\n";
            table.clear();
            return 1;
        }

        table.push_back(item);
    }

    return 0;
}

void printTable(const std::vector<TableItem>& table) {
    if (table.empty()) {
        std::cout << "Таблица пустая.\n";
        return;
    }

    for (std::size_t i = 0; i < table.size(); i++) {
        std::cout << i + 1 << ". " << table[i].key << ":" << table[i].data << "\n";
    }
}

int findInsertPosition(const std::vector<TableItem>& table, int left, int right, const TableItem& value) {
    while (left <= right) {
        int mid = left + (right - left) / 2;

        if (value.key < table[mid].key) {
            right = mid - 1;
        } else {
            left = mid + 1;
        }
    }

    return left;
}

void binaryInsertionSort(std::vector<TableItem>& table) {
    for (int i = 1; i < static_cast<int>(table.size()); i++) {
        TableItem temp = table[i];
        int position = findInsertPosition(table, 0, i - 1, temp);

        for (int j = i - 1; j >= position; j--) {
            table[j + 1] = table[j];
        }

        table[position] = temp;
    }
}

void reverseTableItems(std::vector<TableItem>& table) {
    std::reverse(table.begin(), table.end());
}

void shuffleTable(std::vector<TableItem>& table) {
    if (table.size() <= 1) {
        return;
    }

    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::shuffle(table.begin(), table.end(), gen);
}

void demonstrateSortCase(const std::string& title, std::vector<TableItem>& table) {
    std::cout << "\n" << title << "\n";
    std::cout << "Исходная таблица:\n";
    printTable(table);

    binaryInsertionSort(table);

    std::cout << "\nПосле сортировки:\n";
    printTable(table);
}

int binarySearchByKey(const std::vector<TableItem>& table, const std::string& key) {
    int left = 0;
    int right = static_cast<int>(table.size()) - 1;

    while (left <= right) {
        int mid = left + (right - left) / 2;

        if (key == table[mid].key) {
            return mid;
        }

        if (key < table[mid].key) {
            right = mid - 1;
        } else {
            left = mid + 1;
        }
    }

    return -1;
}

int main() {
    std::vector<TableItem> table;
    std::vector<TableItem> sorted_case;
    std::vector<TableItem> reverse_case;
    std::vector<TableItem> random_case;
    std::vector<TableItem> search_table;
    int choice;
    std::string search_key;
    const std::string filename = "input.txt";

    std::cout << "1 - Читать таблицу из файла input.txt (формат <ключ>:<значение>)\n";
    std::cout << "2 - Ввести таблицу с консоли\n";
    std::cout << "Ваш выбор: ";

    if (!(std::cin >> choice)) {
        std::cerr << "Ошибка: не удалось прочитать выбор режима ввода.\n";
        return 1;
    }

    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    if (choice == 1) {
        if (loadTableFromFile(filename, table) != 0) {
            std::cerr << "Ошибка: не удалось загрузить таблицу из файла.\n";
            return 1;
        }
    } else if (choice == 2) {
        int count;

        std::cout << "Введите количество элементов (не менее " << MIN_LEN << "): ";
        if (!(std::cin >> count)) {
            std::cerr << "Ошибка: не удалось прочитать количество элементов.\n";
            return 1;
        }

        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (count < MIN_LEN) {
            std::cerr << "Ошибка: количество элементов должно быть не меньше минимального.\n";
            return 1;
        }

        if (loadTableFromConsole(table, count) != 0) {
            std::cerr << "Ошибка: не удалось загрузить таблицу с консоли.\n";
            return 1;
        }
    } else {
        std::cerr << "Ошибка: выбран недопустимый режим.\n";
        return 1;
    }

    sorted_case = table;
    reverse_case = table;
    random_case = table;
    search_table = table;

    binaryInsertionSort(sorted_case);

    binaryInsertionSort(reverse_case);
    reverseTableItems(reverse_case);

    shuffleTable(random_case);

    demonstrateSortCase("1 - Элементы таблицы с самого начала упорядочены", sorted_case);
    demonstrateSortCase("2 - Элементы таблицы расставлены в обратном порядке", reverse_case);
    demonstrateSortCase("3 - Элементы таблицы не упорядочены", random_case);

    binaryInsertionSort(search_table);

    std::cout << "\nПоиск выполняется в отсортированной таблице.\n";
    std::cout << "Введите ключ для поиска: ";

    if (!std::getline(std::cin, search_key)) {
        std::cerr << "Ошибка: не удалось прочитать ключ для поиска.\n";
        return 1;
    }

    int index = binarySearchByKey(search_table, search_key);

    if (index == -1) {
        std::cout << "Ключ не найден.\n";
    } else {
        std::cout << "Найден элемент: " << search_table[index].key << ":" << search_table[index].data << "\n";
    }

    return 0;
}
