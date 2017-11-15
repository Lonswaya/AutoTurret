#ifndef MotionDetector_H_
#define MotionDetector_H_

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>

#include <vector>
#include <list>

typedef struct motion_config {
    int blur_size;      // how much to blue if applied
    int motion_thresh;  // the threshold exceed in order to be considered motion
    int max_x;          // camera's width (will be set by init)
    int max_y;          // camera's height (will be set by init)
    int detect_flag;
} MotionConfig;

typedef struct motion_detector {
    cv::VideoCapture cam;
    std::list<cv::Mat> frame_buffer;
    std::vector<cv::Rect> rect_list;
    MotionConfig *config;
    std::vector<cv::Point> centers_of_motion;
    int run_flag;
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
 * Helper function used to search all pixels belong to an area of motion given a top-left starting point
 */
int _md_search_full_motion(std::vector<cv::Rect> &list, cv::Mat *frame, int x, int y);

/*
 * Clean up i guess
 */
extern "C" int md_clean_up(MotionDetector *md);

/*
 * Detection thread will run this function
 */
extern "C" void *detection_loop(void *arg);

#endif


