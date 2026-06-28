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

void write_file(char **byte_arr, int arr_size, char *name, node **node_arr);
void build_codes(node *root, char *buffer, int depth, char **codes);
const char *get_filename_ext(const char *filename);
pair find_two_smallest(node *f_pnt[256]);
node *create_node(char c, int freq);
node *build_huffman(node **arr);
void free_arr(node *arr);
int get_value(char *num);
void free_tree(node *t);

int node_count = 0;

int main (int argc, char *argv[])
{
    fflush(stdout);
    
    // Create an array of frequencies and initialize it to 0
    int frequencies[256] = {0};

    if (argc != 3)
    {
        printf("usage: ./compressor file1 file2\n");
        return 1;
    }

    // Get the first file extension
    const char *first = get_filename_ext(argv[1]);
    const char *second = get_filename_ext(argv[2]);
    if (strcmp(first, second) != 0)
    {
        printf("extensions must match\n");
        return 1;
    }

    // Read file
    FILE *f = fopen(argv[1], "rb");
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
    for (long i = 0; i < fsize; i++)
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
    printf("\n");

    // I need to sort node_arr

    node **cpy_arr = malloc(sizeof(node*) * node_count);

    for (int i = 0; i < node_count; i++)
    {
        cpy_arr[i] = malloc(sizeof(node));
        *cpy_arr[i] = *node_arr[i];

        printf("%c:%i||", node_arr[i]->letter, node_arr[i]->repetition);
    }
    printf("%i\n", node_count);
    printf("\n");
    
    node *t = build_huffman(node_arr);

    if (t == NULL)
    {
        printf("error while building tree\n");
        return 1;
    }

    char *bfr = malloc(sizeof(char) * 32 + 1); // in total its 257
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

    for (int i = 0; i < 256; i++)
    {
        if (codes[i] != NULL)
        {
            printf("%c: %s\n", i, codes[i]);
        }
    }
    
    char *byte_arr[fsize];

    for (int i = 0; i < fsize; i++)
    {
        byte_arr[i] = codes[(unsigned char)buffer[i]];
    }

    write_file(byte_arr, fsize, argv[2], cpy_arr);

    for (int i = 0; i < node_count; i++)
    {
        free_arr(cpy_arr[i]);
    }

    free(bfr);
    free_tree(t);
    for (int i = 0; i < 256; i++)
    {
        free(codes[i]);
    }
    free(codes);
    free(buffer);
    free(frequencies_pnt);
    free(node_arr);
    free(cpy_arr);

    printf("Successful\n");
}

void free_tree(node *t)
{
    if (t == NULL)
        return;

    if (t->left)
        free_tree(t->left);

    if (t->right)
        free_tree(t->right);

    free(t);
}

const char *get_filename_ext(const char *filename)
{
    const char *dot = strrchr(filename, '.');

    if (!dot || dot == filename)
        return "";

    return dot + 1;
}


void free_arr(node *arr)
{
    if (arr->left != NULL)
    {
        free_arr(arr->left);
    }

    if (arr->right != NULL)
    {
        free_arr(arr->right);
    }

    free(arr);
}

void write_file(char **byte_arr, int arr_size, char *name, node **node_arr)
{
    int meta_byte_count = 0;

    FILE *f = fopen(name, "wb");
    if (f == NULL)
    {
        printf("open fail");
        return;
    }

    // Put metadata
    
    for (int i = 0; i < node_count; i++)
    {
        char temp[20];

        snprintf(temp, 20, "%c:%i|", node_arr[i]->letter, node_arr[i]->repetition);

        fwrite(temp, 1, strlen(temp), f);

        // Seek
        fseek(f, 0, SEEK_CUR);
    }
    fwrite("\n", 1, 1, f);

    // Didnt respect the primary rule, never use AI to fully write code
    // Im going to learn bitwise operators and how to code it myself.

    // Point to final so we can change it when needed
    unsigned char final = 0;
    int fcount = 0;
    int arr_index = 0;

    // For every arrays
    while (arr_index < arr_size)
    {
        // For every letters
        for (int i = 0, len = strlen(byte_arr[arr_index]); i < len; i++)
        {
            // left shift
            final <<= 1;

            // Append if there is the number
            if (byte_arr[arr_index][i] == '1')
                final |= 1;

            fcount++;

            // Reset both
            if (fcount >= 8)
            {
                printf("writing: ");
                for (int i = 7; i >= 0; i--)
                {
                    printf("%d", (final >> i) & 1);
                }
                printf("\n");

                // write
                fwrite(&final, 1, 1, f);

                // Reset count
                fcount = 0;
                
                // reset final
                final = 0;
            }

            if (i == (len - 1))
            {
                arr_index++;
            }
        }
    }

    // write remaining
    if (fcount > 0)
    {
        final <<= (8 - fcount);
        fwrite(&final, 1, 1, f);
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
    while (1)
    {
        // Find the two smallest ones globally
        pair values = find_two_smallest(arr);

        // If there is no second node, the tree is fully built
        if (values.seconde == NULL)
        {
            break; 
        }
        
        int sum = values.first->repetition + values.seconde->repetition;

        // Create a new parent node
        node *parent = malloc(sizeof(node));
        parent->repetition = sum;
        parent->letter = '\0';

        parent->right = values.first;
        parent->left = values.seconde;

        int f_value = -1;
        int s_value = -1;

        // Find the matching indices in the array to update them
        for (int j = 0; j < node_count; j++)
        {
            if (arr[j] == values.first && f_value == -1)
            {
                f_value = j;
            }
            if (arr[j] == values.seconde && s_value == -1)
            {
                s_value = j;
            }
        }
        
        // Remove the second node and replace the first one with the parent
        arr[s_value] = NULL;
        arr[f_value] = parent;
    }

    // Find and return the single remaining root node
    for (int i = 0; i < node_count; i++)
    {
        if (arr[i] != NULL)
        {
            return arr[i];
        }
    }
    
    return NULL;
}

pair find_two_smallest(node **arr)
{
    pair results;
    
    results.first = NULL;
    results.seconde = NULL;

    for (int i = 0; i < node_count; i++)
    {
        // Check for null
        if (arr[i] == NULL)
        {
            continue;
        }

        if (results.first == NULL || arr[i]->repetition < results.first->repetition)
        {
            // Swap them and put value
            results.seconde = results.first;
            results.first = arr[i];
        }

        else if (results.seconde == NULL || arr[i]->repetition < results.seconde->repetition)
        {
            results.seconde = arr[i];
        }
    }

    return results;
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