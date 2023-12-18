#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "text.h"


static size_t FileSize(const char *file_name);

static int TextPreprocessing(Buffer *buf);

static void TextDelim(Text *text, Buffer *buf);


static size_t FileSize(const char *file_name)
{
    struct stat file_info = {};
    stat(file_name, &file_info);

    return (size_t)file_info.st_size;
}

int BufText(Buffer *buf, const char *file_name)
{
    FILE *file = fopen(file_name, "rb");
    if(!file)
    {
        fprintf(stderr, "No such file: \"%s\"\n", file_name);
        return EXIT_FAILURE;
    }

    buf->size = FileSize(file_name);
    buf->buf  = (char *)calloc(buf->size + 1, sizeof(char));
    if(!buf->buf)
    {
        LOG("Error: Unable to allocate memory at %s:%s:%d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__ - 3);
        fclose(file);

        return EXIT_FAILURE;
    }

    fread(buf->buf, sizeof(char), buf->size, file);
    fclose(file);

    return EXIT_SUCCESS;
}


void BufDtor(Buffer *buf)
{
    free(buf->buf);
    buf->buf = NULL;

    buf->size    = 0;
    buf->n_lines = 0;
}


static int TextPreprocessing(Buffer *buf)
{
    buf->n_lines = 0;

    if(buf->buf[0] == '\0')
        return EXIT_FAILURE;

    char *read_p = buf->buf;

    for(; *read_p != '\0'; read_p++)
    {
        if(*read_p == '\n')
            buf->n_lines++;
    }

    if(read_p[-1] != '\n')
        buf->n_lines++;

    return EXIT_SUCCESS;
}

static void TextDelim(Text *text, Buffer *buf)
{
    char *buf_p = buf->buf;

    text->lines[0] = buf_p;
    size_t line_count = 1;

    for(size_t i = 0; i < buf->size; i++)
    {
        if(buf_p[i] == '\n' && buf_p[i + 1] != '\0')
        {
            buf_p[i] = '\0';

            text->lines[line_count++] = buf_p + i + 1;
        }
    }
}

Text ReadText(const char *file_name, Buffer *buf)
{
    if(!buf->buf)
    {
        int exit_status = BufText(buf, file_name);

        if(exit_status == EXIT_FAILURE)
            return {};
    }

    int exit_status = TextPreprocessing(buf);
    if(exit_status == EXIT_FAILURE)
    {
        fprintf(stderr, "File \'%s\' is empty.\n", file_name);
        BufDtor(buf);

        return {};
    }

    Text text = {};

    if(buf->n_lines != 0)
    {
        text.n_lines = buf->n_lines;
        text.lines   = (char **)calloc(text.n_lines, sizeof(char *));
        if(!text.lines)
        {
            LOG("Error: Unable to allocate memory at %s:%s:%d\n", __FILE__, __PRETTY_FUNCTION__, __LINE__ - 3);
            BufDtor(buf);

            return {};
        }

        TextDelim(&text, buf);
    }

    return text;
}


void TextDtor(Text *text)
{
    free(text->lines);
    text->lines = NULL;

    text->n_lines = 0;
}