#include <stdio.h>

int main(void) {
    // Store each row separately -- makes it easy to color or edit per line
    const char *tree[] = {
        "    /\\",
        "   /  \\",
        "  /    \\",
        " /______\\",
        "    ||",
        "    ||"
    };
    int rows = sizeof(tree) / sizeof(tree[0]);

    for (int i = 0; i < rows; i++) {
        if (i < 4)
            printf("\033[38;2;34;139;34m%s\033[0m\n", tree[i]);  // forest green leaves
        else
            printf("\033[38;2;139;69;19m%s\033[0m\n", tree[i]);  // saddle brown trunk
    }

    return 0;
}
