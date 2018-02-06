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
//using namespace cv;
using namespace cv::ml;

typedef struct {
	cv::Mat * img;
	int size = 0;
	bool LButtonDown = false;
	bool bExit = false;
	vector < cv::Point > coord;
} BM;
int thresh = 50;

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


int main() {

	string imgTestFileName = "./Nital/frame-1N.jpg";

	cv::Mat img = cv::imread(imgTestFileName.c_str(), 0), img_gray;

	// Convert image to gray
	if (img.channels() == 3) {
		cvtColor(img, img_gray, CV_BGR2GRAY);
	}
	else {
		img_gray = img.clone();
	}

	cv::namedWindow("inputFromImage", cv::WINDOW_NORMAL);
	cv::imshow("inputFromImage", img_gray);

	BM mouseOut;
	mouseOut.img = &img_gray;
	cv::setMouseCallback("inputFromImage", onMouse, (void *)(&mouseOut));

	while (!mouseOut.bExit)
	{
		cv::waitKey(30);
	}

	int thickness = 4;
	int lineType = 8;

	// for accessing all the files in the directory
	//https://stackoverflow.com/questions/612097/how-can-i-get-the-list-of-files-in-a-directory-using-c-or-c/37494654#37494654
	string path = "./Label/";
	if (!fs::is_directory(path)) {	// if directory is not there then create it
		fs::create_directory(path);
	}

	vector < string > fileNames;
	for (auto & p : fs::directory_iterator(path)) {	// look for all csv files in that
		string s;
		s = p.path().string();
		int len = s.length();
		int plen = path.length();
		if (len > 9) {
			if ((s[len - 4] == '.') && (s[len - 3] == 'c') && (s[len - 2] == 's') && (s[len - 1] == 'v')) {
				//cout << s.substr(plen, len - 4) << endl;;
				if ((s[plen + 0] == 'l') && (s[plen + 1] == 'a') && (s[plen + 2] == 'b') && (s[plen + 3] == 'e') && (s[plen + 4] == 'l')) {
					fileNames.push_back(s.substr(plen, len - 4));
					//cout << s.substr(plen, len - 4) << endl;
				}
			}
		}
	}

	vector < int > fileNumbers;
	for (int i = 0; i < fileNames.size(); i++) {
		if (1) {	// add check cond if all the chars are digits
			fileNumbers.push_back(stoi(fileNames[i].substr(5)));
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
	// for labeling images
	// to be written in metadata file
	int numberOfClasses = 3;
	int classLabel = 1;

	//string imgTestFileName = "./Nital/frame-1N.jpg";
	stringstream ssF(imgTestFileName);
	string item;
	while (getline(ssF, item, '/')) {}



	string outFileName;
	outFileName = "./Label/label" + to_string(fileNumberGiven) + ".csv";
	cout << "Writing to ::" << outFileName << endl;
	ofstream my_file;
	my_file.open(outFileName.c_str());
	my_file << item << endl;
	my_file << numberOfClasses << "," << classLabel;

	for (auto i : mouseOut.coord) {
		my_file << "\n" << i.x << ", " << i.y;
		//cv::line(img_gray, i, i+cv::Point(i.x+10, i.y+10), 255, thickness, lineType);
	}
	my_file.close();

	cout << img.rows << " - " << img.cols << endl;

	int wait;
	cin >> wait;

	return 0;
}