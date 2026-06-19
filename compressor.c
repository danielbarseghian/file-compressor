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
void write_file(char **new_arr, int LEN, char *name, node **node_arr);

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
    if (f == NULL)
    {
        printf("Error: Could not open file %s\n", argv[1]);
        return 1;
    }
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    if (fsize == 0)
    {
        printf("File is empty!\n");
        fclose(f);
        return 0;
    }
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

    for (int i = 0, put = 0; i < 256 && put < node_count; i++)
    {
        if (frequencies_pnt[i] != NULL)
        {
            node_arr[put] = frequencies_pnt[i];
            put++;
        }
    }

    node **cpy_arr = malloc(sizeof(node*) * node_count);

    for (int i = 0; i < node_count; i++)
    {
        cpy_arr[i] = malloc(sizeof(node));
        *cpy_arr[i] = *node_arr[i];
    }

    free(frequencies_pnt);
    
    node *t = build_huffman(node_arr);
    if (t == NULL)
    {
        printf("error while building tree\n");
        return 1;
    }

    char *b = malloc(sizeof(char) * 20);

    char *bfr = malloc(sizeof(char) * node_count);
    if (bfr == NULL)
    {
        printf("error while malloc\n");
        return 1;
    }

    int depth = 0;
    char **codes = calloc(256, sizeof(char *));
    if (codes == NULL)
    {
        printf("error while calloc\n");
        return 1;
    }

    build_codes(t, bfr, depth, codes);

    FILE *out = fopen(argv[2], "w");
    const int LEN = strlen(buffer);
    char *new_arr[LEN];

    for (int i = 0; i < LEN; i++)
    {
        new_arr[i] = codes[buffer[i]];
    }
    write_file(new_arr, LEN, argv[2], cpy_arr);

    // Free the array
    free(node_arr);

    // Free the buffer
    free(buffer);

    printf("Successful\n");
}

void write_file(char **new_arr, int LEN, char *name, node **node_arr)
{
    FILE *f = fopen(name, "wb");
    if (f == NULL)
        return;

    // Put metadata
    
    for (int i = 0; i < node_count; i++)
    {
        char temp[20];

        snprintf(temp, 20, "%c:%i|", node_arr[i]->letter, node_arr[i]->repetition);

        fwrite(temp, 1, strlen(temp), f);
    }

    unsigned char byte = 0;
    int bit_count = 0;

    for (int i = 0; i < LEN; i++)
    {
        for (int j = 0; new_arr[i][j] != '\0'; j++)
        {
            byte <<= 1;

            if (new_arr[i][j] == '1')
                byte |= 1;

            bit_count++;

            if (bit_count == 8)
            {
                fwrite(&byte, 1, 1, f);

                byte = 0;
                bit_count = 0;
            }
        }
    }

    if (bit_count > 0)
    {
        byte <<= (8 - bit_count);

        fwrite(&byte, 1, 1, f);
    }

    fclose(f);
}

void build_codes(node *root, char *buffer, int depth, char **codes)
{
    if (!root) return;

    // LEAF
    if (root->left == NULL && root->right == NULL)
    {
        buffer[depth] = '\0';  // CRITICAL FIX
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
        int sum = 0;

        // Find the two smallest ones
        pair values = find_two_smallest(arr);

        if (arr[i] != NULL)
        {
            // Get the sum of thoses two values
            sum = values.first->repetition + values.seconde->repetition;

            // Create a new node
            parent = malloc(sizeof(node));
            parent->repetition = 0;

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

                    isfirst++;
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

                else if ((arr[i]->repetition == smaller) && (arr[i]->repetition != small))
                {
                    final.seconde = arr[i];
                    small = arr[i]->repetition;
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