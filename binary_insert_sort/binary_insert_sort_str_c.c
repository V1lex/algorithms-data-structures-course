#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MIN_LEN 14
#define MAX_KEY_LEN 256
#define MAX_DATA_LEN 256
#define LINE_BUFFER 1024

typedef struct {
    char key[MAX_KEY_LEN];
    char data[MAX_DATA_LEN];
} TableItem;

void remove_newline(char *str) {
    if (str == NULL) {
        return;
    }

    int len = (int)strlen(str);
    if (len > 0 && str[len - 1] == '\n') {
        str[len - 1] = '\0';
    }
}

int count_lines_in_file(const char *filename, int *count) {
    if (filename == NULL || count == NULL) {
        fprintf(stderr, "Ошибка: передан некорректный аргумент.\n");
        return 1;
    }

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Ошибка: не удалось открыть файл.\n");
        return 1;
    }

    char line[LINE_BUFFER];
    int local_count = 0;
    while (fgets(line, sizeof(line), file) != NULL) {
        if (line[0] != '\n' && line[0] != '\0') {
            local_count++;
        }
    }

    fclose(file);
    *count = local_count;

    return 0;
}

int parse_line(const char *line, TableItem *item) {
    if (line == NULL || item == NULL) {
        fprintf(stderr, "Ошибка: передан некорректный аргумент.\n");
        return 1;
    }

    const char *separator = strchr(line, ':');
    if (separator == NULL) {
        fprintf(stderr, "Ошибка: строка не содержит разделитель ':'.\n");
        return 1;
    }

    int key_len = (int)(separator - line);
    int data_len = (int)strlen(separator + 1);

    if (key_len <= 0 || key_len >= MAX_KEY_LEN) {
        fprintf(stderr, "Ошибка: некорректная длина ключа.\n");
        return 1;
    }

    if (data_len <= 0 || data_len >= MAX_DATA_LEN) {
        fprintf(stderr, "Ошибка: некорректная длина значения.\n");
        return 1;
    }

    strncpy(item->key, line, key_len);
    item->key[key_len] = '\0';

    strcpy(item->data, separator + 1);
    remove_newline(item->data);

    return 0;
}

int load_table_from_file(const char *filename, TableItem **table, int *size) {
    if (filename == NULL || table == NULL || size == NULL) {
        fprintf(stderr, "Ошибка: передан некорректный аргумент.\n");
        return 1;
    }

    int count;

    if (count_lines_in_file(filename, &count) != 0) {
        return 1;
    }

    if (count < MIN_LEN) {
        fprintf(stderr, "Ошибка: в файле недостаточно строк.\n");
        return 1;
    }

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Ошибка: не удалось открыть файл.\n");
        return 1;
    }

    *table = (TableItem *)malloc(count * sizeof(TableItem));
    if (*table == NULL) {
        fclose(file);
        fprintf(stderr, "Ошибка: не удалось выделить память.\n");
        return 1;
    }

    char line[LINE_BUFFER];
    int index = 0;
    while (fgets(line, sizeof(line), file) != NULL) {
        if (line[0] == '\n' || line[0] == '\0') {
            continue;
        }

        if (parse_line(line, &((*table)[index])) != 0) {
            free(*table);
            *table = NULL;
            fclose(file);
            fprintf(stderr, "Ошибка: не удалось обработать строку файла.\n");
            return 1;
        }

        index++;
    }

    fclose(file);
    *size = index;

    return 0;
}

int load_table_from_console(TableItem **table, int *size, int count) {
    if (table == NULL || size == NULL) {
        fprintf(stderr, "Ошибка: передан некорректный аргумент.\n");
        return 1;
    }

    if (count < MIN_LEN) {
        fprintf(stderr, "Ошибка: количество элементов слишком мало.\n");
        return 1;
    }

    *table = (TableItem *)malloc(count * sizeof(TableItem));
    if (*table == NULL) {
        fprintf(stderr, "Ошибка: не удалось выделить память.\n");
        return 1;
    }

    for (int i = 0; i < count; i++) {
        printf("Введите ключ %d: ", i + 1);
        if (fgets((*table)[i].key, MAX_KEY_LEN, stdin) == NULL) {
            free(*table);
            *table = NULL;
            fprintf(stderr, "Ошибка: не удалось прочитать ключ.\n");
            return 1;
        }
        remove_newline((*table)[i].key);

        printf("Введите данные %d: ", i + 1);
        if (fgets((*table)[i].data, MAX_DATA_LEN, stdin) == NULL) {
            free(*table);
            *table = NULL;
            fprintf(stderr, "Ошибка: не удалось прочитать данные.\n");
            return 1;
        }
        remove_newline((*table)[i].data);
    }

    *size = count;
    return 0;
}

void print_table(const TableItem *table, int size) {
    if (table == NULL || size <= 0) {
        printf("Таблица пустая.\n");
        return;
    }

    for (int i = 0; i < size; i++) {
        printf("%d. %s:%s\n", i + 1, table[i].key, table[i].data);
    }
}

TableItem *copy_table(const TableItem *table, int size) {
    if (table == NULL || size <= 0) {
        return NULL;
    }

    TableItem *copy = (TableItem *)malloc(size * sizeof(TableItem));
    if (copy == NULL) {
        fprintf(stderr, "Ошибка: не удалось выделить память.\n");
        return NULL;
    }

    for (int i = 0; i < size; i++) {
        copy[i] = table[i];
    }

    return copy;
}

int find_insert_position(const TableItem *table, int left, int right, const TableItem *value) {
    while (left <= right) {
        int mid = left + (right - left) / 2;

        if (strcmp(value->key, table[mid].key) < 0) {
            right = mid - 1;
        } else {
            left = mid + 1;
        }
    }

    return left;
}

void binary_insertion_sort(TableItem *table, int size) {
    for (int i = 1; i < size; i++) {
        TableItem temp = table[i];
        int position = find_insert_position(table, 0, i - 1, &temp);

        for (int j = i - 1; j >= position; j--) {
            table[j + 1] = table[j];
        }

        table[position] = temp;
    }
}

void reverse_table_items(TableItem *table, int size) {
    int left = 0;
    int right = size - 1;

    while (left < right) {
        TableItem temp = table[left];
        table[left] = table[right];
        table[right] = temp;
        left++;
        right--;
    }
}

void shuffle_table(TableItem *table, int size) {
    if (table == NULL || size <= 1) {
        return;
    }

    for (int i = size - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        TableItem temp = table[i];
        table[i] = table[j];
        table[j] = temp;
    }
}

void demonstrate_sort_case(const char *title, TableItem *table, int size) {
    printf("\n%s\n", title);
    printf("Исходная таблица:\n");
    print_table(table, size);

    binary_insertion_sort(table, size);

    printf("\nПосле сортировки:\n");
    print_table(table, size);
}

int binary_search_by_key(const TableItem *table, int size, const char *key) {
    int left = 0;
    int right = size - 1;

    if (table == NULL || key == NULL) {
        return -1;
    }

    while (left <= right) {
        int mid = left + (right - left) / 2;
        int cmp = strcmp(key, table[mid].key);

        if (cmp == 0) {
            return mid;
        }

        if (cmp < 0) {
            right = mid - 1;
        } else {
            left = mid + 1;
        }
    }

    return -1;
}

void free_table(TableItem *table) {
    if (table != NULL) {
        free(table);
    }
}

int main(void) {
    TableItem *table = NULL;
    TableItem *sorted_case = NULL;
    TableItem *reverse_case = NULL;
    TableItem *random_case = NULL;
    TableItem *search_table = NULL;
    int size = 0;
    int choice;
    char search_key[MAX_KEY_LEN];
    const char *filename = "input.txt";

    srand((unsigned int)time(NULL));

    printf("1 - Читать таблицу из файла input.txt (формат <ключ>:<значение>)\n");
    printf("2 - Ввести таблицу с консоли\n");
    printf("Ваш выбор: ");

    if (scanf("%d", &choice) != 1) {
        fprintf(stderr, "Ошибка: не удалось прочитать выбор режима ввода.\n");
        return 1;
    }

    while (getchar() != '\n');

    if (choice == 1) {
        if (load_table_from_file(filename, &table, &size) != 0) {
            fprintf(stderr, "Ошибка: не удалось загрузить таблицу из файла.\n");
            return 1;
        }
    } else if (choice == 2) {
        int count;

        printf("Введите количество элементов (не менее %d): ", MIN_LEN);
        if (scanf("%d", &count) != 1) {
            fprintf(stderr, "Ошибка: не удалось прочитать количество элементов.\n");
            return 1;
        }

        while (getchar() != '\n');

        if (count < MIN_LEN) {
            fprintf(stderr, "Ошибка: количество элементов должно быть не меньше минимального.\n");
            return 1;
        }

        if (load_table_from_console(&table, &size, count) != 0) {
            fprintf(stderr, "Ошибка: не удалось загрузить таблицу с консоли.\n");
            return 1;
        }
    } else {
        fprintf(stderr, "Ошибка: выбран недопустимый режим.\n");
        return 1;
    }

    sorted_case = copy_table(table, size);
    reverse_case = copy_table(table, size);
    random_case = copy_table(table, size);
    search_table = copy_table(table, size);

    if (sorted_case == NULL || reverse_case == NULL || random_case == NULL || search_table == NULL) {
        free_table(table);
        free_table(sorted_case);
        free_table(reverse_case);
        free_table(random_case);
        free_table(search_table);
        fprintf(stderr, "Ошибка: не удалось подготовить копии таблицы.\n");
        return 1;
    }

    binary_insertion_sort(sorted_case, size);

    binary_insertion_sort(reverse_case, size);
    reverse_table_items(reverse_case, size);

    shuffle_table(random_case, size);

    demonstrate_sort_case("1 - Элементы таблицы с самого начала упорядочены", sorted_case, size);
    demonstrate_sort_case("2 - Элементы таблицы расставлены в обратном порядке", reverse_case, size);
    demonstrate_sort_case("3 - Элементы таблицы не упорядочены", random_case, size);

    binary_insertion_sort(search_table, size);

    printf("\nПоиск выполняется в отсортированной таблице.\n");
    printf("Введите ключ для поиска: ");

    if (fgets(search_key, sizeof(search_key), stdin) == NULL) {
        free_table(table);
        free_table(sorted_case);
        free_table(reverse_case);
        free_table(random_case);
        free_table(search_table);
        fprintf(stderr, "Ошибка: не удалось прочитать ключ для поиска.\n");
        return 1;
    }

    remove_newline(search_key);

    int index = binary_search_by_key(search_table, size, search_key);

    if (index == -1) {
        printf("Ключ не найден.\n");
    } else {
        printf("Найден элемент: %s:%s\n", search_table[index].key, search_table[index].data);
    }

    free_table(table);
    free_table(sorted_case);
    free_table(reverse_case);
    free_table(random_case);
    free_table(search_table);

    return 0;
}
