

i64 main()
{
    var i64 i = 0;
    while (i != 10)
    {
        printn(fact(i));
        putchar('\n');
        i = i + 1;
    }
    test_vars();
    putchar('\n');
    return 0;
}

i64 fact(i64 n)
{
    if (n < 2)
        return 1;
    return n * fact(n - 1);
}

void printn(i64 n)
{
    var i64 a = n / 10;
    if (a)
        printn(a);
    putchar(n % 10 + '0');
}

void test_vars()
{
    var char a = 'a';
    var char b = a + 1;
    var char c = b + 1;
    var char d = c + 1;
    putchar(a);
    putchar(b);
    putchar(c);
    putchar(d);
}