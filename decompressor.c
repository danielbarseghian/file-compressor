#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

typedef struct list
{
    unsigned char letter;
    uint32_t repetition;
    struct list *next;
} list;

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

uint32_t node_count = 0;
uint32_t byte_count = 0;
uint32_t rep_length = 0;
long after_meta = 0;

void build_codes(node *root, char *buffer, int depth, char **codes);
char *get_result(node *tree, char *byte[node_count]);
const char *get_filename_ext(const char *filename);
void put_in_arr(list *l, uint32_t buffer, node **arr);
void reverse_arr(node **arr, int len);
pair find_two_smallest(node **arr);
node *build_huffman(node **arr);
void initialize_list(list *l);
void free_list(list *first);
void print_all(list *l);\

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        printf("Usage: ./decompressor compressed decompressed\n");
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

    // read the metadata
    FILE *f = fopen(argv[1], "rb");
    if (f == NULL)
    {
        printf("Error while opening compressed file\n");
        return 1;
    }

    // Put it in a char of size of one byte
    const int BYTELEN = 8;
    char *buffer = malloc(sizeof(char) * BYTELEN);

    list *temp_list = malloc(sizeof(list));
    initialize_list(temp_list);

    int doend = 0;

    // Get the size
    fread(&node_count, sizeof(uint32_t), 1, f);
    printf("size: %" PRIu32 "\n", node_count);

    unsigned char letter;
    uint32_t rep = 0;

    for (uint32_t i = 0; i < node_count && fread(&letter, sizeof(unsigned char), 1, f) == 1 &&fread(&rep, sizeof(uint32_t), 1, f) == 1; i++)
    {
        // Put in full repetition
        rep_length += rep;

        // Malloc
        list *temp = malloc(sizeof(list));

        // Put the values
        temp->letter = letter;
        temp->repetition = rep;

        // Put temporary pnt to point to the front
        list *tmp_pnt = temp_list;

        // Put the head to the temp
        temp_list = temp;

        // Put the next to the old head
        temp_list->next = tmp_pnt;
    }

    // Capture file position right after all metadata has been read
    after_meta = ftell(f);
    printf("after_meta: %ld\n", after_meta);

    // I cant put directly here since i can't know the count before
    node **meta_arr = malloc(sizeof(node *) * node_count);
    if (meta_arr == NULL)
    {
        printf("error while malloc\n");
        return 1;
    }

    // Malloc every index
    for (uint32_t i = 0; i < node_count; i++)
    {
        meta_arr[i] = malloc(sizeof(node));
    }

    // Put it in the meta_arr
    uint32_t b = 0;
    put_in_arr(temp_list, b, meta_arr);

    // Reverse array
    reverse_arr(meta_arr, node_count);

    for (uint32_t i = 0; i < node_count; i++)
    {
        printf("%c:%" PRIu32 "||", (unsigned char) meta_arr[i]->letter, meta_arr[i]->repetition);
        byte_count += meta_arr[i]->repetition;
    }
    printf("\n");

    // Build the three
    node *tree = build_huffman(meta_arr);

    if (tree == NULL)
    {
        printf("error while building\n");
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

    build_codes(tree, bfr, depth, codes);

    for (int i = 0; i < 256; i++)
    {
        if (codes[i] != NULL)
        {
            printf("%c: %s\n", i, codes[i]);
        }
    }

    fclose(f);

    // Now reopen the file in byte
    FILE *fb = fopen(argv[1], "rb");

    // Skip metadata
    fseek(fb, after_meta, SEEK_SET);
    long current = ftell(fb);

    // get file size
    fseek(fb, 0, SEEK_END);
    long end = ftell(fb);
    long fsize = end - current;
    fseek(fb, after_meta, SEEK_SET);

    char *byte_arr[fsize];
    uint32_t byte = 0;

    int s = fread(&byte, sizeof(uint32_t), 1, fb);
    printf("%i\n", s);

    // read
    for (int i = 0; fread(&byte, sizeof(uint32_t), 1, fb); i++)
    {
        char *temp = malloc(33); // 32 bits + null terminator

        printf("Found: ");
        // Build bit string for each uint32_t (MSB first)
        for (int j = 31; j >= 0; j--)
        {
            int bit = (byte >> j) & 1;
            temp[31 - j] = '0' + bit;
        }
        temp[32] = '\0';
        printf("%s\n", temp);

        byte_arr[i] = temp;
    }
    printf("after\n");

    char *result = get_result(tree, byte_arr);

    if (result == NULL)
    {
        printf("error: result is NULL\n");
        return 1;
    }

    fclose(fb);

    FILE *fw = fopen(argv[2], "w");

    if (fw == NULL)
    {
        printf("error opening %s\n", argv[2]);
        return 1;
    }

    fwrite(result, 1, strlen(result), fw);

    fclose(fw);
    return 0;
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

const char *get_filename_ext(const char *filename)
{
    const char *dot = strrchr(filename, '.');

    if (!dot || dot == filename)
        return "";

    return dot + 1;
}

char *get_result(node *tree, char *byte[node_count])
{
    const int bytelen = 32; // 32 bits per uint32_t
    node *current = tree;
    uint32_t rep_sum = 0;

    char *final = malloc(sizeof(char) * rep_length + 1);
    final[0] = '\0';
    int times = 0;
    uint32_t byte_read = 0;

    // For every nodes
    for (int i = 0; byte_read < rep_length; i++)
    {
        // For every letters in the byte
        for (int j = 0; j < bytelen; j++)
        {
            if (byte_read >= rep_length)
                break;

            if ((byte[i][j]) == '0')
                current = current->left;

            else if ((byte[i][j]) == '1')
                current = current->right;

            // if its a leaf
            if (current->left == NULL && current->right == NULL)
            {
                // Append letter to the last character written
                int len = strlen(final);

                final[len] = current->letter;
                final[len + 1] = '\0';

                byte_read++;
                current = tree; // reset
            }
        }
    }

    return final;
}

void reverse_arr(node **arr, int len)
{
    int middle = len / 2;

    for (int i = 0; i < middle; i++)
    {
        node *tmp = arr[i];
        arr[i] = arr[len - 1 - i];
        arr[len - 1 - i] = tmp;
    }
}

void print_all(list *l)
{
    if (l == NULL)
    {
        return;
    }

    if (l->letter != '\0')
    {
        // Print metadata
        printf("%c:%" PRIu32 "||", l->letter, l->repetition);
    }

    if (l->next != NULL)
    {
        print_all(l->next);
    }
}

/// Build a huffman tree and return the top element
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

        uint32_t sum = values.first->repetition + values.seconde->repetition;

        // Create a new parent node
        node *parent = malloc(sizeof(node));
        parent->repetition = sum;
        parent->letter = '\0';

        parent->right = values.first;
        parent->left = values.seconde;

        int f_value = -1;
        int s_value = -1;

        // Find the matching indices in the array to update them
        for (uint32_t j = 0; j < node_count; j++)
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
    for (uint32_t i = 0; i < node_count; i++)
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

    for (uint32_t i = 0; i < node_count; i++)
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

void put_in_arr(list *l, uint32_t buffer, node **arr)
{
    // put in arr
    arr[buffer]->letter = l->letter;
    arr[buffer]->repetition = l->repetition;

    // initialize the other values
    arr[buffer]->right = NULL;
    arr[buffer]->left = NULL;

    // Incremente
    buffer++;

    // do next if not null
    if ((l->next != NULL) && (buffer < node_count))
    {
        put_in_arr(l->next, buffer, arr);
    }
}

void free_list(list *first)
{
    if (first->next != NULL)
    {
        free_list(first->next);
    }

    free(first);
}

void initialize_list(list *l)
{
    if (l != NULL)
    {
        l->letter = '\0';
        l->repetition = 0;
        l->next = NULL;
    }
}