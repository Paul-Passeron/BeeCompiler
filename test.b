void printn(i64 n, i8 b)
{
    auto i64 a = n / b;
    if (a)
        printn(a, b);
    putchar(n % b + '0');
}

i64 main(i64 argc, char **argv)
{
    for (auto i64 i = 0; i < 1000; i++)
        printn(i, 10);
    return 0;
}

i64 div_by_10(float a)
{
    return a / 10;
}
