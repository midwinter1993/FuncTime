#include <stdio.h>

int main() {
    char str[100];
    char ch;
    int idx = 0;

    printf("Input your name: ");
    for (;;) {
        ch = getchar();
        if (ch == '\n' || idx > 99)
            break;
        str[idx++] = ch;
    }
    str[idx] = '\0';
    printf("Hello, %s\n", str);

    return 0;
}
