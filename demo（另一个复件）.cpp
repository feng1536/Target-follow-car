#include "yolo-fastestv2.h"
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

int main()
{   
    static const char* class_names[] = {
        "person"/*, "bicycle", "car", "motorcycle", "airplane", "bus", "train", "truck", "boat", "traffic light",
        "fire hydrant", "stop sign", "parking meter", "bench", "bird", "cat", "dog", "horse", "sheep", "cow",
        "elephant", "bear", "zebra", "giraffe", "backpack", "umbrella", "handbag", "tie", "suitcase", "frisbee",
        "skis", "snowboard", "sports ball", "kite", "baseball bat", "baseball glove", "skateboard", "surfboard",
        "tennis racket", "bottle", "wine glass", "cup", "fork", "knife", "spoon", "bowl", "banana", "apple",
        "sandwich", "orange", "broccoli", "carrot", "hot dog", "pizza", "donut", "cake", "chair", "couch",
        "potted plant", "bed", "dining table", "toilet", "tv", "laptop", "mouse", "remote", "keyboard", "cell phone",
        "microwave", "oven", "toaster", "sink", "refrigerator", "book", "clock", "vase", "scissors", "teddy bear",
        "hair drier", "toothbrush"*/
    };
    
    yoloFastestv2 api;

    api.loadModel("./model/yolo-fastestv2-opt.param",
                  "./model/yolo-fastestv2-opt.bin");

    cv::VideoCapture cap(0);
    cap.set(cv::CAP_PROP_FRAME_WIDTH,320);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT,240);
    
    // 检查摄像头是否打开
    if (!cap.isOpened())
    {
        std::cout << "Error opening video stream" << std::endl;
        return -1;
    }
    //cv::namedWindow("Frame", cv::WINDOW_AUTOSIZE); 
    
     while (1)
    {
        //motor_write(0,0);
        cv::Mat cvImg; // 创建一个Mat对象来存储摄像头的图像

        cap >> cvImg; // 从摄像头获取图像

        std::vector<TargetBox> boxes; // 创建一个用于存储检测结果的向量
        
        // 运行模型进行检测，并将结果存储在boxes中
        api.detection(cvImg, boxes);
        if(boxes.size()>0&&boxes[0].cate == 0){
        
        // 遍历所有的检测结果
        for (int i = 0; i < 1/*boxes.size()*/; i++) {
            // 打印每个检测框的坐标、得分和类别
            
            std::cout<<boxes[i].x1<<" "<<boxes[i].y1<<" "<<boxes[i].x2<<" "<<boxes[i].y2
                     <<" "<<boxes[i].score<<" "<<boxes[i].cate<<std::endl;
            
            char text[256];
            // 格式化检测结果的文本
            sprintf(text, "%s %.1f%%", class_names[boxes[i].cate], boxes[i].score * 100);

            int baseLine = 0;
            // 获取文本的大小
            cv::Size label_size = cv::getTextSize(text, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);

            int x = boxes[i].x1;
            int y = boxes[i].y1 - label_size.height - baseLine;
            if (y < 0)
                y = 0;
            if (x + label_size.width > cvImg.cols)
                x = cvImg.cols - label_size.width;
	/*
            // 在图像上画出文本的背景框
            cv::rectangle(cvImg, cv::Rect(cv::Point(x, y), cv::Size(label_size.width, label_size.height + baseLine)),
                          cv::Scalar(255, 255, 255), -1);

            // 在图像上画出文本
            cv::putText(cvImg, text, cv::Point(x, y + label_size.height),
                        cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0));
            
            // 在图像上画出检测框
            cv::rectangle (cvImg, cv::Point(boxes[i].x1, boxes[i].y1), 
                           cv::Point(boxes[i].x2, boxes[i].y2), cv::Scalar(255, 255, 0), 2, 2, 0);
        */
        
            int height = boxes[0].y2-boxes[0].y1,width= boxes[0].x2-boxes[0].x1,
                people_x=(boxes[0].x1+boxes[0].x2)/2 ,people_y=(boxes[0].y1+boxes[0].y2)/2;
                
                
        }
    }
        
        // 在窗口中显示图像
       // cv::imshow("Frame", cvImg);
        
        // 按下ESC键退出
        //if(cv::waitKey(0) >= 0)
        //   break;

        
    }


    return 0;
}
