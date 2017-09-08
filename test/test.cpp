#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/objdetect.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>

#include <string>
#include <vector>

using namespace cv;

struct fps_info {
    int frame_count;
    int rate;   
    int dead;
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
    while(!shared_info->dead) {
        sleep(1);
        shared_info->rate = shared_info->frame_count;
        shared_info->frame_count = 0;
    }
}

struct config_info {
    char resize;
    char show_frame;
    char show_rect;
    char hog;
    char die;

    int resize_x;
    int resize_y;
    int hog_stride;
    int hog_pad;

    VideoCapture *cap;
};

void *capture(void *args) {
    //sets up image processing
    Mat frame;
    HOGDescriptor hog;
    hog.setSVMDetector(HOGDescriptor::getDefaultPeopleDetector());
    std::vector<Rect> rects;

    struct config_info *configs = (struct config_info *) args;
    
    //starts up fps thread
    pthread_t fps_thread;
    struct fps_info fps;
    fps.frame_count = 0;
    fps.rate = 0;
    fps.dead = 0;
    pthread_create(&fps_thread, NULL, &fps_counter, (void *) &fps);
    
    while(true) {
        
        if(configs->die) {
            fps.dead = 1;
            pthread_join(fps_thread, NULL);        
            break;
        }

        configs->cap->read(frame);        
        fps.frame_count++;

        if(configs->resize)
            resize(frame, frame, Size(configs->resize_x, configs->resize_y));

        if(configs->hog)
            hog.detectMultiScale(frame, rects, 0, Size(configs->hog_stride, configs->hog_stride), Size(configs->hog_pad, configs->hog_pad));
        
        if(configs->show_rect) {
            for(int i = 0; i < rects.size(); i++) 
                rectangle(frame, Point2i(rects[i].x, rects[i].y), Point2i(rects[i].x + rects[i].width, rects[i].y + rects[i].height), Scalar(0, 255, 0), 2);
        }
        //laser tracking should be done before grayscale
        
        //color doesnt matter in motion dection, convert to gray scale
        //cvtColor(frame, frame, COLOR_BGR2GRAY);
        //blur to remove noise pixels
        //GaussianBlur(frame, frame, Size(21, 21), 0);

        if(configs->show_frame) {
            putText(frame, std::to_string(fps.rate), Point2f(10, 30), FONT_HERSHEY_SIMPLEX, 1, Scalar(0, 0, 255)); 
            imshow("ss", frame);    
        }

        waitKey(1);
    }
}

int main(int argc, char** argv) {
    
    VideoCapture webcam;
    webcam.open(0 + CAP_ANY);
    if(!webcam.isOpened()) {
        printf("Camera not opened\n");
        return 1;
    }
    
    //starts up capture thread
    pthread_t capture_thread;
    struct config_info config;
    config.die = 0; 
    config.resize = 0;
    config.hog = 1;
    config.hog_stride = 4;
    config.hog_pad = 8;
    config.show_rect = 1;
    config.show_frame = 1;
    config.resize_x = 600;
    config.resize_y = 400;
    config.cap = &webcam;
    pthread_create(&capture_thread, NULL, &capture, (void *) &config);

    int input, opt;
    while(true) {
        scanf("%d", &input);

        //die = 0, resize = 1, hog = 2, show_rect = 3, show_frame = 4, re_x = 5, re_y = 6, hog_stride = 7, hog_pad = 8
        switch(input) {
            case 0:
                config.die = 1;
                pthread_join(capture_thread, NULL);
                exit(1);
            case 1:
                config.resize = !config.resize;
                break;
            case 2:
                config.hog = !config.hog;
                break;
            case 3:
                config.show_rect = !config.show_rect;
                break;
            case 4:
                config.show_frame = !config.show_frame;
                break;
            default:
                char buff[16];
                sprintf(buff, "%d", input);
                int val = atoi(&buff[1]);
                if(buff[0] == '5') {
                    config.resize_x = val; 
                } else if(buff[0] == '6') {
                    config.resize_y = val;
                } else if(buff[0] == '7') {
                    config.hog_stride = val;
                } else if(buff[0] == '8') {
                    config.hog_pad = val;
                }
        }

    }
    return 0;
}
