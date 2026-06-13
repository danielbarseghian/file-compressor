#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct node
{
    char letter;
    int repetition;
    struct node *right;
    struct node *left;
} node;

typedef struct pair
{
    char first;
    char seconde;
} pair;

node *create_node(char c, int freq);
pair find_two_smallest(node *f_pnt[256]);

int main (int argc, char *argv[])
{
    // Create an array of frequencies and initialize it to 0
    int frequencies[256] = {0};

    if (argc != 3)
    {
        printf("usage: ./compressor file1 file2\n");
        return 1;
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

    // Make a list of pointer to the nodes
    node **frequencies_pnt = malloc(sizeof(node*) * 256);

    // Initialize everything
    for (int i = 0; i < 256; i++)
    {
        frequencies_pnt[i] = NULL;
    }

    for (int i = 0; i < 256; i++)
    {
        if (frequencies[i] > 0)
        {
            frequencies_pnt[i] = create_node((unsigned char) i, frequencies[i]);
        }
    }


    for (int i = 0; i < 256; i++)
    {
        if (frequencies[i] > 0)
        {
            frequencies_pnt[i] = create_node((unsigned char) i, frequencies[i]);
        }
    }

    for (int i = 0; i < 256; i++)
    {
        if (frequencies[i] > 0)
        {
            printf("%c, %i\n", frequencies_pnt[i]->letter, frequencies_pnt[i]->repetition);
        }
    }
    
    // Find the two smallest ones
    pair values = find_two_smallest(frequencies_pnt);

    printf("%c %c\n", values.first, values.seconde);

    free(buffer);
}

pair find_two_smallest(node **f_pnt)
{
    pair final;
    for (int i = 0; i < 256; i++)
    {
        if (f_pnt[i] != NULL)
        {
            // Put initial value
            final.seconde = f_pnt[i]->letter;
        }
    }
    int isfirst = 0;
    int small;
    int smaller;

    for (int i = 0; i < 256; i++)
    {
        if (f_pnt[i] != NULL)
        {
            if (f_pnt[i]->repetition > 0)
            {
                if (isfirst == 0)
                {
                    // Put the value
                    smaller = f_pnt[i]->repetition;
                    final.first = f_pnt[i]->letter;
                    isfirst++;
                }

                else if (isfirst == 1)
                {
                    if (f_pnt[i]->repetition < smaller)
                    {
                        small = smaller;
                        smaller = f_pnt[i]->repetition;

                        final.seconde = final.first;
                        final.first = f_pnt[i]->letter;
                    }

                    else if ((f_pnt[i]->repetition > smaller) && (f_pnt[i]->repetition < small))
                    {
                        small = f_pnt[i]->repetition;

                        final.seconde = f_pnt[i]->letter;
                    }
                }
            }
        }
        // Putting else if not else if ever there is a problem and the value is bigger we don't want to risk anything
        else if (isfirst == 3)
        { 
            if (f_pnt[i]->repetition < smaller)
            {
                if ((f_pnt[i]->repetition > small) && (f_pnt[i]->repetition < smaller))
                {
                    final.seconde = f_pnt[i]->letter;
                    small = f_pnt[i]->repetition;
                }
                else if (f_pnt[i]->repetition < smaller)
                {
                    // Swap them
                    final.seconde = final.first;
                    final.first = f_pnt[i]->letter;

                    small = smaller;
                    smaller = f_pnt[i]->repetition;
                }
            }
        }
    }

    return final;
}

// Build Huffman tree from frequency table
node *create_node(char c, int freq)
{
    node *n = malloc(sizeof(node));

    n->left = NULL;
    n->right = NULL;

    n->repetition = freq;
    n->letter = c;

    return n;
}