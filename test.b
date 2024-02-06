i64 main()
{
    printn(1544578);
    return 0;
}

void printn(i64 n)
{
    auto i64 a = n / 10;
    if (a)
        printn(a);
    putchar(n % 10 + '0');
}