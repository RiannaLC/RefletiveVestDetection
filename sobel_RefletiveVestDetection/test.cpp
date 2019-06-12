#include<stdlib.h>
#include<stdio.h>
#include<opencv2\opencv.hpp>
#include<cmath>

using namespace std;
using namespace cv;

/* Load image and resize the image into one size. */
Mat loadImage(String IMG_PATH) {
	Mat img = imread(IMG_PATH);
	imshow("origin.jpg", img);
	resize(img, img, Size(200, 500));
	medianBlur(img, img, 7);
	return img;
}

/* Use a horizontal filter (h<<w) to convolve the binarized image.
   The propose is to connect horizonatlly inconsistent lines. */
Mat binaryFilter(Mat binary, int number) {
	int w = binary.cols;
	int h = binary.rows;
	int sizew = int(w / 15);
	int sizeh = 2;

	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	findContours(binary, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
	for (int i = 0; i < contours.size(); i++) {
		double area = contourArea(contours[i]);
		Rect ret1 = boundingRect(contours[i]);
		int x = ret1.x;
		int y = ret1.y;
		int ww = ret1.width;
		int hh = ret1.height;
		if (area <= 40 || area >= 500) {
			for (int m = y; m < y + hh; m++) {
				for (int n = x; n < x + ww; n++) {
					binary.at<uchar>(m, n) = 0;
				}
			}
		}
	}
	imshow("2", binary);

	for (int i = 0; i < h - sizeh; i += sizeh) {
		for (int j = number; j < w - sizew; j += sizew) {
			int sum = 0;
			for (int m = i; m < i + sizeh; m++) {
				for (int n = j; n < j + sizew; n++) {
					sum += binary.at<uchar>(m, n);
				}
			}
			if (sum / 255 >= sizeh*sizew / 4) {
				for (int m = i; m < i + sizeh; m++) {
					for (int n = j; n < j + sizew; n++) {
						binary.at<uchar>(m, n) = 255;
					}
				}
			}
			else {
				for (int m = i; m < i + sizeh; m++) {
					for (int n = j; n < j + sizew; n++) {
						binary.at<uchar>(m, n) = 0;
					}
				}
			}
		}
	}
	return binary;
}



String goalFinder(Mat binary) {
	Mat img;
	binary.copyTo(img);
	int w = img.cols;
	int h = img.rows;
	int sizew = int(w / 15);
	int sizeh = int(h / 5);

	vector<vector<Point>> contours;
	vector<Vec4i> hierarchy;
	vector<int> smallCon;
	vector<vector<int>> con;
	vector<double> area;
	int averageWidth = 0;
	findContours(binary, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
	int x, y, ww, hh;
	if (contours.size() != 0) {
		for (int i = 0; i < contours.size(); i++) {
			averageWidth += boundingRect(contours[i]).width;
		}
		averageWidth = int(averageWidth / contours.size());
	}
		
	for (int i = 0; i < contours.size(); i++) {
		x = boundingRect(contours[i]).x;
		y = boundingRect(contours[i]).y;
		ww = boundingRect(contours[i]).width;
		hh = boundingRect(contours[i]).height;
		if (ww <= averageWidth*0.75) {
			for (int m = y; m < y + hh; m++) {
				for (int n = x; n < x + ww; n++) {
					img.at<uchar>(m, n) = 0;
				}
			}
		}
	}
	imshow("111", img);

	vector<int> changeColorArr;
	int changeColor;
	int maxNbOfChange, sum;
	for (int i = int(h*0.15); i<int(h*0.85) - sizeh; i += int(sizeh / 4)) {
		for (int j = int(w*0.2); j<int(w*0.8) - sizew; j += int(sizew / 4)) {
			for (int n = 0; n < j + sizew - 1; n++) {
				changeColor = 0;
				for (int m = i; m < i + sizeh - 1; m++) {
					if (img.at<uchar>(m, n) != img.at<uchar>(m + 1, n)) {
						changeColor += 1;
					}
				}
				changeColorArr.push_back(changeColor);
			}

			sum = 0;
			for (int m = i; m < i + sizeh; m++) {
				for (int n = j; n < j + sizew; n++) {
					sum += img.at<uchar>(m, n);
				}
			}
			sum = int(sum/255);

			maxNbOfChange = 0;
			for (int k = 0; k < changeColorArr.size(); k++) {
				if (changeColorArr[k] > maxNbOfChange) {
					maxNbOfChange = changeColorArr[k];
				}
			}
			maxNbOfChange /=2;

			if (sum <= sizeh * sizew * 0.8 && sum >= sizeh * sizew / 10 && maxNbOfChange < 8 && maxNbOfChange > 3) {
				printf("Staff member\n");
				return "yes";
			}
		}
	}
	printf("Padastrains\n");
	return "no";
}



String search(Mat img) {
	int w = img.cols;
	int h = img.rows;
	Mat gray, sobel_x, sobel_y;
	cvtColor(img, gray, COLOR_BGR2GRAY);
	Sobel(gray, sobel_x, CV_64F, 1, 0, 3, 1, 0, BORDER_DEFAULT);
	Sobel(gray, sobel_y, CV_64F, 0, 1, 3, 1, 0, BORDER_DEFAULT);
	convertScaleAbs(sobel_x, sobel_x);
	convertScaleAbs(sobel_y, sobel_y);

	for (int r = 0; r < h; r++) {
		for (int c = 0; c < w; c++) {
			if (r > h*0.85 || r<h*0.15 || c>w*0.8 || c < w*0.2) {
				sobel_y.at<uchar>(r, c) = 0;
			}
			if (sobel_x.at<uchar>(r, c) > 50) {
				sobel_y.at<uchar>(r, c) = 0;
			}
		}
	}

	Mat binary;
	threshold(sobel_y, binary, 150, 255, THRESH_OTSU);

	GaussianBlur(binary, binary,Size(7,7),1.0);
	imshow("-1", binary);
	medianBlur(binary, binary, 11);
	imshow("0", binary);

	threshold(sobel_y, binary, 100, 255, THRESH_OTSU);
	imshow("1", binary);

	Mat element1 = getStructuringElement(MORPH_RECT, Size(13, 3));
	dilate(binary, binary, element1);//≈Ú’Õ

	Mat element2 = getStructuringElement(MORPH_RECT, Size(13, 3));
	erode(binary, binary, element2);//∏Ø ¥
	imshow("fs", binary);

	Mat afterBinaryFilter = binaryFilter(binary, 0);
	imshow("afterBinaryFilter", afterBinaryFilter);

	String goal = goalFinder(afterBinaryFilter);
	Point point;
	point.x = 0;
	point.y = 40;
	if (goal == "yes") {
		putText(img, "Yes", point, FONT_HERSHEY_COMPLEX, 1.2, (0, 255, 0), 2);
		/*imshow("result", img);
		waitKey(0);*/
		return "yes";
	}
	else {
		putText(img, "no", point, FONT_HERSHEY_COMPLEX, 1.2, (0, 0, 255), 2);
		/*imshow("result", img);
		waitKey(0);*/
		return "no";
	}
}


int main() {
	vector<String> files;
	vector<String> nfiles;
	String IMG_PATH = "C:\\Users\\kl\\Desktop\\BOT\\yes\\*.jpg";
	glob(IMG_PATH, files);
	size_t count = files.size();
	int nbOfYes = 0;
	int nbOfNo = 0;
	int total = 0;

	for (int i = 0; i < count; i++) {
		Mat img = loadImage(files[i]);
		String result = search(img);
		if (result == "yes") {
			nbOfYes++;
			total++;
		}
		else {
			nbOfNo++;
			total++;
		}
	}

	int nbOfYes1 = 0;
	int nbOfNo1 = 0;
	vector<String> files1;
	vector<String> nfiles1;
	String IMG_PATH1 = "C:\\Users\\kl\\Desktop\\BOT\\no\\*.jpg";
	glob(IMG_PATH1, files1);
	size_t count1 = files1.size();

	for (int i = 0; i < count1; i++) {
		Mat img1 = loadImage(files1[i]);
		String result1 = search(img1);
		if (result1 == "no") {
			nbOfYes1++;
			total++;
		}
		else {
			nbOfNo1++;
			total++;
		}
	}

	cout << "The total number of people in the test is " << total << "." << endl;
	cout<< "The number of staff member and pedestrians in the data set is " << nbOfNo + nbOfYes << " and " << nbOfNo1 + nbOfYes1 << " respectively." << endl;
	cout<< "Correctly detected "<<nbOfYes<<" staff members and "<< nbOfNo <<" pedestrians. The total correct rate of detection is: "<< (nbOfYes + nbOfYes1)/(nbOfNo + nbOfYes + nbOfYes1 + nbOfNo1)*100 << "%." <<endl;
	waitKey(0);
	return 0;
}


