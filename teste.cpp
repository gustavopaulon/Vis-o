#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<iostream>

using namespace std;
using namespace cv;

int iLowH = 0;
int iHighH = 0;

int iLowS = 0;
int iHighS = 0;

int iLowV = 0;
int iHighV = 0;

int c = 0;
Vec3i avg;

double dist(Point2f p, Point l){

  return (((p.x))/l.x)-1;

}

void onMouse(int event, int x, int y, int, void* data) {
	Mat* image = static_cast<Mat*>(data);
	double distLower = 0.5;
	double distHigher = 1.5;

	if (event == EVENT_RBUTTONDOWN) {
		avg.val[0] = 0;
		avg.val[1] = 0;
		avg.val[2] = 0;
		c = 0;
		return;
	} else if (event != EVENT_LBUTTONDOWN) {
		return;
	}

	Vec3b pixel = image->at<Vec3b>(y, x);

	for (int i = 0; i < 3; i++) {
		avg.val[i] = (avg.val[i] * c + pixel.val[i]) / (c + 1);
	}

	iLowH = avg.val[0] * distLower;
	iHighH = avg.val[0] * distHigher;

	iLowS = avg.val[1] * distLower;
	iHighS = avg.val[1] * distHigher;

	iLowV = avg.val[2] * distLower;
	iHighV = avg.val[2] * distHigher;

	Mat imgThresholded;

	inRange(*image, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV),
			imgThresholded); //pega entre os minimos e maximos para jogar no thresholded
	imshow("Frame", imgThresholded);

	c++;

	cout << pixel << avg << endl;
}

int main() {

  int thresh = 100;

  VideoCapture cap(0);

  Mat image;

  Mat imageHSV;


  while(true) {

    cap.read(image);
    flip(image, image, 1);
    Point center((image.cols)/2, (image.rows)/2);
    medianBlur(image, image, 7); //"embaça" a imagem
    cvtColor(image, imageHSV, CV_BGR2HSV); //muda o tipo de paleta de cor

    Mat imgThresholded;
    inRange(imageHSV, Scalar(iLowH, iLowS, iLowV), Scalar(iHighH, iHighS, iHighV),
        imgThresholded);

    Mat element = getStructuringElement(MORPH_RECT, Size(31, 31), Point(15, 15));

    erode(imgThresholded, imgThresholded, element);
    dilate(imgThresholded, imgThresholded, element);
    dilate(imgThresholded, imgThresholded, element);
    erode(imgThresholded, imgThresholded, element);

  Mat canny_output;
  vector<vector<Point> > contours;
  vector<Vec4i> hierarchy;

  /// Detect edges using canny
  //Canny(imgThresholded, canny_output, thresh, thresh*2, 3 );
  /// Find contours
  findContours( imgThresholded, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );

  /// Get the moments
  vector<Moments> mu(contours.size() );
  for( int i = 0; i < contours.size(); i++ ){
    mu[i] = moments( contours[i], false );
  }

  ///  Get the mass centers:
  vector<Point2f> mc( contours.size() );
  for( int i = 0; i < contours.size(); i++ )
     { mc[i] = Point2f( mu[i].m10/mu[i].m00 , mu[i].m01/mu[i].m00 ); }

  /// Draw contours
  for( int i = 0; i< contours.size(); i++ )
     {
       Scalar color = Scalar(0, 0, 255);
       drawContours( image, contours, i, color, 2, 8, hierarchy, 0, Point() );
       circle( image, mc[i], 4, color, -1, 8, 0 );
       line(image, mc[i], center, Scalar(0,255,0), 5);
      cout << dist(mc[i], center) <<endl;
     }


    imshow("Camera", image);
    //imshow("janela", canny_output);
    //imshow("Janela", imageHSV);
    imshow("Frame", imgThresholded);

    int tecla = waitKey(10);
    tecla = (char)tecla;
    switch(tecla){
      case 27:
        return 0;
      case 'c':
        setMouseCallback("Camera", &onMouse, &imageHSV);
        waitKey(0);
    }
  }
}
