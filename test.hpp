#include <iostream>
#include <memory>
#include <mutex>
#include <queue>

using namespace std;

// 单例模式演示类

class Person
{
public:
    Person(){str=new string("yyt");}
    ~Person(){
        delete str;
        cout << "person done\n";};
    string *str;
};

class Singleton

{

public:
    // 公有接口获取唯一实例

    static shared_ptr<Singleton> getInstance()
    {

        cout << "获取实例" << endl;

        cout << "地址为:" << instance << endl;

        return instance;
    }

    // 毁灭实例

    static void destoryInstance(Singleton *x)
    {
        x->~Singleton();
        cout << "自定义释放实例" << endl;

        delete x;
    }

    void stop()
    {
        this->~Singleton();
    }

    // 私有构造函数

    Singleton()
    {
        str = new string("yyt");
        cout << "构造函数启动。" << endl;
        que=new queue<int>();
        //p=new Person();
        // for(int i=0;i<10;i++)
        // {
        //     connectionQue_.push(i);
        // }
    };

public:
    // 私有析构函数
    ~Singleton() {}
    // ~Singleton()
    // {
    //     if(a==0)
    //     {
    //         a++;
    //         delete str;
    //         str=NULL;
    //         cout << "析构函数启动。" << endl;
    //         // for(int i=0;i<10;i++)
    //         // {
    //         //     int *p=connectionQue_.front();
    //         //     delete p;
    //         //     connectionQue_.front()=nullptr;
    //         //     connectionQue_.pop();
    //         // }
    //     }

    // };
    string *str;
    std::queue<int> *que;
    //Person *p;
    int a = 0;
    void func()
    {
        cout << "yyt\n";
    }

    // 静态私有对象

    static shared_ptr<Singleton> instance;
};

// 初始化

shared_ptr<Singleton> Singleton::instance(new Singleton(), [](Singleton *p) -> void
                                          {delete p; });