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

int* circleMethod1(Mat& img);
int* circleMethod2(Mat& img);


int main(int argc, char** argv) {
	//记录运行时间
	clock_t start = 0;
	clock_t end = 0;
	start = clock();

	//读入原图，返回Mat对象
	Mat src = imread("2021-01-30//NG-2021-01-30 09;06;18.jpg");
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

	//拷贝图片（深拷贝），用于图像处理
	src.copyTo(dst1);

	//调整图像分辨率，减少后期图像处理的计算量，n=0.25表示图片的宽和高均原图的四分之一
	//width为调整分辨率后的图像宽度，heigth为调整分辨率后的图像高度
	float n = 0.25;
	int width = n * src.cols;
	int height = n * src.rows;
	Size size(width, height);
	resize(dst1, dst1, size);
	//拷贝图片（深拷贝），用于显示识别出的圆
	dst1.copyTo(dstCopy1);
	//namedWindow("resize", CV_WINDOW_AUTOSIZE);
	//imshow("resize", dst1);

	//ROI提取
	// 设图片的左上角坐标为（0,0）,右下角坐标为(1,1)
	//ROI的左上角坐标为(w1,h1),右下角坐标为(w2,h2)
	float w1 = 0.25, w2 = 0.7, h1 = 0.2, h2 = 0.75;
	//创建一个矩形，四个参数分别为左上角像素的横坐标，纵坐标，ROI宽度、高度。
	Rect rect1(width * w1, height * h1, (w2 - w1) * width, (h2 - h1) * height);
	dst1 = dst1(rect1);
	namedWindow("ROI", CV_WINDOW_AUTOSIZE);
	imshow("ROI", dst1);

	
	//计算圆心位置，返回一个数组。可选择方法一或方法二
	// returnValue[0]为圆心在ROI图中的像素横坐标,returnValue[1]为圆心在ROI图中的像素纵坐标，returnValue[2]为圆在ROI图中的半径
	int* returnValue =circleMethod1(dst1);
	//int* returnValue = circleMethod2(dst1);
	
	//判断是否识别到圆
	if (returnValue[0] > 0) {
		//根据returnValue，计算原图中的像素坐标和半径
		int Abscissa = (w1 * width + returnValue[0]) / n;
		int Ordinate = (h1 * height + returnValue[1]) / n;
		int radius = returnValue[2] / n;
		printf("圆心横像素坐标：%d  圆心纵像素坐标：%d  圆半径：%d\n", Abscissa, Ordinate, radius);

		//将识别到的圆和圆心显示在dstCopy1上
		circle(dstCopy1, Point(Abscissa * n, Ordinate * n), radius * n, Scalar(0, 0, 255), 2, LINE_AA);
		circle(dstCopy1, Point(Abscissa * n, Ordinate * n), 2, Scalar(0, 0, 255), 1, LINE_AA);
		namedWindow("Circle", CV_WINDOW_AUTOSIZE);
		imshow("Circle", dstCopy1);
	}
	else {
		printf("未识别出圆\n");
		cv::waitKey(0);
		return -2;
	}
	



	end = clock();
	cout << endl << "运行总时间: " << end - start << "ms";

	cv::waitKey(0);
	return 0;
}

/*圆检测方法1
* 参数：Mat& img（ROI图）
* 返回值：ROI图中识别到的圆的圆心横坐标，圆心纵坐标，半径
*/
int* circleMethod1(Mat& img) {
	//创建一个数组，保存圆心坐标和半径，作为函数的返回值
	static int value[3] = { 0,0,0 };

	//高斯模糊,去除噪声
	GaussianBlur(img, img, Size(3, 3), 0, 0);
	//namedWindow("GaussianBlur", CV_WINDOW_AUTOSIZE);
	//imshow("GaussianBlur", img);

	//将RGB图转为灰度图
	cvtColor(img, img, CV_BGR2GRAY);


	//二值化,将灰度图转为只有黑色和白色的二值图像
	threshold(img, img, 28, 255, THRESH_BINARY_INV);
	namedWindow("threshold", CV_WINDOW_AUTOSIZE);
	imshow("threshold", img);

	//形态学处理（开运算+闭运算）
	//定义一个结构元素，用于开运算
	Mat kernel = getStructuringElement(MORPH_ELLIPSE, Size(3, 3), Point(-1, -1));
	//开运算，消除黑色背景上的白点
	morphologyEx(img, img, CV_MOP_OPEN, kernel, Point(-1, -1), 2);
	//namedWindow("open", CV_WINDOW_AUTOSIZE);
	//imshow("open", img);

	
	//定义一个结构元素，用于闭运算
	Mat kernel2 = getStructuringElement(MORPH_ELLIPSE, Size(4, 4), Point(-1, -1));
	//闭运算，消除白色背景上的黑点
	morphologyEx(img, img, CV_MOP_CLOSE, kernel2, Point(-1, -1), 2);
	//namedWindow("close", CV_WINDOW_AUTOSIZE);
	//imshow("close", img);


	//圆检测，霍夫圆变换
	//圆检测可以得到一系列圆的圆心坐标和半径，结果保存在pcircles中
	vector<Vec3f> pcircles;
	HoughCircles(img, pcircles, CV_HOUGH_GRADIENT, 1, 300, 100, 34, 60, 220);
	
	//圆检测完成后,图像转为RGB图,便于画圆
	cvtColor(img, img, CV_GRAY2BGR);
	//遍历找出的所有的圆
	for (int i = 0; i < pcircles.size(); i++) {
		Vec3f cc = pcircles[i];

		//取第一个圆，即算法认为最接近圆的那个圆，返回圆心坐标和半径
		if (i == 0) {
			value[0] = cc[0];
			value[1] = cc[1];
			value[2] = cc[2];
		}
		printf("ROI图圆心:(%.2f,%.2f),半径：%.2f\n", cc[0], cc[1], cc[2]);

		//在ROI图中画出所有的圆和圆心
		circle(img, Point(cc[0], cc[1]), cc[2], Scalar(0, 0, 255), 2, LINE_AA);
		circle(img, Point(cc[0], cc[1]), 4, Scalar(0, 0, 255), 2, LINE_AA);
	}
	namedWindow("HoughCircles1", CV_WINDOW_AUTOSIZE);
	imshow("HoughCircles1", img);

	
	return value;
}

/*圆检测方法2
* 参数：Mat& img（ROI图）
* 返回值：ROI图中识别到的圆的圆心横坐标，圆心纵坐标，半径
*/
int* circleMethod2(Mat& img) {

	static int value[3] = { 0,0,0 };

	//高斯模糊
	GaussianBlur(img, img, Size(3, 3), 0, 0);
	//namedWindow("GaussianBlur", CV_WINDOW_AUTOSIZE);
	//imshow("GaussianBlur", img);

	//转为灰度图
	cvtColor(img, img, CV_BGR2GRAY);

	//圆检测，霍夫圆变换
	//圆检测可以得到一系列圆的圆心坐标和半径，结果保存在pcircles中
	vector<Vec3f> pcircles;
	HoughCircles(img, pcircles, CV_HOUGH_GRADIENT, 1, 300, 100, 34, 60, 220);

	//圆检测完成后,图像转为RGB图,便于画圆
	cvtColor(img, img, CV_GRAY2BGR);
	//遍历找出的所有的圆
	for (int i = 0; i < pcircles.size(); i++) {
		Vec3f cc = pcircles[i];

		//取第一个圆，即算法认为最接近圆的那个圆，返回圆心坐标和半径
		if (i == 0) {
			value[0] = cc[0];
			value[1] = cc[1];
			value[2] = cc[2];
		}
		printf("ROI图圆心:(%.2f,%.2f),半径：%.2f\n", cc[0], cc[1], cc[2]);

		//在ROI图中画出所有的圆和圆心
		circle(img, Point(cc[0], cc[1]), cc[2], Scalar(0, 0, 255), 2, LINE_AA);
		circle(img, Point(cc[0], cc[1]), 4, Scalar(0, 0, 255), 2, LINE_AA);
	}
	namedWindow("HoughCircles1", CV_WINDOW_AUTOSIZE);
	imshow("HoughCircles1", img);


	return value;
}