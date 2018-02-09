#include <opencv2/opencv.hpp>
#include <set>
#include <algorithm>


#include <iostream>
#include <fstream>				// for writing to files
#include <string>
#include <sstream>

#include<opencv2/opencv.hpp>
#include <stdio.h>
#include <stdlib.h>

#include "opencv2/highgui/highgui.hpp" // for mouse clicks and ui
// for time
#include <ctime>
#include <conio.h>

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

using namespace std;
using namespace cv;
using namespace cv::ml;

typedef struct {
	cv::Mat * img;
	int size = 0;
	bool LButtonDown = false;
	bool bExit = false;
	vector < cv::Point > coord;
} BM;

void onMouse(int event, int x, int y, int flags, void* param)
{
	/**
	@brief updates param pointer with the points from image after left click
	@params param contains the flags to avoid global variables and the 2D points
	*/
	//LButtonDown = false;
	BM* par = (BM *)param;

	cv::Mat * img1 = (par->img);

	if (event == cv::MouseEventTypes::EVENT_LBUTTONDOWN)
	{
		//cout << "Left button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
		(par->coord).push_back(cv::Point(x, y));
		par->LButtonDown = true;
		cout << par->coord[0] << endl;
		par->size = 1;
	}
	if ((event == cv::MouseEventTypes::EVENT_MOUSEMOVE) && par->LButtonDown)
	{
		//cout << "Mouse move over the window - position (" << x << ", " << y << ")" << endl;
		if (((x > 0) && (x < img1->cols)) && ((y > 0) && (y < img1->rows))) {
			(par->coord).push_back(cv::Point(x, y));
			cv::line( *img1, par->coord[par->size - 1],par->coord[par->size], 0, 2, 8);
			(par->size)++;
			//cout << par->coord[par->size-1] << endl;
			//cout << par->size << endl;
			cv::imshow("inputFromImage", *img1);
		}
	}
	if (event == cv::MouseEventTypes::EVENT_LBUTTONUP)
	{
		cout << "Left button of the mouse is unclicked - position (" << x << ", " << y << ")" << endl;
		par->LButtonDown = false;
		cv::line(*img1, par->coord[par->size-1], par->coord[0], 0, 2, 8);
		cv::imshow("inputFromImage", *img1);
		//coord.push_back(Point(y, x));
		// Need to put an exit flag here - Done
		// idea taken from
		// https://stackoverflow.com/questions/34413864/opencv-return-value-from-setmousecallback-in-for-loop
		bool* exitflag = &(par->bExit);
		*exitflag = true;
	}
	//cout << endl << (par->coord).size() << endl;
}


void showLabeledImage(string imageName, int numberOfClasses) {
	
	//cv::destroyAllWindows();
	
	cv::Mat img = cv::imread(imageName.c_str(), 1), img_color;

	// Convert image to color
	if (img.channels() == 1) {
		cv::cvtColor(img, img_color, CV_GRAY2BGR);
	}
	else {
		img_color = img.clone();
	}

	cv::Mat img_color_filled = img_color.clone();
	cv::Mat img_color_filled_clone = img_color.clone();

	cv::namedWindow("ShowLabeledRegions", cv::WINDOW_NORMAL);

	string path = "./Region/";
	if (!fs::is_directory(path)) {	// if directory is not there then create it
		fs::create_directory(path);
	}

	stringstream ss(imageName);	// to remove path from image name
	string imageName_;
	while (getline(ss, imageName_, '/')) {}

	bool isFirstTimeForRegion = false;
	string imagePath = path + imageName_ + '/';

	if (!fs::is_directory(imagePath)) {	// if directory is not there then create it mydir/imgName/
		isFirstTimeForRegion = true;
		fs::create_directory(imagePath);
	}

	vector < string > fileNames;
	vector < int > fileNumbers;

	if (isFirstTimeForRegion) { // first time
		cv::imshow("ShowLabeledRegions", img_color);
		return;	// no previous regions
	}
	else {
		for (auto & p : fs::directory_iterator(imagePath)) {	// look for all csv files in that
			string s;
			s = p.path().string();
			int len = s.length();
			int plen = imagePath.length();
			if (len > 10) {  // region*.csv
				if ((s[len - 4] == '.') && (s[len - 3] == 'c') && (s[len - 2] == 's') && (s[len - 1] == 'v')) {
					if ((s[plen + 0] == 'r') && (s[plen + 1] == 'e') && (s[plen + 2] == 'g') && (s[plen + 3] == 'i') && (s[plen + 4] == 'o') && (s[plen + 5] == 'n')) {
						fileNames.push_back(s.substr(plen, len - 4));
					}
				}
			}
		}

		for (int i = 0; i < fileNames.size(); i++) {
			cout << fileNames[i] << endl;
			if (1) {	// add check cond if all the chars are digits
				fileNumbers.push_back(stoi(fileNames[i].substr(6)));
			}
		}
		sort(fileNumbers.begin(), fileNumbers.end());
	}

	// show previously labeled data
	string labeledFileNames;
	ifstream my_file_to_read;
	string curr_img_name_read;
	string line;
	char delim;
	int xx, yy;
	int centroidx, centroidy;
	int num_of_class_read, label_read;

	for (int i = 0; i < fileNumbers.size(); i++) {
		labeledFileNames = imagePath + "region" + to_string(fileNumbers[i]) + ".csv";
		vector <cv::Point> vp;

		my_file_to_read.open(labeledFileNames.c_str());

		my_file_to_read >> num_of_class_read >> delim >> label_read;
		//cout << "Classes::" << num_of_class_read << " Class Label::" << label_read << endl;
		if (numberOfClasses != num_of_class_read) {	// different training data
			continue;
		}
		// based on number of classes and class label decide color code
		cv::Scalar mask_colour;
		if (num_of_class_read <= 3) {
			if (label_read == 1) {
				mask_colour = Scalar(255, 0, 0);
			}
			else if (label_read == 2) {
				mask_colour = Scalar(0, 255, 0);
			}
			else {
				mask_colour = Scalar(0, 0, 255);
			}
		}
		else {
			mask_colour = Scalar(255, 0, 0);
		}

		my_file_to_read >> centroidx >> delim >> centroidy;
		while (my_file_to_read >> xx >> delim >> yy) {
			vp.push_back(cv::Point(xx, yy));
		}
		int total_points = vp.size();
		for (int j = 0; j < total_points; j++) {
			cv::line(img_color_filled, vp[j], vp[(j + 1) % total_points], cv::Scalar(0,0,0), 1, 8);
		}
		vector < vector < cv::Point > > tmpp1;
		tmpp1.push_back(vp);
		cv::fillPoly(img_color_filled_clone, tmpp1, mask_colour, 8, 0); // fill the inside by 0's
																		   //cv::waitKey(1);
		cv::putText(img_color_filled, "R"+to_string(fileNumbers[i]), cv::Point(centroidx, centroidy), cv::FONT_HERSHEY_SCRIPT_COMPLEX, 0.8, cv::Scalar(0, 0, 0), 2, 8);

		my_file_to_read.close();
	}

	double alpha = 0.3;
	//https://stackoverflow.com/questions/24480751/how-to-create-a-semi-transparent-shape
	cv::addWeighted(img_color_filled_clone, alpha, img_color_filled, 1.0 - alpha, 0, img_color_filled);

	cv::imshow("ShowLabeledRegions", img_color_filled);
	cv::imwrite(imagePath + "regionsSelected.jpg", img_color_filled);

	return;
}


void polygonFromMouse(string imageName, int numberOfClasses, int classLabel, bool selectionOnRegionImg) {

	bool flag_regionImgExist = false;
	string imageNameRegion(imageName);
	cv::Mat img_region;

	if (selectionOnRegionImg) {	// if user want to draw region on regional img
		
		stringstream ssT(imageName);	// to remove path from image name
		string imageName_T;
		while (getline(ssT, imageName_T, '/')) {}

		imageNameRegion = "./Region/" + imageName_T + '/' + "regionsSelected.jpg";	// the img with regions
	
		if(fs::exists(imageNameRegion)){
			flag_regionImgExist = true;
			img_region = cv::imread(imageNameRegion.c_str(), 1);
		}
	}

	cv::destroyAllWindows();

	cv::Mat img = cv::imread(imageName.c_str(),1), img_color;
	
	// Convert image to color
	if (img.channels() == 1) {
		cv::cvtColor(img, img_color, CV_GRAY2BGR);
	}
	else {
		img_color = img.clone();
	}

	cv::Mat img_color_filled = img_color.clone();
	cv::Mat img_color_filled_clone = img_color.clone();


	cv::namedWindow("inputFromImage", cv::WINDOW_NORMAL);
	BM mouseOut;

	if (flag_regionImgExist) {
		cv::imshow("inputFromImage", img_region);
		mouseOut.img = &img_region;
	}
	else {
		cv::imshow("inputFromImage", img_color);
		mouseOut.img = &img_color;
	}
	
	cv::setMouseCallback("inputFromImage", onMouse, (void *)(&mouseOut));

	while (!mouseOut.bExit)
	{
		cv::waitKey(30);
	}


	string path = "./Region/";
	if (!fs::is_directory(path)) {	// if directory is not there then create it
		fs::create_directory(path);
	}


	stringstream ss(imageName);	// to remove path from image name
	string imageName_;
	while (getline(ss, imageName_, '/')) {}

	bool isFirstTimeForRegion = false;
	string imagePath = path + imageName_ + '/';
	
	if (!fs::is_directory(imagePath)) {	// if directory is not there then create it mydir/imgName/
		isFirstTimeForRegion = true;
		fs::create_directory(imagePath);
	}


	string outFileName;
	if (isFirstTimeForRegion) { // first time
		outFileName = imagePath + "region" + to_string(1) + ".csv";
		cout << "Writing to ::" << outFileName << endl;

	}
	else {
		vector < string > fileNames;
		for (auto & p : fs::directory_iterator(imagePath)) {	// look for all csv files in that
			string s;
			s = p.path().string();
			int len = s.length();
			int plen = imagePath.length();
			if (len > 10) {  // region*.csv
				if ((s[len - 4] == '.') && (s[len - 3] == 'c') && (s[len - 2] == 's') && (s[len - 1] == 'v')) {
					if ((s[plen + 0] == 'r') && (s[plen + 1] == 'e') && (s[plen + 2] == 'g') && (s[plen + 3] == 'i') && (s[plen + 4] == 'o') && (s[plen + 5] == 'n')) {
						fileNames.push_back(s.substr(plen, len - 4));
					}
				}
			}
		}

		vector < int > fileNumbers;
		for (int i = 0; i < fileNames.size(); i++) {
			//cout << fileNames[i] << endl;
			if (1) {	// add check cond if all the chars are digits
				fileNumbers.push_back(stoi(fileNames[i].substr(6)));
			}
		}
		sort(fileNumbers.begin(), fileNumbers.end());

		int fileNumberGiven;
		if (fileNumbers.size()) {
			fileNumberGiven = fileNumbers[fileNumbers.size() - 1] + 1;
		}
		else {
			fileNumberGiven = 1;
		}

		outFileName = imagePath + "region" + to_string(fileNumberGiven) + ".csv";
		cout << "Writing to ::" << outFileName << endl;
	}


	ofstream my_file;
	my_file.open(outFileName.c_str());
	my_file << numberOfClasses << "," << classLabel;

	int centroidx, centroidy;
	centroidx = centroidy = 0;
	for (auto i : mouseOut.coord) {
		centroidx += i.x;
		centroidy += i.y;
	}
	centroidx /= mouseOut.coord.size();
	centroidy /= mouseOut.coord.size();

	my_file << "\n" << centroidx << "," << centroidy;

	for (auto i : mouseOut.coord) {
		my_file << "\n" << i.x << ", " << i.y;
	}
	my_file.close();

	cv::destroyAllWindows();
	vector < vector < cv::Point > > tmpp;
	//cv::waitKey(0);

	tmpp.push_back(mouseOut.coord);
	cv::namedWindow("Region Selected", cv::WINDOW_NORMAL);
	cv::imshow("Region Selected", img_color);
	cv::waitKey(0);
	cv::imwrite(imagePath+"regions.jpg", img_color);

	showLabeledImage(imageName, numberOfClasses);

	return;
}

int main() {

	//string imgTestFileName = "./Nital/frame-1N.jpg";
	string imgTestFileName = "./Coal/364-2jpg.jpg";
	//string imgTestFileName = "./Coal/364-4jpg0002.jpg";
	//string imgTestFileName = "./Coal/364-5jpg0003.jpg";

	polygonFromMouse(imgTestFileName, 3, 1, true);
	//polygonFromMouse(imgTestFileName, 3, 2);
	//polygonFromMouse(imgTestFileName, 3, 3);
	//showLabeledImage(imgTestFileName, 3);


	cv::waitKey(0);

	return 0;
}
