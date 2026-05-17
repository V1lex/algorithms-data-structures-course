#include <cctype>
#include <cstdint>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

const char* COLOR_RED = "\x1b[31m";
const char* COLOR_RESET = "\x1b[0m";

struct Token {
    int64_t value;
    char op;
};

struct TreeNode {
    bool isNumber;
    int64_t value;
    char op;
    std::unique_ptr<TreeNode> left;
    std::unique_ptr<TreeNode> right;
};

bool isBinaryOperator(char ch) {
    return ch == '+' || ch == '-' || ch == '*' || ch == '/';
}

int operatorPriority(char op) {
    if (op == '(') return 0;
    if (op == '+' || op == '-') return 2;
    if (op == '*' || op == '/') return 3;
    if (op == '~') return 4;
    return -1;
}

std::unique_ptr<TreeNode> createNumberNode(int64_t value) {
    return std::make_unique<TreeNode>(TreeNode{true, value, '\0', nullptr, nullptr});
}

std::unique_ptr<TreeNode> createOperatorNode(char op, std::unique_ptr<TreeNode> left, std::unique_ptr<TreeNode> right) {
    return std::make_unique<TreeNode>(TreeNode{false, 0, op, std::move(left), std::move(right)});
}

int parseNumber(const std::string& expression, std::size_t& position, int64_t& value) {
    std::size_t start = position;

    while (position < expression.size() && std::isdigit(static_cast<unsigned char>(expression[position]))) {
        position++;
    }

    try {
        std::string number_text = expression.substr(start, position - start);
        long long parsed = std::stoll(number_text);

        if (parsed > std::numeric_limits<int64_t>::max()) {
            std::cerr << "Ошибка: число выходит за пределы int64_t.\n";
            return 1;
        }

        value = static_cast<int64_t>(parsed);
    } catch (const std::out_of_range&) {
        std::cerr << "Ошибка: число выходит за пределы int64_t.\n";
        return 1;
    } catch (const std::invalid_argument&) {
        std::cerr << "Ошибка: не удалось прочитать число.\n";
        return 1;
    }

    return 0;
}

int pushOperator(std::vector<Token>& postfix, char op) {
    postfix.push_back({0, op});
    return 0;
}

int infixToPostfix(const std::string& expression, std::vector<Token>& postfix) {
    std::vector<char> stack;
    char prev = '\0';

    postfix.clear();

    for (std::size_t i = 0; i < expression.size();) {
        unsigned char ch = static_cast<unsigned char>(expression[i]);

        if (std::isspace(ch)) {
            i++;
            continue;
        }

        if (std::isdigit(ch)) {
            int64_t value;
            if (parseNumber(expression, i, value) != 0) {
                return 1;
            }

            postfix.push_back({value, '\0'});
            prev = 'n';
            continue;
        }

        if (expression[i] == '(') {
            stack.push_back('(');
            i++;
            prev = '(';
            continue;
        }

        if (expression[i] == ')') {
            if (prev == '\0' || prev == 'o' || prev == '(') {
                std::cerr << "Ошибка: некорректное расположение закрывающей скобки.\n";
                return 1;
            }

            while (!stack.empty() && stack.back() != '(') {
                pushOperator(postfix, stack.back());
                stack.pop_back();
            }

            if (stack.empty()) {
                std::cerr << "Ошибка: несогласованные скобки.\n";
                return 1;
            }

            stack.pop_back();
            i++;
            prev = ')';
            continue;
        }

        if (isBinaryOperator(expression[i])) {
            char op = expression[i];

            if (op == '-' && (prev == '\0' || prev == 'o' || prev == '(')) {
                op = '~';
            } else if (prev != 'n' && prev != ')') {
                std::cerr << "Ошибка: оператор '" << expression[i] << "' стоит в недопустимом месте.\n";
                return 1;
            }

            int current_priority = operatorPriority(op);
            while (!stack.empty() && stack.back() != '(') {
                int top_priority = operatorPriority(stack.back());
                if (top_priority < current_priority || (op == '~' && top_priority == current_priority)) {
                    break;
                }
                pushOperator(postfix, stack.back());
                stack.pop_back();
            }

            stack.push_back(op);
            i++;
            prev = 'o';
            continue;
        }

        std::cerr << "Ошибка: недопустимый символ '" << expression[i] << "'.\n";
        return 1;
    }

    if (prev == '\0' || prev == 'o' || prev == '(') {
        std::cerr << "Ошибка: выражение оканчивается некорректно.\n";
        return 1;
    }

    while (!stack.empty()) {
        if (stack.back() == '(') {
            std::cerr << "Ошибка: несогласованные скобки.\n";
            return 1;
        }

        pushOperator(postfix, stack.back());
        stack.pop_back();
    }

    return 0;
}

std::unique_ptr<TreeNode> buildExpressionTree(const std::vector<Token>& postfix) {
    if (postfix.empty()) {
        std::cerr << "Ошибка: постфиксная запись пуста.\n";
        return nullptr;
    }

    std::vector<std::unique_ptr<TreeNode>> stack;

    for (const Token& token : postfix) {
        if (token.op == '\0') {
            stack.push_back(createNumberNode(token.value));
            continue;
        }

        if (token.op == '~') {
            if (stack.empty()) {
                std::cerr << "Ошибка: недостаточно операндов для унарного минуса.\n";
                return nullptr;
            }

            std::unique_ptr<TreeNode> operand = std::move(stack.back());
            stack.pop_back();
            stack.push_back(createOperatorNode('~', std::move(operand), nullptr));
            continue;
        }

        if (stack.size() < 2) {
            std::cerr << "Ошибка: недостаточно операндов для бинарной операции.\n";
            return nullptr;
        }

        std::unique_ptr<TreeNode> right = std::move(stack.back());
        stack.pop_back();
        std::unique_ptr<TreeNode> left = std::move(stack.back());
        stack.pop_back();
        stack.push_back(createOperatorNode(token.op, std::move(left), std::move(right)));
    }

    if (stack.size() != 1) {
        std::cerr << "Ошибка: не удалось построить корректное дерево выражения.\n";
        return nullptr;
    }

    return std::move(stack.back());
}

void setNodeToNumber(TreeNode& node, int64_t value) {
    node.left.reset();
    node.right.reset();
    node.isNumber = true;
    node.value = value;
    node.op = '\0';
}

int replaceWithChild(TreeNode& node, bool use_left_child) {
    std::unique_ptr<TreeNode> child = use_left_child ? std::move(node.left) : std::move(node.right);

    if (child == nullptr) {
        return 1;
    }

    node = std::move(*child);
    return 0;
}

int convertToUnaryMinus(TreeNode& node) {
    if (node.right == nullptr) {
        return 1;
    }

    node.left = std::move(node.right);
    node.right.reset();
    node.isNumber = false;
    node.value = 0;
    node.op = '~';
    return 0;
}

int simplifyTree(TreeNode& node, int64_t& value) {
    int64_t left_value;
    int64_t right_value;

    if (node.isNumber) {
        value = node.value;
        return 0;
    }

    if (node.op == '~') {
        if (node.left == nullptr || simplifyTree(*node.left, value) != 0) {
            return 1;
        }

        if (value == 0) {
            setNodeToNumber(node, 0);
            return 0;
        }

        value = -value;
        return 0;
    }

    if (node.left == nullptr || node.right == nullptr) {
        std::cerr << "Ошибка: некорректный узел дерева выражения.\n";
        return 1;
    }

    if (simplifyTree(*node.left, left_value) != 0) return 1;
    if (simplifyTree(*node.right, right_value) != 0) return 1;

    if (node.op == '+') {
        if (left_value == 0 && right_value == 0) {
            setNodeToNumber(node, 0);
        } else if (left_value == 0) {
            if (replaceWithChild(node, false) != 0) {
                std::cerr << "Ошибка: не удалось заменить узел правым поддеревом.\n";
                return 1;
            }
        } else if (right_value == 0) {
            if (replaceWithChild(node, true) != 0) {
                std::cerr << "Ошибка: не удалось заменить узел левым поддеревом.\n";
                return 1;
            }
        }

        value = left_value + right_value;
        return 0;
    }

    if (node.op == '-') {
        if (left_value == 0 && right_value == 0) {
            setNodeToNumber(node, 0);
        } else if (right_value == 0) {
            if (replaceWithChild(node, true) != 0) {
                std::cerr << "Ошибка: не удалось заменить узел левым поддеревом.\n";
                return 1;
            }
        } else if (left_value == 0) {
            if (convertToUnaryMinus(node) != 0) {
                std::cerr << "Ошибка: не удалось преобразовать вычитание в унарный минус.\n";
                return 1;
            }
        }

        value = left_value - right_value;
        return 0;
    }

    if (node.op == '*') {
        value = left_value * right_value;
        return 0;
    }

    if (node.op == '/') {
        if (right_value == 0) {
            std::cerr << "Ошибка: деление на ноль.\n";
            return 1;
        }

        value = left_value / right_value;
        return 0;
    }

    std::cerr << "Ошибка: неизвестная операция.\n";
    return 1;
}

bool needsParentheses(const TreeNode& node, char parent_op, bool is_right_child) {
    if (node.isNumber || parent_op == '\0') {
        return false;
    }

    int node_priority = operatorPriority(node.op);
    int parent_priority = operatorPriority(parent_op);

    if (parent_op == '~') return node_priority <= parent_priority;
    if (node.op == '~') return true;
    if (node_priority < parent_priority) return true;
    if (!is_right_child) return false;
    if (parent_op == '-' || parent_op == '/') return node_priority <= parent_priority;
    return parent_op == '*' && node.op == '/';
}

void printExpression(const TreeNode& node, char parent_op, bool is_right_child) {
    bool need_brackets = needsParentheses(node, parent_op, is_right_child);
    if (need_brackets) std::cout << "(";

    if (node.isNumber) {
        std::cout << node.value;
    } else if (node.op == '~') {
        std::cout << "-";
        if (node.left != nullptr) {
            printExpression(*node.left, '~', false);
        }
    } else {
        if (node.left != nullptr) {
            printExpression(*node.left, node.op, false);
        }
        std::cout << " " << node.op << " ";
        if (node.right != nullptr) {
            printExpression(*node.right, node.op, true);
        }
    }

    if (need_brackets) std::cout << ")";
}

void printExpressionTree(const TreeNode& node, int level, const std::string& label) {
    for (int i = 0; i < level; i++) {
        std::cout << "  ";
    }

    std::cout << label;

    if (node.isNumber) {
        std::cout << node.value << "\n";
    } else if (node.op == '~') {
        std::cout << "унарный -\n";
    } else {
        std::cout << node.op << "\n";
    }

    if (node.left != nullptr) {
        printExpressionTree(*node.left, level + 1, node.op == '~' ? "ARG: " : "L: ");
    }

    if (node.right != nullptr) {
        printExpressionTree(*node.right, level + 1, "R: ");
    }
}

int processExpression(const std::string& expression) {
    std::vector<Token> postfix;
    int64_t value;

    if (expression.empty()) {
        std::cerr << "Ошибка: пустое выражение.\n";
        return 1;
    }

    if (infixToPostfix(expression, postfix) != 0) return 1;

    std::unique_ptr<TreeNode> tree = buildExpressionTree(postfix);
    if (tree == nullptr) return 1;

    std::cout << "\nИсходное выражение: " << expression << "\n";
    std::cout << "\nДерево исходного выражения:\n";
    printExpressionTree(*tree, 0, "");

    if (simplifyTree(*tree, value) != 0) {
        return 1;
    }

    std::cout << "\nПреобразованное дерево:\n";
    printExpressionTree(*tree, 0, "");
    std::cout << "\nПреобразованное выражение: ";
    printExpression(*tree, '\0', false);
    std::cout << "\n";

    return 0;
}

int main() {
    std::vector<std::string> examples = {
        "(7 * (3 + 2) - (-1)) / 5",
        "15 + 0",
        "(0 + 8) + ((5 - 5) + (12 - 0))"
    };

    std::cout << "Проверочные примеры:\n";
    for (std::size_t i = 0; i < examples.size(); i++) {
        std::cout << COLOR_RED << "\nПример " << i + 1 << ":\n" << COLOR_RESET;
        processExpression(examples[i]);
    }

    std::string expression;
    std::cout << "\n\nДля завершения работы введите пустую строку\n";

    for (;;) {
        std::cout << COLOR_RED << "\nВведите выражение: " << COLOR_RESET;
        if (!std::getline(std::cin, expression)) {
            break;
        }
        if (expression.empty()) {
            break;
        }
        if (processExpression(expression) != 0) {
            std::cout << "Не удалось обработать выражение.\n";
        }
    }

    return 0;
}
