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
node *build_huffman(node **arr);
void build_codes(node *root, char *buffer, int depth, char **codes);

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
    
    node *t = build_huffman(node_arr);

    char *bfr = malloc(sizeof(char) * node_count);
    int depth = 0;
    char **codes = malloc(256 * sizeof(char *));
    for (int i = 0; i < 256; i++)
        codes[i] = NULL;

    build_codes(t, bfr, depth, codes);
    printf("a: %s\n", codes['a']);
    printf("b: %s\n", codes['b']);

    free(buffer);
}

void build_codes(node *root, char *buffer, int depth, char **codes)
{
    if (!root) return;

    // LEAF
    if (root->left == NULL && root->right == NULL)
    {
        buffer[depth] = '\0';  // 🔥 CRITICAL FIX
        codes[(unsigned char)root->letter] = strdup(buffer);
        return;
    }

    // LEFT = 0
    if (root->left)
    {
        buffer[depth] = '0';
        build_codes(root->left, buffer, depth + 1, codes);
    }

    // RIGHT = 1
    if (root->right)
    {
        buffer[depth] = '1';
        build_codes(root->right, buffer, depth + 1, codes);
    }
}

// Build a huffman tree and return the top element
node *build_huffman(node **arr)
{
    node *parent = NULL;

    for (int i = 0; i < node_count; i++)
    {
        // Find the two smallest ones
        pair values = find_two_smallest(arr);

        if (arr[i] != NULL)
        {
            // Get the sum of thoses two values
            int sum = values.first->repetition + values.seconde->repetition;

            // Create a new node
            parent = malloc(sizeof(node));

            // Point to thoses parents
            parent->right = values.first;
            parent->left = values.seconde;

            // Put the sum
            parent->repetition = sum;
            parent->letter = '\0';

            int f_value = 0;
            int s_value = 0;
            int f_found = 0;
            int s_found = 0;

            // Find the letter corresponding to the arr
            for (int j = 0; j < node_count; j++)
            {
                // Get the first one
                if ((arr[j] == values.first) && (f_found == 0))
                {
                    f_value = j;
                    f_found++;
                }

                if ((arr[j] == values.seconde) && (s_found == 0))
                {
                    s_value = j;
                    s_found++;
                }
            }
            
            // make this one null
            arr[s_value] = NULL;

            // And put the sum on the other
            arr[f_value] = parent;

            printf("MERGED: %c + %c -> freq %d\n",
            values.first->letter,
            values.seconde->letter,
            parent->repetition);
        }   
    }

    // Only one node should be here
    int null_count = 0;
    int parent_index = 0;
    for (int i = 0; i < node_count; i++)
    {
        if (arr[i] == NULL)
        {
            null_count++;
        }
        else 
        {
            parent_index = i;
        }
    }

    if (null_count == (node_count - 1))
    {
        return arr[parent_index];
    }
    
    return NULL;
}

pair find_two_smallest(node **arr)
{
    pair final;
    for (int i = 0; i < node_count; i++)
    {
        if (arr[i] != NULL)
        {
            // Put initial value
            final.seconde = arr[i];
        }
    }
    int isfirst = 0;
    int small;
    int smaller;

    for (int i = 0; i < node_count; i++)
    {
        if (arr[i] != NULL)
        {
            if (arr[i]->repetition > 0)
            {
                if (isfirst == 0)
                {
                    // Put the value
                    smaller = arr[i]->repetition;
                    final.first = arr[i];
                    isfirst++;
                }

                else if (isfirst == 1)
                {
                    if (arr[i]->repetition < smaller)
                    {
                        small = smaller;
                        smaller = arr[i]->repetition;

                        final.seconde = final.first;
                        final.first = arr[i];
                    }

                    else if (((arr[i]->repetition > smaller) && (arr[i]->repetition < small)) || (arr[i]->repetition == smaller) || (arr[i]->repetition != small))
                    {
                        small = arr[i]->repetition;

                        final.seconde = arr[i];
                    }
                }
            }
        }
        // Putting else if not else if ever there is a problem and the value is bigger we don't want to risk anything
        else if (isfirst == 3)
        { 
            if (arr[i]->repetition < smaller)
            {
                if ((arr[i]->repetition > small) && (arr[i]->repetition < smaller))
                {
                    final.seconde = arr[i];
                    small = arr[i]->repetition;
                }
                else if (arr[i]->repetition < smaller)
                {
                    // Swap them
                    final.seconde = final.first;
                    final.first = arr[i];

                    small = smaller;
                    smaller = arr[i]->repetition;
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