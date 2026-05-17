#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>

const int MAX_KEY_LEN = 6;
const std::string DEFAULT_INPUT_FILE = "input.txt";
const std::string DEFAULT_OUTPUT_FILE = "output.txt";
const char* COLOR_RED = "\x1b[31m";
const char* COLOR_BLACK = "\x1b[90m";
const char* COLOR_RESET = "\x1b[0m";

enum Color {
    RED,
    BLACK
};

struct Node {
    std::string key;
    double value;
    Color color;
    Node* left;
    Node* right;
    Node* parent;
};

struct RBTree {
    Node* root;
    Node* nil;
    bool coloredOutput;
};

bool isValidKey(const std::string& key) {
    if (key.empty() || static_cast<int>(key.size()) > MAX_KEY_LEN) {
        return false;
    }

    for (char ch : key) {
        if (!((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'))) {
            return false;
        }
    }

    return true;
}

const char* colorEscape(Color color) {
    return color == RED ? COLOR_RED : COLOR_BLACK;
}

const char* colorText(Color color) {
    return color == RED ? "R" : "B";
}

int initTree(RBTree& tree) {
    tree.nil = new Node{"NIL", 0.0, BLACK, nullptr, nullptr, nullptr};
    tree.nil->left = tree.nil;
    tree.nil->right = tree.nil;
    tree.nil->parent = tree.nil;
    tree.root = tree.nil;
    tree.coloredOutput = true;
    return 0;
}

Node* createNode(RBTree& tree, const std::string& key, double value) {
    return new Node{key, value, RED, tree.nil, tree.nil, tree.nil};
}

void leftRotate(RBTree& tree, Node* x) {
    Node* y = x->right;

    x->right = y->left;
    if (y->left != tree.nil) {
        y->left->parent = x;
    }

    y->parent = x->parent;
    if (x->parent == tree.nil) {
        tree.root = y;
    } else if (x == x->parent->left) {
        x->parent->left = y;
    } else {
        x->parent->right = y;
    }

    y->left = x;
    x->parent = y;
}

void rightRotate(RBTree& tree, Node* y) {
    Node* x = y->left;

    y->left = x->right;
    if (x->right != tree.nil) {
        x->right->parent = y;
    }

    x->parent = y->parent;
    if (y->parent == tree.nil) {
        tree.root = x;
    } else if (y == y->parent->left) {
        y->parent->left = x;
    } else {
        y->parent->right = x;
    }

    x->right = y;
    y->parent = x;
}

Node* findNode(const RBTree& tree, const std::string& key) {
    Node* current = tree.root;

    while (current != tree.nil) {
        if (key == current->key) {
            return current;
        }
        current = (key < current->key) ? current->left : current->right;
    }

    return tree.nil;
}

void insertFixup(RBTree& tree, Node* z) {
    while (z->parent->color == RED) {
        if (z->parent == z->parent->parent->left) {
            Node* y = z->parent->parent->right;
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
            Node* y = z->parent->parent->left;
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

    tree.root->color = BLACK;
}

bool insertNode(RBTree& tree, const std::string& key, double value) {
    Node* parent = tree.nil;
    Node* current = tree.root;

    while (current != tree.nil) {
        parent = current;
        if (key == current->key) {
            return false;
        }
        current = (key < current->key) ? current->left : current->right;
    }

    Node* node = createNode(tree, key, value);
    node->parent = parent;

    if (parent == tree.nil) {
        tree.root = node;
    } else if (node->key < parent->key) {
        parent->left = node;
    } else {
        parent->right = node;
    }

    insertFixup(tree, node);
    return true;
}

Node* treeMinimum(const RBTree& tree, Node* node) {
    while (node->left != tree.nil) {
        node = node->left;
    }
    return node;
}

void transplant(RBTree& tree, Node* u, Node* v) {
    if (u->parent == tree.nil) {
        tree.root = v;
    } else if (u == u->parent->left) {
        u->parent->left = v;
    } else {
        u->parent->right = v;
    }
    v->parent = u->parent;
}

void deleteFixup(RBTree& tree, Node* x) {
    while (x != tree.root && x->color == BLACK) {
        if (x == x->parent->left) {
            Node* w = x->parent->right;
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
                x = tree.root;
            }
        } else {
            Node* w = x->parent->left;
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
                x = tree.root;
            }
        }
    }

    x->color = BLACK;
}

bool deleteNode(RBTree& tree, const std::string& key) {
    Node* z = findNode(tree, key);
    if (z == tree.nil) {
        return false;
    }

    Node* y = z;
    Node* x;
    Color original_color = y->color;

    if (z->left == tree.nil) {
        x = z->right;
        transplant(tree, z, z->right);
    } else if (z->right == tree.nil) {
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

    delete z;

    if (original_color == BLACK) {
        deleteFixup(tree, x);
    }

    return true;
}

void printTreeRecursive(const RBTree& tree, const Node* node, const std::string& branch, int depth, std::ostream& out) {
    for (int i = 0; i < depth; i++) {
        out << "    ";
    }

    if (node == tree.nil) {
        if (tree.coloredOutput) {
            out << branch << ": " << COLOR_BLACK << "NIL" << COLOR_RESET << "\n";
        } else {
            out << branch << ": NIL (" << colorText(BLACK) << ")\n";
        }
        return;
    }

    if (tree.coloredOutput) {
        out << branch << ": " << colorEscape(node->color) << node->key << COLOR_RESET << "\n";
    } else {
        out << branch << ": " << node->key << " (" << colorText(node->color) << ")\n";
    }

    printTreeRecursive(tree, node->left, "L", depth + 1, out);
    printTreeRecursive(tree, node->right, "R", depth + 1, out);
}

void printTree(const RBTree& tree, std::ostream& out) {
    out << "Дерево:\n";

    if (tree.root == tree.nil) {
        if (tree.coloredOutput) {
            out << "ROOT: " << COLOR_BLACK << "NIL" << COLOR_RESET << "\n";
        } else {
            out << "ROOT: NIL (" << colorText(BLACK) << ")\n";
        }
        return;
    }

    printTreeRecursive(tree, tree.root, "ROOT", 0, out);
}

void freeSubtree(RBTree& tree, Node* node) {
    if (node == tree.nil) {
        return;
    }

    freeSubtree(tree, node->left);
    freeSubtree(tree, node->right);
    delete node;
}

void destroyTree(RBTree& tree) {
    freeSubtree(tree, tree.root);
    delete tree.nil;
    tree.root = nullptr;
    tree.nil = nullptr;
}

void printInvalidCommand(std::ostream& out) {
    out << "Ошибка: некорректная команда.\n";
}

int processCommand(RBTree& tree, const std::string& line, std::ostream& out) {
    std::istringstream input(line);
    int operation;
    std::string key;
    double value;

    if (!(input >> operation)) {
        printInvalidCommand(out);
        out << "\n";
        return 1;
    }

    if (operation == 0) {
        out << "Завершение программы.\n\n";
        return 0;
    } else if (operation == 1) {
        if (!(input >> key >> value)) {
            printInvalidCommand(out);
        } else if (!isValidKey(key)) {
            out << "Ошибка: ключ должен содержать только латинские буквы и иметь длину от 1 до 6.\n";
        } else if (!insertNode(tree, key, value)) {
            out << "Ошибка: ключ " << key << " уже существует.\n";
        } else {
            out << "Узел " << key << " добавлен.\n";
        }
    } else if (operation == 2) {
        if (!(input >> key)) {
            printInvalidCommand(out);
        } else if (!isValidKey(key)) {
            out << "Ошибка: ключ должен содержать только латинские буквы и иметь длину от 1 до 6.\n";
        } else if (!deleteNode(tree, key)) {
            out << "Ошибка: ключ " << key << " не найден.\n";
        } else {
            out << "Узел " << key << " удален.\n";
        }
    } else if (operation == 3) {
        printTree(tree, out);
    } else if (operation == 4) {
        if (!(input >> key)) {
            printInvalidCommand(out);
        } else if (!isValidKey(key)) {
            out << "Ошибка: ключ должен содержать только латинские буквы и иметь длину от 1 до 6.\n";
        } else {
            Node* node = findNode(tree, key);
            if (node == tree.nil) {
                out << "Не найден.\n";
            } else {
                out << std::setprecision(10) << node->value << "\n";
            }
        }
    } else {
        printInvalidCommand(out);
    }

    out << "\n";
    return 1;
}

void printMenu() {
    std::cout << "Поддерживаемые команды:\n";
    std::cout << "0                - завершить программу\n";
    std::cout << "1 <key> <value>  - добавить узел\n";
    std::cout << "2 <key>          - удалить узел по ключу\n";
    std::cout << "3                - напечатать дерево\n";
    std::cout << "4 <key>          - найти значение по ключу\n";
    std::cout << "Ограничения на ключ: только латинские буквы, длина от 1 до 6 символов.\n\n";
    std::cout << "Выберите режим работы:\n";
    std::cout << "1 - ввод команд из консоли\n";
    std::cout << "2 - чтение команд из input.txt и запись результата в output.txt\n";
    std::cout << "Ваш выбор: ";
}

int main() {
    RBTree tree;
    int mode = 0;

    if (initTree(tree) != 0) {
        return 1;
    }

    printMenu();

    if (!(std::cin >> mode)) {
        std::cerr << "Ошибка: режим должен быть числом 1 или 2.\n";
        destroyTree(tree);
        return 1;
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    if (mode == 2) {
        tree.coloredOutput = false;

        std::ifstream in(DEFAULT_INPUT_FILE);
        if (!in.is_open()) {
            std::cerr << "Ошибка: не удалось открыть input.txt для чтения.\n";
            destroyTree(tree);
            return 1;
        }

        std::ofstream out(DEFAULT_OUTPUT_FILE);
        if (!out.is_open()) {
            std::cerr << "Ошибка: не удалось открыть output.txt для записи.\n";
            destroyTree(tree);
            return 1;
        }

        std::string line;
        while (std::getline(in, line)) {
            if (line.empty()) {
                continue;
            }
            if (!processCommand(tree, line, out)) {
                break;
            }
        }

        std::cout << "Результат записан в " << DEFAULT_OUTPUT_FILE << "\n";
    } else if (mode == 1) {
        tree.coloredOutput = true;

        std::string line;
        while (true) {
            std::cout << "Введите команду: ";
            if (!std::getline(std::cin, line)) {
                break;
            }
            if (line.empty()) {
                continue;
            }
            if (!processCommand(tree, line, std::cout)) {
                break;
            }
        }
    } else {
        std::cerr << "Ошибка: нужно выбрать 1 или 2.\n";
        destroyTree(tree);
        return 1;
    }

    destroyTree(tree);
    return 0;
}
