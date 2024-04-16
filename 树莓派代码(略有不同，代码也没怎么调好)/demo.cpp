#include "yolo-fastestv2.h"
#include <opencv2/opencv.hpp>
#include <wiringPi.h>
#include <softPwm.h>

#define speed_pwm_max       20
#define speed_pwm_min       10
#define Center_X            200     //相机图像高度一半
#define Center_Y            150     //相机图像宽度一半
#define Servo_threshold_max  Center_X*3/2
#define Servo_threshold_min  Center_Y*3/2


void print_debug(void);
void GPIO_Init(void);
void motor_write(int motor_speed_R,int motor_speed_L);
void Servo_Write(int PWM_Pulse);
int Servo_Duty_Get(int y);
void chases(int width,int height,int x,int y);

int debugger[5];

float PID_Motor_Turn[3] = {0.4 ,0.0 ,0 }; //p i d
float PID_Motor_Run[3] = {0.3 ,0 ,1 };  //p i d
//float PID_Servo[3] = {0.5 ,0 ,0 };      //p i d

//定义引脚
int Left_motor_go = 28;       //左电机前进AIN2连接Raspberry的wiringPi编码28口
int Left_motor_back = 29;     //左电机后退AIN1连接Raspberry的wiringPi编码29口

int Right_motor_go = 24;      //右电机前进BIN2连接Raspberry的wiringPi编码24口
int Right_motor_back = 25;    //右电机后退BIN1连接Raspberry的wiringPi编码25口

int Left_motor_pwm = 27;      //左电机控速PWMA连接Raspberry的wiringPi编码27口
int Right_motor_pwm = 23;     //右电机控速PWMB连接Raspberry的wiringPi编码23口

int Servo_Y = 13;

int main()
{
    
    int mode_select;
    GPIO_Init();

    // 定义一个只包含"person"类别的数组
    static const char* class_names[] = {
        "person"
    };
    
    yoloFastestv2 api; // 创建YOLO模型的实例

    // 加载模型参数和权重
    api.loadModel("./model/yolo-fastestv2-opt.param",
                  "./model/yolo-fastestv2-opt.bin");
                  
    // 打开摄像头
    cv::VideoCapture cap(0);
    cap.set(CV_CAP_PROP_FRAME_WIDTH,Center_X*2);
    cap.set(CV_CAP_PROP_FRAME_HEIGHT,Center_Y*2);

    // 检查摄像头是否打开
    if (!cap.isOpened())
    {
        std::cout << "Error opening video stream" << std::endl;
        return -1;
    }

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
            if(mode_select ==1){
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

            // 在图像上画出文本的背景框
            cv::rectangle(cvImg, cv::Rect(cv::Point(x, y), cv::Size(label_size.width, label_size.height + baseLine)),
                          cv::Scalar(255, 255, 255), -1);

            // 在图像上画出文本
            cv::putText(cvImg, text, cv::Point(x, y + label_size.height),
                        cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0));
            }
            // 在图像上画出检测框
            cv::rectangle (cvImg, cv::Point(boxes[i].x1, boxes[i].y1), 
                           cv::Point(boxes[i].x2, boxes[i].y2), cv::Scalar(255, 255, 0), 2, 2, 0);
        
            int height = boxes[0].y2-boxes[0].y1,width= boxes[0].x2-boxes[0].x1,
                people_x=(boxes[0].x1+boxes[0].x2)/2 ,people_y=(boxes[0].y1+boxes[0].y2)/2;
                
                //printf("%d      %d      %d\n",height,people_x,people_y);
                chases(width,height,people_x,people_y);
                
        }
    }
        
        // 在窗口中显示图像
        cv::imshow("Frame", cvImg);
        
        // 按下ESC键退出
        char c = (char)cv::waitKey(5);
        if (c == 27){
            motor_write(0,0);
            return 0;
        }
    }

    return 0; // 程序正常结束
}

void print_debug(void)
{
    static int a =0;
    printf("%d\n",a);
    a++;
}

void GPIO_Init(void)
{
    //wiringPi初始化
    wiringPiSetup();
  
    //初始化电机驱动IO口为输出方式
    pinMode(Left_motor_go, OUTPUT);
    pinMode(Left_motor_back, OUTPUT);
    pinMode(Right_motor_go, OUTPUT);
    pinMode(Right_motor_back, OUTPUT);

    //创建两个软件控制的PWM脚
    //int softPwmCreate(int pin,int initialValue,int pwmRange);
    softPwmCreate(Left_motor_pwm,0,100); 
    softPwmCreate(Right_motor_pwm,0,100);
    //softPwmCreate(Servo_Y,0,200);
    
    
    //pwmSetMode(PWM_MODE_MS);//设置pwm的输出模式
    //pwmSetRange(2000);//2000設置步長 就是一個周期2000步
    //pwmSetClock(192);//设置分频
    
    return;
}

void motor_write(   int motor_speed_R , int motor_speed_L)
{
    debugger[0]=motor_speed_R;
    debugger[1]=motor_speed_L;
    int motor_direct_R_F,motor_direct_R_B,motor_direct_L_F,motor_direct_L_B;
    if(motor_speed_R >= 0){
        motor_direct_R_F = HIGH;
        motor_direct_R_B = LOW;
    }
    else if(motor_speed_R < 0){
        motor_speed_R = -motor_speed_R;
        motor_direct_R_F = LOW; 
        motor_direct_R_B = HIGH;
    }
    if(motor_speed_L >= 0){
        motor_direct_L_F = HIGH;
        motor_direct_L_B = LOW;
    }
    else if(motor_speed_L < 0){
        motor_speed_L = -motor_speed_L;
        motor_direct_L_F = LOW;
        motor_direct_L_B = HIGH;
    }
    
    /*speed limit*/
    if(motor_speed_R > speed_pwm_max){
        motor_speed_R = speed_pwm_max;
        motor_speed_L = speed_pwm_max;
    }
    else if(motor_speed_R < speed_pwm_min){
        motor_speed_R = speed_pwm_min;
        motor_speed_L = speed_pwm_min;
    }
    
    //右电机
    digitalWrite(Right_motor_go, motor_direct_R_F);  //右电机使能
    digitalWrite(Right_motor_back, motor_direct_R_B); 
    softPwmWrite(Right_motor_pwm, motor_speed_R);  //PWM比例0-100调速，左右轮差异略增减
    
    //左电机
    digitalWrite(Left_motor_go, motor_direct_L_F);   //左电机使能
    digitalWrite(Left_motor_back, motor_direct_L_B);  
    softPwmWrite(Left_motor_pwm, motor_speed_L);   //PWM比例0-100调速，左右轮差异略增减

    //延时
    //delay(time * 100);
        
    return;
}

void Servo_Write(int PWM_Pulse)
{
    pwmWrite(Servo_Y, PWM_Pulse); 
    return;
}

int Servo_Duty_Get(int y)
{
    static int y_last= 0 ,Angle_step=15;
    if(y>Servo_threshold_max &&y_last<Servo_threshold_max){
        Angle_step+=5;
    }
    else   if(y<Servo_threshold_min &&y_last>Servo_threshold_min){
        Angle_step-=5;
    }
    
    if(Angle_step>200){
        Angle_step = 200;
    }
    else     if(Angle_step<100){
    Angle_step = 100;
    }
    y_last=y;
    return Angle_step;
}

void chases(int width,int height,int x,int y)
{
    int motor_speed_L=0 , motor_speed_R=0 ;
    static int last_height=0,height_division=0,height_intergrate=0;
    static int last_x=0,x_division=0,x_intergrate=0;
    
    height=height-Center_Y;         // the num over half height
    height_division = height - last_height;
    motor_speed_L =(int)(   PID_Motor_Run[0]*height +
                            PID_Motor_Run[1]*height_intergrate + 
                            PID_Motor_Run[2]*height_division);
    motor_speed_R = motor_speed_L;  
    height_intergrate += height;
    
    motor_speed_L = -motor_speed_L;
    motor_speed_R = -motor_speed_R;
    
    /*  speed num  */
    
    x = x - Center_X;
    x_division = x - last_x;
    int trun_value =(int)(  PID_Motor_Turn[0]*x + 
                            PID_Motor_Turn[1]*x_intergrate + 
                            PID_Motor_Turn[2]*x_division);
    x_intergrate += x;
    
    if(motor_speed_R >= 0 ||motor_speed_L>=0){
        motor_speed_R = motor_speed_R - trun_value;
        motor_speed_L = motor_speed_L + trun_value;
    }
     else if(motor_speed_R<0||motor_speed_L<0){
        motor_speed_L = motor_speed_L - trun_value;
        motor_speed_R = motor_speed_R + trun_value;
    }
    
    motor_write(motor_speed_R,motor_speed_L);
    //Servo_Write(Servo_Duty_Get(y));
    return;
}
