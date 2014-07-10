#pragma once
#include "sparse_vector.h"
#include <iostream>
#include "test.h"
#include <assert.h>
class sparse_vector_tester1 : public test{
    virtual bool make(int showpassed){
        sparse_vector<int> a;
        a.push_back(1);
        a.push_back(2);
        a.push_back(3);

        a.erase(a.begin()+1);

        a.push_back(5);

        bool fail = false;
        TEST_ASSERT_EQUAL(a[1], 5, showpassed, fail);

        return fail;
    }
};

class sparse_vector_tester2 : public test{
    virtual bool make(int showpassed){
        sparse_vector<int> a;
        a.push_back(1);
        a.push_back(2);
        a.push_back(3);

        a.pop_back();
        a.pop_back();

        a.push_back(8);
        a.push_back(7);

        bool fail = false;
        TEST_ASSERT_EQUAL(a[1], 8, showpassed, fail);
        TEST_ASSERT_EQUAL(a[2], 7, showpassed, fail);

        return fail;
    }
};



