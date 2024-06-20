# orange_car
基于香橙派5b和树莓派4b的使用yolofastest v2的实时人体跟随小车
存在巡线，目标跟踪，远程桌面控制的功能

环境需求 ncnn opencv wriningpi yolofastestv2
本车采用yolofastest v2进行目标检测，drv8833作为电机驱动

使用方式 终端中运行 sh build.sh 对代码进行编译
./demo 运行代码

特别感谢马雪浩大佬的哟咯fastest v2目标检测算法
其中树莓派的车模为亚博智能4wd小车
香橙派小车是我自己搭建的


注：由于树莓派4b的性能问题，要提高实时性，应该编写多线程获取摄像头的代码
