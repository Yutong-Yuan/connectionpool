c++11 数据库连接池  
概览：本程序基于c++11构建线程池，用到了c++11中的新特性，如thread并发库，智能指针，lambda表达式等等  
  
环境：Ubuntu 22.04.3 g++ 11.4.0（请确保g++版本在4.8.5之上，支持c++11） 库环境中要包含mysql-client库  
  
测试案例：  
（1）编译：g++ testcase.cpp connectionpool.cpp -o testcase -I/usr/include/mysql/ -lmysqlclient  
（2）./testcase 0（运行利用数据库连接池来连接访问数据库） or ./testcase 1（没有利用数据库连接处来连接访问数据库，可用做对比实验）  
  
ps：具体数据库连接池访问的步骤请参考testcase.cpp中的步骤（1）-（4），且由于环境不同，若需动态库则自行编译  
  
本机对比结果（效果显著）：
<img width="1347" alt="image" src="https://github.com/Yutong-Yuan/connectionpool/assets/67001644/cfd1da20-6264-426e-9051-83468c932bae">


