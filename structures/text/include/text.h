#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

struct Buffer
{
    char *buf;

    size_t size;
    size_t n_lines;
};

struct Text
{
    char **lines;

    size_t n_lines;
};

int BufText(Buffer *buf, const char *file_name);

void BufDtor(Buffer *buf);

Text ReadText(const char *file_name, Buffer *buf);

void TextDtor(Text *text);

#endif //PREPROCESSOR_H