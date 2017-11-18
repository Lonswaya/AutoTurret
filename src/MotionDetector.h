#ifndef MotionDetector_H_
#define MotionDetector_H_

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>

#include <vector>
#include <list>
#include <pthread.h>

typedef struct motion_config {
    int blur_size;      // how much to blur if applied
    int motion_thresh;  // the threshold exceed in order to be considered motion
    int max_x;          // camera's width (will be set by init)
    int max_y;          // camera's height (will be set by init)
    int detect_flag;    //will be used to halt detection ex. when camera moves
} MotionConfig;

typedef struct motion_detector {
    cv::VideoCapture cam;                   //camera object for opencv
    std::list<cv::Mat> frame_buffer;        //a buffer of 3 frames max for delta image
    std::vector<cv::Rect> rect_list;        //a list that contains all motions found from a delta image
    MotionConfig *config;                   //a struct of configurations md_detect needs
    
    //keeping a buffer and iterate all everytime could be expensive
    //std::vector<cv::Point> centers_of_motion;
    

    unsigned int total_center_x;            //instead of a list we just keep a sum
    unsigned int total_center_y;            //and divide by a total when we need to find average
    unsigned int center_count;

    int run_flag;                           //flag to stop network thread
    pthread_mutex_t lock;                   //lock whenever something is changed on motion config

} MotionDetector;

/* 
 * Initialize camera
 * Check configs
 */
extern "C" int md_init(MotionDetector *md, MotionConfig *config);

/*
 * Fetches a single frame, compare against frame buffer then
 * detect motions. It will fill up rect_list with motions, 
 */
extern "C" int md_detect(MotionDetector *md);

/*
 * Reads the rect_list and condense them into a single average a
 * area of motion.
 */
extern "C" int md_condense(MotionDetector *md, int *x, int *y, int *dx, int *dy);

/*
 * Reads differentiated frame and finds all motions then fills rect_list.
 */
extern "C" int md_find_motion(MotionDetector *md, cv::Mat *diff_frame);

/*
 * Calculates the average position of all otions in center_of_motion list and fills x and y
 */
extern "C" int md_get_average_center(MotionDetector *md, int *x, int *y);

/*
 *  Disable motion detection from detection loop
 */
extern "C" int md_disable_detection(MotionDetector *md);


/*
 * Restore motion detection from detection loop
 * Meant to be called after md_disable_detection()
 */
extern "C" int md_enable_detection(MotionDetector *md);

/*
 * Use this to stop thread
 */
extern "C" int md_stop_thread(MotionDetector *md);

/*
 * Clean up i guess
 */
extern "C" int md_clean_up(MotionDetector *md);

/*
 * Detection thread will run this function
 */
extern "C" void *detection_loop(void *arg);

#endif


