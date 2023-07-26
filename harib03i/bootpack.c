void io_hlt(void);

void HariMain(void)
{
    char *vram = 0xa0000;
    char *eVram = 0xaffff;
    char *p;

    for (p=vram; p<eVram; p++) {
        *p = (int)p & 0xf;
    }

    for (;;) {
        io_hlt();
    }
}
