#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct node
{
    char letter;
    struct node *next;
} node;

int main (int argc, char *argv[])
{
    // Create an array of frequencies and initialize it to 0
    unsigned int FREQLN = 256;
    int frequencies[256] = {0};

    if (argc != 3)
    {
        printf("usage: ./compressor file1 file2\n");
    }  

    // Read file
    FILE *f = fopen(argv[1], "r");
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);  // same as rewind(f)

    char *buffer = malloc(fsize + 1);
    fread(buffer, fsize, 1, f);
    fclose(f);

    // Put the last value as \0
    buffer[fsize] = '\0';

    // loop until Null
    for (int i = 0; buffer[i] != '\0'; i++)
    {
        frequencies[(unsigned char) buffer[i]]++;
    }
    
    // Sort it from biggest to smallest

    for (int i = 0; i < 256; i++)
    {
        if (frequencies[i] != 0)
        {
            printf("%c, %i\n", (char) i, frequencies[i]);
        }
    }

    free(buffer);
}

node *linked_list(int frequencie[], unsigned int FREQLN)
{
    node *first = NULL;
    int nodeln = 0;
    // Iterate on very frequencie
    for (int i = 0; i < FREQLN; i++)
    {
        // Create a node if the value is not 0
        if (frequencie[i] != 0)
        {
            // Create a pointer to where the value shoud be
            node *placement = NULL;

            // Iterate on every nodes
            if (first != NULL)
            {
                // For every nodes
                for (int j = 0; j < nodeln; j++)
                {
                    // Do something
                }
            }
        }
    }
}

// node *get_placement(int )