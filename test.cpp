#include <iostream>
#include <vector>

class SharedPtr {
   public:
    int *value;
    int *amount;

    SharedPtr() {
        value = NULL;
        amount = NULL;
    }

    SharedPtr(SharedPtr &other) {
        *this = other;
    }

    SharedPtr(int *ptr) {
        value = ptr;
        amount = new int(1);
    }

    ~SharedPtr() {
        // check
        if (amount && *amount == 1) {
            *this = (int *)NULL;
        } else if (amount) {
            *amount -= 1;
        }
    }

    SharedPtr &operator=(SharedPtr &other) {
        value = other.value;
        amount = other.amount;
        *amount += 1;
        return *this;
    }

    int &operator*() {
        return *value;
    }

    SharedPtr &operator=(int newValue) {
        std::cout << newValue << "\n";
        *value = newValue;
        return *this;
    }

    SharedPtr &operator=(int *ptr) {
        if (amount && *amount == 1) {
            delete value;
            value = NULL;
            delete amount;
            amount = NULL;
        }
        if (amount) {
            *amount -= 1;
        }
        value = ptr;
        amount = ptr;
        return *this;
    }
};

class Stuff {
   public:
    SharedPtr ptr;
    SharedPtr ptrOther;

    Stuff() : ptr(new int(1)) {
    }
};

int main() {
    Stuff *s1 = new Stuff();
    Stuff *s2 = new Stuff();
    Stuff *s3 = new Stuff();

    s1->ptrOther = s2->ptr;
    s2->ptrOther = s1->ptr;
    s3->ptrOther = s1->ptr;

    delete s1;
    s2->ptrOther = (int *)NULL;
    delete s2;
    delete s3;

    // SharedPtr *newPtr = new SharedPtr(new int(1));
    // std::cout << **newPtr << '\n';

    // SharedPtr ptr1Copy;
    // ptr1Copy = *newPtr;

    // **newPtr = -1;
    // std::cout << *ptr1Copy << '\n';
    // delete newPtr;

    // std::cout << *ptr1Copy << '\n';
    return 0;
}