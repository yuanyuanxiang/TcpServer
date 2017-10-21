# TcpServer
一个解析二维码的TCP服务。

# About
该项目是一个解析二维码的服务程序。此程序监听指定端口，客户端向此端口发送二维码的URL，服务器对此URL进行解析，然后将解码结果回复给客户端。

# Reference
ThreadpoolLib：https://github.com/yuanyuanxiang/ThreadpoolLib/

# Tips
为了程序能够编译通过，请新建目录，并将3DCode项目中的libqrdecode.lib复制到相关目录，将ThreadpoolLib复制到ThreadpoolLib目录。
