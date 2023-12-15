#include<iostream>
#include<mysql.h>
#include<assert.h>

#include "connectionpool.h"

using namespace std;

int main()
{
    // int ret;
    // Connection mySql;
    // ret=mySql.connect("192.168.58.128",0,"yyt","yyt","test");
    // assert(ret!=0);
    // ret=mySql.update("update  account set money=1500 where id=1;");
    // assert(ret!=0);
    // MYSQL_RES *res=mySql.query("select * from account;");
    // if(res!=nullptr)
    // {
    //     int num_fields=mysql_num_fields(res);
    //     MYSQL_FIELD *feilds=mysql_fetch_fields(res);
    //     for(int i=0;i<num_fields;i++)
    //     {
    //         cout<<feilds[i].name<<"\t";
    //     }
    //     cout<<endl;
    //     MYSQL_ROW row;
    //     while ((row=mysql_fetch_row(res))!=NULL)
    //     {
    //         for(int i=0;i<num_fields;i++)
    //         {
    //             if(row[i]==NULL)     cout<<"NULL\t";
    //             else    cout<<row[i]<<"\t";
    //         }
    //         cout<<endl;
    //     } 
    //     mysql_free_result(res);
    // }
    {
        shared_ptr<ConnectionPool> connectionpool=ConnectionPool::getConnectpoolInstance();
        connectionpool->start();
        {
            shared_ptr<Connection> conn = connectionpool->getConnection();
            MYSQL_RES *res=conn->query("select * from account;");
            if(res!=nullptr)
            {
                int num_fields=mysql_num_fields(res);
                MYSQL_FIELD *feilds=mysql_fetch_fields(res);
                for(int i=0;i<num_fields;i++)
                {
                    cout<<feilds[i].name<<"\t";
                }
                cout<<endl;
                MYSQL_ROW row;
                while ((row=mysql_fetch_row(res))!=NULL)
                {
                    for(int i=0;i<num_fields;i++)
                    {
                        if(row[i]==NULL)     cout<<"NULL\t";
                        else    cout<<row[i]<<"\t";
                    }
                    cout<<endl;
                } 
                mysql_free_result(res);
            }
        }
        connectionpool->stop();
    }
    return 0;
}
