#include <iostream>

class SharedPtr {
   public:
    int *value;
    int *amount;

    SharedPtr() {
        value = NULL;
        amount = NULL;
    }

    SharedPtr(SharedPtr &other) {
        this->operator=(other);
    }

    SharedPtr(int *ptr) {
        value = ptr;
        amount = new int(0);
    }

    ~SharedPtr() {
        if (!amount || *amount == 1) {
            delete value;
            delete amount;
        }
    }

    SharedPtr &operator=(SharedPtr &other) {
        std::cout << "copying\n";
        value = other.value;
        amount = other.amount;
        *amount += 1;
        return *this;
    }

    int &operator*() {
        return *value;
    }

    SharedPtr &operator=(int newValue) {
        *value = newValue;
        return *this;
    }

    SharedPtr &operator=(int *ptr) {
        delete value;
        value = NULL;
        delete amount;
        amount = NULL;
        std::cout << "resetting\n";
        return *this;
    }
};

int main() {
    SharedPtr ptr1(new int(0));

    std::cout << *ptr1 << '\n';
    SharedPtr ptr1Copy;
    ptr1Copy = ptr1;
    *ptr1 = -1;

    std::cout << *ptr1Copy << '\n';

    ptr1Copy = (int *)NULL;
    return 0;
}