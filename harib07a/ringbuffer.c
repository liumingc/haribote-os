#include <stdio.h>
#include <malloc.h>
#include <memory.h>

#define BUF_SIZE 4

typedef struct {
    char buf[BUF_SIZE];
    int ri, wi;
    int len; // without this field, then can't make full use of the buf!
} KbdBuffer;

// typedef struct KbdBuffer KbdBuffer;

static KbdBuffer kbd_buf;

KbdBuffer* newKbdBuf(void) {
    KbdBuffer* kb = (KbdBuffer*)malloc(sizeof(KbdBuffer));
    memset(kb, 0, sizeof(KbdBuffer));
    // kb->ri = -1;
    // kb->wi = -1;
    return kb;
}

static int inc_index(int oi) {
    if (oi == BUF_SIZE - 1) { // 3 -> wrap to 0
        oi = 0; // reset
    } else {
        oi += 1;
    }
    return oi;
}

void putChar(KbdBuffer *kb, char c) {
    if (kb->len == BUF_SIZE) {
        // catch up with read index, can't overwrite!
        printf("WRITE catch up, drop %c\n", c);
        return ;
    }
    printf("WRITE %c\n", c);
    kb->buf[kb->wi] = c;
    kb->wi = inc_index(kb->wi);
    kb->len++;
}

int getChar(KbdBuffer *kb) {
    if (kb->len == 0) {
        printf("READ buffer empty!\n");
        return 0;
    }
    char c = kb->buf[kb->ri];
    kb->ri = inc_index(kb->ri);
    printf("READ get %c\n", c);
    kb->len--;
    return c;
}

int main(int argc, char **argv) {
    KbdBuffer *kb = newKbdBuf();
    // int c;
    // getChar(kb);
    // getChar(kb);

    putChar(kb, 'a');
    putChar(kb, 'b');
    putChar(kb, 'c');
    putChar(kb, 'd');
    putChar(kb, 'e');
    getChar(kb); // a
    getChar(kb); // b
    getChar(kb); // c
    getChar(kb); // empty buffer
    getChar(kb); // empty buffer

    
    putChar(kb, 'f');
    putChar(kb, 'g');
    putChar(kb, 'h'); // full
    putChar(kb, 'i'); // full

    int i;
    for (i=0; i<6; i++) {
        getChar(kb);
    }

    return 0;
}