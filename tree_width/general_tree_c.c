#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    int key;
    struct Node* parent;
    struct Node* firstChild;
    struct Node* nextSibling;
} Node;

Node* createNode(int key) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) {
        fprintf(stderr, "Ошибка выделения памяти\n");
        return NULL;
    }
    newNode->key = key;
    newNode->parent = NULL;
    newNode->firstChild = NULL;
    newNode->nextSibling = NULL;
    return newNode;
}

Node* insertChild(Node* parent, int key) {
    if (parent == NULL) return NULL;

    Node* newNode = createNode(key);
    if (newNode == NULL) return NULL;

    newNode->parent = parent;

    if (parent->firstChild == NULL) {
        parent->firstChild = newNode;
    } else {
        Node* sibling = parent->firstChild;
        while (sibling->nextSibling != NULL) {
            sibling = sibling->nextSibling;
        }
        sibling->nextSibling = newNode;
    }
    return newNode;
}

Node* findNode(Node* root, int key) {
    if (root == NULL) return NULL;
    if (root->key == key) return root;

    Node* child = root->firstChild;
    while (child != NULL) {
        Node* found = findNode(child, key);
        if (found != NULL) return found;
        child = child->nextSibling;
    }
    return NULL;
}
int createRoot(Node** root, int key) {
    if (*root != NULL) {
        fprintf(stderr, "Корень уже существует\n");
        return 1;
    }
    *root = createNode(key);

    if (*root != NULL) {
        return 0;
    } else {
        return 1;
    }
}

int addByParentKey(Node* root, int parentKey, int key) {
    if (root == NULL) {
        fprintf(stderr, "Дерево пустое. Сначала создайте корень\n");
        return 1;
    }

    Node* parent_node = findNode(root, parentKey);
    if (parent_node == NULL) {
        fprintf(stderr, "Родительский узел с ключом %d не найден\n", parentKey);
        return 1;
    }

    if (findNode(root, key) != NULL) {
        fprintf(stderr, "Узел с ключом %d уже существует\n", key);
        return 1;
    }

    if (insertChild(parent_node, key) == NULL) {
        return 1;
    }
    return 0;
}

void printTree(Node* node, int level) {
    if (node == NULL) return;

    for (int i = 0; i < level; i++) {
        printf("  ");
    }
    printf("%d\n", node->key);

    printTree(node->firstChild, level + 1);
    printTree(node->nextSibling, level);
}

void freeTree(Node* node) {
    if (node == NULL) return;

    Node* child = node->firstChild;
    while (child != NULL) {
        Node* next = child->nextSibling;
        freeTree(child);
        child = next;
    }

    free(node);
}

int detachFromParent(Node* node) {
    if (node == NULL || node->parent == NULL) return 1;

    Node* parent = node->parent;

    if (parent->firstChild == node) {
        parent->firstChild = node->nextSibling;
    } else {
        Node* prev = parent->firstChild;
        while (prev != NULL && prev->nextSibling != node) {
            prev = prev->nextSibling;
        }
        if (prev == NULL) return 1;
        prev->nextSibling = node->nextSibling;
    }

    node->parent = NULL;
    node->nextSibling = NULL;
    return 0;
}

int deleteByKey(Node** root, int key) {
    if (*root == NULL) {
        fprintf(stderr, "Дерево пустое\n");
        return 1;
    }

    Node* node_to_delete = findNode(*root, key);
    if (node_to_delete == NULL) {
        fprintf(stderr, "Узел с ключом %d не найден\n", key);
        return 1;
    }

    if (node_to_delete == *root) {
        freeTree(*root);
        *root = NULL;
        return 0;
    }

    if (detachFromParent(node_to_delete) != 0) {
        fprintf(stderr, "Ошибка при отсоединении узла от родителя\n");
        return 1;
    }

    freeTree(node_to_delete);
    return 0;
}

int countAllNodes(Node* root) {
    if (root == NULL) return 0;
    int cnt = 1;
    for (Node* c = root->firstChild; c != NULL; c = c->nextSibling)
        cnt += countAllNodes(c);
    return cnt;
}

int treeWidth(Node* root) {
    if (root == NULL) return 0;

    int n = countAllNodes(root);
    if (n <= 0) return 0;

    Node** queue = (Node**)malloc((size_t)n * sizeof(Node*));
    if (queue == NULL) {
        fprintf(stderr, "Ошибка выделения памяти для очереди\n");
        return 0;
    }

    int head = 0;
    int tail = 0;
    queue[tail] = root;
    tail++;

    int max_width = 0;

    while (head < tail) {
        int level_size = tail - head;
        if (level_size > max_width) max_width = level_size;

        for (int i = 0; i < level_size; ++i) {
            Node* v = queue[head];
            head++;
            for (Node* c = v->firstChild; c != NULL; c = c->nextSibling) {
                queue[tail] = c;
                tail++;
            }
        }
    }

    free(queue);
    return max_width;
}

int main(void) {
    printf("Дерево общего вида (вариант 14 - ширина дерева)\n");
    Node* root = NULL;

    for (;;) {
        printf("\nМеню:\n");
        printf("1) Создать корень\n");
        printf("2) Добавить узел (по ключу родителя)\n");
        printf("3) Вывести дерево\n");
        printf("4) Удалить узел (поддерево)\n");
        printf("5) Вычислить ширину дерева\n");
        printf("0) Выход\n");
        printf("Выберите пункт: ");

        int cmd;
        if (scanf("%d", &cmd) != 1) {
            fprintf(stderr, "Ошибка ввода команды. Завершение программы.\n");
            break;
        }

        if (cmd == 0) {
            break;
        } 
        else if (cmd == 1) {
            int key;
            printf("Введите ключ корня: ");
            if (scanf("%d", &key) != 1) {
                fprintf(stderr, "Ошибка ввода ключа.\n");
                break;
            }

            if (createRoot(&root, key) == 0) {
                printf("Корень создан успешно.\n");
            } else {
                printf("Не удалось создать корень.\n");
            }
        } else if (cmd == 2) {
            int parent_key, key;

            printf("Введите ключ родителя: ");
            if (scanf("%d", &parent_key) != 1) {
                fprintf(stderr, "Ошибка ввода ключа родителя.\n");
                break;
            }

            printf("Введите ключ нового узла: ");
            if (scanf("%d", &key) != 1) {
                fprintf(stderr, "Ошибка ввода ключа узла.\n");
                break;
            }

            if (addByParentKey(root, parent_key, key) == 0) {
                printf("Узел добавлен успешно.\n");
            } else {
                printf("Не удалось добавить узел.\n");
            }
        } else if (cmd == 3) {
            if (root == NULL) {
                printf("Дерево пустое.\n");
            } else {
                printf("\nСтруктура дерева:\n");
                printTree(root, 0);
            }
        } else if (cmd == 4) {
            int key;
            printf("Введите ключ удаляемого узла: ");
            if (scanf("%d", &key) != 1) {
                fprintf(stderr, "Ошибка ввода ключа.\n");
                break;
            }

            if (deleteByKey(&root, key) == 0) {
                printf("Узел (и поддерево) удалён успешно.\n");
            } else {
                printf("Не удалось удалить узел.\n");
            }
        } else if (cmd == 5) {
            int width = treeWidth(root);
            printf("Ширина дерева: %d\n", width);
        } else {
            printf("Неизвестная команда.\n");
        }
    }

    freeTree(root);
    return 0;
}