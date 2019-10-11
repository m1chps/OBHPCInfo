#include <cmath>
#include <chrono>
#include <vector>
#include <stdint.h>
#include <iostream>
#include "opencv2/opencv.hpp"

//
#include <x86intrin.h>

//
#define INDEX(x, y, w) (((x) * (w)) + (y))

//
using timing     = std::chrono::steady_clock;
using time_unit  = std::chrono::nanoseconds;
using time_point = std::chrono::time_point<timing>;

//
#define IMG_H	600
#define IMG_W	1000
#define IMG_RES	IMG_H * IMG_W

#define FILTER_H 3
#define FILTER_W 3

//
typedef unsigned char byte;
typedef unsigned long long u64;

//
int filter_edge1[FILTER_H * FILTER_H] = { 1, 0, -1,
					  0, 0,  0,
					 -1, 0,  1    };

int filter_edge2[FILTER_H * FILTER_H] = { 0,  1, 0,
					  1, -4, 1,
					  0,  1, 0    };

int filter_edge3[FILTER_H * FILTER_H] = { -1, -1, -1,
					  -1,  8, -1,
					  -1, -1, -1  };

//
int filter_sharp[FILTER_H * FILTER_W] = {  0, -1,  0,
					  -1,  5, -1,
					   0, -1,  0  };

//
int filter_box_blur_sum = 9;
int filter_box_blur[FILTER_H * FILTER_W] = {  1, 1, 1,
					      1, 1, 1,
					      1, 1, 1  };

//
int filter_gauss_blur_sum = 16;
int filter_gauss_blur[FILTER_H * FILTER_W] = {  1, 2, 1,
						2, 4, 2,
						1, 2, 1  };

//
static inline int convolve(byte *m, int *f, u64 fh, u64 fw)
{
  int r = 0;
  
  for (u64 i = 0; i < fh * fw; i++)
    r += m[i] * f[i];
  
  return r;
}

//
static inline void apply_filter(byte *img_in, u64 h, u64 w, int *f, u64 fh, u64 fw, byte *img_out, int sum)
{
  double one_over_sum = 1.0 / sum;
  
  for (u64 i = 0; i < (h - fh) * (w - fh); i++)
    img_out[i] = (convolve(&img_in[i], f, fh, fw) * one_over_sum);
}

//
static inline void apply_sobel_filter(byte *img_in, byte *img_out, u64 h, u64 w, float threshold)
{
  int gx, gy;
  double mag = 0;
  static int f1[9] = { -1, 0, 1, -2, 0, 2, -1, 0, 1 }; //3x3 matrix
  static int f2[9] = { -1, -2, -1, 0, 0, 0, 1, 2, 1 }; //3x3 matrix

  for (u64 i = 0; i < (h - 3) * (w - 3); i++)
    {
      gx = convolve(&img_in[i], f1, 3, 3);
      gy = convolve(&img_in[i], f2, 3, 3);
      
      mag = sqrt((gx * gx) + (gy * gy));
      
      img_out[i] = (mag > threshold) ? 255 : mag;
    }
}

//
static inline void apply_sobel_filter_fast(byte *img_in, byte *img_out, u64 h, u64 w, float threshold)
{
  int gx, gy;
  double mag = 0;
  static int f1[9] = { -1, 0, 1, -2, 0, 2, -1, 0, 1 }; //3x3 matrix
  static int f2[9] = { -1, -2, -1, 0, 0, 0, 1, 2, 1 }; //3x3 matrix

  for (u64 i = 0; i < (h - 3) * (w - 3); i++)
    {
      gx = gy = 0;
      
      for (u64 j = 0; j < 9; j++)
	gx += img_in[i + j] * f1[j];

      for (u64 j = 0; j < 9; j++)
	gy += img_in[i + j] * f2[j];

      //Sqrt
      mag = sqrt((gx * gx) + (gy * gy));

      //Condition
      img_out[i] = (mag > threshold) ? 255 : mag;
    }
}

//
static inline void apply_sobel_filter_fast2(byte *img_in, byte *img_out, u64 h, u64 w, float threshold)
{
  int gx, gy;
  double mag = 0;
  static int f1[9] = { -1, 0, 1, -2, 0, 2, -1, 0, 1 }; //3x3 matrix
  static int f2[9] = { -1, -2, -1, 0, 0, 0, 1, 2, 1 }; //3x3 matrix

  for (u64 i = 0; i < (h - 3) * (w - 3); i++)
    {
      gx = 0;
      gy = 0;
      
      gx += img_in[i + 0] * f1[0];
      gx += img_in[i + 1] * f1[1];
      gx += img_in[i + 2] * f1[2];
      gx += img_in[i + 3] * f1[3];
      gx += img_in[i + 4] * f1[4];
      gx += img_in[i + 5] * f1[5];
      gx += img_in[i + 6] * f1[6];
      gx += img_in[i + 7] * f1[7];
      gx += img_in[i + 8] * f1[8];

      gy += img_in[i + 0] * f2[0];
      gy += img_in[i + 1] * f2[1];
      gy += img_in[i + 2] * f2[2];
      gy += img_in[i + 3] * f2[3];
      gy += img_in[i + 4] * f2[4];
      gy += img_in[i + 5] * f2[5];
      gy += img_in[i + 6] * f2[6];
      gy += img_in[i + 7] * f2[7];
      gy += img_in[i + 8] * f2[8];

      //Sqrt
      mag = sqrt((gx * gx) + (gy * gy));

      //Condition
      img_out[i] = (mag > threshold) ? 255 : mag;
    }
}

//
#include "durations.ipp"

//
int main(int argc, char **argv)
{
  //
  cv::VideoCapture cap(0);
  
  cap.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
  cap.set(cv::CAP_PROP_FRAME_WIDTH,  IMG_W);
  cap.set(cv::CAP_PROP_FRAME_HEIGHT, IMG_H);
  cap.set(cv::CAP_PROP_FPS, 60);
  
  cv::Mat gray;
  cv::Mat frame;
  
  std::vector<double> durations;
  
  //
  if (!cap.isOpened())
    {
      std::cerr << "Could not open default video stream.\n";
      return -1;
    }
  
  //
  while (true)
    {
      std::string buf;
      
      if (!cap.read(frame))
	{
	  std::cerr << "Could not read the next video frame.\n";
	  
	  return -1;
	}
      
      //Convert to gray
      cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
      
      cv::Mat out(gray.rows, gray.cols, CV_8UC1);
      
      time_point start = timing::now();
      
      //The magic function	
      //apply_sobel_filter((byte *)gray.data, (byte *)out.data, gray.rows, gray.cols, 100); 
      apply_sobel_filter_fast2((byte *)gray.data, (byte *)out.data, gray.rows, gray.cols, 100); 
      //apply_filter((byte *)gray.data, gray.rows, gray.cols, filter_edge3, 3, 3, (byte *)out.data, 1);
      
      time_point end = timing::now();

      int duration = std::chrono::duration_cast<time_unit>(end - start).count();

      buf = std::to_string(append_durations(&durations, double(duration)) / 1000.0) + " us/frame";
      
      cv::putText(out, buf.c_str(), cv::Point(5, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 255, 0), 4);
      
      cv::imshow("Video", out);
      
      if (cv::waitKey(1) == 'q')
	{
	  cv::destroyAllWindows();

	  break;
	}
    }
}
