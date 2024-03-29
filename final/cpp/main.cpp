#include "estimating.hpp"
#include "geometry.hpp"
#include "globla.hpp"

#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <vector>

#include <opencv2/opencv.hpp>

void Usage(char **argv) {
    fprintf(stderr, "Usage: %s <left-image> <right-image> <calib.txt>\n",
            argv[0]);
}

int main(int argc, char **argv) {
    /* [Testing area] */
    // std::cout << std::boolalpha;
    // std::cout << std::is_trivial<CamConf>::value << std::endl;
    // std::cout << std::is_trivial<SpatialPoint>::value << std::endl;
    /* [/Testing area] */

    /* [Variables] */
    cv::Mat limg, rimg;
    int     factor = 1;
    /* [/Variables] */

    /* [Parse args] */
    if (argc < 4) {
        Usage(argv);
        return 1;
    }
    limg     = cv::imread(argv[1], cv::IMREAD_COLOR);
    rimg     = cv::imread(argv[2], cv::IMREAD_COLOR);
    int rows = limg.rows;
    int cols = rimg.cols;
    if (rows != rimg.rows || cols != rimg.cols) {
        eprintf("The given 2 stereo images has different sizes\n");
    }
    MiscConf conf  = read_calib(argv[3]);
    CamConf  lconf = conf.left;
    CamConf  rconf = conf.right;
    /* [/Parse args] */

    cv::Mat l_rect, r_rect;
    /* [Stereo rectification] */
    flt              baseline = std::numeric_limits<flt>::lowest();
    std::vector<ppp> pixel_map =
        stereo_rectification(limg, rimg, lconf, rconf, l_rect, r_rect);
    cv::imwrite("l_rect.jpg", l_rect);
    cv::imwrite("r_rect.jpg", r_rect);
    vprintf("Rectified images written\n");
    /* [/Stereo rectification] */
    // /* [No rectification] */
    // l_rect = limg;
    // r_rect = rimg;
    // std::vector<ppp> pixel_map;
    // /* [/No rectification] */

    int wr = 5;

    /* [SAD] */
    cv::Mat disp_SAD = SAD(l_rect, r_rect, wr, conf);
    disp_SAD         = map_back(pixel_map, rows, cols, disp_SAD);
    cv::imwrite("disp_SAD.pgm", disp_SAD);
    cv::Mat disp_SAD_vis = visualize(disp_SAD);
    cv::imwrite("disp_SAD.jpg", disp_SAD_vis);
    /* [/SAD] */

    /* [NCC] */
    cv::Mat disp_NCC = NCC(l_rect, r_rect, wr, conf);
    disp_NCC         = map_back(pixel_map, rows, cols, disp_NCC);
    cv::imwrite("disp_NCC.pgm", disp_NCC);
    cv::Mat disp_NCC_vis = visualize(disp_NCC);
    cv::imwrite("disp_NCC.jpg", disp_NCC_vis);
    /* [/NCC] */

    /* [Global] */
    cv::Mat data        = downsample<int>(disp_NCC, factor);
    cv::Mat disp_global = global_optimization(data, conf);
    disp_global         = upsample<int>(disp_global, factor);
    cv::imwrite("disp_global.pgm", disp_global);
    cv::Mat disp_global_vis = visualize(disp_global);
    cv::imwrite("disp_global.jpg", disp_global_vis);
    /* [/Global] */

    return 0;
}

// Author: Blurgy <gy@blurgy.xyz>
// Date:   Feb 25 2021, 15:34 [CST]
