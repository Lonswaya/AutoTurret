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

    md->run_flag = 1;
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
    
    md->centers_of_motion.clear();

    if(config->blur_size < 0) {
        config->blur_size = 0; //no blur
    }

    if(config->motion_thresh < 0) {
        config->motion_thresh = 35; //some default value here
    }

    return 1;
}

int md_detect(MotionDetector *md) {
   
    Mat curr_frame;

    Mat out; 

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


    out = curr_frame;
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

    int x, y, dx, dy;

    md_condense(md, &x, &y, &dx, &dy);

    for(int i = 0; i < md->rect_list.size(); i++) {
        rectangle(out, md->rect_list[i], Scalar(255, 0, 0), 2, 8, 0 );    
    }

    md->centers_of_motion.push_back( Point(x + dx / 2, y + dy / 2) );
    circle(out, md->centers_of_motion.back(), 3, Scalar(0, 255, 0), 2, 8, 0);
    circle(out, Point( (x + dx), (y + dy)), 3, Scalar(0, 255, 255), 2, 8, 0);
    circle(out, Point(x, y), 3, Scalar(0, 0, 255), 2, 8, 0);
    
    
    imshow("ss", out);
    waitKey(1);
    return 1;
}

int md_get_average_center(MotionDetector *md, int *x, int *y) {
    
    //some how we called this without any frames put in the list we just return center of camera
    size_t size = md->centers_of_motion.size();
    if(size == 0) {
        *x = md->config->max_x / 2;
        *y = md->config->max_y / 2;
        return 1;
    }

    int tmp_x = 0, tmp_y = 0;

    for(int i = 0; i < size; i++) {
        tmp_x += md->centers_of_motion[i].x;
        tmp_y += md->centers_of_motion[i].y;
    }

    *x = tmp_x / size;
    *y = tmp_y / size;
    
    md->centers_of_motion.clear();

    return 1;
}

int md_find_motion(MotionDetector *md, Mat *diff_frame) {
    //turns out there is a countour function that will find all bounding rects commenting out these for now

   
    md->rect_list.clear();    
    std::vector< std::vector<Point> > contours;
    std::vector<Vec4i> hierarchy;
    
    //making copy just in case countour changes it
    Mat input = *diff_frame;

    findContours(input, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0,0));
    for(int i = 0; i < contours.size(); i++) {
        md->rect_list.push_back(boundingRect(contours[i]));
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

void *detection_loop(void* arg) {
    MotionDetector *md = (MotionDetector *) arg;

    while(md->run_flag) {
        if(md->config->detect_flag) {
            md_detect(md);
        }
    }
    //thread ends clean up code    
}




/*

int main() {
    
    MotionConfig config;
    config.blur_size = 5;
    config.motion_thresh = 10;

    MotionDetector md;
    md_init(&md, &config);

    int x, y, dx, dy;

    while(1) {
        md_detect(&md, &x, &y, &dx, &dy);
    }

}
*/
