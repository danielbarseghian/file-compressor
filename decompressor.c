#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        printf("Usage: ./decinoressor compressed output\n");
        return 1;
    }

    // read the metadata
    FILE *f = fopen(argv[1], "r");
    if (f == NULL)
    {
        printf("Error while opening compressed filen\n");
        return 1;
    }

    int c;
    char *buffer = malloc(sizeof(char) * 256);

    // Read whole file
    while (fread(buffer, sizeof(buffer), 1, f))
    {
        for (int i = 0; i < sizeof(buffer); i++)
        {
            if (buffer[i] == '\n')
            {
                break;
            }

            printf("%c", buffer[i]);
        }
    }

    free(buffer);

    return 0;
}