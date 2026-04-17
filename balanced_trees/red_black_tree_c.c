#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LEN 256
#define MAX_KEY_LEN 6
#define DEFAULT_INPUT_FILE "input.txt"
#define DEFAULT_OUTPUT_FILE "output.txt"
#define COLOR_RED "\x1b[31m"
#define COLOR_BLACK "\x1b[90m"
#define COLOR_RESET "\x1b[0m"

typedef enum {
    RED,
    BLACK
} Color;

typedef struct Node {
    char key[MAX_KEY_LEN + 1];
    double value;
    Color color;
    struct Node *left;
    struct Node *right;
    struct Node *parent;
} Node;

typedef struct {
    Node *root;
    Node *nil;
    int coloredOutput;  // цветной вывод для консоли, отключается при работе с файлами
} RBTree;

void remove_newline(char *line) {
    if (line == NULL) {
        return;
    }

    int len = (int)strlen(line);
    while (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r')) {
        line[--len] = '\0';
    }
}

int isValidKey(const char *key) {
    if (key == NULL || key[0] == '\0') {
        return 0;
    }

    int len = (int)strlen(key);
    if (len == 0 || len > MAX_KEY_LEN) {
        return 0;
    }

    for (int i = 0; i < len; i++) {
        if (!((key[i] >= 'a' && key[i] <= 'z') || (key[i] >= 'A' && key[i] <= 'Z'))) {
            return 0;
        }
    }

    return 1;
}

const char *colorEscape(Color color) {
    return color == RED ? COLOR_RED : COLOR_BLACK;
}

const char *colorText(Color color) {
    return color == RED ? "R" : "B";
}

int initTree(RBTree *tree) {
    tree->nil = (Node *)malloc(sizeof(Node));
    if (tree->nil == NULL) {
        fprintf(stderr, "Ошибка: не удалось выделить память для NIL.\n");
        return 1;
    }

    strcpy(tree->nil->key, "NIL");
    tree->nil->value = 0.0;
    tree->nil->color = BLACK;
    tree->nil->left = tree->nil;
    tree->nil->right = tree->nil;
    tree->nil->parent = tree->nil;
    tree->root = tree->nil;
    tree->coloredOutput = 1;
    return 0;
}

Node *createNode(RBTree *tree, const char *key, double value) {
    Node *new_node = (Node *)malloc(sizeof(Node));
    if (new_node == NULL) {
        fprintf(stderr, "Ошибка: не удалось выделить память для узла.\n");
        return NULL;
    }

    strcpy(new_node->key, key);
    new_node->value = value;
    new_node->color = RED;
    new_node->left = tree->nil;
    new_node->right = tree->nil;
    new_node->parent = tree->nil;
    return new_node;
}

void leftRotate(RBTree *tree, Node *x) {
    Node *y = x->right;

    x->right = y->left;
    if (y->left != tree->nil) {
        y->left->parent = x;
    }

    y->parent = x->parent;
    if (x->parent == tree->nil) {
        tree->root = y;
    } else if (x == x->parent->left) {
        x->parent->left = y;
    } else {
        x->parent->right = y;
    }

    y->left = x;
    x->parent = y;
}

void rightRotate(RBTree *tree, Node *y) {
    Node *x = y->left;

    y->left = x->right;
    if (x->right != tree->nil) {
        x->right->parent = y;
    }

    x->parent = y->parent;
    if (y->parent == tree->nil) {
        tree->root = x;
    } else if (y == y->parent->left) {
        y->parent->left = x;
    } else {
        y->parent->right = x;
    }

    x->right = y;
    y->parent = x;
}

Node *findNode(const RBTree *tree, const char *key) {
    Node *current = tree->root;

    while (current != tree->nil) {
        int cmp = strcmp(key, current->key);
        if (cmp == 0) {
            return current;
        }
        current = (cmp < 0) ? current->left : current->right;
    }

    return tree->nil;
}

void insertFixup(RBTree *tree, Node *z) {
    while (z->parent->color == RED) {
        if (z->parent == z->parent->parent->left) {
            Node *y = z->parent->parent->right;
            if (y->color == RED) {
                z->parent->color = BLACK;
                y->color = BLACK;
                z->parent->parent->color = RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->right) {
                    z = z->parent;
                    leftRotate(tree, z);
                }
                z->parent->color = BLACK;
                z->parent->parent->color = RED;
                rightRotate(tree, z->parent->parent);
            }
        } else {
            Node *y = z->parent->parent->left;
            if (y->color == RED) {
                z->parent->color = BLACK;
                y->color = BLACK;
                z->parent->parent->color = RED;
                z = z->parent->parent;
            } else {
                if (z == z->parent->left) {
                    z = z->parent;
                    rightRotate(tree, z);
                }
                z->parent->color = BLACK;
                z->parent->parent->color = RED;
                leftRotate(tree, z->parent->parent);
            }
        }
    }

    tree->root->color = BLACK;
}

int insertNode(RBTree *tree, const char *key, double value) {
    Node *parent = tree->nil;
    Node *current = tree->root;
    Node *node;

    while (current != tree->nil) {
        int cmp = strcmp(key, current->key);
        parent = current;
        if (cmp == 0) {
            return 0;
        }
        current = (cmp < 0) ? current->left : current->right;
    }

    node = createNode(tree, key, value);
    if (node == NULL) {
        return 0;
    }
    node->parent = parent;

    if (parent == tree->nil) {
        tree->root = node;
    } else if (strcmp(node->key, parent->key) < 0) {
        parent->left = node;
    } else {
        parent->right = node;
    }

    insertFixup(tree, node);
    return 1;
}

Node *treeMinimum(const RBTree *tree, Node *node) {
    while (node->left != tree->nil) {
        node = node->left;
    }
    return node;
}

void transplant(RBTree *tree, Node *u, Node *v) {
    if (u->parent == tree->nil) {
        tree->root = v;
    } else if (u == u->parent->left) {
        u->parent->left = v;
    } else {
        u->parent->right = v;
    }
    v->parent = u->parent;
}

void deleteFixup(RBTree *tree, Node *x) {
    while (x != tree->root && x->color == BLACK) {
        if (x == x->parent->left) {
            Node *w = x->parent->right;
            if (w->color == RED) {
                w->color = BLACK;
                x->parent->color = RED;
                leftRotate(tree, x->parent);
                w = x->parent->right;
            }
            if (w->left->color == BLACK && w->right->color == BLACK) {
                w->color = RED;
                x = x->parent;
            } else {
                if (w->right->color == BLACK) {
                    w->left->color = BLACK;
                    w->color = RED;
                    rightRotate(tree, w);
                    w = x->parent->right;
                }
                w->color = x->parent->color;
                x->parent->color = BLACK;
                w->right->color = BLACK;
                leftRotate(tree, x->parent);
                x = tree->root;
            }
        } else {
            Node *w = x->parent->left;
            if (w->color == RED) {
                w->color = BLACK;
                x->parent->color = RED;
                rightRotate(tree, x->parent);
                w = x->parent->left;
            }
            if (w->right->color == BLACK && w->left->color == BLACK) {
                w->color = RED;
                x = x->parent;
            } else {
                if (w->left->color == BLACK) {
                    w->right->color = BLACK;
                    w->color = RED;
                    leftRotate(tree, w);
                    w = x->parent->left;
                }
                w->color = x->parent->color;
                x->parent->color = BLACK;
                w->left->color = BLACK;
                rightRotate(tree, x->parent);
                x = tree->root;
            }
        }
    }

    x->color = BLACK;
}

int deleteNode(RBTree *tree, const char *key) {
    Node *z = findNode(tree, key);
    Node *y;
    Node *x;
    Color original_color = z->color;

    if (z == tree->nil) {
        return 0;
    }

    y = z;

    if (z->left == tree->nil) {
        x = z->right;
        transplant(tree, z, z->right);
    } else if (z->right == tree->nil) {
        x = z->left;
        transplant(tree, z, z->left);
    } else {
        y = treeMinimum(tree, z->right);
        original_color = y->color;
        x = y->right;
        if (y->parent == z) {
            x->parent = y;
        } else {
            transplant(tree, y, y->right);
            y->right = z->right;
            y->right->parent = y;
        }
        transplant(tree, z, y);
        y->left = z->left;
        y->left->parent = y;
        y->color = z->color;
    }

    free(z);

    if (original_color == BLACK) {
        deleteFixup(tree, x);
    }

    return 1;
}

void printTreeRecursive(const RBTree *tree, const Node *node, const char *branch, int depth, FILE *out) {
    for (int i = 0; i < depth; i++) {
        fprintf(out, "    ");
    }

    if (node == tree->nil) {
        if (tree->coloredOutput) {
            fprintf(out, "%s: %sNIL%s\n", branch, COLOR_BLACK, COLOR_RESET);
        } else {
            fprintf(out, "%s: NIL (%s)\n", branch, colorText(BLACK));
        }
        return;
    }

    if (tree->coloredOutput) {
        fprintf(out, "%s: %s%s%s\n", branch, colorEscape(node->color), node->key, COLOR_RESET);
    } else {
        fprintf(out, "%s: %s (%s)\n", branch, node->key, colorText(node->color));
    }

    printTreeRecursive(tree, node->left, "L", depth + 1, out);
    printTreeRecursive(tree, node->right, "R", depth + 1, out);
}

void printTree(const RBTree *tree, FILE *out) {
    fprintf(out, "Дерево:\n");

    if (tree->root == tree->nil) {
        if (tree->coloredOutput) {
            fprintf(out, "ROOT: %sNIL%s\n", COLOR_BLACK, COLOR_RESET);
        } else {
            fprintf(out, "ROOT: NIL (%s)\n", colorText(BLACK));
        }
        return;
    }

    printTreeRecursive(tree, tree->root, "ROOT", 0, out);
}

void freeSubtree(RBTree *tree, Node *node) {
    if (node == tree->nil) {
        return;
    }
    freeSubtree(tree, node->left);
    freeSubtree(tree, node->right);
    free(node);
}

void destroyTree(RBTree *tree) {
    freeSubtree(tree, tree->root);
    free(tree->nil);
}

void printInvalidCommand(FILE *out) {
    fprintf(out, "Ошибка: некорректная команда.\n");
}

int processCommand(RBTree *tree, const char *line, FILE *out) {
    int operation = 0;
    char key[64];
    double value;
    int count;

    key[0] = '\0';
    count = sscanf(line, "%d %63s %lf", &operation, key, &value);
    if (count < 1) {
        printInvalidCommand(out);
        fprintf(out, "\n");
        return 1;
    }

    if (operation == 0) {
        fprintf(out, "Завершение программы.\n\n");
        return 0;
    } else if (operation == 1) {
        if (count != 3) {
            printInvalidCommand(out);
        } else if (!isValidKey(key)) {
            fprintf(out, "Ошибка: ключ должен содержать только латинские буквы и иметь длину от 1 до 6.\n");
        } else if (!insertNode(tree, key, value)) {
            fprintf(out, "Ошибка: ключ %s уже существует.\n", key);
        } else {
            fprintf(out, "Узел %s добавлен.\n", key);
        }
    } else if (operation == 2) {
        if (count != 2) {
            printInvalidCommand(out);
        } else if (!isValidKey(key)) {
            fprintf(out, "Ошибка: ключ должен содержать только латинские буквы и иметь длину от 1 до 6.\n");
        } else if (!deleteNode(tree, key)) {
            fprintf(out, "Ошибка: ключ %s не найден.\n", key);
        } else {
            fprintf(out, "Узел %s удален.\n", key);
        }
    } else if (operation == 3) {
        if (count != 1) {
            printInvalidCommand(out);
        } else {
            printTree(tree, out);
        }
    } else if (operation == 4) {
        Node *node;
        if (count != 2) {
            printInvalidCommand(out);
        } else if (!isValidKey(key)) {
            fprintf(out, "Ошибка: ключ должен содержать только латинские буквы и иметь длину от 1 до 6.\n");
        } else {
            node = findNode(tree, key);
            if (node == tree->nil) {
                fprintf(out, "Не найден.\n");
            } else {
                fprintf(out, "%.10g\n", node->value);
            }
        }
    } else {
        printInvalidCommand(out);
    }

    fprintf(out, "\n");
    return 1;
}

void printMenu(void) {
    printf("Поддерживаемые команды:\n");
    printf("0                - завершить программу\n");
    printf("1 <key> <value>  - добавить узел\n");
    printf("2 <key>          - удалить узел по ключу\n");
    printf("3                - напечатать дерево\n");
    printf("4 <key>          - найти значение по ключу\n");
    printf("Ограничения на ключ: только латинские буквы, длина от 1 до 6 символов.\n\n");
    printf("Выберите режим работы:\n");
    printf("1 - ввод команд из консоли\n");
    printf("2 - чтение команд из input.txt и запись результата в output.txt\n");
    printf("Ваш выбор: ");
}

FILE *openInputFile(const char **input_path, const char **output_path) {
    FILE *in = fopen(DEFAULT_INPUT_FILE, "r");
    if (in != NULL) {
        *input_path = DEFAULT_INPUT_FILE;
        *output_path = DEFAULT_OUTPUT_FILE;
        return in;
    }
    return NULL;
}

int main(void) {
    RBTree tree;
    FILE *in = stdin;
    FILE *out = stdout;
    char line[MAX_LINE_LEN];
    int mode = 0;
    const char *input_path = NULL;
    const char *output_path = NULL;

    if (initTree(&tree) != 0) {
        return 1;
    }
    printMenu();

    if (scanf("%d", &mode) != 1) {
        fprintf(stderr, "Ошибка: режим должен быть числом 1 или 2.\n");
        destroyTree(&tree);
        return 1;
    }
    getchar();

    if (mode == 2) {
        tree.coloredOutput = 0;
        in = openInputFile(&input_path, &output_path);
        if (in == NULL) {
            fprintf(stderr, "Ошибка: не удалось открыть input.txt для чтения.\n");
            destroyTree(&tree);
            return 1;
        }

        out = fopen(output_path, "w");
        if (out == NULL) {
            fprintf(stderr, "Ошибка: не удалось открыть output.txt для записи.\n");
            fclose(in);
            destroyTree(&tree);
            return 1;
        }
    } else if (mode != 1) {
        fprintf(stderr, "Ошибка: нужно выбрать 1 или 2.\n");
        destroyTree(&tree);
        return 1;
    } else {
        tree.coloredOutput = 1;
    }

    while (1) {
        if (in == stdin) {
            printf("Введите команду: ");
            fflush(stdout);
        }

        if (fgets(line, sizeof(line), in) == NULL) {
            break;
        }

        remove_newline(line);
        if (line[0] == '\0') {
            continue;
        }
        if (!processCommand(&tree, line, out)) {
            break;
        }
    }

    if (in != stdin) {
        fclose(in);
    }
    if (out != stdout) {
        fclose(out);
        printf("Результат записан в %s\n", output_path);
    }

    destroyTree(&tree);
    return 0;
}
