#include <stdio.h>
#include <string.h>

int main(void) {
    const char *ramp = " .:-=+*#%@";
    const char *ramp_long  = " .'`^\",:;Il!i><~+_-?][}{1)(|\\/tfjrxnuvczXYUJCLQ0OZmwqpdbkhao*#MW&8%B@$";
    int n = (int)strlen(ramp);

    printf("ramp has %d characters -> valid indices are 0 to %d\n\n", n, n - 1);

    float samples[] = {0.0f, 0.1f, 0.25f, 0.42f, 0.5f, 0.75f, 0.9f, 1.0f};
    int count = (int)(sizeof(samples) / sizeof(samples[0]));

    for (int i = 0; i < count; i++) {
        float brightness = samples[i];
        int index = (int)(brightness * (n - 1) + 0.5f);
        printf("brightness %.2f  ->  index %d  ->  char '%c'\n", brightness, index, ramp[index]);
    }

    printf("\nsmooth gradient (brightness 0.0 -> 1.0):\n");
    for (int step = 0; step <= 60; step++) {
        float brightness = step / 15.0f;
        int index = (int)(brightness * (n - 1) + 0.5f);
        putchar(ramp[index]);
    }
    putchar('\n');

    return 0;
}
