#include "csv.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <regex.h>


// Future encapsulation


struct buffer_csv *new_buffer_csv (char filename[], int columns, char *regex[], char delim[2])
{
    struct buffer_csv *buffer;

    // Buffer alloc
    buffer = NULL;
    buffer = (struct buffer_csv *) malloc (sizeof (struct buffer_csv));

    // Buffer fullfillment
    if (buffer)
    {
        // filename
        strcpy (buffer->from_file, filename);
        
        // column delim
        strcpy (buffer->delim, delim);

        // total columns
        buffer->total_columns = columns;

        // regex
        buffer->token_regex = (regex_t *) calloc (columns, sizeof (regex_t));
        // regex compile
        for (int i=0; i<columns; i++)
        {
            if ( regcomp (buffer->token_regex+i, regex[i], REG_EXTENDED | REG_ICASE) )
            {}
            // to-do: error handling
        }

        // tokens -per se-
        buffer->line_token = (char **) calloc (columns, sizeof (char *));

        // flag invalid tokens
        buffer->flag_invalid_token = (int *) calloc (columns, sizeof (int));
    }
    // to-do: error handling

    return buffer;
}

void init_buffer_csv (struct buffer_csv *buffer)
{
    // Open file
    buffer->file_pointer = fopen (buffer->from_file, "r");

    // Reset values
    buffer->line_length = 0;
    buffer->line_content = NULL;
}

int tokenize_csv_line (struct buffer_csv *buffer)
{
    int i; // token index a.k.a column
    char *token;
    char *line_ptr;
    ssize_t readed_bytes;

    // Read line
    readed_bytes = -1;
    readed_bytes = getline ( &(buffer->line_content), &(buffer->line_length), buffer->file_pointer ); // to-test: \r (carriege returns)
    
    // EoF
    if ( readed_bytes == -1 )
        i = readed_bytes;
    // Readed
    else
    {
        // Drop line feed: \n -> \0
        buffer->line_content [readed_bytes-1] = '\0';

        /* - - Tokenize - - */
        i = 0;
        line_ptr = buffer->line_content;
        
        // Per column readed...
        while ( (token = strsep(&line_ptr, buffer->delim)) )
        {
            // Check column range
            if (i < buffer->total_columns)
            {
                // Check column format
                if ( ! regexec (buffer->token_regex+i, token, 0, NULL, 0) )
                {
                    buffer->line_token[i] = token;
                    buffer->flag_invalid_token[i] = 0;
                }
                else
                {
                    // Empty token
                    buffer->line_token[i] = "\0";
                    // Raise flag
                    buffer->flag_invalid_token[i] = 1;
                }
            }
            // to-do: error handling

            // Advance column
            i++;
        }
    }
    
    // Total columns reached    
    return i; //{-1, 1, ... , total_columns}
}

void end_buffer_csv (struct buffer_csv *buffer)
{
    // Close file
    fclose (buffer->file_pointer);
}

void free_buffer_csv (struct buffer_csv *buffer)
{
    if ( buffer )
    {
        // Line buffer
        free ( buffer->line_content );
        // Tokens array
        free ( buffer->line_token );

        // Regex objects
        for (int i=0; i<buffer->total_columns; i++)
            regfree(buffer->token_regex+i);

        // Regex array
        free (buffer->token_regex);

        // Column flags
        free ( buffer->flag_invalid_token );

        // Buffer
        free ( buffer );
    }
}
