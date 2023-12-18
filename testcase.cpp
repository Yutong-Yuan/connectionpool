#include<iostream>
#include<mysql.h>
#include<assert.h>
#include<atomic>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "connectionpool.h"

using namespace std;

atomic_int num=0;
int flags=0;

void func(shared_ptr<ConnectionPool> connectionpool,FILE *fp)
{   
    if(flags==0)
    {
        //应用数据库连接池
        for(int i=0;i<2500;i++)
        {
            num++;
            shared_ptr<Connection> conn = connectionpool->getConnection();
            MYSQL_RES *res=conn->query("select * from account;");
            if(res!=nullptr)
            {
                int num_fields=mysql_num_fields(res);
                MYSQL_FIELD *feilds=mysql_fetch_fields(res);
                for(int i=0;i<num_fields;i++)
                {
                    string mes=string(feilds[i].name)+"\t";
                    fprintf(fp,"%s",mes.c_str());
                }
                fprintf(fp,"%s","\n");
                MYSQL_ROW row;
                while ((row=mysql_fetch_row(res))!=NULL)
                {
                    for(int i=0;i<num_fields;i++)
                    {
                        string mes;
                        if(row[i]==NULL)     mes="NULL\t";
                        else    mes=string(row[i])+"\t";
                        fprintf(fp,"%s",mes.c_str());
                    }
                    fprintf(fp,"%s","\n");
                } 
                mysql_free_result(res);
            }
        }
    }
    
    else
    {
        //不应用数据库连接池
        for(int i=0;i<2500;i++)
        {
            num++;
            //Connection *conn=new Connection();
            unique_ptr<Connection> conn(new Connection());
            //输入自己的连接属性
            while (conn->connect("127.0.0.1",3306,"yyt","yyt","test")!=false)
            {
                this_thread::sleep_for(10ms);
            }
            MYSQL_RES *res=conn->query("select * from account;");
            if(res!=nullptr)
            {
                int num_fields=mysql_num_fields(res);
                MYSQL_FIELD *feilds=mysql_fetch_fields(res);
                for(int i=0;i<num_fields;i++)
                {
                    string mes=string(feilds[i].name)+"\t";
                    fprintf(fp,"%s",mes.c_str());
                }
                fprintf(fp,"%s","\n");
                MYSQL_ROW row;
                while ((row=mysql_fetch_row(res))!=NULL)
                {
                    for(int i=0;i<num_fields;i++)
                    {
                        string mes;
                        if(row[i]==NULL)     mes="NULL\t";
                        else    mes=string(row[i])+"\t";
                        fprintf(fp,"%s",mes.c_str());
                    }
                    fprintf(fp,"%s","\n");
                } 
                mysql_free_result(res);
            }
        }
    }

    
}


int main(int argc,char **argv)
{
    FILE *fp=fopen("./log","w");
    //当flags为0的时候才应用数据库连接池 不为零的时候不启用（起到对比实验的作用）
    if(argc>1)  flags=atoi(argv[1]);
    shared_ptr<ConnectionPool> connectionpool=ConnectionPool::getConnectpoolInstance();
    if(flags==0)
    {
        //利用start方法来设置数据库连接池的属性（或者直接修改connectionpool.h来用文档的形式修改，这样就不用在start中给属性）
        connectionpool->start("127.0.0.1",3306,"yyt","yyt","test",5,1024,60000,1000);
    }
    thread th1(&func,connectionpool,fp);
    th1.detach();
    thread th2(&func,connectionpool,fp);
    th2.detach();
    thread th3(&func,connectionpool,fp);
    th3.detach();
    thread th4(&func,connectionpool,fp);
    th4.detach();
    thread th5(&func,connectionpool,fp);
    th5.detach();
    thread th6(&func,connectionpool,fp);
    th6.detach();
    thread th7(&func,connectionpool,fp);
    th7.detach();
    thread th8(&func,connectionpool,fp);
    th8.detach();
    while (num<20000)
    {
    }
    this_thread::sleep_for(10ms);
    fclose(fp);
    return 0;
}



