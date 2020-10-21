#pragma once

#include <stdlib.h>

struct buffer
{
    size_t cap;
    size_t fill;
    char* data;
};

static inline bool
balloc(struct buffer* buf, size_t n)
{
    char* data = malloc(n);
    if (!data) {
        buf->data = NULL;
        return false;
    }

    buf->cap = n;
    buf->fill = 0;
    buf->data = data;
    return true;
}

static inline bool
brealloc(struct buffer* buf, size_t n)
{
    char* data = realloc(buf->data, n);
    if (!data) return false;

    buf->cap = n;
    buf->data = data;
    return true;
}

static inline size_t
bfread(struct buffer* buf, size_t pad, FILE* fin)
{
    if (buf->fill + pad >= buf->cap &&
            !brealloc(buf, 2 * buf->cap))
        return 0;

    size_t count = fread(buf->data + buf->fill,
                         1, buf->cap - (buf->fill + pad),
                         fin);
    buf->fill += count;
    return count;
}
