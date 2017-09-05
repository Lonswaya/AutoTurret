#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>

#include <string>

using namespace cv;

struct fps_info {
    int frame_count;
    int rate;   
};

long long get_time() {
    struct timeval tmp;
    if (gettimeofday(&tmp, NULL) < 0) {
        return -1;
    }
    return (long long) (tmp.tv_sec * 1e3 +  tmp.tv_usec / 1e3);
}

void *fps_counter(void *args) {
    struct fps_info *shared_info = (struct fps_info *) args;
    while(true) {
        sleep(1);
        shared_info->rate = shared_info->frame_count;
        shared_info->frame_count = 0;
    }
}


int main(int argc, char** argv) {
    
    VideoCapture webcam;
    webcam.open(1 + CAP_ANY);
    if(!webcam.isOpened()) {
        printf("Camera not opened\n");
        return 1;
    }
    
    Mat frame;
    Mat gray;
    pthread_t fps_thread;
    struct fps_info fps;
    fps.frame_count = 0;
    fps.rate = 0;
    pthread_create(&fps_thread, NULL, &fps_counter, (void *) &fps);

    while(true) {
        webcam.read(frame);        
        fps.frame_count++;
        
        //laser tracking should be done before grayscale
        
        //color doesnt matter in motion dection, convert to gray scale
        cvtColor(frame, frame, COLOR_BGR2GRAY);
        //blur to remove noise pixels
        GaussianBlur(frame, frame, Size(21, 21), 0);

        putText(frame, std::to_string(fps.rate), Point2f(10, 30), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255)); 
        imshow("ss", frame);    

        waitKey(1);
    }

    return 0;
}
