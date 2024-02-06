i8 main()
{
    auto i64 a = 0;
    auto i64 max = 12;
    while (max - a)
    {
        printn(fact(a));
        putchar(',');
        putchar(' ');
        a = a + 1;
    }
    putchar('\n');

    return 0;
}

void fact(i64 n)
{
    if (n)
    {
        auto i64 a = n;
        return a * fact(a - 1);
    }
    return 1;
}

void fib(i64 n)
{
    if (n)
    {
    }
    else
        return 1;
    if (n - 1)
        return fib(n - 1) + fib(n - 2);
    return 1;
}

void printn(i64 a)
{
    if (a / 10)
        printn(a / 10);
    putchar(a % 10 + '0');
}