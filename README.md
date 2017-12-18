# mychatroom
bin文件夹下包含 serv 、cli两个可执行文件分别对应服务器与客户端 用命令行运行时若不带参数 默认使用55555端口 运行服务器文件时可指定使用的端口号 运行客户端文件时可指定服务器端口号 亦可将服务器ip与端口一同指定
code文件夹下存放源码 编译服务器时 将serv.c与drawer.c一同编辑 例如：gcc serv.c drawer.c -o serv 编译客户端时需引入pthread库 gcc cli.c -o cli -lpthread