#include <iostream>
#include <memory>
#include <mutex>

#include "test.hpp"
using namespace std;

int main()
{
    //Person *p=new Person;
    {
        shared_ptr<Singleton> ptr=Singleton::getInstance();
        //ptr->que.push(1);
        ptr->stop();
        // cout<<ptr->a<<endl;
        // cout<<ptr->a<<endl;
        // cout<<ptr->a<<endl;
        // cout<<ptr->que.front()<<endl;
    }
    // {
    //     Singleton ptr;
    //     ptr.que.push(1);
    //     ptr.stop();
    //     cout<<endl<<ptr.a<<endl;
    //     cout<<ptr.a<<endl;
    //     cout<<ptr.a<<endl;
    //     cout<<ptr.que.front()<<endl;
    // }
    cin.get();
    return 0;
}