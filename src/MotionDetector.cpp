#include "MotionDetector.h"

using namespace cv;

int md_init(MotionDetector *md, MotionConfig *config) {
        
    //cam.release() is called automatically upon subsequent open
    //no need to clean up before another open
    md->cam.open(0 + CAP_ANY);
    if(!md->cam.isOpened()) {
        //Failed to open camera
        return 0;
    }

    md->config = config;
    md->config->max_x = (int) md->cam.get(CAP_PROP_FRAME_WIDTH);
    md->config->max_y = (int) md->cam.get(CAP_PROP_FRAME_HEIGHT);

    if(md->config->max_x == 0 || md->config->max_y == 0) {
        //API can't decide the width and height of camera 
        //something went wrong
        return 0;
    }

    md->frame_buffer.clear();
    md->rect_list.clear();

    if(config->blur_size < 0) {
        config->blur_size = 0; //no blur
    }

    if(config->motion_thresh < 0) {
        config->motion_thresh = 35; //some default value here
    }

    return 1;
}

int md_detect(MotionDetector *md, int *x, int *y, int *dx, int *dy) {
   
    Mat curr_frame;
    
    if(md->frame_buffer.size() == 0) {
        //motion detector first initialized
        //squeeze in 3 grabs, first grab is wasted from next pop
        for(int i = 0; i < 3; i++) {

            if(!md->cam.read(curr_frame)) {
                //could not read an image
                //camera disconnect or no more image???
                return 0;
            }
            
            if(md->config->blur_size > 0) {
                GaussianBlur(curr_frame, curr_frame, Size(md->config->blur_size, md->config->blur_size), 0);  
            }

            cvtColor(curr_frame, curr_frame, CV_RGB2GRAY);
            md->frame_buffer.push_back(curr_frame);
        }
    }

    //grab the frame
    if(!md->cam.read(curr_frame)) {
        //could not read an image
        //camera disconnect or no more image???
        return 0;
    }

    if(md->config->blur_size > 0) {
        GaussianBlur(curr_frame, curr_frame, Size(md->config->blur_size, md->config->blur_size), 0);  
    }
    cvtColor(curr_frame, curr_frame, CV_RGB2GRAY);

    md->frame_buffer.pop_front();

    Mat d1, d2, result;
    absdiff(md->frame_buffer.front(), md->frame_buffer.back(), d1);
    
    absdiff(md->frame_buffer.back(), curr_frame, d2);
    md->frame_buffer.push_back(curr_frame);

    bitwise_and(d1, d2, result); 
    threshold(result, result, md->config->motion_thresh, 255, CV_THRESH_BINARY);    

    md_find_motion(md, &result);

    md_condense(md, x, y, dx, dy);
    return 1;
}


int _md_search_full_motion(std::vector<cv::Rect> &list, Mat *frame, int x, int y) {
    
    int max_x = x;
    int max_y = y; 
    
    //TODO: probing algorithm

    return 1;
}

int md_find_motion(MotionDetector *md, Mat *diff_frame) {
    
    int rows = diff_frame->rows;
    int cols = diff_frame->cols;
    
    //loop over ethe entire image
    for(int i = 0; i < rows; i++) {
        for(int j = 0; j < cols; j++) {
            
            //if some pixel is not black AKA there are changes
            if((int) diff_frame->at<uchar>(i, j) > 0) {
                _md_search_full_motion(md->rect_list, diff_frame, i, j);   
            }
        }
    }

    return 1;
}

int md_condense(MotionDetector *md, int *x, int *y, int *dx, int *dy) {
    int min_x = md->config->max_x;
    int min_y = md->config->max_y;
    int max_x = 0;
    int max_y = 0;

    std::vector<Rect>::iterator iter;
    for(iter = md->rect_list.begin(); iter != md->rect_list.end(); iter++) {
        int l_x_min = iter->x;
        int l_y_min = iter->y;
        int l_x_max = iter->x + iter->width;
        int l_y_max = iter->y + iter->height;

        if(l_x_min < min_x) {
            min_x = l_x_min;
        }
        if(l_y_min < min_y) {
            min_y = l_y_min;
        }
        if(l_x_max > max_x) {
            max_x = l_x_max;
        }
        if(l_y_max > max_y) {
            max_y = l_y_max;
        }
    }

    *x = min_x;
    *y = min_y;
    *dx = max_x - min_x;
    *dy = max_y - min_y;

    return 1;
}
