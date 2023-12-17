#include "../webserv.hpp"

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
        if (amount && *amount == 1) {
            *this = (int *)NULL;
        } else if (amount) {
            *amount -= 1;
        }
    }

    SharedPtr &operator=(SharedPtr &other) {
        value = other.value;
        amount = other.amount;
        // std::cout << *other.value << "\n";
        // std::cout << *other.amount << "\n";
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
        std::cout << "deleting shared ptr value\n";
        delete value;
        std::cout << "deleting shared ptr amount\n";
        delete amount;
        value = ptr;
        amount = ptr;
        std::cout << "done deleting shared ptr\n";
        return *this;
    }
};
