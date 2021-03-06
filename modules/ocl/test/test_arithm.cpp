///////////////////////////////////////////////////////////////////////////////////////
//
//  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
//
//  By downloading, copying, installing or using the software you agree to this license.
//  If you do not agree to this license, do not download, install,
//  copy or use the software.
//
//
//                           License Agreement
//                For Open Source Computer Vision Library
//
// Copyright (C) 2010-2012, Institute Of Software Chinese Academy Of Science, all rights reserved.
// Copyright (C) 2010-2012, Advanced Micro Devices, Inc., all rights reserved.
// Third party copyrights are property of their respective owners.
//
// @Authors
//    Niko Li, newlife20080214@gmail.com
//    Jia Haipeng, jiahaipeng95@gmail.com
//    Shengen Yan, yanshengen@gmail.com
//    Jiang Liyuan,jlyuan001.good@163.com
//    Rock Li, Rock.Li@amd.com
//    Zailong Wu, bullet@yeah.net
//
// Redistribution and use in source and binary forms, with or without modification,
// are permitted provided that the following conditions are met:
//
//   * Redistribution's of source code must retain the above copyright notice,
//     this list of conditions and the following disclaimer.
//
//   * Redistribution's in binary form must reproduce the above copyright notice,
//     this list of conditions and the following disclaimer in the documentation
//     and/or other oclMaterials provided with the distribution.
//
//   * The name of the copyright holders may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// This software is provided by the copyright holders and contributors "as is" and
// any express or implied warranties, including, but not limited to, the implied
// warranties of merchantability and fitness for a particular purpose are disclaimed.
// In no event shall the Intel Corporation or contributors be liable for any direct,
// indirect, incidental, special, exemplary, or consequential damages
// (including, but not limited to, procurement of substitute goods or services;
// loss of use, data, or profits; or business interruption) however caused
// and on any theory of liability, whether in contract, strict liability,
// or tort (including negligence or otherwise) arising in any way out of
// the use of this software, even if advised of the possibility of such damage.
//
//M*/

//#define PRINT_CPU_TIME 1000
//#define PRINT_TIME


#include "precomp.hpp"
#include <iomanip>

#ifdef HAVE_OPENCL

using namespace cv;
using namespace cv::ocl;
using namespace cvtest;
using namespace testing;
using namespace std;

PARAM_TEST_CASE(ArithmTestBase, MatType, bool)
{
    int type;
    cv::Scalar val;

    //src mat
    cv::Mat mat1;
    cv::Mat mat2;
    cv::Mat mask;
    cv::Mat dst;
    cv::Mat dst1; //bak, for two outputs

    // set up roi
    int roicols;
    int roirows;
    int src1x;
    int src1y;
    int src2x;
    int src2y;
    int dstx;
    int dsty;
    int maskx;
    int masky;


    //src mat with roi
    cv::Mat mat1_roi;
    cv::Mat mat2_roi;
    cv::Mat mask_roi;
    cv::Mat dst_roi;
    cv::Mat dst1_roi; //bak
    //std::vector<cv::ocl::Info> oclinfo;
    //ocl dst mat for testing
    cv::ocl::oclMat gdst_whole;
    cv::ocl::oclMat gdst1_whole; //bak

    //ocl mat with roi
    cv::ocl::oclMat gmat1;
    cv::ocl::oclMat gmat2;
    cv::ocl::oclMat gdst;
    cv::ocl::oclMat gdst1;   //bak
    cv::ocl::oclMat gmask;

    virtual void SetUp()
    {
        type = GET_PARAM(0);

        cv::RNG &rng = TS::ptr()->get_rng();

        cv::Size size(MWIDTH, MHEIGHT);

        mat1 = randomMat(rng, size, type, 5, 16, false);
        //mat2 = randomMat(rng, size, type, 5, 16, false);
        mat2 = randomMat(rng, size, type, 5, 16, false);
        dst  = randomMat(rng, size, type, 5, 16, false);
        dst1  = randomMat(rng, size, type, 5, 16, false);
        mask = randomMat(rng, size, CV_8UC1, 0, 2,  false);

        cv::threshold(mask, mask, 0.5, 255., CV_8UC1);

        val = cv::Scalar(rng.uniform(-10.0, 10.0), rng.uniform(-10.0, 10.0), rng.uniform(-10.0, 10.0), rng.uniform(-10.0, 10.0));

        //int devnums = getDevice(oclinfo, OPENCV_DEFAULT_OPENCL_DEVICE);
        //CV_Assert(devnums > 0);
        ////if you want to use undefault device, set it here
        ////setDevice(oclinfo[0]);
    }

    void random_roi()
    {
        cv::RNG &rng = TS::ptr()->get_rng();

#ifdef RANDOMROI
        //randomize ROI
        roicols = rng.uniform(1, mat1.cols);
        roirows = rng.uniform(1, mat1.rows);
        src1x   = rng.uniform(0, mat1.cols - roicols);
        src1y   = rng.uniform(0, mat1.rows - roirows);
        dstx    = rng.uniform(0, dst.cols  - roicols);
        dsty    = rng.uniform(0, dst.rows  - roirows);
#else
        roicols = mat1.cols;
        roirows = mat1.rows;
        src1x = 0;
        src1y = 0;
        dstx = 0;
        dsty = 0;
#endif
        maskx   = rng.uniform(0, mask.cols - roicols);
        masky   = rng.uniform(0, mask.rows - roirows);
        src2x   = rng.uniform(0, mat2.cols - roicols);
        src2y   = rng.uniform(0, mat2.rows - roirows);
        mat1_roi = mat1(Rect(src1x, src1y, roicols, roirows));
        mat2_roi = mat2(Rect(src2x, src2y, roicols, roirows));
        mask_roi = mask(Rect(maskx, masky, roicols, roirows));
        dst_roi  = dst(Rect(dstx, dsty, roicols, roirows));
        dst1_roi = dst1(Rect(dstx, dsty, roicols, roirows));

        gdst_whole = dst;
        gdst = gdst_whole(Rect(dstx, dsty, roicols, roirows));

        gdst1_whole = dst1;
        gdst1 = gdst1_whole(Rect(dstx, dsty, roicols, roirows));

        gmat1 = mat1_roi;
        gmat2 = mat2_roi;
        gmask = mask_roi; //end
    }

};
////////////////////////////////lut/////////////////////////////////////////////////

struct Lut : ArithmTestBase {};
#define VARNAME(A) string(#A);



TEST_P(Lut, Mat)
{

    cv::Mat mat2(3, 512, CV_8UC1);
    cv::RNG &rng = TS::ptr()->get_rng();
    rng.fill(mat2, cv::RNG::UNIFORM, cv::Scalar::all(0), cv::Scalar::all(256));

    for(int j = 0; j < LOOP_TIMES; j ++)
    {
        random_roi();

        src2x = rng.uniform( 0, mat2.cols - 256);
        src2y = rng.uniform (0, mat2.rows - 1);

        cv::Mat mat2_roi = mat2(Rect(src2x, src2y, 256, 1));

        cv::ocl::oclMat gmat2(mat2_roi);

        cv::LUT(mat1_roi, mat2_roi, dst_roi);
        cv::ocl::LUT(gmat1, gmat2, gdst);

        cv::Mat cpu_dst;
        gdst_whole.download (cpu_dst);
        char s[1024];
        sprintf(s, "roicols=%d,roirows=%d,src1x=%d,src1y=%d,dstx=%d,dsty=%d,maskx=%d,masky=%d,src2x=%d,src2y=%d", roicols, roirows, src1x, src1y, dstx, dsty, maskx, masky, src2x, src2y);
        EXPECT_MAT_NEAR(dst, cpu_dst, 0, s);
    }
}




////////////////////////////////exp/////////////////////////////////////////////////

struct Exp : ArithmTestBase {};

TEST_P(Exp, Mat)
{
    for(int j = 0; j < LOOP_TIMES; j++)
    {
        random_roi();

        cv::exp(mat1_roi, dst_roi);
        cv::ocl::exp(gmat1, gdst);

        cv::Mat cpu_dst;
        gdst_whole.download(cpu_dst);

        char s[1024];
        sprintf(s, "roicols=%d,roirows=%d,src1x=%d,src1y=%d,dstx=%d,dsty=%d,maskx=%d,masky=%d,src2x=%d,src2y=%d", roicols, roirows, src1x, src1y, dstx, dsty, maskx, masky, src2x, src2y);
        EXPECT_MAT_NEAR(dst, cpu_dst, 1, s);

    }
}


////////////////////////////////log/////////////////////////////////////////////////

struct Log : ArithmTestBase {};

TEST_P(Log, Mat)
{
    for(int j = 0; j < LOOP_TIMES; j++)
    {
        random_roi();


        cv::log(mat1_roi, dst_roi);
        cv::ocl::log(gmat1, gdst);

        cv::Mat cpu_dst;
        gdst_whole.download(cpu_dst);
        char s[1024];
        sprintf(s, "roicols=%d,roirows=%d,src1x=%d,src1y=%d,dstx=%d,dsty=%d,maskx=%d,masky=%d,src2x=%d,src2y=%d", roicols, roirows, src1x, src1y, dstx, dsty, maskx, masky, src2x, src2y);
        EXPECT_MAT_NEAR(dst, cpu_dst, 1, s);

    }
}




////////////////////////////////add/////////////////////////////////////////////////

struct Add : ArithmTestBase {};

TEST_P(Add, Mat)
{
    for(int j = 0; j < LOOP_TIMES; j++)
    {
        random_roi();

        cv::add(mat1_roi, mat2_roi, dst_roi);
        cv::ocl::add(gmat1, gmat2, gdst);

        cv::Mat cpu_dst;
        gdst_whole.download(cpu_dst);
        char s[1024];
        sprintf(s, "roicols=%d,roirows=%d,src1x=%d,src1y=%d,dstx=%d,dsty=%d,maskx=%d,masky=%d,src2x=%d,src2y=%d", roicols, roirows, src1x, src1y, dstx, dsty, maskx, masky, src2x, src2y);
        EXPECT_MAT_NEAR(dst, cpu_dst, 0.0, s);
    }
}

TEST_P(Add, Mat_Mask)
{
    for(int j = 0; j < LOOP_TIMES; j++)
    {
        random_roi();

        cv::add(mat1_roi, mat2_roi, dst_roi, mask_roi);
        cv::ocl::add(gmat1, gmat2, gdst, gmask);

        cv::Mat cpu_dst;
        gdst_whole.download(cpu_dst);
        char s[1024];
        sprintf(s, "roicols=%d,roirows=%d,src1x=%d,src1y=%d,dstx=%d,dsty=%d,maskx=%d,masky=%d,src2x=%d,src2y=%d", roicols, roirows, src1x, src1y, dstx, dsty, maskx, masky, src2x, src2y);
        EXPECT_MAT_NEAR(dst, cpu_dst, 0.0, s);
    }
}
TEST_P(Add, Scalar)
{
    for(int j = 0; j < LOOP_TIMES; j++)
    {
        random_roi();

        cv::add(mat1_roi, val, dst_roi);
        cv::ocl::add(gmat1, val, gdst);

        cv::Mat cpu_dst;
        gdst_whole.download(cpu_dst);
        char s[1024];
        sprintf(s, "roicols=%d,roirows=%d,src1x=%d,src1y=%d,dstx=%d,dsty=%d,maskx=%d,masky=%d,src2x=%d,src2y=%d", roicols, roirows, src1x, src1y, dstx, dsty, maskx, masky, src2x, src2y);
        EXPECT_MAT_NEAR(dst, cpu_dst, 1e-5, s);
    }
}

TEST_P(Add, Scalar_Mask)
{
    for(int j = 0; j < LOOP_TIMES; j++)
    {
        random_roi();

        cv::add(mat1_roi, val, dst_roi, mask_roi);
        cv::ocl::add(gmat1, val, gdst, gmask);

        cv::Mat cpu_dst;
        gdst_whole.download(cpu_dst);
        char s[1024];
        sprintf(s, "roicols=%d,roirows=%d,src1x=%d,src1y=%d,dstx=%d,dsty=%d,maskx=%d,masky=%d,src2x=%d,src2y=%d", roicols, roirows, src1x, src1y, dstx, dsty, maskx, masky, src2x, src2y);
        EXPECT_MAT_NEAR(dst, cpu_dst, 1e-5, s);
    }
}



////////////////////////////////sub/////////////////////////////////////////////////
struct Sub : ArithmTestBase {};

TEST_P(Sub, Mat)
{
    for(int j = 0; j < LOOP_TIMES; j++)
    {
        random_roi();

        cv::subtract(mat1_roi, mat2_roi, dst_roi);
        cv::ocl::subtract(gmat1, gmat2, gdst);

        cv::Mat cpu_dst;
        gdst_whole.download(cpu_dst);
        char s[1024];
        sprintf(s, "roicols=%d,roirows=%d,src1x=%d,src1y=%d,dstx=%d,dsty=%d,maskx=%d,masky=%d,src2x=%d,src2y=%d", roicols, roirows, src1x, src1y, dstx, dsty, maskx, masky, src2x, src2y);
        EXPECT_MAT_NEAR(dst, cpu_dst, 0.0, s);
    }
}

TEST_P(Sub, Mat_Mask)
{
    for(int j = 0; j < LOOP_TIMES; j++)
    {
        random_roi();

        cv::subtract(mat1_roi, mat2_roi, dst_roi, mask_roi);
        cv::ocl::subtract(gmat1, gmat2, gdst, gmask);

        cv::Mat cpu_dst;
        gdst_whole.download(cpu_dst);
        char s[1024];
        sprintf(s, "roicols=%d,roirows=%d,src1x=%d,src1y=%d,dstx=%d,dsty=%d,maskx=%d,masky=%d,src2x=%d,src2y=%d", roicols, roirows, src1x, src1y, dstx, dsty, maskx, masky, src2x, src2y);
        EXPECT_MAT_NEAR(dst, cpu_dst, 0.0, s);
    }
}
TEST_P(Sub, Scalar)
{
    for(int j = 0; j < LOOP_TIMES; j++)
    {
        random_roi();

        cv::subtract(mat1_roi, val, dst_roi);
        cv::ocl::subtract(gmat1, val, gdst);

        cv::Mat cpu_dst;
        gdst_whole.download(cpu_dst);
        char s[1024];
        sprintf(s, "roicols=%d,roirows=%d,src1x=%d,src1y=%d,dstx=%d,dsty=%d,maskx=%d,masky=%d,src2x=%d,src2y=%d", roicols, roirows, src1x, src1y, dstx, dsty, maskx, masky, src2x, src2y);
        EXPECT_MAT_NEAR(dst, cpu_dst, 1e-5, s);
    }
}

TEST_P(Sub, Scalar_Mask)
{
    for(int j = 0; j < LOOP_TIMES; j++)
    {
        random_roi();

        cv::subtract(mat1_roi, val, dst_roi, mask_roi);
        cv::ocl::subtract(gmat1, val, gdst, gmask);

        cv::Mat cpu_dst;
        gdst_whole.download(cpu_dst);
        char s[1024];
        sprintf(s, "roicols=%d,roirows=%d,src1x=%d,src1y=%d,dstx=%d,dsty=%d,maskx=%d,masky=%d,src2x=%d,src2y=%d", roicols, roirows, src1x, src1y, dstx, dsty, maskx, masky, src2x, src2y);
        EXPECT_MAT_NEAR(dst, cpu_dst, 1e-5, s);
    }
}



////////////////////////////////Mul/////////////////////////////////////////////////
struct Mul : ArithmTestBase {};

TEST_P(Mul, Mat)
{
    for(int j = 0; j < LOOP_TIMES; j++)
    {
        random_roi();

        cv::multiply(mat1_roi, mat2_roi, dst_roi);
        cv::ocl::multiply(gmat1, gmat2, gdst);

        cv::Mat cpu_dst;
        gdst_whole.download(cpu_dst);
        char s[1024];
        sprintf(s, "roicols=%d,roirows=%d,src1x=%d,src1y=%d,dstx=%d,dsty=%d,maskx=%d,masky=%d,src2x=%d,src2y=%d", roicols, roirows, src1x, src1y, dstx, dsty, maskx, masky, src2x, src2y);
        EXPECT_MAT_NEAR(dst, cpu_dst, 0.0, s);
    }
}

TEST_P(Mul, Mat_Scalar)
{
    for(int j = 0; j < LOOP_TIMES; j++)
    {
        random_roi();

        cv::RNG &rng = TS::ptr()->get_rng();
        double s = rng.uniform(-10.0, 10.0);

        cv::multiply(mat1_roi, mat2_roi, dst_roi, s);
        cv::ocl::multiply(gmat1, gmat2, gdst, s);

        cv::Mat cpu_dst;
        gdst_whole.download(cpu_dst);
        char sss[1024];
        sprintf(sss, "roicols=%d,roirows=%d,src1x=%d,src1y=%d,dstx=%d,dsty=%d,maskx=%d,masky=%d,src2x=%d,src2y=%d", roicols, roirows, src1x, src1y, dstx, dsty, maskx, masky, src2x, src2y);
        EXPECT_MAT_NEAR(dst, cpu_dst, 0.001, sss);
    }
}



struct Div : ArithmTestBase {};

TEST_P(Div, Mat)
{
    for(int j = 0; j < LOOP_TIMES; j++)
    {
        random_roi();

        cv::divide(mat1_roi, mat2_roi, dst_roi);
        cv::ocl::divide(gmat1, gmat2, gdst);

        cv::Mat cpu_dst;
        gdst_whole.download(cpu_dst);
        char sss[1024];
        sprintf(sss, "roicols=%d,roirows=%d,src1x=%d,src1y=%d,dstx=%d,dsty=%d,maskx=%d,masky=%d,src2x=%d,src2y=%d", roicols, roirows, src1x, src1y, dstx, dsty, maskx, masky, src2x, src2y);

        EXPECT_MAT_NEAR(dst, cpu_dst, 1, sss);
    }
}

TEST_P(Div, Mat_Scalar)
{
    for(int j = 0; j < LOOP_TIMES; j++)
    {
        random_roi();

        cv::RNG &rng = TS::ptr()->get_rng();
        double s = rng.uniform(-10.0, 10.0);

        cv::divide(mat1_roi, mat2_roi, dst_roi, s);
        cv::ocl::divide(gmat1, gmat2, gdst, s);

        cv::Mat cpu_dst;
        gdst_whole.download(cpu_dst);
        char sss[1024];
        sprintf(sss, "roicols=%d,roirows=%d,src1x=%d,src1y=%d,dstx=%d,dsty=%d,maskx=%d,masky=%d,src2x=%d,src2y=%d", roicols, roirows, src1x, src1y, dstx, dsty, maskx, masky, src2x, src2y);

        EXPECT_MAT_NEAR(dst, cpu_dst, 0.001, sss);
    }
}


struct Absdiff : ArithmTestBase {};

TEST_P(Absdiff, Mat)
{
    for(int j = 0; j < LOOP_TIMES; j++)
    {
        random_roi();

        cv::absdiff(mat1_roi, mat2_roi, dst_roi);
        cv::ocl::absdiff(gmat1, gmat2, gdst);

        cv::Mat cpu_dst;
        gdst_whole.download(cpu_dst);
        char sss[1024];
        sprintf(sss, "roicols=%d,roirows=%d,src1x=%d,src1y=%d,dstx=%d,dsty=%d,maskx=%d,masky=%d,src2x=%d,src2y=%d", roicols, roirows, src1x, src1y, dstx, dsty, maskx, masky, src2x, src2y);

        EXPECT_MAT_NEAR(dst, cpu_dst, 0, sss);
    }
}

TEST_P(Absdiff, Mat_Scalar)
{
    for(int j = 0; j < LOOP_TIMES; j++)
    {
        random_roi();

        cv::absdiff(mat1_roi, val, dst_roi);
        cv::ocl::absdiff(gmat1, val, gdst);

        cv::Mat cpu_dst;
        gdst_whole.download(cpu_dst);
        char sss[1024];
        sprintf(sss, "roicols=%d,roirows=%d,src1x=%d,src1y=%d,dstx=%d,dsty=%d,maskx=%d,masky=%d,src2x=%d,src2y=%d", roicols, roirows, src1x, src1y, dstx, dsty, maskx, masky, src2x, src2y);

        EXPECT_MAT_NEAR(dst, cpu_dst, 1e-5, sss);
    }
}



struct CartToPolar : ArithmTestBase {};

TEST_P(CartToPolar, angleInDegree)
{
    for(int j = 0; j < LOOP_TIMES; j++)
    {
        random_roi();

        cv::cartToPolar(mat1_roi, mat2_roi, dst_roi, dst1_roi, 1);
        cv::ocl::cartToPolar(gmat1, gmat2, gdst, gdst1, 1);

        cv::Mat cpu_dst;
        gdst_whole.download(cpu_dst);

        cv::Mat cpu_dst1;
        gdst1_whole.download(cpu_dst1);
        char sss[1024];
        sprintf(sss, "roicols=%d,roirows=%d,src1x=%d,src1y=%d,dstx=%d,dsty=%d,maskx=%d,masky=%d,src2x=%d,src2y=%d", roicols, roirows, src1x, src1y, dstx, dsty, maskx, masky, src2x, src2y);
        EXPECT_MAT_NEAR(dst, cpu_dst, 0.5, sss);
        EXPECT_MAT_NEAR(dst1, cpu_dst1, 0.5, sss);
    }
}

TEST_P(CartToPolar, angleInRadians)
{
    for(int j = 0; j < LOOP_TIMES; j++)
    {
        random_roi();

        cv::cartToPolar(mat1_roi, mat2_roi, dst_roi, dst1_roi, 0);
        cv::ocl::cartToPolar(gmat1, gmat2, gdst, gdst1, 0);

        cv::Mat cpu_dst;
        gdst_whole.download(cpu_dst);

        cv::Mat cpu_dst1;
        gdst1_whole.download(cpu_dst1);
        char sss[1024];
        sprintf(sss, "roicols=%d,roirows=%d,src1x=%d,src1y=%d,dstx=%d,dsty=%d,maskx=%d,masky=%d,src2x=%d,src2y=%d", roicols, roirows, src1x, src1y, dstx, dsty, maskx, masky, src2x, src2y);
        EXPECT_MAT_NEAR(dst, cpu_dst, 0.5, sss);
        EXPECT_MAT_NEAR(dst1, cpu_dst1, 0.5, sss);
    }
}




struct PolarToCart : ArithmTestBase {};

TEST_P(PolarToCart, angleInDegree)
{
    for(int j = 0; j < LOOP_TIMES; j++)
    {
        random_roi();

        cv::polarToCart(mat1_roi, mat2_roi, dst_roi, dst1_roi, 1);
        cv::ocl::polarToCart(gmat1, gmat2, gdst, gdst1, 1);

        cv::Mat cpu_dst;
        gdst_whole.download(cpu_dst);

        cv::Mat cpu_dst1;
        gdst1_whole.download(cpu_dst1);
        char sss[1024];
        sprintf(sss, "roicols=%d,roirows=%d,src1x=%d,src1y=%d,dstx=%d,dsty=%d,maskx=%d,masky=%d,src2x=%d,src2y=%d", roicols, roirows, src1x, src1y, dstx, dsty, maskx, masky, src2x, src2y);

        EXPECT_MAT_NEAR(dst, cpu_dst, 0.5, sss);
        EXPECT_MAT_NEAR(dst1, cpu_dst1, 0.5, sss);
    }
}

TEST_P(PolarToCart, angleInRadians)
{
    for(int j = 0; j < LOOP_TIMES; j++)
    {
        random_roi();

        cv::polarToCart(mat1_roi, mat2_roi, dst_roi, dst1_roi, 0);
        cv::ocl::polarToCart(gmat1, gmat2, gdst, gdst1, 0);

        cv::Mat cpu_dst;
        gdst_whole.download(cpu_dst);

        cv::Mat cpu_dst1;
        gdst1_whole.download(cpu_dst1);
        char sss[1024];
        sprintf(sss, "roicols=%d,roirows=%d,src1x=%d,src1y=%d,dstx=%d,dsty=%d,maskx=%d,masky=%d,src2x=%d,src2y=%d", roicols, roirows, src1x, src1y, dstx, dsty, maskx, masky, src2x, src2y);

        EXPECT_MAT_NEAR(dst, cpu_dst, 0.5, sss);
        EXPECT_MAT_NEAR(dst1, cpu_dst1, 0.5, sss);
    }
}




struct Magnitude : ArithmTestBase {};

TEST_P(Magnitude, Mat)
{
    for(int j = 0; j < LOOP_TIMES; j++)
    {
        random_roi();

        cv::magnitude(mat1_roi, mat2_roi, dst_roi);
        cv::ocl::magnitude(gmat1, gmat2, gdst);

        cv::Mat cpu_dst;
        gdst_whole.download(cpu_dst);
        char sss[1024];
        sprintf(sss, "roicols=%d,roirows=%d,src1x=%d,src1y=%d,dstx=%d,dsty=%d,maskx=%d,masky=%d,src2x=%d,src2y=%d", roicols, roirows, src1x, src1y, dstx, dsty, maskx, masky, src2x, src2y);

        EXPECT_MAT_NEAR(dst, cpu_dst, 1e-5, sss);
    }
}




struct Transpose : ArithmTestBase {};

TEST_P(Transpose, Mat)
{
    for(int j = 0; j < LOOP_TIMES; j++)
    {
        random_roi();

        cv::transpose(mat1_roi, dst_roi);
        cv::ocl::transpose(gmat1, gdst);

        cv::Mat cpu_dst;
        gdst_whole.download(cpu_dst);
        char sss[1024];
        sprintf(sss, "roicols=%d,roirows=%d,src1x=%d,src1y=%d,dstx=%d,dsty=%d,maskx=%d,masky=%d,src2x=%d,src2y=%d", roicols, roirows, src1x, src1y, dstx, dsty, maskx, masky, src2x, src2y);

        EXPECT_MAT_NEAR(dst, cpu_dst, 1e-5, sss);
    }
}





struct Flip : ArithmTestBase {};

TEST_P(Flip, X)
{
    for(int j = 0; j < LOOP_TIMES; j++)
    {
        random_roi();

        cv::flip(mat1_roi, dst_roi, 0);
        cv::ocl::flip(gmat1, gdst, 0);

        cv::Mat cpu_dst;
        gdst_whole.download(cpu_dst);
        char sss[1024];
        sprintf(sss, "roicols=%d,roirows=%d,src1x=%d,src1y=%d,dstx=%d,dsty=%d,maskx=%d,masky=%d,src2x=%d,src2y=%d", roicols, roirows, src1x, src1y, dstx, dsty, maskx, masky, src2x, src2y);

        EXPECT_MAT_NEAR(dst, cpu_dst, 1e-5, sss);
    }
}

TEST_P(Flip, Y)
{
    for(int j = 0; j < LOOP_TIMES; j++)
    {
        random_roi();

        cv::flip(mat1_roi, dst_roi, 1);
        cv::ocl::flip(gmat1, gdst, 1);

        cv::Mat cpu_dst;
        gdst_whole.download(cpu_dst);
        char sss[1024];
        sprintf(sss, "roicols=%d,roirows=%d,src1x=%d,src1y=%d,dstx=%d,dsty=%d,maskx=%d,masky=%d,src2x=%d,src2y=%d", roicols, roirows, src1x, src1y, dstx, dsty, maskx, masky, src2x, src2y);

        EXPECT_MAT_NEAR(dst, cpu_dst, 1e-5, sss);
    }
}

TEST_P(Flip, BOTH)
{
    for(int j = 0; j < LOOP_TIMES; j++)
    {
        random_roi();

        cv::flip(mat1_roi, dst_roi, -1);
        cv::ocl::flip(gmat1, gdst, -1);

        cv::Mat cpu_dst;
        gdst_whole.download(cpu_dst);
        char sss[1024];
        sprintf(sss, "roicols=%d,roirows=%d,src1x=%d,src1y=%d,dstx=%d,dsty=%d,maskx=%d,masky=%d,src2x=%d,src2y=%d", roicols, roirows, src1x, src1y, dstx, dsty, maskx, masky, src2x, src2y);

        EXPECT_MAT_NEAR(dst, cpu_dst, 1e-5, sss);
    }
}



struct MinMax : ArithmTestBase {};

TEST_P(MinMax, MAT)
{
    for(int j = 0; j < LOOP_TIMES; j++)
    {
        random_roi();
        double minVal, maxVal;
        cv::Point minLoc, maxLoc;

        if (mat1.depth() != CV_8S)
        {
            cv::minMaxLoc(mat1_roi, &minVal, &maxVal, &minLoc, &maxLoc);
        }
        else
        {
            minVal = std::numeric_limits<double>::max();
            maxVal = -std::numeric_limits<double>::max();
            for (int i = 0; i < mat1_roi.rows; ++i)
                for (int j = 0; j < mat1_roi.cols; ++j)
                {
                    signed char val = mat1_roi.at<signed char>(i, j);
                    if (val < minVal) minVal = val;
                    if (val > maxVal) maxVal = val;
                }
        }

        double minVal_, maxVal_;
        cv::ocl::minMax(gmat1, &minVal_, &maxVal_);

        //check results
        char sss[1024];
        sprintf(sss, "roicols=%d,roirows=%d,src1x=%d,src1y=%d,dstx=%d,dsty=%d,maskx=%d,masky=%d,src2x=%d,src2y=%d", roicols, roirows, src1x, src1y, dstx, dsty, maskx, masky, src2x, src2y);

        EXPECT_DOUBLE_EQ(minVal_, minVal) << sss;
        EXPECT_DOUBLE_EQ(maxVal_, maxVal) << sss;
    }
}

TEST_P(MinMax, MASK)
{
    for(int j = 0; j < LOOP_TIMES; j++)
    {
        random_roi();
        double minVal, maxVal;
        cv::Point minLoc, maxLoc;

        if (mat1.depth() != CV_8S)
        {
            cv::minMaxLoc(mat1_roi, &minVal, &maxVal, &minLoc, &maxLoc, mask_roi);
        }
        else
        {
            minVal = std::numeric_limits<double>::max();
            maxVal = -std::numeric_limits<double>::max();
            for (int i = 0; i < mat1_roi.rows; ++i)
                for (int j = 0; j < mat1_roi.cols; ++j)
                {
                    signed char val = mat1_roi.at<signed char>(i, j);
                    unsigned char m = mask_roi.at<unsigned char>(i, j);
                    if (val < minVal && m) minVal = val;
                    if (val > maxVal && m) maxVal = val;
                }
        }

        double minVal_, maxVal_;
        cv::ocl::minMax(gmat1, &minVal_, &maxVal_, gmask);

        //check results
        char sss[1024];
        sprintf(sss, "roicols=%d,roirows=%d,src1x=%d,src1y=%d,dstx=%d,dsty=%d,maskx=%d,masky=%d,src2x=%d,src2y=%d", roicols, roirows, src1x, src1y, dstx, dsty, maskx, masky, src2x, src2y);

        EXPECT_DOUBLE_EQ(minVal_, minVal) << sss;
        EXPECT_DOUBLE_EQ(maxVal_, maxVal) << sss;
    }
}


struct MinMaxLoc : ArithmTestBase {};

TEST_P(MinMaxLoc, MAT)
{
    for(int j = 0; j < LOOP_TIMES; j++)
    {
        random_roi();
        double minVal, maxVal;
        cv::Point minLoc, maxLoc;
        int depth = mat1.depth();
        if (depth != CV_8S)
        {
            cv::minMaxLoc(mat1_roi, &minVal, &maxVal, &minLoc, &maxLoc);
        }
        else
        {
            minVal = std::numeric_limits<double>::max();
            maxVal = -std::numeric_limits<double>::max();
            for (int i = 0; i < mat1_roi.rows; ++i)
                for (int j = 0; j < mat1_roi.cols; ++j)
                {
                    signed char val = mat1_roi.at<signed char>(i, j);
                    if (val < minVal)
                    {
                        minVal = val;
                        minLoc.x = j;
                        minLoc.y = i;
                    }
                    if (val > maxVal)
                    {
                        maxVal = val;
                        maxLoc.x = j;
                        maxLoc.y = i;
                    }
                }
        }

        double minVal_, maxVal_;
        cv::Point minLoc_, maxLoc_;
        cv::ocl::minMaxLoc(gmat1, &minVal_, &maxVal_, &minLoc_, &maxLoc_, cv::ocl::oclMat());

        double error0, error1, minlocVal, minlocVal_, maxlocVal, maxlocVal_;
        if(depth == 0)
        {
            minlocVal = mat1_roi.at<unsigned char>(minLoc);
            minlocVal_ = mat1_roi.at<unsigned char>(minLoc_);
            maxlocVal = mat1_roi.at<unsigned char>(maxLoc);
            maxlocVal_ = mat1_roi.at<unsigned char>(maxLoc_);
            error0 = ::abs(mat1_roi.at<unsigned char>(minLoc_) - mat1_roi.at<unsigned char>(minLoc));
            error1 = ::abs(mat1_roi.at<unsigned char>(maxLoc_) - mat1_roi.at<unsigned char>(maxLoc));
        }
        if(depth == 1)
        {
            minlocVal = mat1_roi.at<signed char>(minLoc);
            minlocVal_ = mat1_roi.at<signed char>(minLoc_);
            maxlocVal = mat1_roi.at<signed char>(maxLoc);
            maxlocVal_ = mat1_roi.at<signed char>(maxLoc_);
            error0 = ::abs(mat1_roi.at<signed char>(minLoc_) - mat1_roi.at<signed char>(minLoc));
            error1 = ::abs(mat1_roi.at<signed char>(maxLoc_) - mat1_roi.at<signed char>(maxLoc));
        }
        if(depth == 2)
        {
            minlocVal = mat1_roi.at<unsigned short>(minLoc);
            minlocVal_ = mat1_roi.at<unsigned short>(minLoc_);
            maxlocVal = mat1_roi.at<unsigned short>(maxLoc);
            maxlocVal_ = mat1_roi.at<unsigned short>(maxLoc_);
            error0 = ::abs(mat1_roi.at<unsigned short>(minLoc_) - mat1_roi.at<unsigned short>(minLoc));
            error1 = ::abs(mat1_roi.at<unsigned short>(maxLoc_) - mat1_roi.at<unsigned short>(maxLoc));
        }
        if(depth == 3)
        {
            minlocVal = mat1_roi.at<signed short>(minLoc);
            minlocVal_ = mat1_roi.at<signed short>(minLoc_);
            maxlocVal = mat1_roi.at<signed short>(maxLoc);
            maxlocVal_ = mat1_roi.at<signed short>(maxLoc_);
            error0 = ::abs(mat1_roi.at<signed short>(minLoc_) - mat1_roi.at<signed short>(minLoc));
            error1 = ::abs(mat1_roi.at<signed short>(maxLoc_) - mat1_roi.at<signed short>(maxLoc));
        }
        if(depth == 4)
        {
            minlocVal = mat1_roi.at<int>(minLoc);
            minlocVal_ = mat1_roi.at<int>(minLoc_);
            maxlocVal = mat1_roi.at<int>(maxLoc);
            maxlocVal_ = mat1_roi.at<int>(maxLoc_);
            error0 = ::abs(mat1_roi.at<int>(minLoc_) - mat1_roi.at<int>(minLoc));
            error1 = ::abs(mat1_roi.at<int>(maxLoc_) - mat1_roi.at<int>(maxLoc));
        }
        if(depth == 5)
        {
            minlocVal = mat1_roi.at<float>(minLoc);
            minlocVal_ = mat1_roi.at<float>(minLoc_);
            maxlocVal = mat1_roi.at<float>(maxLoc);
            maxlocVal_ = mat1_roi.at<float>(maxLoc_);
            error0 = ::abs(mat1_roi.at<float>(minLoc_) - mat1_roi.at<float>(minLoc));
            error1 = ::abs(mat1_roi.at<float>(maxLoc_) - mat1_roi.at<float>(maxLoc));
        }
        if(depth == 6)
        {
            minlocVal = mat1_roi.at<double>(minLoc);
            minlocVal_ = mat1_roi.at<double>(minLoc_);
            maxlocVal = mat1_roi.at<double>(maxLoc);
            maxlocVal_ = mat1_roi.at<double>(maxLoc_);
            error0 = ::abs(mat1_roi.at<double>(minLoc_) - mat1_roi.at<double>(minLoc));
            error1 = ::abs(mat1_roi.at<double>(maxLoc_) - mat1_roi.at<double>(maxLoc));
        }

        //check results
        char sss[1024];
        sprintf(sss, "roicols=%d,roirows=%d,src1x=%d,src1y=%d,dstx=%d,dsty=%d,maskx=%d,masky=%d,src2x=%d,src2y=%d", roicols, roirows, src1x, src1y, dstx, dsty, maskx, masky, src2x, src2y);

        EXPECT_DOUBLE_EQ(minVal_, minVal) << sss;
        EXPECT_DOUBLE_EQ(maxVal_, maxVal) << sss;
        EXPECT_DOUBLE_EQ(minlocVal_, minlocVal) << sss;
        EXPECT_DOUBLE_EQ(maxlocVal_, maxlocVal) << sss;

        EXPECT_DOUBLE_EQ(error0, 0.0) << sss;
        EXPECT_DOUBLE_EQ(error1, 0.0) << sss;
    }
}


TEST_P(MinMaxLoc, MASK)
{
    for(int j = 0; j < LOOP_TIMES; j++)
    {
        random_roi();
        double minVal, maxVal;
        cv::Point minLoc, maxLoc;
        int depth = mat1.depth();
        if (depth != CV_8S)
        {
            cv::minMaxLoc(mat1_roi, &minVal, &maxVal, &minLoc, &maxLoc, mask_roi);
        }
        else
        {
            minVal = std::numeric_limits<double>::max();
            maxVal = -std::numeric_limits<double>::max();
            for (int i = 0; i < mat1_roi.rows; ++i)
                for (int j = 0; j < mat1_roi.cols; ++j)
                {
                    signed char val = mat1_roi.at<signed char>(i, j);
                    unsigned char m = mask_roi.at<unsigned char>(i , j);
                    if (val < minVal && m)
                    {
                        minVal = val;
                        minLoc.x = j;
                        minLoc.y = i;
                    }
                    if (val > maxVal && m)
                    {
                        maxVal = val;
                        maxLoc.x = j;
                        maxLoc.y = i;
                    }
                }
        }

        double minVal_, maxVal_;
        cv::Point minLoc_, maxLoc_;
        cv::ocl::minMaxLoc(gmat1, &minVal_, &maxVal_, &minLoc_, &maxLoc_, gmask);

        double error0, error1, minlocVal, minlocVal_, maxlocVal, maxlocVal_;
        if(minLoc_.x == -1 || minLoc_.y == -1 || maxLoc_.x == -1 || maxLoc_.y == -1) continue;
        if(depth == 0)
        {
            minlocVal = mat1_roi.at<unsigned char>(minLoc);
            minlocVal_ = mat1_roi.at<unsigned char>(minLoc_);
            maxlocVal = mat1_roi.at<unsigned char>(maxLoc);
            maxlocVal_ = mat1_roi.at<unsigned char>(maxLoc_);
            error0 = ::abs(mat1_roi.at<unsigned char>(minLoc_) - mat1_roi.at<unsigned char>(minLoc));
            error1 = ::abs(mat1_roi.at<unsigned char>(maxLoc_) - mat1_roi.at<unsigned char>(maxLoc));
        }
        if(depth == 1)
        {
            minlocVal = mat1_roi.at<signed char>(minLoc);
            minlocVal_ = mat1_roi.at<signed char>(minLoc_);
            maxlocVal = mat1_roi.at<signed char>(maxLoc);
            maxlocVal_ = mat1_roi.at<signed char>(maxLoc_);
            error0 = ::abs(mat1_roi.at<signed char>(minLoc_) - mat1_roi.at<signed char>(minLoc));
            error1 = ::abs(mat1_roi.at<signed char>(maxLoc_) - mat1_roi.at<signed char>(maxLoc));
        }
        if(depth == 2)
        {
            minlocVal = mat1_roi.at<unsigned short>(minLoc);
            minlocVal_ = mat1_roi.at<unsigned short>(minLoc_);
            maxlocVal = mat1_roi.at<unsigned short>(maxLoc);
            maxlocVal_ = mat1_roi.at<unsigned short>(maxLoc_);
            error0 = ::abs(mat1_roi.at<unsigned short>(minLoc_) - mat1_roi.at<unsigned short>(minLoc));
            error1 = ::abs(mat1_roi.at<unsigned short>(maxLoc_) - mat1_roi.at<unsigned short>(maxLoc));
        }
        if(depth == 3)
        {
            minlocVal = mat1_roi.at<signed short>(minLoc);
            minlocVal_ = mat1_roi.at<signed short>(minLoc_);
            maxlocVal = mat1_roi.at<signed short>(maxLoc);
            maxlocVal_ = mat1_roi.at<signed short>(maxLoc_);
            error0 = ::abs(mat1_roi.at<signed short>(minLoc_) - mat1_roi.at<signed short>(minLoc));
            error1 = ::abs(mat1_roi.at<signed short>(maxLoc_) - mat1_roi.at<signed short>(maxLoc));
        }
        if(depth == 4)
        {
            minlocVal = mat1_roi.at<int>(minLoc);
            minlocVal_ = mat1_roi.at<int>(minLoc_);
            maxlocVal = mat1_roi.at<int>(maxLoc);
            maxlocVal_ = mat1_roi.at<int>(maxLoc_);
            error0 = ::abs(mat1_roi.at<int>(minLoc_) - mat1_roi.at<int>(minLoc));
            error1 = ::abs(mat1_roi.at<int>(maxLoc_) - mat1_roi.at<int>(maxLoc));
        }
        if(depth == 5)
        {
            minlocVal = mat1_roi.at<float>(minLoc);
            minlocVal_ = mat1_roi.at<float>(minLoc_);
            maxlocVal = mat1_roi.at<float>(maxLoc);
            maxlocVal_ = mat1_roi.at<float>(maxLoc_);
            error0 = ::abs(mat1_roi.at<float>(minLoc_) - mat1_roi.at<float>(minLoc));
            error1 = ::abs(mat1_roi.at<float>(maxLoc_) - mat1_roi.at<float>(maxLoc));
        }
        if(depth == 6)
        {
            minlocVal = mat1_roi.at<double>(minLoc);
            minlocVal_ = mat1_roi.at<double>(minLoc_);
            maxlocVal = mat1_roi.at<double>(maxLoc);
            maxlocVal_ = mat1_roi.at<double>(maxLoc_);
            error0 = ::abs(mat1_roi.at<double>(minLoc_) - mat1_roi.at<double>(minLoc));
            error1 = ::abs(mat1_roi.at<double>(maxLoc_) - mat1_roi.at<double>(maxLoc));
        }

        //check results
        char sss[1024];
        sprintf(sss, "roicols=%d,roirows=%d,src1x=%d,src1y=%d,dstx=%d,dsty=%d,maskx=%d,masky=%d,src2x=%d,src2y=%d", roicols, roirows, src1x, src1y, dstx, dsty, maskx, masky, src2x, src2y);

        EXPECT_DOUBLE_EQ(minVal_, minVal) << sss;
        EXPECT_DOUBLE_EQ(maxVal_, maxVal) << sss;
        EXPECT_DOUBLE_EQ(minlocVal_, minlocVal) << sss;
        EXPECT_DOUBLE_EQ(maxlocVal_, maxlocVal) << sss;

        EXPECT_DOUBLE_EQ(error0, 0.0) << sss;
        EXPECT_DOUBLE_EQ(error1, 0.0) << sss;
    }
}


struct Sum : ArithmTestBase {};

TEST_P(Sum, MAT)
{
    for(int j = 0; j < LOOP_TIMES; j++)
    {
        random_roi();
        Scalar cpures = cv::sum(mat1_roi);
        Scalar gpures = cv::ocl::sum(gmat1);
        char sss[1024];
        sprintf(sss, "roicols=%d,roirows=%d,src1x=%d,src1y=%d,dstx=%d,dsty=%d,maskx=%d,masky=%d,src2x=%d,src2y=%d", roicols, roirows, src1x, src1y, dstx, dsty, maskx, masky, src2x, src2y);

        //check results
        EXPECT_DOUBLE_EQ(cpures[0], gpures[0]) << sss;
        EXPECT_DOUBLE_EQ(cpures[1], gpures[1]) << sss;
        EXPECT_DOUBLE_EQ(cpures[2], gpures[2]) << sss;
        EXPECT_DOUBLE_EQ(cpures[3], gpures[3]) << sss;
    }
}

//TEST_P(Sum, MASK)
//{
//	for(int j=0; j<LOOP_TIMES; j++)
//	{
//		random_roi();
//
//	}
//}



struct CountNonZero : ArithmTestBase {};

TEST_P(CountNonZero, MAT)
{
    for(int j = 0; j < LOOP_TIMES; j++)
    {
        random_roi();
        int cpures = cv::countNonZero(mat1_roi);
        int gpures = cv::ocl::countNonZero(gmat1);

        //check results
        char sss[1024];
        sprintf(sss, "roicols=%d,roirows=%d,src1x=%d,src1y=%d,dstx=%d,dsty=%d,maskx=%d,masky=%d,src2x=%d,src2y=%d", roicols, roirows, src1x, src1y, dstx, dsty, maskx, masky, src2x, src2y);

        EXPECT_DOUBLE_EQ((double)cpures, (double)gpures) << sss;
    }
}



////////////////////////////////phase/////////////////////////////////////////////////
struct Phase : ArithmTestBase {};

TEST_P(Phase, Mat)
{
    if(mat1.depth() != CV_32F && mat1.depth() != CV_64F)
    {
        cout << "\tUnsupported type\t\n";
    }
    for(int angelInDegrees = 0; angelInDegrees < 2; angelInDegrees++)
    {
        for(int j = 0; j < LOOP_TIMES; j++)
        {
            random_roi();
            cv::phase(mat1_roi, mat2_roi, dst_roi, angelInDegrees);
            cv::ocl::phase(gmat1, gmat2, gdst, angelInDegrees);

            cv::Mat cpu_dst;
            gdst_whole.download(cpu_dst);

            char sss[1024];
            sprintf(sss, "roicols=%d,roirows=%d,src1x=%d,src1y=%d,dstx=%d,dsty=%d,maskx=%d,masky=%d,src2x=%d,src2y=%d", roicols, roirows, src1x, src1y, dstx, dsty, maskx, masky, src2x, src2y);
            EXPECT_MAT_NEAR(dst, cpu_dst, 1e-2, sss);
        }
    }
}


////////////////////////////////bitwise_and/////////////////////////////////////////////////
struct Bitwise_and : ArithmTestBase {};

TEST_P(Bitwise_and, Mat)
{
    for(int j = 0; j < LOOP_TIMES; j++)
    {
        random_roi();

        cv::bitwise_and(mat1_roi, mat2_roi, dst_roi);
        cv::ocl::bitwise_and(gmat1, gmat2, gdst);

        cv::Mat cpu_dst;
        gdst_whole.download(cpu_dst);
        char sss[1024];
        sprintf(sss, "roicols=%d,roirows=%d,src1x=%d,src1y=%d,dstx=%d,dsty=%d,maskx=%d,masky=%d,src2x=%d,src2y=%d", roicols, roirows, src1x, src1y, dstx, dsty, maskx, masky, src2x, src2y);

        EXPECT_MAT_NEAR(dst, cpu_dst, 0.0, sss);
    }
}

TEST_P(Bitwise_and, Mat_Mask)
{
    for(int j = 0; j < LOOP_TIMES; j++)
    {
        random_roi();

        cv::bitwise_and(mat1_roi, mat2_roi, dst_roi, mask_roi);
        cv::ocl::bitwise_and(gmat1, gmat2, gdst, gmask);

        cv::Mat cpu_dst;
        gdst_whole.download(cpu_dst);
        char sss[1024];
        sprintf(sss, "roicols=%d,roirows=%d,src1x=%d,src1y=%d,dstx=%d,dsty=%d,maskx=%d,masky=%d,src2x=%d,src2y=%d", roicols, roirows, src1x, src1y, dstx, dsty, maskx, masky, src2x, src2y);

        EXPECT_MAT_NEAR(dst, cpu_dst, 0.0, sss);
    }
}
TEST_P(Bitwise_and, Scalar)
{
    for(int j = 0; j < LOOP_TIMES; j++)
    {
        random_roi();

        cv::bitwise_and(mat1_roi, val, dst_roi);
        cv::ocl::bitwise_and(gmat1, val, gdst);

        cv::Mat cpu_dst;
        gdst_whole.download(cpu_dst);
        char sss[1024];
        sprintf(sss, "roicols=%d,roirows=%d,src1x=%d,src1y=%d,dstx=%d,dsty=%d,maskx=%d,masky=%d,src2x=%d,src2y=%d", roicols, roirows, src1x, src1y, dstx, dsty, maskx, masky, src2x, src2y);

        EXPECT_MAT_NEAR(dst, cpu_dst, 1e-5, sss);

    }
}

TEST_P(Bitwise_and, Scalar_Mask)
{
    for(int j = 0; j < LOOP_TIMES; j++)
    {
        random_roi();

        cv::bitwise_and(mat1_roi, val, dst_roi, mask_roi);
        cv::ocl::bitwise_and(gmat1, val, gdst, gmask);

        cv::Mat cpu_dst;
        gdst_whole.download(cpu_dst);
        char *sss = new char[1024];
        sprintf(sss, "roicols=%d,roirows=%d,src1x=%d,src1y=%d,dstx=%d,dsty=%d,maskx=%d,masky=%d,src2x=%d,src2y=%d", roicols, roirows, src1x, src1y, dstx, dsty, maskx, masky, src2x, src2y);

        EXPECT_MAT_NEAR(dst, cpu_dst, 1e-5, sss);
        delete[] sss;
    }
}



////////////////////////////////bitwise_or/////////////////////////////////////////////////

struct Bitwise_or : ArithmTestBase {};

TEST_P(Bitwise_or, Mat)
{
    for(int j = 0; j < LOOP_TIMES; j++)
    {
        random_roi();

        cv::bitwise_or(mat1_roi, mat2_roi, dst_roi);
        cv::ocl::bitwise_or(gmat1, gmat2, gdst);

        cv::Mat cpu_dst;
        gdst_whole.download(cpu_dst);
        char sss[1024];
        sprintf(sss, "roicols=%d,roirows=%d,src1x=%d,src1y=%d,dstx=%d,dsty=%d,maskx=%d,masky=%d,src2x=%d,src2y=%d", roicols, roirows, src1x, src1y, dstx, dsty, maskx, masky, src2x, src2y);

        EXPECT_MAT_NEAR(dst, cpu_dst, 0.0, sss);
    }
}

TEST_P(Bitwise_or, Mat_Mask)
{
    for(int j = 0; j < LOOP_TIMES; j++)
    {
        random_roi();

        cv::bitwise_or(mat1_roi, mat2_roi, dst_roi, mask_roi);
        cv::ocl::bitwise_or(gmat1, gmat2, gdst, gmask);

        cv::Mat cpu_dst;
        gdst_whole.download(cpu_dst);
        char sss[1024];
        sprintf(sss, "roicols=%d,roirows=%d,src1x=%d,src1y=%d,dstx=%d,dsty=%d,maskx=%d,masky=%d,src2x=%d,src2y=%d", roicols, roirows, src1x, src1y, dstx, dsty, maskx, masky, src2x, src2y);

        EXPECT_MAT_NEAR(dst, cpu_dst, 0.0, sss);
    }
}
TEST_P(Bitwise_or, Scalar)
{
    for(int j = 0; j < LOOP_TIMES; j++)
    {
        random_roi();

        cv::bitwise_or(mat1_roi, val, dst_roi);
        cv::ocl::bitwise_or(gmat1, val, gdst);

        cv::Mat cpu_dst;
        gdst_whole.download(cpu_dst);
        char sss[1024];
        sprintf(sss, "roicols=%d,roirows=%d,src1x=%d,src1y=%d,dstx=%d,dsty=%d,maskx=%d,masky=%d,src2x=%d,src2y=%d", roicols, roirows, src1x, src1y, dstx, dsty, maskx, masky, src2x, src2y);

        EXPECT_MAT_NEAR(dst, cpu_dst, 1e-5, sss);
    }
}

TEST_P(Bitwise_or, Scalar_Mask)
{
    for(int j = 0; j < LOOP_TIMES; j++)
    {
        random_roi();

        cv::bitwise_or(mat1_roi, val, dst_roi, mask_roi);
        cv::ocl::bitwise_or(gmat1, val, gdst, gmask);

        cv::Mat cpu_dst;
        gdst_whole.download(cpu_dst);
        char sss[1024];
        sprintf(sss, "roicols=%d,roirows=%d,src1x=%d,src1y=%d,dstx=%d,dsty=%d,maskx=%d,masky=%d,src2x=%d,src2y=%d", roicols, roirows, src1x, src1y, dstx, dsty, maskx, masky, src2x, src2y);

        EXPECT_MAT_NEAR(dst, cpu_dst, 1e-5, sss);
    }
}



////////////////////////////////bitwise_xor/////////////////////////////////////////////////

struct Bitwise_xor : ArithmTestBase {};

TEST_P(Bitwise_xor, Mat)
{
    for(int j = 0; j < LOOP_TIMES; j++)
    {
        random_roi();

        cv::bitwise_xor(mat1_roi, mat2_roi, dst_roi);
        cv::ocl::bitwise_xor(gmat1, gmat2, gdst);

        cv::Mat cpu_dst;
        gdst_whole.download(cpu_dst);
        char sss[1024];
        sprintf(sss, "roicols=%d,roirows=%d,src1x=%d,src1y=%d,dstx=%d,dsty=%d,maskx=%d,masky=%d,src2x=%d,src2y=%d", roicols, roirows, src1x, src1y, dstx, dsty, maskx, masky, src2x, src2y);

        EXPECT_MAT_NEAR(dst, cpu_dst, 0.0, sss);
    }
}

TEST_P(Bitwise_xor, Mat_Mask)
{
    for(int j = 0; j < LOOP_TIMES; j++)
    {
        random_roi();

        cv::bitwise_xor(mat1_roi, mat2_roi, dst_roi, mask_roi);
        cv::ocl::bitwise_xor(gmat1, gmat2, gdst, gmask);

        cv::Mat cpu_dst;
        gdst_whole.download(cpu_dst);
        char sss[1024];
        sprintf(sss, "roicols=%d,roirows=%d,src1x=%d,src1y=%d,dstx=%d,dsty=%d,maskx=%d,masky=%d,src2x=%d,src2y=%d", roicols, roirows, src1x, src1y, dstx, dsty, maskx, masky, src2x, src2y);

        EXPECT_MAT_NEAR(dst, cpu_dst, 0.0, sss);
    }
}
TEST_P(Bitwise_xor, Scalar)
{
    for(int j = 0; j < LOOP_TIMES; j++)
    {
        random_roi();

        cv::bitwise_xor(mat1_roi, val, dst_roi);
        cv::ocl::bitwise_xor(gmat1, val, gdst);

        cv::Mat cpu_dst;
        gdst_whole.download(cpu_dst);
        char sss[1024];
        sprintf(sss, "roicols=%d,roirows=%d,src1x=%d,src1y=%d,dstx=%d,dsty=%d,maskx=%d,masky=%d,src2x=%d,src2y=%d", roicols, roirows, src1x, src1y, dstx, dsty, maskx, masky, src2x, src2y);

        EXPECT_MAT_NEAR(dst, cpu_dst, 1e-5, sss);
    }
}

TEST_P(Bitwise_xor, Scalar_Mask)
{
    for(int j = 0; j < LOOP_TIMES; j++)
    {
        random_roi();

        cv::bitwise_xor(mat1_roi, val, dst_roi, mask_roi);
        cv::ocl::bitwise_xor(gmat1, val, gdst, gmask);

        cv::Mat cpu_dst;
        gdst_whole.download(cpu_dst);
        char sss[1024];
        sprintf(sss, "roicols=%d,roirows=%d,src1x=%d,src1y=%d,dstx=%d,dsty=%d,maskx=%d,masky=%d,src2x=%d,src2y=%d", roicols, roirows, src1x, src1y, dstx, dsty, maskx, masky, src2x, src2y);

        EXPECT_MAT_NEAR(dst, cpu_dst, 1e-5, sss);
    }
}


////////////////////////////////bitwise_not/////////////////////////////////////////////////

struct Bitwise_not : ArithmTestBase {};

TEST_P(Bitwise_not, Mat)
{
    for(int j = 0; j < LOOP_TIMES; j++)
    {
        random_roi();

        cv::bitwise_not(mat1_roi, dst_roi);
        cv::ocl::bitwise_not(gmat1, gdst);

        cv::Mat cpu_dst;
        gdst_whole.download(cpu_dst);
        char sss[1024];
        sprintf(sss, "roicols=%d,roirows=%d,src1x=%d,src1y=%d,dstx=%d,dsty=%d,maskx=%d,masky=%d,src2x=%d,src2y=%d", roicols, roirows, src1x, src1y, dstx, dsty, maskx, masky, src2x, src2y);

        EXPECT_MAT_NEAR(dst, cpu_dst, 0.0, sss);
    }
}


////////////////////////////////compare/////////////////////////////////////////////////
struct Compare : ArithmTestBase {};

TEST_P(Compare, Mat)
{
    if(mat1.type() == CV_8SC1)
        //if(mat1.type() != CV_8UC1 || mat1.type()!= CV_16UC1 || mat1.type()!= CV_16SC1|| mat1.type()!= CV_32SC1 || mat1.type()!= CV_32FC1|| mat1.type()!= CV_64FC1)
    {
        cout << "\tUnsupported type\t\n";
    }

    int cmp_codes[] = {CMP_EQ, CMP_GT, CMP_GE, CMP_LT, CMP_LE, CMP_NE};
    const char *cmp_str[] = {"CMP_EQ", "CMP_GT", "CMP_GE", "CMP_LT", "CMP_LE", "CMP_NE"};
    int cmp_num = sizeof(cmp_codes) / sizeof(int);

    for (int i = 0; i < cmp_num; ++i)
    {

        for(int j = 0; j < LOOP_TIMES; j++)
        {
            random_roi();

            cv::compare(mat1_roi, mat2_roi, dst_roi, cmp_codes[i]);
            cv::ocl::compare(gmat1, gmat2, gdst, cmp_codes[i]);

            cv::Mat cpu_dst;
            gdst_whole.download(cpu_dst);
            char sss[1024];
            sprintf(sss, "cmptype=%s, roicols=%d,roirows=%d,src1x=%d,src1y=%d,dstx=%d,dsty=%d,maskx=%d,masky=%d,src2x=%d,src2y=%d", cmp_str[i], roicols, roirows, src1x, src1y, dstx, dsty, maskx, masky, src2x, src2y);

            EXPECT_MAT_NEAR(dst, cpu_dst, 0.0, sss);
        }
    }

}


struct Pow : ArithmTestBase {};

TEST_P(Pow, Mat)
{
    if(mat1.depth() != CV_32F && mat1.depth() != CV_64F)
    {
        cout << "\tUnsupported type\t\n";
    }

    for(int j = 0; j < LOOP_TIMES; j++)
    {
        random_roi();
        double p = 4.5;
        cv::pow(mat1_roi, p, dst_roi);
        cv::ocl::pow(gmat1, p, gdst);

        cv::Mat cpu_dst;
        gdst_whole.download(cpu_dst);

        char sss[1024];
        sprintf(sss, "roicols=%d,roirows=%d,src1x=%d,src1y=%d,dstx=%d,dsty=%d,maskx=%d,masky=%d,src2x=%d,src2y=%d", roicols, roirows, src1x, src1y, dstx, dsty, maskx, masky, src2x, src2y);

        EXPECT_MAT_NEAR(dst, cpu_dst, 1, sss);
    }
}


struct MagnitudeSqr : ArithmTestBase {};

TEST_P(MagnitudeSqr, Mat)
{
    for(int j = 0; j < LOOP_TIMES; j++)
    {
        // random_roi();
        int64 start, end;
        start = cv::getTickCount();
        for(int i = 0; i < mat1.rows; ++i)
            for(int j = 0; j < mat1.cols; ++j)
            {
                float val1 = mat1.at<float>(i, j);
                float val2 = mat2.at<float>(i, j);

                ((float *)(dst.data))[i *dst.step/4 +j] = val1 * val1 + val2 * val2;

                //        float val1 =((float *)( mat1.data))[(i*mat1.step/8 +j)*2];
                //
                //     float val2 =((float *)( mat1.data))[(i*mat1.step/8 +j)*2+ 1 ];

                //  ((float *)(dst.data))[i*dst.step/4 +j]= val1 * val1 +val2 * val2;
            }
        end = cv::getTickCount();



        cv::ocl::oclMat clmat1(mat1), clmat2(mat2), cldst;
        cv::ocl::magnitudeSqr(clmat1, clmat2, cldst);

        cv::Mat cpu_dst;
        cldst.download(cpu_dst);
        char sss[1024];
        sprintf(sss, "roicols=%d,roirows=%d,src1x=%d,src1y=%d,dstx=%d,dsty=%d,maskx=%d,masky=%d,src2x=%d,src2y=%d", roicols, roirows, src1x, src1y, dstx, dsty, maskx, masky, src2x, src2y);

        EXPECT_MAT_NEAR(dst, cpu_dst, 1, sss);
    }
}


struct AddWeighted : ArithmTestBase {};

TEST_P(AddWeighted, Mat)
{
    for(int j = 0; j < LOOP_TIMES; j++)
    {
        random_roi();
        double alpha = 2.0, beta = 1.0, gama = 3.0;


        cv::addWeighted(mat1_roi, alpha, mat2_roi, beta, gama, dst_roi);

        //	cv::ocl::oclMat clmat1(mat1),clmat2(mat2),cldst;

        cv::ocl::addWeighted(gmat1, alpha, gmat2, beta, gama, gdst);


        cv::Mat cpu_dst;
        gdst_whole.download(cpu_dst);

        char sss[1024];
        sprintf(sss, "roicols=%d,roirows=%d,src1x=%d,src1y=%d,dstx=%d,dsty=%d,maskx=%d,masky=%d,src2x=%d,src2y=%d", roicols, roirows, src1x, src1y, dstx, dsty, maskx, masky, src2x, src2y);

        EXPECT_MAT_NEAR(dst, cpu_dst, 1e-5, sss);
    }
}





//********test****************

INSTANTIATE_TEST_CASE_P(Arithm, Lut, Combine(
                            Values(CV_8UC1, CV_8UC3, CV_8UC4),
                            Values(false))); // Values(false) is the reserved parameter

INSTANTIATE_TEST_CASE_P(Arithm, Exp, Combine(
                            Values(CV_32FC1, CV_32FC1),
                            Values(false))); // Values(false) is the reserved parameter

INSTANTIATE_TEST_CASE_P(Arithm, Log, Combine(
                            Values(CV_32FC1, CV_32FC1),
                            Values(false))); // Values(false) is the reserved parameter

INSTANTIATE_TEST_CASE_P(Arithm, Add, Combine(
                            Values(CV_8UC1, CV_8UC3,CV_8UC4, CV_32SC1, CV_32SC4, CV_32FC1,  CV_32FC4),
                            Values(false)));

INSTANTIATE_TEST_CASE_P(Arithm, Mul, Combine(
                            Values(CV_8UC1, CV_8UC3,CV_8UC4, CV_32SC1, CV_32SC4, CV_32FC1, CV_32FC4),
                            Values(false))); // Values(false) is the reserved parameter

INSTANTIATE_TEST_CASE_P(Arithm, Div, Combine(
                            Values(CV_8UC1, CV_8UC3,CV_8UC4, CV_32SC1, CV_32SC4, CV_32FC1, CV_32FC4),
                            Values(false))); // Values(false) is the reserved parameter


INSTANTIATE_TEST_CASE_P(Arithm, Absdiff, Combine(
                            Values(CV_8UC1,CV_8UC3, CV_8UC4, CV_32SC1, CV_32SC4, CV_32FC1, CV_32FC4),
                            Values(false))); // Values(false) is the reserved parameter

INSTANTIATE_TEST_CASE_P(Arithm, CartToPolar, Combine(
                            Values(CV_32FC1, CV_32FC3,CV_32FC4),
                            Values(false))); // Values(false) is the reserved parameter

INSTANTIATE_TEST_CASE_P(Arithm, PolarToCart, Combine(
                            Values(CV_32FC1, CV_32FC3,CV_32FC4),
                            Values(false))); // Values(false) is the reserved parameter

INSTANTIATE_TEST_CASE_P(Arithm, Magnitude, Combine(
                            Values(CV_32FC1, CV_32FC3,CV_32FC4),
                            Values(false))); // Values(false) is the reserved parameter

INSTANTIATE_TEST_CASE_P(Arithm, Transpose, Combine(
                            Values(CV_8UC1, CV_8UC3,CV_8UC4, CV_32SC1, CV_32FC1),
                            Values(false))); // Values(false) is the reserved parameter

INSTANTIATE_TEST_CASE_P(Arithm, Flip, Combine(
                            Values(CV_8UC1, CV_8UC3,CV_8UC4, CV_32SC1, CV_32SC4, CV_32FC1, CV_32FC4),
                            Values(false))); // Values(false) is the reserved parameter

INSTANTIATE_TEST_CASE_P(Arithm, MinMax, Combine(
                            Values(CV_8UC1, CV_32SC1, CV_32FC1),
                            Values(false)));

INSTANTIATE_TEST_CASE_P(Arithm, MinMaxLoc, Combine(
                            Values(CV_8UC1, CV_32SC1, CV_32FC1),
                            Values(false)));

INSTANTIATE_TEST_CASE_P(Arithm, Sum, Combine(
                            Values(CV_8U, CV_32S, CV_32F),
                            Values(false)));

INSTANTIATE_TEST_CASE_P(Arithm, CountNonZero, Combine(
                            Values(CV_8U, CV_32S, CV_32F),
                            Values(false)));


INSTANTIATE_TEST_CASE_P(Arithm, Phase, Combine(Values(CV_32FC1, CV_32FC3,CV_32FC4), Values(false)));
// Values(false) is the reserved parameter


INSTANTIATE_TEST_CASE_P(Arithm, Bitwise_and, Combine(
                            Values(CV_8UC1, CV_32SC1, CV_32SC4, CV_32FC1,CV_32FC3, CV_32FC4), Values(false)));
//Values(false) is the reserved parameter

INSTANTIATE_TEST_CASE_P(Arithm, Bitwise_or, Combine(
                            Values(CV_8UC1, CV_32SC1, CV_32FC1, CV_32FC3,CV_32FC4), Values(false)));
//Values(false) is the reserved parameter

INSTANTIATE_TEST_CASE_P(Arithm, Bitwise_xor, Combine(
                            Values(CV_8UC1, CV_32SC1, CV_32FC1, CV_32FC3,CV_32FC4), Values(false)));
//Values(false) is the reserved parameter

INSTANTIATE_TEST_CASE_P(Arithm, Bitwise_not, Combine(
                            Values(CV_8UC1, CV_32SC1, CV_32FC1, CV_32FC3,CV_32FC4), Values(false)));
//Values(false) is the reserved parameter

INSTANTIATE_TEST_CASE_P(Arithm, Compare, Combine(Values(CV_8UC1, CV_32SC1, CV_32FC1), Values(false)));
// Values(false) is the reserved parameter

INSTANTIATE_TEST_CASE_P(Arithm, Pow, Combine(Values(CV_32FC1, CV_32FC3, CV_32FC4), Values(false)));
// Values(false) is the reserved parameter

INSTANTIATE_TEST_CASE_P(Arithm, MagnitudeSqr, Combine(
                            Values(CV_32FC1, CV_32FC1),
                            Values(false))); // Values(false) is the reserved parameter

INSTANTIATE_TEST_CASE_P(Arithm, AddWeighted, Combine(
                            Values(CV_8UC1, CV_32SC1, CV_32FC1),
                            Values(false))); // Values(false) is the reserved parameter



#endif // HAVE_OPENCL
