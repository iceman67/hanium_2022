#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <pthread.h>
#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include <queue>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>
#include <sys/timeb.h> 

#include "merkle_tree.h"
#include "../Client/client.cpp"             //Client
//#include "../src/sign_verify.cpp"       //FOR SIGN_VERIFY

using namespace std;
using namespace cv;

int width = 640;
int height = 480;
int FPS = 30;
unsigned int frame_count = 0;
int total_data_size;

cv::VideoCapture cap;
cv::Mat frame(cv::Size(width, height), CV_8UC3);

pthread_mutex_t frameLocker;
pthread_t UpdThread;  
 
queue<cv::Mat> bgr_queue;                       //for original frame(BGR)Mat queue
queue<cv::Mat> yuv420_queue;                    //for original frame(yuv)Mat queue
queue<cv::Mat> y_queue;                         //for y_frame Mat queue
queue<cv::Mat> feature_vector_queue;            //for edge detection Canny
queue<string> hash_queue;                       //for hash made by feature vector
queue<string> cid_queue;                        //for CID for images

int init();                                                 //Init Camera Setting and OPEN CAP
void init_all_setting();                                    //Init all settings at the end
void init_queue();                                          //Init all datas in queues
void *UpdateFrame(void *arg);                               //Update frames 
void capture();                                             //Capture frames;
void convert_frames(queue<cv::Mat> &BGR_QUEUE);             //Convert saved frames;
void show_frames_bgr(queue<cv::Mat> &BGR_QUEUE);            //show frames by bgr
void show_frames_yuv(queue<cv::Mat> &YUV420_QUEUE);         //show frames by yuv
void show_frames_y(queue<cv::Mat> &Y_QUEUE);                //show frames by y
void show_frames_feature_vector(queue<cv::Mat> &FV_QUEUE);  //show frames by feature vector
void edge_detection(queue<cv::Mat> &Y_QUEUE);               //Edge detact by y frames
void make_hash(queue<cv::Mat> &FV_QUEUE);                   //make hash using feature vector
string getCID();                                            //Make CID for each frames 
unsigned char* reshape_yuv(cv::Mat mat);                    //Reshape yuv420 row size to 1 and return char
void send_datas_to_server(queue<cv::Mat> &YUV420_QUEUE, queue<string> &HASH_QUEUE, queue<string> &CID_QUEUE);   //send datas to Server
int make_merkle_tree();

/*/NEED to make
* 1. MAKE PUBLIC KEY, PRIVATE KEY
* 2. SEND PUBLIC KEY to SERVER 
* 3. int camera_config_from_web()
* 4. Encrypt yuv420 data
* 5. Send CID to Server
* 6. Send yuv420 data to server
* 7. Send hash to server
* 8. Send yuv420 data to Web
* 9. Send hash to Web
*/


int init() {
    cout << "----Initalizing----------" << endl << endl;

    // open the default camera using default API
    int deviceID = 0;             // 0 = open default camera
    int apiID = cv::CAP_V4L2;      // use V4L2
    
    // open selected camera using selected API
    cap.open(deviceID, apiID);
    cap.set(cv::CAP_PROP_FRAME_WIDTH, width);
    cap.set(cv::CAP_PROP_FRAME_HEIGHT, height);

    cout << "    FPS: " << FPS << endl;
    cout << "    Frame Size: " << "WIDHT = " << width << "  HEIGHT = " << height << endl << endl;

    // check if we succeeded
    if (!cap.isOpened()) {
        cerr << "ERROR! Unable to open camera\n";
        return -1;
    }
    
    //--- GRAB AND WRITE LOOP
    if (!cap.isOpened()) {
        cerr << "ERROR! Unable to open camera\n";
        return -1;
    }
    
    cout << "----Initalized----------" <<endl;
    return 0;
}
 
void init_all_setting() {
    cap.release();
    width = 640;
    height = 480;
    FPS = 30;
    init_queue();
    
    cout << endl << "----Initializing all settings." << endl <<endl;
    cout << "    cap closed." <<endl;
    cout << "    bgr queue size: " << bgr_queue.size() << endl;
    cout << "    yuv420 queue size: " << yuv420_queue.size() << endl;
    cout << "    y_frame queue size: " << y_queue.size() << endl;
    cout << "    feature vector queue size: " << feature_vector_queue.size() << endl;
    cout << "    hash queue size: " << hash_queue.size() << endl;
    cout << "    CID queue size: " << cid_queue.size() << endl;
    cout << endl << "--------------------------------" << endl << endl;
}

void init_queue() {         
    yuv420_queue = queue<cv::Mat>();                    //for original frame(yuv)Mat queue
    bgr_queue = queue<cv::Mat>();                    //for original frame(BGR)Mat queue
    y_queue = queue<cv::Mat>();                        //for y_frame Mat queue
    feature_vector_queue = queue<cv::Mat>();                  //for edge detection Canny
    hash_queue = queue<string>();          //for hash made by feature vector
    cid_queue = queue<string>();                          //for CID for frames
}


void *UpdateFrame(void *arg) {
    while(true) {
        cv::Mat tempFrame(cv::Size(width, height), CV_8UC3);
        cap >> tempFrame;
 
        pthread_mutex_lock(&frameLocker);
        frame = tempFrame;
        pthread_mutex_unlock(&frameLocker);
    }
 
    pthread_exit( (void *)0 );
}


void capture() {
    sleep(1);
    pthread_mutex_init(&frameLocker,NULL);  
    pthread_create(&UpdThread, NULL, UpdateFrame, NULL);
    
    cout << endl << "----Starting Capturing" << endl << endl;
    cout << "    press ESC if you want to END" << endl << endl;
    
    while(true){
        cv::Mat currentFrame(cv::Size(height, width), CV_8UC3);
        
        pthread_mutex_lock(&frameLocker);
        currentFrame = frame;
        pthread_mutex_unlock(&frameLocker);
        
        //cout << CID.back() << endl;
        bgr_queue.push(currentFrame);
        
        //Make CID for FRAMES
        string s_cid = getCID();
        //char *cid = const_cast<char*>(s_cid.c_str());
        
        cid_queue.push(s_cid);
        
        if(currentFrame.empty())
            continue;
        
        if (bgr_queue.size() == 50) {
 
            int ret = pthread_cancel( UpdThread );
            int status;
            cv::destroyAllWindows();
            break;
 
            if (ret == 0 )
            {
                //Fored termination
                ret = pthread_join( UpdThread, (void **)&status );
                if ( ret == 0 )
                    //if the thred Forced to terminate then -1 return
                    cout << "thread end successfully" << status << endl;
                else
                    cout << "thread end error" << ret << endl;
 
                break;
            }
        }
    
        
        //if ESC is pressed, then force thread to end
        if (cv::waitKey(20) == 27 ) 
        {
 
            int ret = pthread_cancel( UpdThread );
            int status;
            cv::destroyAllWindows();
            break;
 
            if (ret == 0 )
            {
                //Fored termination
                ret = pthread_join( UpdThread, (void **)&status );
                if ( ret == 0 )
                    //if the thred Forced to terminate then -1 return
                    cout << "thread end successfully" << status << endl;
                else
                    cout << "thread end error" << ret << endl;
 
                break;
            }
        }
    }
}

void convert_frames(queue<cv::Mat> &BGR_QUEUE) {
    
    cout << endl << "----Start to convert Frames into YUV420 and Y----" << endl << endl;
    
    queue<cv::Mat> BGR_queue(BGR_QUEUE);
    
    while(true){
        
        if(BGR_queue.size() == 0) {break;}
        
        cv::Mat original = BGR_queue.front();
        cv::Mat yuv_frame(cv::Size(height*3/2, width), CV_8UC1);
        cv::Mat y_frame(cv::Size(height, width), CV_8UC1);
        BGR_queue.pop();

        //CONVERT BGR To YUV420 and YUV420 to Y
        cv::cvtColor(original, yuv_frame, cv::COLOR_BGR2YUV_I420);
        cv::cvtColor(yuv_frame, y_frame, cv::COLOR_YUV2GRAY_I420);
    
        //save frames into queue 
        yuv420_queue.push(yuv_frame);
        y_queue.push(y_frame);
        
    }
    
    cout << "    YUV420 amd Y frame are saved" << endl;
    cout << "    YUV420 frame: " << yuv420_queue.size() << "    Y frame: " << y_queue.size() << endl << endl;
    cout << "----FRAMES CONVERTED---------" << endl << endl;
}


void show_frames_bgr(queue<cv::Mat> &BGR_QUEUE) {
    
    queue<cv::Mat> BGR_queue(BGR_QUEUE);
     
    int cnt = 0;
     
    while(true) {
        if(BGR_queue.size() == 0) {
            cv::destroyAllWindows();
            break;
            
        }
        cv::imshow("Original BGR", BGR_queue.front());
        
        BGR_queue.pop();
        cnt++;
        
        if(cv::waitKey(10) == 't') break;
        
    }
    cout << "bgr queue size: " << cnt << endl;
}

void show_frames_yuv(queue<cv::Mat> &YUV420_QUEUE) {
    
    queue<cv::Mat> YUV420_queue(YUV420_QUEUE);
    
    int cnt = 0;
    
    while(true) {
        if(YUV420_queue.size() == 0) {
            cv::destroyAllWindows();
            break;
        }
        cv::imshow("Original YUV", YUV420_queue.front());
        
        YUV420_queue.pop();
        cnt++;
        
        if(cv::waitKey(10) == 't') break;

    }
    cout << "yuv queue size: " << cnt << endl;
}

void show_frames_y(queue<cv::Mat> &Y_QUEUE){
   
    queue<cv::Mat> Y_queue(Y_QUEUE);
    
    int cnt = 0;
    
    while(true) {
        if(Y_queue.size() == 0) {
            cv::destroyAllWindows();
            break;
        }
        cv::imshow("Y" ,Y_queue.front());

        Y_queue.pop();
        cnt++;
        
        if(cv::waitKey(10) == 't') break;
        
    }
    cout << "y queue size: " << cnt << endl;
}

void show_frames_feature_vector(queue<cv::Mat> &FV_QUEUE){
   
    queue<cv::Mat> Feature_Vector_queue(FV_QUEUE);
    
    int cnt = 0;
    while(true) {
        if(Feature_Vector_queue.size() == 0) {
            cv::destroyAllWindows();
            break;
        }
        cv::imshow("Edge" ,Feature_Vector_queue.front());

        Feature_Vector_queue.pop();
        cnt++;
        
        if(cv::waitKey(10) == 't') break;
        if(Feature_Vector_queue.size() == 0) {
            cv::destroyAllWindows();
            break;
        }
    }
    cout << "    Edge queue size: " << cnt << endl;
}

void edge_detection(queue<cv::Mat> &Y_QUEUE) {
    queue<cv::Mat> Y_queue(Y_QUEUE);
    
    cout << "----Building feature vectors." << endl;
    
    int cnt = 0;
    
    while(true) {
        if(Y_queue.size() == 0) {break;}
        cv::Mat temp;
        
        //Canny(img, threshold1, threshold2) 
        //threshold1 = Determining whether an edge is in the adjacency with another edge
        //threshold2 = Determine if it is an edge or not
        cv::Canny(Y_queue.front(), temp, 20, 40);
        
        feature_vector_queue.push(temp);
        Y_queue.pop();
        cnt++;
    }
    cout << endl << "    Edge Detection made: " << feature_vector_queue.size() << endl;
}

void make_hash(queue<cv::Mat> &FV_QUEUE) {
    
    queue<cv::Mat> Feature_Vector_queue(FV_QUEUE);
    
    cout << endl << "----Make HASH from feature vectors." << endl << endl;
    
    while(true) {
        if(Feature_Vector_queue.size() == 0) {break;}
        cv::Mat temp = Feature_Vector_queue.front();
        Feature_Vector_queue.pop();
        
        string mat_data = "";
        string sha_result = "";
        
        
        for(int i =0; i<temp.rows; i++) {
            for(int j =0; j < temp.cols; j++) {
                mat_data += to_string(temp.at<uchar>(i,j));
            }
        }
        
        
        sha_result = hash_sha256(mat_data);
        hash_queue.push(sha_result);
    }
    
    cout << "    hash made : " << hash_queue.size() << endl;
}

string getCID() {
    struct timeb tb;   // <sys/timeb.h>                       
    struct tm tstruct;                      
    std::ostringstream oss;   
    
    string s_CID;                             
    char buf[128];                                            
                                                              
    ftime(&tb);
    // For Thread safe, use localtime_r
    if (nullptr != localtime_r(&tb.time, &tstruct)) {         
        strftime(buf, sizeof(buf), "%Y-%m-%d_%T.", &tstruct);  
        oss << buf; // YEAR-MM-DD HH-mm_SS            
        oss << tb.millitm; // millisecond               
    }              

    s_CID = oss.str();
    
    s_CID = s_CID.substr(0,23);
    if(s_CID.length() == 22) {
        s_CID = s_CID.append("0");
    }
    if(s_CID.length() == 21) {
        s_CID = s_CID.append("00");
    }
    
    return s_CID;
}



void send_datas_to_server(queue<cv::Mat> &YUV420_QUEUE, queue<string> &HASH_QUEUE, queue<string> &CID_QUEUE) {
    
    queue<cv::Mat> yuv_send(YUV420_QUEUE);
    queue<string> hash_send(HASH_QUEUE);
    queue<string> cid_send(CID_QUEUE);
    total_data_size = 0;
    
    while(true) {
        if(yuv_send.size() == 0 && hash_send.size() ==0 && cid_send.size() == 0) {break;}
        int video_rows = yuv_send.front().rows;
        int video_cols = yuv_send.front().cols;
        int video_channels = yuv_send.front().channels();
        int video_bufsize = video_rows * video_cols * video_channels;

        total_data_size += video_bufsize;

        int hash_bufsize = hash_send.front().size();
        int cid_bufsize = cid_send.front().size();
        total_data_size += hash_bufsize;
        total_data_size += cid_bufsize;
        
        unsigned char *video_buffer = new unsigned char[video_bufsize];
        cout << "video buffer size : " << video_bufsize << endl;
        char *hash_buffer = new char[hash_bufsize];
        char *cid_buffer = new char[cid_bufsize];
        
        memcpy(video_buffer, yuv_send.front().data, video_bufsize);
        strcpy(hash_buffer, hash_send.front().c_str());
        strcpy(cid_buffer, cid_send.front().c_str());

        makePacket(VIDEO_DATA_SND, 0, total_data_size);
        cout << "port is : " << &g_pNetwork->port.s << endl;
        if(!send_packet(&g_pNetwork->port, sizeof(HEADERPACKET), &sendDataPacket)){
            cout << "Packet send Error!!" << endl;
            break;
        }
        
        //cout << "video rows: " << video_rows << endl << "video cols: " << video_cols << endl;
        //cout << "size: " << (strlen((char*)video_buffer)) * sizeof(unsigned char) << endl;
        cout << "hash: " << hash_buffer << endl << "size: " << strlen(hash_buffer) * sizeof(char) << endl;
        cout << "CID: " << cid_buffer << endl << "size: " << strlen(cid_buffer) * sizeof(char) << endl;
        
        if(!send_binary(&g_pNetwork->port, cid_bufsize, (void*)cid_buffer)){
            cout << "CID send Error!!" << endl;
        }

        if(!send_binary(&g_pNetwork->port, hash_bufsize, (void*)hash_buffer)){
            cout << "hash send Error!!" << endl;
        }

        if(!send_binary(&g_pNetwork->port, video_bufsize, (void*)video_buffer)){
            cout << "Image send Error!!" << endl;
        }
        yuv_send.pop();
        hash_send.pop();
        cid_send.pop();
    }
}


void test() {
    cap >> frame;

    Mat xxx ;
    xxx = imread("./lenna.bmp");
    bgr_queue.push(xxx);
    string s;
    s = getCID();
    cid_queue.push(s);
}

int main(int, char**) { 
    
    if(!initClient()){
        cout << "init client error!!" << endl;
        return -1;
    }
    
    // main
    //init setting for OpenCV by Settings from Web
    init();
    
    //if you need only one frame for test then use test() not capture();
    //Capture frames. if you want to end, press ESC
    //capture();
    test();
    
    //Convert fames to YUV and Y
    convert_frames(bgr_queue);
    
    //USE Canny Edge Detection with Y_frames
    edge_detection(y_queue);
    
    
    // //Make hash by edge_detected datas;
    make_hash(feature_vector_queue);
    
    //Send Data to Server
    send_datas_to_server(yuv420_queue, hash_queue, cid_queue);
    
    init_all_setting();
    return 0;
}
 
