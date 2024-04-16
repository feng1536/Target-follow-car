#g++ -g -o demo demo.cpp src/yolo-fastestv2.cpp -I src/include -I include/ncnn lib/libncnn.a `pkg-config --libs --cflags opencv` -fopenmp

#g++ -g -o 123456 123456.cpp src/yolo-fastestv2.cpp -I src/include -I include/ncnn lib/libncnn.a `pkg-config --libs --cflags opencv4` -fopenmp

#g++ -g -o demo demo.cpp src/yolo-fastestv2.cpp -I src/include -I include/ncnn lib/libncnn.a -lopencv_core -lopencv_highgui `pkg-config --libs --cflags opencv4` -fopenmp

g++ -g -o demo demo.cpp src/yolo-fastestv2.cpp -I src/include -I include/ncnn lib/libncnn.a -lopencv_core -lopencv_highgui -lwiringPi `pkg-config --libs --cflags opencv4` -fopenmp
