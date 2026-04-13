#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_EXPRESSION_LEN 1024
#define MAX_TOKENS 1024
#define COLOR_RED "\x1b[31m"
#define COLOR_RESET "\x1b[0m"

typedef struct {
    int64_t value;
    char op;
} Token;

typedef struct TreeNode {
    int isNumber;
    int64_t value;
    char op;
    struct TreeNode *left;
    struct TreeNode *right;
} TreeNode;

void remove_newline(char *str) {
    if (str == NULL) return;
    int len = (int)strlen(str);
    if (len > 0 && str[len - 1] == '\n') str[len - 1] = '\0';
}

int is_binary_operator(char ch) {
    return ch == '+' || ch == '-' || ch == '*' || ch == '/';
}

int operator_priority(char op) {
    if (op == '(') return 0;
    // if (op == ')') return 1; нет смысла устанавливать приоритет для закрывающей скобки, так как она не обрабатывается в стеке операторов
    if (op == '+' || op == '-') return 2;
    if (op == '*' || op == '/') return 3;
    if (op == '~') return 4;
    return -1;
}

TreeNode *create_node(int is_number, int64_t value, char op, TreeNode *left, TreeNode *right) {
    TreeNode *node = (TreeNode *)malloc(sizeof(TreeNode));
    if (node == NULL) {
        fprintf(stderr, "Ошибка: не удалось выделить память.\n");
        return NULL;
    }
    node->isNumber = is_number;
    node->value = value;
    node->op = op;
    node->left = left;
    node->right = right;
    return node;
}

void free_tree(TreeNode *node) {
    if (node == NULL) return;
    free_tree(node->left);
    free_tree(node->right);
    free(node);
}

void free_tree_stack(TreeNode **stack, int top) {
    while (top >= 0) {
        free_tree(stack[top]);
        top--;
    }
}

int push_operator(Token *postfix, int *postfix_size, char op) {
    if (*postfix_size >= MAX_TOKENS) {
        fprintf(stderr, "Ошибка: выражение слишком длинное.\n");
        return 1;
    }
    postfix[*postfix_size].value = 0;
    postfix[*postfix_size].op = op;
    (*postfix_size)++;
    return 0;
}

int infix_to_postfix(const char *expression, Token *postfix, int *postfix_size) {
    if (expression == NULL || postfix == NULL || postfix_size == NULL) {
        fprintf(stderr, "Ошибка: передан некорректный аргумент.\n");
        return 1;
    }

    char stack[MAX_TOKENS];
    int top = -1;
    int out = 0;
    char prev = '\0';

    for (int i = 0; expression[i] != '\0';) {
        unsigned char ch = (unsigned char)expression[i];

        if (isspace(ch)) {
            i++;
            continue;
        }

        if (isdigit(ch)) {
            char *end_ptr;
            intmax_t value;
            errno = 0;
            value = strtoimax(&expression[i], &end_ptr, 10);
            if (errno == ERANGE || value < INT64_MIN || value > INT64_MAX) {
                fprintf(stderr, "Ошибка: число выходит за пределы int64_t.\n");
                return 1;
            }
            if (out >= MAX_TOKENS) {
                fprintf(stderr, "Ошибка: выражение слишком длинное.\n");
                return 1;
            }
            postfix[out].value = (int64_t)value;
            postfix[out].op = '\0';
            out++;
            i = (int)(end_ptr - expression);
            prev = 'n';
            continue;
        }

        if (expression[i] == '(') {
            if (top + 1 >= MAX_TOKENS) {
                fprintf(stderr, "Ошибка: стек операторов переполнен.\n");
                return 1;
            }
            stack[++top] = '(';
            i++;
            prev = '(';
            continue;
        }

        if (expression[i] == ')') {
            if (prev == '\0' || prev == 'o' || prev == '(') {
                fprintf(stderr, "Ошибка: некорректное расположение закрывающей скобки.\n");
                return 1;
            }
            while (top >= 0 && stack[top] != '(') {
                if (push_operator(postfix, &out, stack[top--]) != 0) return 1;
            }
            if (top < 0) {
                fprintf(stderr, "Ошибка: несогласованные скобки.\n");
                return 1;
            }
            top--;
            i++;
            prev = ')';
            continue;
        }

        if (is_binary_operator(expression[i])) {
            char op = expression[i];
            int current_priority;

            if (op == '-' && (prev == '\0' || prev == 'o' || prev == '(')) {
                op = '~';
            } else if (prev != 'n' && prev != ')') {
                fprintf(stderr, "Ошибка: оператор '%c' стоит в недопустимом месте.\n", expression[i]);
                return 1;
            }

            current_priority = operator_priority(op);
            while (top >= 0 && stack[top] != '(') {
                int top_priority = operator_priority(stack[top]);
                if (top_priority < current_priority || (op == '~' && top_priority == current_priority)) {
                    break;
                }
                if (push_operator(postfix, &out, stack[top--]) != 0) return 1;
            }

            if (top + 1 >= MAX_TOKENS) {
                fprintf(stderr, "Ошибка: стек операторов переполнен.\n");
                return 1;
            }
            stack[++top] = op;
            i++;
            prev = 'o';
            continue;
        }

        fprintf(stderr, "Ошибка: недопустимый символ '%c'.\n", expression[i]);
        return 1;
    }

    if (prev == '\0' || prev == 'o' || prev == '(') {
        fprintf(stderr, "Ошибка: выражение оканчивается некорректно.\n");
        return 1;
    }

    while (top >= 0) {
        if (stack[top] == '(') {
            fprintf(stderr, "Ошибка: несогласованные скобки.\n");
            return 1;
        }
        if (push_operator(postfix, &out, stack[top--]) != 0) return 1;
    }

    *postfix_size = out;
    return 0;
}

TreeNode *build_expression_tree(const Token *postfix, int postfix_size) {
    if (postfix == NULL || postfix_size <= 0) {
        fprintf(stderr, "Ошибка: постфиксная запись пуста.\n");
        return NULL;
    }

    TreeNode *stack[MAX_TOKENS];
    int top = -1;

    for (int i = 0; i < postfix_size; i++) {
        TreeNode *node;

        if (postfix[i].op == '\0') {
            node = create_node(1, postfix[i].value, '\0', NULL, NULL);
            if (node == NULL) {
                free_tree_stack(stack, top);
                return NULL;
            }
            stack[++top] = node;
            continue;
        }

        if (postfix[i].op == '~') {
            TreeNode *operand;
            if (top < 0) {
                fprintf(stderr, "Ошибка: недостаточно операндов для унарного минуса.\n");
                free_tree_stack(stack, top);
                return NULL;
            }
            operand = stack[top--];
            node = create_node(0, 0, '~', operand, NULL);
            if (node == NULL) {
                free_tree(operand);
                free_tree_stack(stack, top);
                return NULL;
            }
            stack[++top] = node;
            continue;
        }

        if (top < 1) {
            fprintf(stderr, "Ошибка: недостаточно операндов для бинарной операции.\n");
            free_tree_stack(stack, top);
            return NULL;
        }

        TreeNode *right = stack[top--];
        TreeNode *left = stack[top--];
        node = create_node(0, 0, postfix[i].op, left, right);
        if (node == NULL) {
            free_tree(left);
            free_tree(right);
            free_tree_stack(stack, top);
            return NULL;
        }
        stack[++top] = node;
    }

    if (top != 0) {
        fprintf(stderr, "Ошибка: не удалось построить корректное дерево выражения.\n");
        free_tree_stack(stack, top);
        return NULL;
    }

    return stack[0];
}

void set_node_to_number(TreeNode *node, int64_t value) {
    if (node == NULL) return;
    free_tree(node->left);
    free_tree(node->right);
    node->isNumber = 1;
    node->value = value;
    node->op = '\0';
    node->left = NULL;
    node->right = NULL;
}

int replace_with_child(TreeNode *node, int use_left_child) {
    TreeNode *child;
    TreeNode *other_child;
    TreeNode saved;

    if (node == NULL) return 1;
    child = use_left_child ? node->left : node->right;
    other_child = use_left_child ? node->right : node->left;
    if (child == NULL) return 1;

    saved = *child;
    free_tree(other_child);
    free(child);
    *node = saved;
    return 0;
}

int convert_to_unary_minus(TreeNode *node) {
    if (node == NULL || node->right == NULL) return 1;
    free_tree(node->left);
    node->left = node->right;
    node->right = NULL;
    node->isNumber = 0;
    node->value = 0;
    node->op = '~';
    return 0;
}

int simplify_tree(TreeNode *node, int64_t *value) {
    int64_t left_value;
    int64_t right_value;

    if (node == NULL || value == NULL) {
        fprintf(stderr, "Ошибка: передан некорректный аргумент.\n");
        return 1;
    }

    if (node->isNumber) {
        *value = node->value;
        return 0;
    }

    if (node->op == '~') {
        if (simplify_tree(node->left, value) != 0) return 1;
        if (*value == 0) {
            set_node_to_number(node, 0);
            return 0;
        }
        *value = -*value;
        return 0;
    }

    if (simplify_tree(node->left, &left_value) != 0) return 1;
    if (simplify_tree(node->right, &right_value) != 0) return 1;

    if (node->op == '+') {
        if (left_value == 0 && right_value == 0) set_node_to_number(node, 0);
        else if (left_value == 0) {
            if (replace_with_child(node, 0) != 0) {
                fprintf(stderr, "Ошибка: не удалось заменить узел правым поддеревом.\n");
                return 1;
            }
        } else if (right_value == 0) {
            if (replace_with_child(node, 1) != 0) {
                fprintf(stderr, "Ошибка: не удалось заменить узел левым поддеревом.\n");
                return 1;
            }
        }
        *value = left_value + right_value;
        return 0;
    }

    if (node->op == '-') {
        if (left_value == 0 && right_value == 0) set_node_to_number(node, 0);
        else if (right_value == 0) {
            if (replace_with_child(node, 1) != 0) {
                fprintf(stderr, "Ошибка: не удалось заменить узел левым поддеревом.\n");
                return 1;
            }
        } else if (left_value == 0) {
            if (convert_to_unary_minus(node) != 0) {
                fprintf(stderr, "Ошибка: не удалось преобразовать вычитание в унарный минус.\n");
                return 1;
            }
        }
        *value = left_value - right_value;
        return 0;
    }

    if (node->op == '*') {
        *value = left_value * right_value;
        return 0;
    }

    if (node->op == '/') {
        if (right_value == 0) {
            fprintf(stderr, "Ошибка: деление на ноль.\n");
            return 1;
        }
        *value = left_value / right_value;
        return 0;
    }

    fprintf(stderr, "Ошибка: неизвестная операция.\n");
    return 1;
}

int needs_parentheses(const TreeNode *node, char parent_op, int is_right_child) {
    int node_priority;
    int parent_priority;

    if (node == NULL || node->isNumber || parent_op == '\0') return 0;
    node_priority = operator_priority(node->op);
    parent_priority = operator_priority(parent_op);

    if (parent_op == '~') return node_priority <= parent_priority;
    if (node->op == '~') return 1;
    if (node_priority < parent_priority) return 1;
    if (!is_right_child) return 0;
    if (parent_op == '-' || parent_op == '/') return node_priority <= parent_priority;
    return parent_op == '*' && node->op == '/';
}

void print_expression(const TreeNode *node, char parent_op, int is_right_child) {
    if (node == NULL) return;
    
    int need_brackets = needs_parentheses(node, parent_op, is_right_child);
    if (need_brackets) printf("(");

    if (node->isNumber) {
        printf("%" PRId64, node->value);
    } else if (node->op == '~') {
        printf("-");
        print_expression(node->left, '~', 0);
    } else {
        print_expression(node->left, node->op, 0);
        printf(" %c ", node->op);
        print_expression(node->right, node->op, 1);
    }

    if (need_brackets) printf(")");
}

void print_expression_tree(const TreeNode *node, int level, const char *label) {
    if (node == NULL) return;

    for (int i = 0; i < level; i++) printf("  ");
    if (label != NULL) printf("%s", label);

    if (node->isNumber) printf("%" PRId64 "\n", node->value);
    else if (node->op == '~') printf("унарный -\n");
    else printf("%c\n", node->op);

    if (node->left != NULL) {
        print_expression_tree(node->left, level + 1, node->op == '~' ? "ARG: " : "L: ");
    }
    if (node->right != NULL) print_expression_tree(node->right, level + 1, "R: ");
}

int process_expression(const char *expression) {
    Token postfix[MAX_TOKENS];
    int postfix_size = 0;
    int64_t value;
    TreeNode *tree;

    if (expression == NULL || expression[0] == '\0') {
        fprintf(stderr, "Ошибка: пустое выражение.\n");
        return 1;
    }

    if (infix_to_postfix(expression, postfix, &postfix_size) != 0) return 1;
    tree = build_expression_tree(postfix, postfix_size);
    if (tree == NULL) return 1;

    printf("\nИсходное выражение: %s\n", expression);
    printf("\nДерево исходного выражения:\n");
    print_expression_tree(tree, 0, NULL);

    if (simplify_tree(tree, &value) != 0) {
        free_tree(tree);
        return 1;
    }

    printf("\nПреобразованное дерево:\n");
    print_expression_tree(tree, 0, NULL);
    printf("\nПреобразованное выражение: ");
    print_expression(tree, '\0', 0);
    printf("\n");

    free_tree(tree);
    return 0;
}

int main(void) {
    const char *examples[] = {
        "(7 * (3 + 2) - (-1)) / 5",
        "15 + 0",
        "(0 + 8) + ((5 - 5) + (12 - 0))"
    };

    printf("Проверочные примеры:\n");
    for (int i = 0; i < 3; i++) {
        printf(COLOR_RED "\nПример %d:\n" COLOR_RESET, i + 1);
        process_expression(examples[i]);
    }

    char expression[MAX_EXPRESSION_LEN];
    printf("\n\nДля завершения работы введите пустую строку\n");

    for (;;) {
        printf(COLOR_RED "\nВведите выражение: " COLOR_RESET);
        if (fgets(expression, sizeof(expression), stdin) == NULL) break;
        remove_newline(expression);
        if (expression[0] == '\0') break;
        if (process_expression(expression) != 0) printf("Не удалось обработать выражение.\n");
    }

    return 0;
}
