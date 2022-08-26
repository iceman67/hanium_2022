#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <stdio.h>
#include <cstdlib>
#include <queue>
#include <pthread.h>
#include <thread>
#include <vector>
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

using namespace std;

int width = 352;
int height = 288;
int FPS = 30;
unsigned int frame_count = 1;

cv::VideoCapture cap;
cv::Mat frame(cv::Size(width, height), CV_8UC3);

pthread_mutex_t frameLocker;
pthread_t UpdThread; 

queue<cv::Mat> bgr_queue;                       //for original frame(BGR)Mat queue
queue<cv::Mat> yuv420_queue;                    //for original frame(yuv)Mat queue
queue<cv::Mat> y_queue;                         //for y_frame Mat queue
queue<cv::Mat> feature_vector_queue;            //for edge detection Canny
queue<string> hash_server_queue;                //for hash from server
queue<string> hash_verifier_queue;              //for hash made by feature vector by verifier
queue<string> cid_queue;                        //for CID for images 

int init();
void init_all_setting();                            //Init all settings at the end
void init_queue();                                  //Init all datas in queues
void get_data_from_server();                        //GET Data from server and save into queues

void *UpdateFrame(void *arg);                        
void capture(); 
void convert_frames(queue<cv::Mat> &BGR_QUEUE); 

void extract_Y_frame(queue<cv::Mat> YUV420_QUEUE);
void show_frames_bgr(queue<cv::Mat> &BGR_QUEUE);            //show frames by bgr
void show_frames_yuv(queue<cv::Mat> &YUV_QUEUE);            //show frames by yuv
void show_frames_y(queue<cv::Mat> &Y_QUEUE);                //show frames by y
void show_frames_feature_vector(queue<cv::Mat> &FV_QUEUE);  //show frames by feature vector
void edge_detection(queue<cv::Mat> &Y_QUEUE);               //Edge detact by y frames
void make_hash(queue<cv::Mat> &FV_QUEUE);                   //make hash using feature vector
int compare_hashs();								        //compare two hashs
string getCID();                                            //Make CID for frames
int make_merkle_tree();

/*NEED to make
* 1. Get Public Key from Server
* 2. Get CID from web
* 3. Request hash, yuv420 data to server
* 4. decrypt yuv420 data
* 6. Send yuv420 data to Web
* 7. Send two hashs to web
* 8. Send Verified Result to Server by CID
* 9. Send Verified Result to Web
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
    width = 352;
    height = 288;
    FPS = 30;
    init_queue();
    
    cout << "----Initializing all settings." << endl <<endl;
    cout << "    cap closed." <<endl;
    cout << "    bgr queue size: " << bgr_queue.size() << endl;
    cout << "    yuv420 queue size: " << yuv420_queue.size() << endl;
    cout << "    y_frame queue size: " << y_queue.size() << endl;
    cout << "    feature vector queue size: " << feature_vector_queue.size() << endl;
    cout << "    hash-server queue size: " << hash_server_queue.size() << endl;
    cout << "    hash-verifier queue size: " << hash_verifier_queue.size() << endl;
    cout << "    CID queue size: " << cid_queue.size() << endl;
    cout << endl << "--------------------------------" << endl << endl;
}

void init_queue() {         
    yuv420_queue = queue<cv::Mat>();                    //for original frame(yuv)Mat queue
    bgr_queue = queue<cv::Mat>();                    //for original frame(BGR)Mat queue
    y_queue = queue<cv::Mat>();                        //for y_frame Mat queue
    feature_vector_queue = queue<cv::Mat>();                  //for edge detection Canny
    hash_server_queue = queue<string>();
    hash_verifier_queue = queue<string>();          
    cid_queue = queue<string>();                          //for CID for frames
}

void get_data_from_server() {
    cv::Mat from_server = cv::imread("./2022-08-24_15:54:10.909");
    cout <<from_server.size() << endl;
 
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
    cout << "Edge queue size: " << cnt << endl;
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
    cout << endl << "Edge Detection made: " << feature_vector_queue.size() << endl;
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
                mat_data += to_string(temp.at<uchar>(i,j)) + " ";
            }
        }
        
        sha_result = hash_sha256(mat_data);
        hash_verifier_queue.push(sha_result);
    }
    cout << "    hash made : " << hash_verifier_queue.size() << endl << endl;
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
        strftime(buf, sizeof(buf), "%Y-%m-%d %T.", &tstruct);  
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
/*
int compare_hashes() {	
	
	for(unsigned int i =0; i<frame_count; i++) {
		
        cout << "h_server: " << feature_vector_hash_server.front() << endl;
        cout << "h_verifier: " << feature_vector_hash_verifier.front() << endl;
       
		if(strcmp(h_server, h_verifier) == 1) {
			cout << "hash is different." << endl;
			break;
		}
		
		feature_vector_hash_server.pop();
		feature_vector_hash_verifier.pop();
		
		if(feature_vector_hash_server.size() == 0 && feature_vector_hash_verifier.size() == 0) {
            break;
        }
	}
	cout << "all hashes are same" << endl;
	
    
    }  
    return 0; 
}
*/

int make_merkle_tree() {
    std::vector<Node*> leaves;
    
    int i = 0;
    while(true) {
        string hash = (hash_verifier_queue.front());
        leaves.push_back(new Node(hash));
        hash_verifier_queue.pop();
        i = i+1;
        if(hash_verifier_queue.size() == 0) break;
    }
    
    // initialize leaves
    for (unsigned int i = 0; i < leaves.size(); i++) {
        leaves[i]->left = NULL;
        leaves[i]->right = NULL;
    }

    MerkleTree *hashTree = new MerkleTree(leaves);
    std::cout << hashTree->root->hash << std::endl;
    hashTree->printTree(hashTree->root, 0);

    for (unsigned int k = 0; k < leaves.size(); k++) {
        delete leaves[k];
    }

    delete hashTree;
    
    return 0;
}

int main(int, char**) { 
    
    // main
    //init setting for OpenCV by Settings from Web
    init();
    
    //if you need only one frame for test then use test() not capture();
    //Capture frames. if you want to end, press ESC
    capture();
    
    //Convert fames to YUV and Y
    convert_frames(bgr_queue);
    
    //USE Canny Edge Detection with Y_frames
    edge_detection(y_queue);
    
    
    // //Make hash by edge_detected datas;
    make_hash(feature_vector_queue);

    //Make Merkle Tree
    make_merkle_tree();
    
    return 0;
}
 
