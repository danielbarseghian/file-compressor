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
    node *first;
    node *seconde;
} pair;

typedef struct list
{
    char *letter;
    int repetition;
    struct list *next;
} list;

node *create_node(char c, int freq);
pair find_two_smallest(node *f_pnt[256]);
node *build_huffman(node **arr, node **f_pnt);

int node_count = 0;

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

    free(buffer);

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
            node_count++;
        }
    }

    node **node_arr = malloc(sizeof(node*) * node_count);

    for (int i = 0, put = 0; put <= 4; i++)
    {
        if (frequencies_pnt[i] != NULL)
        {
            node_arr[put] = frequencies_pnt[i];
            put++;
        }
    }

    for (int i = 0; i < node_count; i++)
    {
        printf("%c: %i || ", node_arr[i]->letter, node_arr[i]->repetition);
    }
    printf("\n");
    
    node *t = build_huffman(node_arr, frequencies_pnt);
}

// Build a huffman tree and return the top element
node *build_huffman(node **arr, node **f_pnt)
{
    // Find the two smallest ones
    pair values = find_two_smallest(f_pnt);

    // Get the old names
    char *f_old = values.first->letter;
    char *s_old = values.seconde->letter;

    // Get the new name
    const unsigned int LEN = (strlen(f_old) + strlen(s_old));
    char *new_name = malloc(sizeof(char) * LEN);

    // Get the sum of thoses two values
    int sum = values.first->repetition + values.seconde->repetition;

    // Create a new node
    node *parent = malloc(sizeof(node));

    // Point to thoses parents
    parent->right = values.first;
    parent->left = values.seconde;

    int f_value = 0;
    int s_value = 0;
    int f_found = 0;
    int s_found = 0;

    // Find the letter corresponding to the arr
    for (int i = 0; i < node_count; i++)
    {
        // Get the first one
        if ((arr[i]->letter == values.first->letter) && (f_found == 0))
        {
            f_value = i;
            f_found++;
        }

        if ((arr[i]->letter == values.seconde->letter) && (s_found == 0))
        {
            s_value = i;
            s_found++;
        }
    }
    
    // Remove from the list
    free(s_value);
    arr[s_value] = NULL;

    // I've intentionally not freed the first one so i can replace values not create a new node

    // I need a new list only with the values and each time something goes i
    // left it NULL and i repeat until only one value is not NULL

    // Put the first value as our new value
    arr[f_value]->left = NULL;
    arr[f_value]->right = NULL;
    arr[f_value]->repetition = sum;
    arr[f_value]->letter = new_name;
}

pair find_two_smallest(node **f_pnt)
{
    pair final;
    for (int i = 0; i < 256; i++)
    {
        if (f_pnt[i] != NULL)
        {
            // Put initial value
            final.seconde = f_pnt[i];
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
                    final.first = f_pnt[i];
                    isfirst++;
                }

                else if (isfirst == 1)
                {
                    if (f_pnt[i]->repetition < smaller)
                    {
                        small = smaller;
                        smaller = f_pnt[i]->repetition;

                        final.seconde = final.first;
                        final.first = f_pnt[i];
                    }

                    else if (((f_pnt[i]->repetition > smaller) && (f_pnt[i]->repetition < small)) || (f_pnt[i]->repetition == smaller) || (f_pnt[i]->repetition != small))
                    {
                        small = f_pnt[i]->repetition;

                        final.seconde = f_pnt[i];
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
                    final.seconde = f_pnt[i];
                    small = f_pnt[i]->repetition;
                }
                else if (f_pnt[i]->repetition < smaller)
                {
                    // Swap them
                    final.seconde = final.first;
                    final.first = f_pnt[i];

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