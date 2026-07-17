#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdint.h>

typedef struct node
{
    unsigned char letter;
    uint32_t repetition;
    struct node *right;
    struct node *left;
} node;

typedef struct pair
{
    node *first;
    node *seconde;
} pair;

void write_file(unsigned char *m, unsigned char **codes, long long arr_size, char *name, node **node_arr);
void build_codes(node *root, char *buffer, int depth, unsigned char **codes);
const char *get_filename_ext(const char *filename);
pair find_two_smallest(node *f_pnt[256]);
node *create_node(char c, int freq);
node *build_huffman(node **arr);
void free_arr(node *arr);
int get_value(char *num);
void free_tree(node *t);

uint32_t node_count = 0;

int main (int argc, char *argv[])
{
    fflush(stdout);

    if (argc != 2)
    {
        printf("usage: ./compressor file1\n");
        return 1;
    }

    int f = open(argv[1], O_RDONLY);
    if (f == -1)
    {
        printf("open failed on %s\n", argv[1]);
        return 1;
    }

    struct stat buffer;

    int stat = fstat(f, &buffer);
    if (stat == -1)
    {
        printf("fstat failed\n");
        return 1;
    }

    long long fsize = buffer.st_size;

    printf("size: %lli\n", fsize);

    if (fsize == 0)
    {
        printf("file is empty\n");
        return 1;
    }

    unsigned char *m = mmap(NULL, fsize, PROT_READ, MAP_PRIVATE, f, 0);
    if (m == MAP_FAILED)
    {
        printf("mmap failled\n");
        return 1;
    }

    // Create an array of frequencies and initialize it to 0
    long long frequencies[256] = {0};

    // loop until Null
    for (long i = 0; i < fsize; i++)
    {
        frequencies[m[i]]++;
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

        for (int j = 7; j >= 0; j--)
        {
            printf("%d", (node_arr[i]->letter >> j) & 1);
        }
        printf(":%i||", node_arr[i]->repetition);
    }
    
    node *t = build_huffman(node_arr);

    if (t == NULL)
    {
        printf("error while building tree\n");
        return 1;
    }

    char *bfr = malloc(257); // in total its 257
    if (bfr == NULL)
    {
        printf("error while malloc\n");
        return 1;
    }

    int depth = 0;
    unsigned char **codes = calloc(256, sizeof(char *));
    if (codes == NULL)
    {
        printf("error while calloc\n");
        return 1;
    }

    if (node_count == 1)
    {
        codes[node_arr[0]->letter] = strdup("0");
    }
    else
    {
        build_codes(t, bfr, depth, codes);
    }

    // Get the first file extension
    const char *first = get_filename_ext(argv[1]);

    size_t size = strlen("output.") + strlen(first) + strlen(".huff") + 1;

    char *final_name = malloc(size);

    snprintf(final_name, size, "output.%s.huff", first);

    write_file(m, codes, fsize, final_name, cpy_arr);

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
    munmap(m, fsize);
    free(frequencies_pnt);
    free(node_arr);
    free(cpy_arr);

    close(f);

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

void write_file(unsigned char *m, unsigned char **codes, long long arr_size, char *name, node **node_arr)
{
    int meta_byte_count = 0;

    FILE *f = fopen(name, "wb");
    if (f == NULL)
    {
        printf("open fail");
        return;
    }

    // write metadata
    // The first byte is the binary value of byte_count
    fwrite(&node_count, sizeof(uint32_t), 1, f);

    for (int i = 0; i < node_count; i++)
    {
        unsigned char byte_letter = (int) node_arr[i]->letter;
        fwrite(&byte_letter, 1, 1, f);

        uint32_t byte_repetition = (uint32_t) node_arr[i]->repetition;
        fwrite(&byte_repetition, sizeof(uint32_t), 1, f);

        // Seek
        fseek(f, 0, SEEK_CUR);
    }

    // Didnt respect the primary rule, never use AI to fully write code
    // Im going to learn bitwise operators and how to code it myself.

    // Point to final so we can change it when needed
    unsigned char final = 0;
    int fcount = 0;
    long long arr_index = 0;

    // For every arrays
    while (arr_index < arr_size)
    {
        char *code = codes[(unsigned char) m[arr_index]];
        // For every letters
        for (int i = 0, len = strlen(code); i < len; i++)
        {
            // left shift
            final <<= 1;

            // Append if there is the number
            if (code[i] == '1')
                final |= 1;

            fcount++;

            // Reset both
            if (fcount >= 8)
            {
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
        printf("remaingings\n");
        final <<= (8 - fcount);
        fwrite(&final, 1, 1, f);
    }

    fclose(f);
}

void build_codes(node *root, char *buffer, int depth, unsigned char **codes)
{
    if (!root) return;

    // LEAF
    if (root->left == NULL && root->right == NULL)
    {
        buffer[depth] = '\0';
        unsigned int index = 0;
        index = (unsigned int) root->letter;
        
        codes[index] = strdup(buffer);
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
        parent->letter = 0;

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