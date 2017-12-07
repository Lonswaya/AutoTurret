#include "MotionDetector.h"
#include <cmath>

using namespace cv;

int md_init(MotionDetector *md, MotionConfig *config) {
        
    //cam.release() is called automatically upon subsequent open
    //no need to clean up before another open
    md->cam.open("http://localhost:8090/stream");
    if(!md->cam.isOpened()) {
        printf("cannot open camera\n");
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
	printf("missing camera specs\n");
        return 0;
    }

    md->frame_buffer.clear();
    md->rect_list.clear();
    
    //md->centers_of_motion.clear();
    //md->center_of_motion.x = config->max_x / 2;
    //md->center_of_motion.y = config->max_y / 2;
    md->total_center_x = 0;
    md->total_center_y = 0;
    md->center_count = 0;

    if(config->blur_size < 0) {
        config->blur_size = 0; //no blur
    }

    if(config->motion_thresh < 0) {
        config->motion_thresh = 35; //some default value here
    }


    if(pthread_mutex_init(&(md->lock), NULL) != 0) {
        //couldn't init lock
        return -1;
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
		printf("cannot read\n");
                //could not read an image
                //camera disconnect or no more image???
                return 0;
            }
        
            /*          
            if(pthread_mutex_lock(&(md->lock)) != 0) {
        //can't lock ????
                return -1;
            }*/

            if(md->config->blur_size > 0) {
                GaussianBlur(curr_frame, curr_frame, Size(md->config->blur_size, md->config->blur_size), 0);  
            }

            //pthread_mutex_unlock(&(md->lock));
            
            cvtColor(curr_frame, curr_frame, CV_RGB2GRAY);
            md->frame_buffer.push_back(curr_frame);
        }
    }
    //grab the frame
    if(!md->cam.read(curr_frame)) {
        //could not read an image
        //camera disconnect or no more image???
        printf("i cannot read\n");
	    return 0;
    }

    out = curr_frame;

    /*
    if(pthread_mutex_lock(&(md->lock)) != 0) {
        //can't lock ????
        return -1;
    }*/


    if(pthread_mutex_lock(&(md->lock)) != 0) {
            //can't lock ????
    	return NULL;
    }
    int tmp_detect_flag = md->config->detect_flag;

    pthread_mutex_unlock(&(md->lock));

    //printf("detect flag: %d\n", tmp_detect_flag);
    if(tmp_detect_flag) {
    
	    if(md->config->blur_size > 0) {
		GaussianBlur(curr_frame, curr_frame, Size(md->config->blur_size, md->config->blur_size), 0);  
	    }
	     
	    //pthread_mutex_unlock(&(md->lock));
	    
	    cvtColor(curr_frame, curr_frame, CV_RGB2GRAY);

	    md->frame_buffer.pop_front();

	    Mat d1, d2, result;
	    absdiff(md->frame_buffer.front(), md->frame_buffer.back(), d1);
	    
	    absdiff(md->frame_buffer.back(), curr_frame, d2);
	    md->frame_buffer.push_back(curr_frame);

	    bitwise_and(d1, d2, result); 

	    /*
	    if(pthread_mutex_lock(&(md->lock)) != 0) {
		//can't lock ????
		return -1;
	    }*/

	    threshold(result, result, md->config->motion_thresh, 255, CV_THRESH_BINARY);    

	    //pthread_mutex_unlock(&(md->lock));
	    
	    md_find_motion(md, &result);

	    int x, y, dx, dy;

	    md_condense(md, &x, &y, &dx, &dy);

	    //int maxRectSize = 0;
	    int combinedRectSize = 0;
	    for(int i = 0; i < md->rect_list.size(); i++) {
		Rect myRect = md->rect_list[i];
		    int rectSize = abs(myRect.height * myRect.width);
		    combinedRectSize += rectSize;
		    //if (rectSize > maxRectSize) maxRectSize = rectSize;
		    //rectangle(out, md->rect_list[i], Scalar(255, 0, 0), 2, 8, 0 );    
	    }
	    //md->strength = maxRectSize;
	    md->strength = combinedRectSize;
	    //using buffer can be expensive
	    //md->ngion.push_back( Point(x + dx / 2, y + dy / 2) );
	    //calculate average every frame instead
	    Point tmp_center(x + dx / 2, y + dy / 2);
	    
	    /*
	    if(pthread_mutex_lock(&(md->lock)) != 0) {
		//can't lock ????
		return -1;
	    }*/
	    
	    md->total_center_x += tmp_center.x;
	    md->total_center_y += tmp_center.y;
	    md->center_count++;
	    //pthread_mutex_unlock(&(md->lock));

	    //draw avg center
	    //circle(out, Point(md->total_center_x / md->center_count, md->total_center_y / md->center_count), 3, Scalar(0, 255, 255), 2, 8, 0);

	    //centor of motion circle
	    //circle(out, tmp_center, 3, Scalar(0, 255, 0), 2, 8, 0);
	    
	    //top left red circle
	    //circle(out, Point(x, y), 3, Scalar(0, 0, 255), 2, 8, 0);
    
    }


    //imshow("ss", out);
    //waitKey(1);
    return 1;
}

//we will preserve this function just in case if we ever need a buffer again
/*
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
}*/

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

/*
 * Why go through the trouble of making 3 functions ?
 * IMO: It's clear to have 2 functions that is very jarring in the code
 * so that when we maintain it it's very clear what's going on. since 
 * restore needs to be called after disable everytime! Sort of like 
 * disable and enable interrupt in OS
 */
int _md_tog_detect(MotionDetector *md, int flag) {

    if(pthread_mutex_lock(&(md->lock)) != 0) {
            //can't lock ????
        return -1;
    }
    md->config->detect_flag = flag;
    pthread_mutex_unlock(&(md->lock));
    return 1;
}
int md_disable_detection(MotionDetector *md) {
    return _md_tog_detect(md, 0);
}
int md_enable_detection(MotionDetector *md) {
    return _md_tog_detect(md, 1);
}

int md_stop_thread(MotionDetector *md) {

    if(pthread_mutex_lock(&(md->lock)) != 0) {
            //can't lock ????
        return -1;
    }
    md->run_flag = 0;
    pthread_mutex_unlock(&(md->lock));
    return 1;
}

void *detection_loop(void* arg) {
    
    MotionDetector *md = (MotionDetector *) arg;

    while(1) {
        
        if(pthread_mutex_lock(&(md->lock)) != 0) {
            //can't lock ????
            return NULL;
        }
        int tmp_run_flag = md->run_flag;
        pthread_mutex_unlock(&(md->lock));
        
        if(!tmp_run_flag) {
            break;
        }

        
    
        int err = md_detect(md);
	    //printf("detect() err = %d\n", err);
     
    }

    //thread ends clean up code    
}
