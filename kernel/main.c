void kernel_main()
{
#ifdef EARLY_PRINT
    extern void early_print(const char *);
    early_print("Hello World!");
#endif

    while (1);
}
