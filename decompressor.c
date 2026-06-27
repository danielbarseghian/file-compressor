#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

typedef struct list
{
    char letter;
    int repetition;
    struct list *next;
} list;

typedef struct node
{
    char letter;
    unsigned int repetition;
    struct node *right;
    struct node *left;
} node;

typedef struct pair
{
    node *first;
    node *seconde;
} pair;

int count = 0;
long node_count = 0;
int byte_count = 0;
int rep_length = 0;

char *get_result(node *tree, char *byte[node_count]);
const char *get_filename_ext(const char *filename);
void put_in_arr(list *l, int buffer, node **arr);
void reverse_arr(node **arr, int len);
pair find_two_smallest(node **arr);
node *build_huffman(node **arr);
void initialize_list(list *l);
void free_list(list *first);
void print_all(list *l);
void reverse(char *s);

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
    FILE *f = fopen(argv[1], "r");
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

    char letter;
    int rep = 0;

    while (fscanf(f, "%c:%d|", &letter, &rep) == 2) 
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

        // Incremente
        node_count++;
    }

    // I cant put directly here since i can't know the count before
    node **meta_arr = malloc(sizeof(node *) * node_count);
    if (meta_arr == NULL)
    {
        printf("error while malloc\n");
        return 1;
    }

    // Malloc every index
    for (int i = 0; i < node_count; i++)
    {
        meta_arr[i] = malloc(sizeof(node));
    }
    
    // Put it in the meta_arr
    unsigned int b = 0;
    put_in_arr(temp_list, b, meta_arr);

    // Reverse array
    reverse_arr(meta_arr, node_count);

    for (int i = 0; i < node_count; i++)
    {
        printf("%c:%i||", meta_arr[i]->letter, meta_arr[i]->repetition);
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

    fclose(f);

    // Now reopen the file in byte
    FILE *fb = fopen(argv[1], "rb");

    // Skip metadata
    int c;
    while ((c = fgetc(fb)) != EOF && c != '\n');

    // get file size
    long start = ftell(fb);
    fseek(fb, 0, SEEK_END);
    long end = ftell(fb);
    long fsize = end - start;
    fseek(fb, start, SEEK_SET);

    char *byte_arr[fsize];
    unsigned char byte;

    // read
    for (int i = 0; fread(&byte, 1, 1, fb) == 1; i++)
    {
        char *temp = malloc(9); // 8 bits + null terminator

        // Build bit string for each byte
        for (int j = 7; j >= 0; j--)
        {
            int bit = (byte >> j) & 1;

            temp[7 - j] = '0' + bit;
        }
        temp[8] = '\0';

        byte_arr[i] = temp;

        printf("Found: %s\n", temp);
    }
    printf("\n");

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

    reverse(result);
    printf("write: %s\n", result);
    fwrite(result, 1, strlen(result), fw);

    fclose(fw);
    return 0;
}

void reverse(char *s)
{
    int len = strlen(s);
    int middle = len / 2;

    for (int i = 0; i < middle; i++)
    {
        char tmp = s[i];
        s[i] = s[len - 1 - i];  
        s[len - 1 - i] = tmp; 
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
    const int bytelen = 8;
    node *current = tree;
    int rep_sum = 0;

    char *final = malloc(sizeof(char) * rep_length + 1);
    final[0] = '\0';
    int times = 0;
    int byte_read = 0;

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

                printf("found %c\n", current->letter);
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
        printf("%c:%i||", l->letter, l->repetition);
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

void put_in_arr(list *l, int buffer, node **arr)
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