#include <iostream>

void test()
{
    throw 2;
}

int main()
{
    try
    {
        test();
    }
    catch(int err)
    {
        std::cout << err << "\n";
        test();
        std::cout << err << "\n";
    }
}