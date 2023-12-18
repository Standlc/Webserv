#include "../webserv.hpp"

class SharedPtr {
    int *_value;
    int *_amount;

   public:
    SharedPtr();
    SharedPtr(SharedPtr &other);
    SharedPtr(int *ptr);
    ~SharedPtr();
    SharedPtr &operator=(SharedPtr &other);
    int &operator*();
    SharedPtr &operator=(int newValue);
    SharedPtr &operator=(int *ptr);
};
