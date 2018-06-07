#include <stdio.h>
#include <iostream>
#include <climits>
#include <boost/lexical_cast.hpp>
#include "opencv2/opencv.hpp"
#include "opencv2/xfeatures2d.hpp"
#include "opencv2/highgui.hpp"
#include "imagedescripted.h"


using namespace std;
using namespace cv;

vector<ImageDescripted> patterns;


int showKeypoints( Mat frame, vector<KeyPoint> keypoints, int time){
	
	int key;
	
	namedWindow("Points detected", cv::WINDOW_AUTOSIZE);
	cv::Ptr<Feature2D> f2d = xfeatures2d::SIFT::create();
	
	int r = 4;
	
	
	for( size_t i = 0; i < keypoints.size(); i++ )
	{ 
		circle( frame, keypoints[i].pt, r, Scalar(0,0,255));
		
	}
	
	imshow("Points detected", frame);
	
	key = waitKey(0);
	
	destroyWindow("Points detected");
	
	
	return key;
	
}



void setLabel(cv::Mat& im, const std::string label, const cv::Point &point)
{
	int fontface = cv::FONT_HERSHEY_SIMPLEX;
	double scale = 0.4;
	int thickness = 1;
	int baseline = 0;
	
	cv::Size text = cv::getTextSize(label, fontface, scale, thickness, &baseline);
	cv::rectangle(im, point + cv::Point(0, baseline), point + cv::Point(text.width, -text.height), CV_RGB(255,0, 0), CV_FILLED);
	cv::putText(im, label, point, fontface, scale, CV_RGB(255,255,255), thickness, 8);
}

Rect2i roi;
Rect2i drawing_roi;

void CallBackFunc(int event, int x, int y, int flags, void* userdata)
{
     if  ( event == EVENT_LBUTTONDOWN )
     {
	roi.x = x;
	roi.y = y;
	drawing_roi.x = x;
	drawing_roi.y = y;
     }
     else if  ( event == EVENT_LBUTTONUP )
     {
        roi.width = x - roi.x;
	roi.height = y - roi.y;
	cout<< "roi "<<roi.x << " " << roi.y << " " << roi.width <<" " << roi.height << endl;
     }
     else if ( event == EVENT_MOUSEMOVE )
     {
       if(drawing_roi.x != -1 && drawing_roi.y != -1)
       {
	  drawing_roi.width = x - drawing_roi.x;
	  drawing_roi.height = y - drawing_roi.y;
	  cout<< "drawing_roi  "<<drawing_roi.x << " " << drawing_roi.y << " " << drawing_roi.width <<" " << drawing_roi.height << endl;
       }

     }
}

vector<ImageDescripted>  learnObjects (VideoCapture cap){

  
  Mat frame;
  Ptr<Feature2D> f2d;
  vector<KeyPoint> keypoints;
  Mat descriptors;
  int key;
  int x=0;
  
  //getting a frame
  namedWindow("a: add object, c:continue", cv::WINDOW_AUTOSIZE);
  while(true)
  {
  // get a new frame from camera
    cap >> frame; 
    imshow("a: add object, c:continue", frame);
    
	key =  waitKey(30);
    if( key== 'c') {
		destroyWindow("a: add object, c:continue");
		break;
	}
	
	if( key== 'a'){
	  
		roi = Rect2i(-1, -1, -1, -1);
		drawing_roi = Rect2i(-1, -1, -1, -1);
		namedWindow("cut input", cv::WINDOW_AUTOSIZE);
		setMouseCallback("cut input", CallBackFunc, NULL);
		
		
		while(!(roi.x != -1 && roi.y != -1 && roi.height !=-1 && roi.width != -1)){
		   
		  Mat toDraw = frame.clone();
		  if(drawing_roi.x != -1 && drawing_roi.y != -1){
		    rectangle(toDraw,drawing_roi, CV_RGB(255,0,0));
		  }
		  imshow("cut input", toDraw);
		  cvWaitKey(30);
		}
		destroyWindow("cut input");
		
		Mat tmp = frame(roi);
		frame = tmp;
// 		imshow("cut input", frame);
// 		cvWaitKey(0);
		
		f2d = xfeatures2d::SIFT::create();
		f2d->detect( frame, keypoints );
		f2d->compute( frame, keypoints, descriptors );
		
		
		string name;
		cout << "Object name: ";
		cin >> name;
		
		//ImageDescripted imgDesc=  ImageDescripted(frame.clone(), keypoints, descriptors, boost::lexical_cast<string>(x++) );
		ImageDescripted imgDesc=  ImageDescripted(frame.clone(), keypoints, descriptors, name );
		showKeypoints(imgDesc.getFrame(),imgDesc.getKeypoints(),0);
		
		patterns.push_back(imgDesc);
	}
  }
  
 

  return patterns;

}


void analizeObject (vector<Point2f>  scene_object, int &min_x, int &max_x, float &average_y ){
	min_x = INT_MAX;
	max_x = INT_MIN;
	average_y = 0;
	
	for(int i = 0; i < scene_object.size(); i++){
		average_y += ( scene_object[i].y / scene_object.size() );
		if (scene_object[i].x > max_x ){
			max_x = scene_object[i].x;
		}
		if(scene_object[i].x < min_x ){
			min_x = scene_object[i].x;
		}
	}
}

vector < vector <ImageDescripted> > analizeScene(vector<ImageDescripted> scene_objects){
	vector < vector <ImageDescripted> > objects_arranged;

	if (scene_objects.size()==0){
		return objects_arranged;
	}
	
	
	ImageDescripted initial_object = scene_objects.back();
	scene_objects.pop_back();
	
	vector <ImageDescripted> inititial_pile;	
	inititial_pile.push_back(initial_object);
	objects_arranged.push_back(inititial_pile);
	
	while (scene_objects.size() != 0)
	{
		
		ImageDescripted unordered_object = scene_objects.back();
		scene_objects.pop_back();
		
		bool addedToPile = false;
		
		for(int pile_num = 0; pile_num < objects_arranged.size(); pile_num++ ){
			
			
			int min_x_inpile =  INT_MAX;
			int max_x_inpile =  INT_MIN;
			
			for(int object_on_pile = 0 ; object_on_pile < objects_arranged[pile_num].size(); object_on_pile++ ){
				if ( objects_arranged[pile_num][object_on_pile].max_x   >   max_x_inpile ){
					max_x_inpile = objects_arranged[pile_num][object_on_pile].max_x;
				}
				if( objects_arranged[pile_num][object_on_pile].min_x  <   min_x_inpile ){
					min_x_inpile = objects_arranged[pile_num][object_on_pile].min_x;
				}
				
			}
			
			if( (unordered_object.min_x >= min_x_inpile && unordered_object.min_x <= max_x_inpile) || 
				(unordered_object.max_x >= min_x_inpile && unordered_object.max_x <= max_x_inpile  ) ||
				(unordered_object.min_x < min_x_inpile && unordered_object.max_x > max_x_inpile)	){
				for (vector <ImageDescripted>::iterator it = objects_arranged[pile_num].begin() ; it != objects_arranged[pile_num].end(); ++it){
					if(unordered_object.average_y > (*it).average_y ){
						objects_arranged[pile_num].insert(it, unordered_object);
						addedToPile = true;
					}
				}
				if (addedToPile ==false){
					objects_arranged[pile_num].push_back(unordered_object);
					addedToPile = true;
				}
			}
		}
		if (addedToPile ==false){
			//its a new pile of objects
			vector <ImageDescripted> new_pile;	
			new_pile.push_back(unordered_object);
			objects_arranged.push_back(new_pile);
			
		}
	}
	
	return objects_arranged;

	
}


int main(int argc, char *argv[])
{
	
	int key;
  
    RNG rng(12345);
    VideoCapture cap(1);
    if(!cap.isOpened())  // check if we succeeded
      return -1;
    
    patterns = learnObjects(cap);
        
    Ptr<Feature2D> f2d;
	Mat frame;
	bool break_while;
	
	cap >> frame; 
	
	
	vector<ImageDescripted> scene_objects;
	
	
	f2d = xfeatures2d::SIFT::create();
	break_while = false;
    
	
	while(break_while == false)
    {
		
        vector<KeyPoint> keypoints;
		Mat descriptors;    
		Mat img_matches;
		
		cap >> frame; // get a new frame from camera
		
		f2d->detect( frame, keypoints );
		f2d->compute( frame, keypoints, descriptors);
		
		img_matches = frame.clone();
		
		scene_objects.clear();
		
		for(int i=0 ; i < patterns.size(); i ++){
			ImageDescripted pattern = patterns[i];
		
			FlannBasedMatcher matcher= FlannBasedMatcher();
			vector< DMatch > matches;
			matcher.match	( pattern.getDescriptors(), descriptors, matches );
			
			//cv::hconcat(pattern.getFrame(), frame, img_matches);
			
			//-- Filter matches
			const double kDistanceCoef = 3.0;
			std::sort(matches.begin(), matches.end());
			while (matches.front().distance * kDistanceCoef < matches.back().distance) {
				matches.pop_back();
			}
			
			
			if(matches.size()>4){
				//-- Localize the object
				vector<Point2f> obj;
				vector<Point2f> scene;	
				Mat mask;
				Mat H;
				
				for( size_t i = 0; i < matches.size(); i++ )
				{
					//-- Get the keypoints from the good matches
					obj.push_back( pattern.getKeypoints()[ matches[i].queryIdx ].pt );
					scene.push_back( keypoints[ matches[i].trainIdx ].pt );
				}
				
				
				H = findHomography( obj, scene, RANSAC,3, mask);
			
				int num_inliers=0;
				vector< DMatch > good_matches;
				for( int i = 0; i < matches.size(); i++ )
				{
					if((unsigned int)mask.at<uchar>(i)==1){
						num_inliers++;
						good_matches.push_back(matches[i]);
					}
				}
// 				cout << "inliers/martches: "<< num_inliers<<"/"<<matches.size() <<endl;
				
				
/*				
				Mat drawed;
				drawMatches( pattern.getFrame(), pattern.getKeypoints(), frame, keypoints,
							 good_matches, drawed, Scalar::all(-1), Scalar::all(-1),
							 std::vector<char>(), DrawMatchesFlags::DRAW_RICH_KEYPOINTS );
				namedWindow("Points matched draw", cv::WINDOW_AUTOSIZE);
				imshow("Points matched draw", drawed);
				waitKey(30);*/
				
				
				//-- Get the corners from the image_1 ( the object to be "detected" )
				if(num_inliers>10){
					
					vector<Point2f> scene_corners(4);
					
					vector<Point2f> obj_corners(4);
					obj_corners[0] = cvPoint(0,0); 
					obj_corners[1] = cvPoint( pattern.getFrame().cols, 0 );
					obj_corners[2] = cvPoint( pattern.getFrame().cols, pattern.getFrame().rows ); 
					obj_corners[3] = cvPoint( 0, pattern.getFrame().rows );
					perspectiveTransform( obj_corners, scene_corners, H);
					
// 					cout << " Area: " << contourArea(scene_corners) << std::endl;
					if(contourArea(scene_corners)>500){
						//-- Draw lines between the corners (the mapped object in the scene - image_2 )
						line( img_matches, scene_corners[0], scene_corners[1], CV_RGB(255,0, 0), 2 );
						line( img_matches, scene_corners[1], scene_corners[2], CV_RGB(255,0, 0), 2 );
						line( img_matches, scene_corners[2] , scene_corners[3], CV_RGB(255,0, 0), 2 );
						line( img_matches, scene_corners[3] , scene_corners[0], CV_RGB(255,0, 0), 2 );
						setLabel(img_matches, pattern.getName(),scene_corners[0]);
						
						int min_x, max_x;
						float average_y;
						analizeObject (scene_corners,min_x, max_x,average_y );
						pattern.average_y = average_y;
						pattern.min_x = min_x;
						pattern.max_x = max_x;
						pattern.scene_corners = scene_corners;
						
						scene_objects.push_back(pattern);

						
					}
// 					else{
// 						cout<< pattern.getName()<<" not found"<<endl;
// 					}
				}
// 				else{
// 					cout<< pattern.getName()<<" not found"<<endl;
// 				}
			}
			
		}
		
		//-- Show detected matches
		namedWindow("Points matched", cv::WINDOW_AUTOSIZE);
		imshow("Points matched", img_matches);
		
		
		key = waitKey(30);
        if( key == 27) {
			break_while = true;
			break;
		}
		
		if(key == 'a'){
			vector < vector <ImageDescripted> > objects_arranged = analizeScene(scene_objects);
			cout<<"piles: "<<objects_arranged.size();
		}
		
		if( key == 's'){
			FileStorage fs("Keypoints.yml", FileStorage::WRITE); 
			fs << "strings" << "[";
			for(int i=0 ; i < patterns.size(); i ++){
				fs << patterns[i].getName(); 
			}
			fs << "]"; 
			
			for(int i=0 ; i < patterns.size(); i ++){
							
				fs << "keypoints_" + patterns[i].getName()<< patterns[i].getKeypoints();
				fs << "descriptors_" + patterns[i].getName() << patterns[i].getDescriptors();
				fs << "image_" + patterns[i].getName() << patterns[i].getFrame();
			}
			fs.release();
		}
		
		if( key == 'l'){
			FileStorage fs("Keypoints.yml", FileStorage::READ);
			
			FileNode n = fs["strings"];                         // Read string sequence - Get node
			if (n.type() != FileNode::SEQ)
			{
				cout << "strings is not a sequence! FAIL" << endl;
				return 1;
			}
			
			patterns.clear();
			
			FileNodeIterator it = n.begin(), it_end = n.end(); // Go through the node
			for (; it != it_end; ++it){
				string name = (string)*it;
				
				vector<KeyPoint> read_keypoint;
				Mat read_descriptors;
				Mat read_frame;
			
				fs["keypoints_"+name] >> read_keypoint;		
				fs["descriptors_"+name] >> read_descriptors;
				fs["image_"+name] >> read_frame;
				
				ImageDescripted pattern = ImageDescripted(read_frame, read_keypoint,read_descriptors,name);
				
				patterns.push_back(pattern);
			}

			fs.release();
		}
    }
    

    
    
    // the camera will be deinitialized automatically in VideoCapture destructor
    return 0;
  
    
  
}




