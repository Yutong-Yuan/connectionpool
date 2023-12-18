#ifndef CONNECTIONPOOL_H
#define CONNECTIONPOOL_H

#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <memory>
#include <functional>
#include <mysql.h>

//用户可选的设置参数
//连接的主机IP
const std::string _host="127.0.0.1";
//连接的端口号（默认为0）
const int _port=3306;
//以什么用户连接
const std::string _user="yyt";
//用户连接的密码
const std::string _passwd="yyt";
//要连接的数据库
const std::string _database="test";
//连接池的初始连接量
const int _connectionInitNum=10; 
//连接池的最大连接量
const int _connectionMaxNum=512; 
//连接池最大空闲时间(ms)
const int _maxIdleTime=60000; 
//连接池获取连接的超时时间(ms)
const int _connectionTimeout=1000; 

//～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～数据库连接类～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～
class Connection
{
public:
    Connection();
    ~Connection();
    //连接数据库
    bool connect(const std::string host, int port ,const std::string user, const std::string passwd, std::string database);
    //增删改
    bool update(std::string sql);
    //查
    MYSQL_RES * query(std::string sql);

private:
    MYSQL *conn_;
};


//～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～数据库连接池类～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～～
class ConnectionPool
{
public:
    //对外公开获取数据库连接池的接口
    static std::shared_ptr<ConnectionPool> getConnectpoolInstance();
    //对外公开从连接池中获取一个可用的空闲连接的接口
	std::shared_ptr<Connection> getConnection();
    //开启数据库连接池
    void start(
        const std::string host=_host, int port=_port ,const std::string user=_user, const std::string passwd=_passwd, std::string database=_database,
        const int &connectionInitNum=_connectionInitNum,const int &connectionMaxNum=_connectionMaxNum,
        const int &maxIdleTime=_maxIdleTime,const int &connectionTimeout=_connectionTimeout);
    //主动关闭数据库连接池
    void stop();
    //数据库连接池析构函数
    ~ConnectionPool();
private:
    static std::shared_ptr<ConnectionPool> connectpool;
    ConnectionPool();
    //单例模式删除复制构造
    ConnectionPool(const Connection &)=delete;

    // 运行在独立的线程中，专门负责生产新连接
	void produceConnectionFunc();
    //扫描超过最大空闲时间的连接，将其回收
    void scannerConnectionFunc();

    //连接的主机IP
    std::string host_;
    //连接的端口号（默认为0）
    int port_;
    //以什么用户连接
    std::string user_;
    //用户连接的密码
    std::string passwd_;
    //要连接的数据库
    std::string database_;
    //连接池的初始连接量
    int connectionInitNum_; 
    //连接池的最大连接量
	int connectionMaxNum_; 
    //连接池最大空闲时间
	int maxIdleTime_; 
    //连接池获取连接的超时时间
	int connectionTimeout_; 

    //连接队列
    std::queue<Connection*> connectionQue_;
    //互斥访问连接队列的锁
    std::mutex connectionQueMux_;
    //现在连接池中可支持连接的总数
    std::atomic_int connectionCurNum_;
    //当前连接队列不空条件变量
    std::condition_variable EmptyCv_;
    //当前连接队列为空条件变量
    std::condition_variable notEmptyCv_;
    //设置连接池不空闲的条件变量
    public:
    std::condition_variable notFreeCv_;

    //设置线程池关闭的表示（不然会死锁）
    bool isPoolExit_;
    //记录stop的次数，防止stop过多影响效率
    int exitFlags_;

};

#endif