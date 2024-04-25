#include "yolo-fastestv2.h"
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <stdio.h>
#include <wiringPi.h>
#include <softPwm.h>

#define cap_half_width 320  // 摄像头半宽
#define cap_half_height 240 // 摄像头半高
#define purpose 500         // 期望面积
#define decline 0.4

/*电机引脚定义*/
#define PWM_Left_motor 16
#define PWM_Right_motor 10
#define DIR_Left_motor 4
#define DIR_Right_motor 3

/*速度范围*/
#define Max_duty 40
#define Min_duty 0
#define turn_compensate 9

/*pid*/
float PID_Motor_Turn[3] = {0.01, 0.0, 0.005}; // p i d
float PID_Motor_Run[3] = {0.61, 0, 2};        // p i d

void GPIO_Init(void);
void motor_write(int motor_speed_R, int motor_speed_L);
void chases(int x_err, int square_err);
void motor_stop(void);

int main()
{
    static const char *class_names[] = {
        "person" , "bicycle", "car", "motorcycle", "airplane", "bus", "train", "truck", "boat", "traffic light",
         "fire hydrant", "stop sign", "parking meter", "bench", "bird", "cat", "dog", "horse", "sheep", "cow",
         "elephant", "bear", "zebra", "giraffe", "backpack", "umbrella", "handbag", "tie", "suitcase", "frisbee",
         "skis", "snowboard", "sports ball", "kite", "baseball bat", "baseball glove", "skateboard", "surfboard",
         "tennis racket", "bottle", "wine glass", "cup", "fork", "knife", "spoon", "bowl", "banana", "apple",
         "sandwich", "orange", "broccoli", "carrot", "hot dog", "pizza", "donut", "cake", "chair", "couch",
         "potted plant", "bed", "dining table", "toilet", "tv", "laptop", "mouse", "remote", "keyboard", "cell phone",
         "microwave", "oven", "toaster", "sink", "refrigerator", "book", "clock", "vase", "scissors", "teddy bear",
         "hair drier", "toothbrush"
    };

    GPIO_Init();

    yoloFastestv2 api;

    api.loadModel("./model/yolo-fastestv2-opt.param",
                  "./model/yolo-fastestv2-opt.bin");

    cv::VideoCapture cap(0);
    cap.set(cv::CAP_PROP_FRAME_WIDTH, cap_half_width * 2);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, cap_half_height * 2);

    // 检查摄像头是否打开
    if (!cap.isOpened())
    {
        std::cout << "Error opening video stream" << std::endl;
        return -1;
    }
    // cv::namedWindow("Frame", cv::WINDOW_AUTOSIZE);

    int mode = 0, y_err = 0, x_err = 0, square_err = 0, i = 0,j =0;

mode_select: // 选择模式的位标

    /*选择模式*/
    printf("Please select mode:\r\n1:Orange_PI_tracking\r\n2:computer_control\r\n3:exit\r\n");
    scanf("%d", &mode);
    if (mode == 2)  /*手动控制*/
    {
        while (1)
        {
            // motor_write(0,0);
            cv::Mat cvImg; // 创建一个Mat对象来存储摄像头的图像

            cap >> cvImg; // 从摄像头获取图像

            std::vector<TargetBox> boxes; // 创建一个用于存储检测结果的向量

            // 运行模型进行检测，并将结果存储在boxes中
            api.detection(cvImg, boxes);
            // 遍历所有的检测结果
            for (i = 0; i < boxes.size() && boxes[i].cate != 0; i++)
            {
                // 打印每个检测框的坐标、得分和类别:
                /*
                std::cout<<boxes[i].x1<<" "<<boxes[i].y1<<" "<<boxes[i].x2<<" "<<boxes[i].y2
                         <<" "<<boxes[i].score<<" "<<boxes[i].cate<<std::endl;
                */
                ;
            }
            if (i < boxes.size())
            {
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

                // 在图像上画出文本的背景框
                cv::rectangle(cvImg, cv::Rect(cv::Point(x, y), cv::Size(label_size.width, label_size.height + baseLine)),
                              cv::Scalar(255, 255, 255), -1);

                // 在图像上画出文本
                cv::putText(cvImg, text, cv::Point(x, y + label_size.height),
                            cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0));

                // 在图像上画出检测框
                cv::rectangle(cvImg, cv::Point(boxes[i].x1, boxes[i].y1),
                              cv::Point(boxes[i].x2, boxes[i].y2), cv::Scalar(255, 255, 0), 2, 2, 0);

            }

            // 在窗口中显示图像
            cv::imshow("Frame", cvImg);
            
            int key_get ;
            //if (cv::waitKey(5) >= 0)
            key_get = cv::waitKey(5);
            key_get = key_get % 255;
            switch(key_get)
            {
                case 82://up
                    j = 0;
                    motor_write(20,20);
                    break;
                case 84://down
                    j = 0;
                    motor_write(-20,-20);
                    break;
                case 81://left
                    j = 0;
                    motor_write(20,-20);
                    break;
                case 83://right
                    j = 0;
                    motor_write(-20,20);
                    break;
                case 27://ESC退出
                    break;
                
            }
            if(key_get == 27) break;
            if(j++ >=8){
                motor_stop();
                j = 0;
            }
        }

        motor_stop();
        goto mode_select;
    }
    
    else if (mode == 3)/*退出*/
    {
        return 0;
    }

    else if (mode == 1)/*自动跟踪*/
    {
        while (1)
        {
            // motor_write(0,0);
            cv::Mat cvImg; // 创建一个Mat对象来存储摄像头的图像

            cap >> cvImg; // 从摄像头获取图像

            std::vector<TargetBox> boxes; // 创建一个用于存储检测结果的向量

            // 运行模型进行检测，并将结果存储在boxes中
            api.detection(cvImg, boxes);
            // 遍历所有的检测结果
            for (i = 0; i < boxes.size() && boxes[i].cate != 0; i++)
            {
                // 打印每个检测框的坐标、得分和类别:
                /*
                std::cout<<boxes[i].x1<<" "<<boxes[i].y1<<" "<<boxes[i].x2<<" "<<boxes[i].y2
                         <<" "<<boxes[i].score<<" "<<boxes[i].cate<<std::endl;
                */
                ;
            }
            if (i < boxes.size())
            {
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

                // 在图像上画出文本的背景框
                cv::rectangle(cvImg, cv::Rect(cv::Point(x, y), cv::Size(label_size.width, label_size.height + baseLine)),
                              cv::Scalar(255, 255, 255), -1);

                // 在图像上画出文本
                cv::putText(cvImg, text, cv::Point(x, y + label_size.height),
                            cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0));

                // 在图像上画出检测框
                cv::rectangle(cvImg, cv::Point(boxes[i].x1, boxes[i].y1),
                              cv::Point(boxes[i].x2, boxes[i].y2), cv::Scalar(255, 255, 0), 2, 2, 0);

                int height = boxes[i].y2 - boxes[i].y1, width = boxes[i].x2 - boxes[i].x1,
                    people_x = (boxes[i].x1 + boxes[i].x2) / 2, people_y = (boxes[i].y1 + boxes[i].y2) / 2;

                square_err = purpose - (height + width);
                x_err = people_x - cap_half_width;
                y_err = people_y - cap_half_height;
                printf("squire_err:%d x_err:%d squire:%d ", square_err, x_err, height * width);
                chases(x_err, square_err);
            }

            else
            {
                motor_stop();
            }

            // serialPrintf(serial_fd,"X_:%d",x_err);

            // 在窗口中显示图像
            cv::imshow("Frame", cvImg);

            // 按下ESC键退出
            if (cv::waitKey(5) >= 0)
                break;
        }

        motor_stop();
        goto mode_select;
    }
    else
    {
        printf("error mode\n");
        goto mode_select;
    }
}

void GPIO_Init(void)
{
    wiringPiSetup();

    /*方向引脚初始化*/
    pinMode(DIR_Left_motor, OUTPUT);
    pinMode(DIR_Right_motor, OUTPUT);

    /*PWM引脚初始化*/
    softPwmCreate(PWM_Left_motor, 0, 100);
    softPwmCreate(PWM_Right_motor, 0, 100);
    digitalWrite(DIR_Right_motor, LOW);
    digitalWrite(DIR_Left_motor, LOW);
}

/*方向引脚 0前 1后*/
void motor_write(int motor_speed_R, int motor_speed_L)
{
    int motor_direct_R, motor_direct_L;

    /*speed limit*/
    if (motor_speed_R > Max_duty)
    {
        motor_speed_R = Max_duty;
    }
    else if (motor_speed_R < -Max_duty)
    {
        motor_speed_R = -Max_duty;
    }
    if (motor_speed_L > Max_duty)
    {
        motor_speed_L = Max_duty;
    }
    else if (motor_speed_L < -Max_duty)
    {
        motor_speed_L = -Max_duty;
    }

    if (motor_speed_R >= 0)
    {
        motor_direct_R = LOW;
    }
    else
    {
        motor_direct_R = HIGH;
        motor_speed_R = 100 + motor_speed_R;
    }
    if (motor_speed_L >= 0)
    {
        motor_direct_L = LOW;
    }
    else
    {
        motor_direct_L = HIGH;
        motor_speed_L = 100 + motor_speed_L;
    }

    digitalWrite(DIR_Right_motor, motor_direct_R);
    digitalWrite(DIR_Left_motor, motor_direct_L);

    softPwmWrite(PWM_Right_motor, motor_speed_R);
    softPwmWrite(PWM_Left_motor, motor_speed_L);
}

void chases(int x_err, int square_err)
{
    int motor_speed_L = 0, motor_speed_R = 0;
    static int last_square_err = 0, square_division = 0;
    static int last_x = 0, x_division = 0, x_intergrate = 0;

    x_division = x_err - last_x;
    float trun_value = (PID_Motor_Turn[0] * x_err +
                        PID_Motor_Turn[1] * x_intergrate +
                        PID_Motor_Turn[2] * x_division);
    x_intergrate += x_err;
    last_x = x_err;

    if (x_err > cap_half_width / 2 || x_err < -cap_half_width / 2)
    {
        motor_write(-turn_compensate * trun_value, turn_compensate * trun_value);
        printf("turn:%f \n", trun_value);
    }
    else
    {
        square_division = square_err - last_square_err;
        motor_speed_L = (int)(decline *
                              (PID_Motor_Run[0] * square_err +
                               PID_Motor_Run[2] * square_division));
        motor_speed_R = motor_speed_L;
        last_square_err = square_err;
        printf("speed:%d", motor_speed_L);

        motor_speed_R = motor_speed_R - (int)trun_value;
        motor_speed_L = motor_speed_L + (int)trun_value;

        printf("R: %d  L: %d turn:%f \n", motor_speed_R, motor_speed_L, trun_value);
        motor_write(motor_speed_R, motor_speed_L);
    }
}

void motor_stop(void)
{
    softPwmWrite(PWM_Right_motor, 0);
    softPwmWrite(PWM_Left_motor, 0);
    digitalWrite(DIR_Right_motor, LOW);
    digitalWrite(DIR_Left_motor, LOW);
}
