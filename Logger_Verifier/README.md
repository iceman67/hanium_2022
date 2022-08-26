# Logger
The logger functions are frame capture, conversion to yuv420, extracting only y-frames, creating feature_vector with y-frames, and creating hash for feature_vector. Datas will provided to Flask Web and Server

## GET STARTED
OS: Raspberry pi OS 32-bit

## OPENCV
OPENCV version: 4.5.1

### Installing OPENCV
1. Update package list
```
sudo apt update
```
2. Upgrade to new packages
```
sudo apt update
```
3. Reboot
```
sudo reboot
```
4. Download packages required
```
sudo apt install build-essential cmake
```
```
sudo apt install libjpeg-dev libtiff5-dev libjasper-dev libpng-dev
```
```
sudo apt install libavcodec-dev libavformat-dev libswscale-dev libxvidcore-dev libx264-dev libxine2-dev
```
```
sudo apt install libv4l-dev v4l-utils
```
```
sudo apt install libgstreamer1.0-dev libgstreamer-plugins-base1.0-dev gstreamer1.0-plugins-good gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly
```
```
sudo apt install libgtk2.0-dev
```
```
sudo apt install mesa-utils libgl1-mesa-dri libgtkgl2.0-dev libgtkglext1-dev   
```
```
sudo apt install libatlas-base-dev gfortran libeigen3-dev
```
```
if you use python,
sudo apt install python3-dev python3-numpy
```
5. make dir
```
mkdir opencv && cd opencv
```
6. Download opencv4.5.1 source code
```
wget -O opencv.zip https://github.com/opencv/opencv/archive/4.5.1.zip
```
```
unzip opencv.zip
```
7. Download opencv_contrib
```
wget -O opencv_contrib.zip https://github.com/opencv/opencv_contrib/archive/4.5.1.zip
```
```
unzip opencv_contrib.zip
```
8. Move to opencv-4.5.1 and make build dir and move to build dir
```
cd opencv-4.5.1
```
```
mkdir build && cd build
```
9. Use cmake to set Opecv comfile settings
```
cmake -D CMAKE_BUILD_TYPE=RELEASE -D CMAKE_INSTALL_PREFIX=/usr/local -D WITH_TBB=OFF -D WITH_IPP=OFF -D WITH_1394=OFF -D BUILD_WITH_DEBUG_INFO=OFF -D BUILD_DOCS=OFF -D INSTALL_C_EXAMPLES=ON -D INSTALL_PYTHON_EXAMPLES=ON -D BUILD_EXAMPLES=OFF -D BUILD_TESTS=OFF -D BUILD_PERF_TESTS=OFF -D ENABLE_NEON=ON -D ENABLE_VFPV3=ON -D WITH_QT=OFF -D WITH_GTK=ON -D WITH_OPENGL=ON -D OPENCV_ENABLE_NONFREE=ON -D OPENCV_EXTRA_MODULES_PATH=../../opencv_contrib-4.5.1/modules -D WITH_V4L=ON -D WITH_FFMPEG=ON -D WITH_XINE=ON -D ENABLE_PRECOMPILED_HEADERS=OFF -D BUILD_NEW_PYTHON_SUPPORT=ON -D OPENCV_GENERATE_PKGCONFIG=ON ../
```
```
if cmake is succeeded, this messeges wil show.
-- Configuring done
-- Generating done
-- Build files have been written to: /home/pi/opencv/opencv-4.5.1/build
```
10. change swapsize you can use any editors
```
sudo nano /etc/dphys-swapfile
```
```
CONF_SWAPSIZE=2048
```
```
sudo /etc/init.d/dphys-swapfile restart
```
```
free
```
11. use Makefile to comfile
```
time make -j4
```
12. Download Result made by comfile
```
sudo make install
```
13. Use ldconfig to find opencv lib
```
sudo ldconfig
```
14. Rollback swapsize to 100
```
sudo nano /etc/dphys-swapfile
```
```
CONF_SWAPSIZE=100
```
```
sudo /etc/init.d/dphys-swapfile restart
```
```
free
```

## Downlad CRYPTOPP
```
git clone https://github.com/weidai11/cryptopp.git
```
```
cd cryptopp
```
```
make 
```
```
sudo make install
```


## To get Logger_Verifier code
```
git clone https://github.com/hanium-22-HF195/Logger_Verifier.git
```

## COMFILE
1. to make Logger
```
make clean && make Logger
```
2. to make Verifier
```
make clean && make Verifier
```
3. to make Logger and Verifier at once
```
make clean && make Logger Verifier
```
4. to Test Generating PRIVATE KEY AND PUBLIC KEY
```
make sign_verify
```
### CODES
- [ ] Logger: Logger.cpp
- [ ] Verifier: Verifier.cpp
- [ ] Merkle Tree related: include folder, src/mercke_tree.cpp, src/node.cpp
- [ ] Key Generation related: ./include/cryptopp, src/sign_verify
- [ ] Makefile


#### Logger.cpp

- [ ] int init(): Chage Camera settings by given datas from web.
- [ ] void UpdateFrame(void arg): Update Frames.
- [ ] void capture(): Capture Frames until User press ESC.
- [ ] void convert_frames(): Convert BGR frames into YUV420 and Y.
- [ ] void show_frames_bgr(queue<Mat> &ori_bgr): show bgr frames in original_bgr queue.
- [ ] void show_frames_yuv(queue<Mat> &ori_yuv): show yuv420 frames in original_yuv queue.
- [ ] void show_frames_y(queue<Mat> &y): show y frames in y_frames queue.
- [ ] void edge_detection(queue<Mat> &y): Use y_frames queue to fine Edges in frames.
- [ ] void show_frames_feature_vector(queue<Mat> &feature_vetor): show feature_vector frames in feature_vector queue.
- [ ] string getCID(): Make CID for Frame.
- [ ] make_hash(queue<Mat> &feature_vector): Use feature_vector queue to make hash for each frames.
- [ ] void init_all_setting(): after all functions, reset all setting for OpenCV and clear queues.
- [ ] void init_queue(): empty all queues.


#### TESTING - Processing
1. To test Verifier
```
./Verifier
```
2. To test Logger
```
./Logger
```

##  future modifies
1. Logger
- [ ] GET OPENCV settings from Server.
- [ ] Sending CID, video data and hash to Server.
- [ ] Generate PUB Key, PRI Key and send PUB Key to Server. Server then transmit PUB Key to Verifier.

2. Verifier
- [ ] request video data and hash to Server.
- [ ] Send Verified Result to Server.
- [ ] Send vidao data, hash, CID and Verified Result to web.
