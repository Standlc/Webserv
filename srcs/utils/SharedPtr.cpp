#include "SharedPtr.hpp"

SharedPtr::SharedPtr() {
    _value = NULL;
    _amount = NULL;
}

SharedPtr::SharedPtr(SharedPtr &other) {
    *this = other;
}

SharedPtr::SharedPtr(int *ptr) {
    _value = ptr;
    _amount = new int(1);
}

SharedPtr::~SharedPtr() {
    if (_amount && *_amount == 1) {
        *this = (int *)NULL;
    } else if (_amount) {
        *_amount -= 1;
    }
}

SharedPtr &SharedPtr::operator=(const SharedPtr &other) {
    _value = other._value;
    _amount = other._amount;
    *_amount += 1;
    return *this;
}

int &SharedPtr::operator*() {
    return *_value;
}

SharedPtr &SharedPtr::operator=(int newValue) {
    *_value = newValue;
    return *this;
}

SharedPtr &SharedPtr::operator=(int *ptr) {
    if (_amount && *_amount == 1) {
        delete _value;
        _value = NULL;
        delete _amount;
        _amount = NULL;
    }
    if (_amount) {
        *_amount -= 1;
    }
    _value = ptr;
    _amount = ptr;
    return *this;
}