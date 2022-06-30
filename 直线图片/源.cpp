#include <opencv2/highgui/highgui_c.h>
#include<opencv2/opencv.hpp>  
#include<stdio.h>
#include<iostream>  
#include <string>    
#include <vector>  
#include <time.h>
#include <ctime>
#include <math.h>

using namespace cv;
using namespace std;

double* LineMethod(Mat& img);

int main(int argc, char** argv) {
	//记录运行时间
	clock_t start = 0;
	clock_t end = 0;
	start = clock();

	//读入原图，返回Mat对象
	Mat src = imread("直线//2.bmp");
	Mat dst1, dstCopy1;

	//判断图片是否读入成功
	if (src.empty()) {
		printf("could not load image...");
		return -1;
	}

	//创建窗口并显示原图
	namedWindow("src", CV_WINDOW_AUTOSIZE);
	imshow("src", src);

	//打印原图像属性
	cout << "原图像属性" << endl;
	//图像尺寸
	cout << "size:" << src.size << endl;
	//列宽
	cout << "cols:" << src.cols << endl;
	//行高
	cout << "rows:" << src.rows << endl;
	//维度
	cout << "dims:" << src.dims << endl;
	//通道数，RGB图为3通道，灰度图为单通道
	cout << "channels:" << src.channels() << endl << endl;

	//拷贝图片，dst1用于图像处理，dstCopy1用于显示识别出的圆
	src.copyTo(dst1);
	src.copyTo(dstCopy1);
	
	//调整图像分辨率，减少后期图像处理的计算量，n=0.25表示图片的宽和高均原图的四分之一
	//width为调整分辨率后的图像宽度，heigth为调整分辨率后的图像高度
	float n = 0.25;
	int width = n * src.cols;
	int height = n * src.rows;
	Size size(width, height);
	resize(dst1, dst1, size);
	//namedWindow("resize", CV_WINDOW_AUTOSIZE);
	//imshow("resize", dst1);

	//ROI提取
	// 设图片的左上角坐标为（0,0）,右下角坐标为(1,1)
	// ROI的左上角坐标为(w1,h1),右下角坐标为(w2,h2)
	float w1 = 0.4, w2 = 0.7, h1 = 0.4, h2 = 0.75;
	Rect rect1(width * w1, height * h1, (w2 - w1) * width, (h2 - h1) * height);
	dst1 = dst1(rect1);
	namedWindow("ROI", CV_WINDOW_AUTOSIZE);
	imshow("ROI", dst1);
	
	//直线检测
	double* returnValue = LineMethod(dst1);
	//判断是否有两条直线，有则计算交点坐标
	if (returnValue[0] != -1 && returnValue[4] != -1) {
		double k1 = (returnValue[3] - returnValue[1]) / (returnValue[2] - returnValue[0]);
		double b1 = returnValue[1] - (returnValue[0] * k1);
		double k2 = (returnValue[7] - returnValue[5]) / (returnValue[6] - returnValue[4]);
		double b2 = returnValue[5] - (returnValue[4] * k2);

		
		long x=0, y=0;
		//两条直线是不平行，则计算交点
		if (k1 != k2) {
			x = (b2 - b1) / (k1 - k2);
			y = (k1 * b2 - b1 * k2) / (k1 - k2);
		}
		else {
			cout << "两条直线平行，交点超出图像范围" << endl;
			cv::waitKey(0);
			return -2;
		}
		
		//计算直线端点在原图像中的坐标
		int v0 = (w1 * width + returnValue[0]) / n;
		int v1 = (h1 * height + returnValue[1]) / n;
		int v2 = (w1 * width + returnValue[2]) / n;
		int v3 = (h1 * height + returnValue[3]) / n;
		int v4 = (w1 * width + returnValue[4]) / n;
		int v5 = (h1 * height + returnValue[5]) / n;
		int v6 = (w1 * width + returnValue[6]) / n;
		int v7 = (h1 * height + returnValue[7]) / n;

		//在dstCopy1上画出直线
		line(dstCopy1, Point(v0, v1), Point(v2, v3), Scalar(0, 0, 255), 2, LINE_AA);
		line(dstCopy1, Point(v4, v5), Point(v6, v7), Scalar(0, 0, 255), 2, LINE_AA);

		//交点像素横坐标和纵坐标
		long Abscissa = (w1 * width + x) / n;
		long Ordinate = (h1 * height + y) / n;
		
		//判断交点是否超出图像范围
		if (Abscissa < dstCopy1.cols && y < dstCopy1.rows) {
			cout << "交点横坐标：" << Abscissa << endl;
			cout << "交点纵坐标：" << Ordinate << endl;
			//在dstCopy1上画出交点
			circle(dstCopy1, Point(Abscissa , Ordinate ), 4, Scalar(0, 0, 255), 2, LINE_AA);
		}
		else {
			cout << "交点超出图像范围" << endl;
			cv::waitKey(0);
			return -3;
		}
		

		
		namedWindow("Point", CV_WINDOW_NORMAL);
		imshow("Point", dstCopy1);
	}
	else {
		cout << "直线不足两条" << endl;
		cv::waitKey(0);
		return -4;
	}


	end = clock();
	cout << endl << "运行总时间: " << end - start << "ms";

	cv::waitKey(0);
	return 0;
}

/*直线检测方法
* 参数：Mat& img（ROI图）
* 返回值：ROI图中计算出的两条线段的四个端点坐标
*/
double* LineMethod(Mat& img) {
	//创建一个数组，保存计算出的两条线段的四个端点坐标，作为函数的返回值
	static double value[8] = { -1,-1,-1,-1,-1,-1,-1,-1};
	Mat imgCopy;
	img.copyTo(imgCopy);

	//高斯模糊
	GaussianBlur(img, img, Size(3, 3), 0, 0);
	//namedWindow("GaussianBlur", CV_WINDOW_AUTOSIZE);
	//imshow("GaussianBlur", img);

	//转为灰度图
	cvtColor(img, img, CV_BGR2GRAY);


	//Canny边缘检测
	Canny(img, img, 80, 200);
	namedWindow("Canny", CV_WINDOW_AUTOSIZE);
	imshow("Canny", img);

	//霍夫直线变换
    //直线检测结果保存在plines中
	vector<Vec4f> plines;
	HoughLinesP(img, plines, 1, CV_PI / 180, 50, 30, 100);

	//遍历所有检测出的直线
	for (int i = 0; i < plines.size(); i++) {
		Vec4f houghLine = plines[i];
		
		//返回第一条和第二条条直线
		if (i == 0) {
			value[0] = houghLine[0];
			value[1] = houghLine[1];
			value[2] = houghLine[2];
			value[3] = houghLine[3];
		}
		if (i == 1) {
			value[4] = houghLine[0];
			value[5] = houghLine[1];
			value[6] = houghLine[2];
			value[7] = houghLine[3];
		}
		//在ROI图中画出所有的线
		line(imgCopy, Point(houghLine[0], houghLine[1]), Point(houghLine[2], houghLine[3]), Scalar(0, 0, 255), 1, LINE_AA);
		printf("ROI图中直线%d起点和终点坐标：（%.1f，%.1f）（%.1f，%.1f)\n", i, houghLine[0], houghLine[1], houghLine[2], houghLine[3]);
	}
	namedWindow("HoughLinesP", CV_WINDOW_AUTOSIZE);
	imshow("HoughLinesP", imgCopy);

	return value;
}