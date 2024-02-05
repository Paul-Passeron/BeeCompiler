i64 main()
{
    putnumb(256);
    return 0;
}

i64 fact(i64 n)
{
    if (n)
        return n * fact(n - 1);
    return 1;
}

void putdig(char n)
{
    putchar(n + '0');
}

void putnumb(i64 n)
{
    auto i64 a = n / 10;
    if (a)
        putnumb(a, 0);
    putdig(n % 10);
}