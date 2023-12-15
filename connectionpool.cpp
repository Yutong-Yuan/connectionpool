#include<iostream>
#include<mysql.h>

#include "connectionpool.h"


//～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～数据库连接类方法实现～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～
Connection::Connection()
{
    conn_=mysql_init(nullptr);
}

Connection::~Connection()
{   
    if(conn_!=nullptr)  mysql_close(conn_);
}

//连接数据库
bool Connection::connect(const std::string host, int port ,const std::string user, const std::string passwd, std::string database)
{
    if(mysql_real_connect(conn_,host.c_str(),user.c_str(),passwd.c_str(),database.c_str(),port,NULL,0)==nullptr) return false;
    return true;
}

//增删改
bool Connection::update(std::string sql)
{
    //允许用户键入sql语句后带分号
    if(sql.back()==';')     sql.pop_back();
    if(mysql_query(conn_,sql.c_str())!=0)   return false;
    return true;
}

//查
MYSQL_RES * Connection::query(std::string sql)
{
    if(sql.back()==';')     sql.pop_back();
    if(mysql_query(conn_,sql.c_str())!=0)   return nullptr;
    return mysql_store_result(conn_);
}


//～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～数据库连接池类方法实现～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～
ConnectionPool::ConnectionPool():
    host_(_host),
    port_(_port),
    user_(_user),
    passwd_(_passwd),
    database_(_database),
    maxIdleTime_(_maxIdleTime),
    connectionTimeout_(_connectionTimeout),
    connectionCurNum_(0){}

ConnectionPool::~ConnectionPool()
{
    stop();
}

//开启数据库连接池
void ConnectionPool::start(const int &connectionInitNum,const int &connectionMaxNum)
{
    connectionInitNum_=connectionInitNum;
    connectionMaxNum_=connectionMaxNum;
    isPoolExit_=false;
    exitFlags_=0;
    for(int i=0;i<connectionInitNum_;i++)
    {
        Connection *conn=new Connection();
        if(conn->connect(host_,port_,user_,passwd_,database_)==false)
        {
            std::cout<<"connect failed\n";
            delete conn;
            i--;
            continue;
        }
        connectionQue_.push(conn);
        connectionCurNum_++;
    }
    //启动专门用于生产连接的线程
    std::thread th1(&ConnectionPool::produceConnectionFunc,this);
    th1.detach();
    std::thread th2(&ConnectionPool::scannerConnectionFunc,this);
    th2.detach();
}

//主动关闭数据库连接池
void ConnectionPool::stop()
{
    //当第一次退出的时候exitFlags_==0释放一遍资源，之后再调用析构函数的时候，不再释放资源只是清理单例模式的内存
    if(exitFlags_==0)
    {
        //提供标识以防止资源二次释放降低效率（即主动stop了之后程序退出之后又调用了stop一次）
        exitFlags_++;
        {
            isPoolExit_=true;
            std::unique_lock<std::mutex> ulock(connectionQueMux_);
            //通知生产线程和监控线程，告诉他们可以自行退出了，如果当他们退出，则整个进程阻塞不能退出
            EmptyCv_.notify_all();
            notFreeCv_.notify_all();
        }
        while (1)
        {
            //当没有连接工作的时候 且需要关闭连接池的时候就关闭连接池
            if(connectionQue_.size()==connectionCurNum_)
            {
                std::unique_lock<std::mutex> ulock(connectionQueMux_);
                for(int i=0;i<connectionCurNum_;i++)
                {
                    Connection *conn=connectionQue_.front();
                    delete conn;
                    connectionQue_.pop();
                    
                }
                //以防死锁
                connectionCurNum_=0;
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}


//单例模式静态接口（自定义删除器）
std::shared_ptr<ConnectionPool> ConnectionPool::connectpool(new ConnectionPool(),[](ConnectionPool * cp)->void{
    //回收单例模式接口，防止内存泄露
    delete cp;
});



std::shared_ptr<ConnectionPool> ConnectionPool::getConnectpoolInstance()
{
    return connectpool;
}

//生产连接
void ConnectionPool::produceConnectionFunc()
{
    while (1)
    {
        {
            std::unique_lock<std::mutex> ulock(connectionQueMux_);
            //当队列不为空的时候等待
            EmptyCv_.wait(ulock,[&]()->bool{return connectionQue_.empty()||isPoolExit_;});
            //若得到连接池结束指令则退出
            if(isPoolExit_)     return;
            //若当前已经可以维护连接的最大值了，那么就不要继续生产连接了
            if(connectionCurNum_>=connectionMaxNum_)    continue;
            //生产连接
            Connection *conn=new Connection();
            conn->connect(host_,port_,user_,passwd_,database_);
            connectionQue_.push(conn);
            connectionCurNum_++;
            //生产完一个连接之后通知消费者
            notEmptyCv_.notify_all();
        }
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
    
}

//消费连接（用户从队列取走连接）
std::shared_ptr<Connection> ConnectionPool::getConnection()
{
    std::unique_lock<std::mutex> ulock(connectionQueMux_);
    //最多等待**ms之后，判断队列中是否有空闲连接
    if(notEmptyCv_.wait_for(ulock,std::chrono::milliseconds(_connectionTimeout),[&]()->bool{return !connectionQue_.empty();})==false)
    {
        //当连接池关闭时通知用户
        if(isPoolExit_)     std::cout<<"connectionpool is closed\n";
        //当等待一阵之后还是没有得到连接，需要通知生产者生产一下连接了
        else    std::cerr<<"no free connection in connectionpool now, try later\n";
        return nullptr;
    }
    //若有连接则取走队首的连接，并且自定义deletor，在智能指针析构时候返回取走的连接给队列
    std::shared_ptr<Connection> sp(connectionQue_.front(),[&](Connection *conn){
        std::unique_lock<std::mutex> ulock(connectionQueMux_);
        connectionQue_.push(conn);
    });
    connectionQue_.pop();
    //当取走一个连接之后队列空了，则要通知生产者新生产连接
    if(connectionQue_.empty())  EmptyCv_.notify_all();
    //通知监控线程要刷新等待空闲时间
    notFreeCv_.notify_all();
    return sp;
}

//监控是否连接太长时间空闲，若如此则释放
void ConnectionPool::scannerConnectionFunc()
{
    while (1)
    {
        {
            std::unique_lock<std::mutex> ulock(connectionQueMux_);
            if(notFreeCv_.wait_for(ulock,std::chrono::milliseconds(_maxIdleTime),[&]()->bool{return isPoolExit_;})==false)
            {
                std::unique_lock<std::mutex> ulock(connectionQueMux_);
                for(int i=0;i<connectionQue_.size()-connectionInitNum_;i++)
                {
                    Connection *p=connectionQue_.front();
                    delete p;
                    connectionQue_.pop();
                    connectionCurNum_--;
                }
            }
            //当连接池关闭时退出线程
            if(isPoolExit_)     return;
        }
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
}