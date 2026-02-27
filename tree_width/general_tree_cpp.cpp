#include <iostream>
#include <queue>

struct Node {
    int key;
    Node* parent;
    Node* firstChild;
    Node* nextSibling;
};

Node* createNode(int key) {
    Node* new_node = new Node;
    new_node->key = key;
    new_node->parent = nullptr;
    new_node->firstChild = nullptr;
    new_node->nextSibling = nullptr;
    return new_node;
}

Node* insertChild(Node* parent, int key) {
    if (parent == nullptr) return nullptr;

    Node* new_node = createNode(key);
    new_node->parent = parent;

    if (parent->firstChild == nullptr) {
        parent->firstChild = new_node;
    } else {
        Node* sibling = parent->firstChild;
        while (sibling->nextSibling != nullptr) {
            sibling = sibling->nextSibling;
        }
        sibling->nextSibling = new_node;
    }
    return new_node;
}

Node* findNode(Node* root, int key) {
    if (root == nullptr) return nullptr;
    if (root->key == key) return root;

    Node* child = root->firstChild;
    while (child != nullptr) {
        Node* found = findNode(child, key);
        if (found != nullptr) return found;
        child = child->nextSibling;
    }
    return nullptr;
}

int createRoot(Node*& root, int key) {
    if (root != nullptr) {
        std::cerr << "Корень уже существует\n";
        return 1;
    }
    root = createNode(key);
    return 0;
}

int addByParentKey(Node* root, int parent_key, int key) {
    if (root == nullptr) {
        std::cerr << "Дерево пустое. Сначала создайте корень\n";
        return 1;
    }

    Node* parent_node = findNode(root, parent_key);
    if (parent_node == nullptr) {
        std::cerr << "Родительский узел с ключом " << parent_key << " не найден\n";
        return 1;
    }

    if (findNode(root, key) != nullptr) {
        std::cerr << "Узел с ключом " << key << " уже существует\n";
        return 1;
    }

    insertChild(parent_node, key);
    return 0;
}

void printTree(Node* node, int level) {
    if (node == nullptr) return;

    for (int i = 0; i < level; ++i) {
        std::cout << "  ";
    }
    std::cout << node->key << "\n";

    printTree(node->firstChild, level + 1);
    printTree(node->nextSibling, level);
}

void freeTree(Node* node) {
    if (node == nullptr) return;

    Node* child = node->firstChild;
    while (child != nullptr) {
        Node* next = child->nextSibling;
        freeTree(child);
        child = next;
    }
    delete node;
}

int detachFromParent(Node* node) {
    if (node == nullptr || node->parent == nullptr) return 1;

    Node* parent = node->parent;

    if (parent->firstChild == node) {
        parent->firstChild = node->nextSibling;
    } else {
        Node* prev = parent->firstChild;
        while (prev != nullptr && prev->nextSibling != node) {
            prev = prev->nextSibling;
        }
        if (prev == nullptr) return 1;
        prev->nextSibling = node->nextSibling;
    }

    node->parent = nullptr;
    node->nextSibling = nullptr;
    return 0;
}

int deleteByKey(Node*& root, int key) {
    if (root == nullptr) {
        std::cerr << "Дерево пустое\n";
        return 1;
    }

    Node* node_to_delete = findNode(root, key);
    if (node_to_delete == nullptr) {
        std::cerr << "Узел с ключом " << key << " не найден\n";
        return 1;
    }

    if (node_to_delete == root) {
        freeTree(root);
        root = nullptr;
        return 0;
    }

    if (detachFromParent(node_to_delete) != 0) {
        std::cerr << "Ошибка при отсоединении узла от родителя\n";
        return 1;
    }

    freeTree(node_to_delete);
    return 0;
}

int treeWidth(Node* root) {
    if (root == nullptr) return 0;

    std::queue<Node*> q;
    q.push(root);

    std::size_t max_width = 0;

    while (!q.empty()) {
        std::size_t level_size = q.size();
        if (level_size > max_width) max_width = level_size;

        for (std::size_t i = 0; i < level_size; ++i) {
            Node* v = q.front();
            q.pop();

            for (Node* c = v->firstChild; c != nullptr; c = c->nextSibling) {
                q.push(c);
            }
        }
    }

    return static_cast<int>(max_width);
}

int main() {
    std::cout << "Дерево общего вида (вариант 14 - ширина дерева)\n";
    Node* root = nullptr;

    for (;;) {
        std::cout << "\nМеню:\n";
        std::cout << "1) Создать корень\n";
        std::cout << "2) Добавить узел (по ключу родителя)\n";
        std::cout << "3) Вывести дерево\n";
        std::cout << "4) Удалить узел (поддерево)\n";
        std::cout << "5) Вычислить ширину дерева\n";
        std::cout << "0) Выход\n";
        std::cout << "Выберите пункт: ";

        int cmd;
        if (!(std::cin >> cmd)) {
            std::cerr << "Ошибка ввода команды. Завершение программы.\n";
            break;
        }

        if (cmd == 0) break;

        if (cmd == 1) {
            int key;
            std::cout << "Введите ключ корня: ";
            if (!(std::cin >> key)) {
                std::cerr << "Ошибка ввода ключа. Завершение программы.\n";
                break;
            }

            if (createRoot(root, key) == 0) std::cout << "Корень создан успешно.\n";
            else std::cout << "Не удалось создать корень.\n";
        }
        else if (cmd == 2) {
            int parent_key, key;

            std::cout << "Введите ключ родителя: ";
            if (!(std::cin >> parent_key)) {
                std::cerr << "Ошибка ввода ключа родителя. Завершение программы.\n";
                break;
            }

            std::cout << "Введите ключ нового узла: ";
            if (!(std::cin >> key)) {
                std::cerr << "Ошибка ввода ключа узла. Завершение программы.\n";
                break;
            }

            if (addByParentKey(root, parent_key, key) == 0) std::cout << "Узел добавлен успешно.\n";
            else std::cout << "Не удалось добавить узел.\n";
        }
        else if (cmd == 3) {
            if (root == nullptr) std::cout << "Дерево пустое.\n";
            else {
                std::cout << "\nСтруктура дерева:\n";
                printTree(root, 0);
            }
        }
        else if (cmd == 4) {
            int key;
            std::cout << "Введите ключ удаляемого узла: ";
            if (!(std::cin >> key)) {
                std::cerr << "Ошибка ввода ключа. Завершение программы.\n";
                break;
            }

            if (deleteByKey(root, key) == 0) std::cout << "Узел (и поддерево) удалён успешно.\n";
            else std::cout << "Не удалось удалить узел.\n";
        }
        else if (cmd == 5) {
            std::cout << "Ширина дерева: " << treeWidth(root) << "\n";
        }
        else {
            std::cout << "Неизвестная команда.\n";
        }
    }

    freeTree(root);
    return 0;
}
