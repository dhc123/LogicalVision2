/* Sampling module
 * ================================
 * Version: 2.0
 * Author: Wang-Zhou Dai <dai.wzero@gmail.com>
 */

#ifndef _SAMPLER_HPP
#define _SAMPLER_HPP

#include "utils.hpp"

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>  // OpenCV window I/O
#include <armadillo>
#include <SWI-cpp.h>
#include <SWI-Prolog.h>

#include <iostream> // for standard I/O
#include <cstdlib>
#include <algorithm>
#include <string>   // for strings
#include <cmath>

using namespace std;
using namespace cv;

#define SWAP(a, b) {a = a + b; b = a - b; a = a - b;}

enum { XY_SHIFT = 16, XY_ONE = 1 << XY_SHIFT, DRAWING_STORAGE_BLOCK = (1 << 12) - 256 };

/********* declarations *********/

/* bound an local area in 3-d space and return the left/right up/down most
 *     points of the local area
 * @point: center
 * @radius: radius of the local area
 * @bound: boundary of the global space (> 0)
 * @return: left/right up/down most point of this area (grid)
 */
vector<Scalar> bound_scalar_3d(Scalar point, Scalar radius, Scalar bound);

/* get LAB color of a point (or average color in its neighborhood)
 * @images: image sequence
 * @point: position of the interest point
 * @radius: radius of the ellipsoid of the local area
 * @return: color in current location
 * REMARK: the 3 dimensions are width, height, duration
 */
Scalar cv_imgs_point_color_loc(vector<Mat> *images, Scalar point,
                               Scalar radius = Scalar(0, 0, 0));

/* calculate image local variance
 * @images: image sequence
 * @point: position of the interest point
 * @radius: radius of the ellipsoid of the local area
 * @return: variation of all the points
 * REMARK: the 3 dimensions are width, height, duration
 */
double cv_imgs_point_var_loc(vector<Mat> *images, Scalar point,
                             Scalar radius = Scalar(3, 3, 0));

/* calculate image gradient with Scharr operator
 * @images: image sequence
 * @point: position of the interest point
 * @return: variation of all the points 
 */
double cv_imgs_point_scharr(vector<Mat> *images, Scalar point);
vector<double> cv_imgs_points_scharr(vector<Mat> *images,
                                     vector<Scalar> points);

/* calculate image local color of a set of points
 * @images: image sequence
 * @points: position of the interest points
 * @radius: radius of the ellipsoid of the local area
 * @return: variation of all the points
 * REMARK: the 3 dimensions are width, height, duration
 */
vector<Scalar> cv_imgs_points_color_loc(vector<Mat> *images,
                                        vector<Scalar> points,
                                        Scalar radius = Scalar(0, 0, 0));

/* calculate image local variance of a set of points
 * @images: image sequence
 * @points: position of the interest points
 * @radius: radius of the ellipsoid of the local area
 * @return: variation of all the points
 * REMARK: the 3 dimensions are width, height, duration
 */
vector<double> cv_imgs_points_var_loc(vector<Mat> *images,
                                      vector<Scalar> points,
                                      Scalar radius = Scalar(3, 3, 0));
/* get all points on a line
 * @point: position of a point on the line
 * @direction: direction of the line
 * @bound: size of the 3d-space
 * @return: all points on the line
 */
vector<Scalar> get_line_points(Scalar point, Scalar direction, Scalar bound);

/* get all points on a line segment
 * @start: position of a point on the line
 * @end: direction of the line
 * @bound: size of the 3d-space
 * @return: all points on the line segment
 */
vector<Scalar> get_line_seg_points(Scalar start, Scalar end, Scalar bound);

/* get all points on an ellipse (ONLY FOR 2D IMAGE, SO z ALWAYS EQUALS TO 0)
 *    When sampling a specific frame, remember to change z value of all points
 * @centre: centre point of the circle
 * @param: parameters of an ellipse (long/short axis length and
 *     axis angle (DEG, not RAD!!!))
 * @bound: size of the 3d-space
 * @return: all points on the circle
 */
vector<Scalar> get_ellipse_points(Scalar centre, Scalar param, Scalar bound);


/* sample a line in 3d space and return the points whose local variance 
 * exceeds the given threshold
 * @images: image sequence
 * @point: position of a point
 * @direction: direction of the line
 * @var_threshold: variance threshold
 * @loc_radius: local area size
 * @return: points meet the requirement
 */
vector<Scalar> cv_line_pts_var_geq_T(vector<Mat> *images, Scalar point,
                              Scalar direction, double var_threshold = 2.0,
                              Scalar loc_radius = Scalar(5, 5, 0));

/* sample a line in 3d space and return the points whose local variance 
 * exceeds the given threshold
 * @images: image sequence
 * @start: start point of line segment
 * @end: end point of line segment
 * @var_threshold: variance threshold
 * @loc_radius: local area size
 * @return: points meet the requirement
 */
vector<Scalar> cv_line_seg_pts_var_geq_T(vector<Mat> *images, Scalar start,
                              Scalar end, double var_threshold = 2.0,
                              Scalar loc_radius = Scalar(5, 5, 0));

/* sample a line in 3d space and return the points whose scharr gradient
 * exceeds the given threshold WITH THINNING
 * @images: image sequence
 * @point: position of a point
 * @direction: direction of the line
 * @grad_threshold: variance threshold
 * @return: points meet the requirement
 */
vector<Scalar> cv_line_pts_scharr_geq_T(vector<Mat> *images,
                                        Scalar point,
                                        Scalar direction,
                                        double var_threshold = 5.0);
vector<Scalar> cv_line_seg_pts_scharr_geq_T(vector<Mat> *images,
                                            Scalar start,
                                            Scalar end,
                                            double var_threshold = 5.0);

/* bresenham's line algorithm
 * @current: starting point to be extended
 * @direction: direction of line extention
 * @inc: increment direction
 * @bound: 3-d space size
 * @&points: vector line points
 * @front: whether new points are pushed to vector's front
 */
void bresenham(Scalar current, Scalar direction, Scalar inc,
               Scalar bound, vector<Scalar> *points);

/* fits a set of points in to a ellipse on an 2d image
 * Based on:
 *   Fitzgibbon, A.W., Pilu, M., and Fischer R.B., Direct least squares
 *   fitting of ellipsees, Proc. of the 13th Internation Conference on Pattern
 *   Recognition, pp 253–257, Vienna, 1996.
 * @points: points for fitting
 * @centre: center of the ellipse
 * @param: other parameters (long/short axis length and axis angle)
 */
void fit_ellipse(vector<Scalar> points, Scalar& centre, Scalar& param);

/* Compare two sets of points' histogram distributions, return KL divergence
 * @images: image sequence
 * @points_1: point set 1
 * @points_2: point set 2
 */
double compare_hist(vector<Mat> *images,
                    vector<Scalar> points_1,
                    vector<Scalar> points_2);

/* determines whether a 3D point is out of a 3D space (positive)
 * @point: input point
 * @bound: boundary of the 3D space (>= 0)
 */
bool out_of_canvas(Scalar point, Scalar bound) {
    int x = point[0];
    int y = point[1];
    int z = point[2];
    
    if (x >= 0 && x <= bound[0] - 1 &&
        y >= 0 && y <= bound[1] - 1 &&
        z >= 0 && z <= bound[2] - 1) 
        return false;
    else
        return true;
}
/* determines whether two points are continuous
 * @p1, p2: input points
 */
bool point_cont(Scalar p1, Scalar p2) {
    arma::vec v = {(double) abs(p1[0] - p2[0]),
                   (double) abs(p1[1] - p2[1]),
                   (double) abs(p1[2] - p2[2])};
    if (v.max() < 2)
        return true;
    else
        return false;
}


/********* implementations *********/
double cv_imgs_point_var_loc(vector<Mat> *images, Scalar point,
                             Scalar radius) {
    // get left_up_most and right_down_most and enumerate all pixels
    int w = (*images)[0].cols;
    int h = (*images)[0].rows;
    int d = images->size();
    vector<Scalar> bounds = bound_scalar_3d(point, radius, Scalar(w, h, d));
    Scalar left_up_most = bounds[0];
    Scalar right_down_most = bounds[1];

    // enumerate pixels that position in local ellipsoid
    // the ellipsoid is:
    //  ((X - P1)/W)^2 + ((Y - P2)/H)^2 + ((Z - P3)/D )^2 = 1
    
    // collect pixels
    vector<Scalar> point_set;
    for (auto i = left_up_most[2]; i <= right_down_most[2]; i++)
        for (auto r = left_up_most[1]; r <= right_down_most[1]; r++)
            for (auto c = left_up_most[0]; c <= right_down_most[0]; c++) {
                double p1, p2, p3;
                p1 = radius[0] > 0 ?
                    pow(((double) c - point[0])/radius[0], 2) : 0;
                p2 = radius[1] > 0 ?
                    pow(((double) r - point[1])/radius[1], 2) : 0;
                p3 = radius[2] > 0 ?
                    pow(((double) i - point[2])/radius[2], 2) : 0;
                /* debug
                cout << c << "-" << point[0] << ","
                     << r << "-" << point[1] << ","
                     << i << "-" << point[2] << "," << "\n\t";
                cout << p1 << ","
                     << p2 << ","
                     << p3 << endl;
                */
                if (p1 + p2 + p3 <= 1.0) {
                    point_set.push_back(Scalar(c, r, i));

                }
            }

    int count = point_set.size();
    Scalar avg = Scalar(.0, .0, .0);
    // compute average
    for (auto it = point_set.begin(); it != point_set.end(); ++it) {
        Scalar pos = *it; // position of the pixel
        Mat img = (*images)[pos[2]];
        Vec3b pixel = img.at<Vec3b>(pos[1], pos[0]);
        for (int channel = 0; channel < 3; channel++)
            avg[channel] += pixel[channel];

    }
    
    avg = avg/(double) count;
    //cout << "avg:\t" << avg[0] << ", " << avg[1] << ", " << avg[2] << endl;    
    // compute variance
    Scalar var(.0, .0, .0);
    for (auto it = point_set.begin(); it != point_set.end(); ++it) {
        Scalar pos = *it; // position of the pixel
        Mat img = (*images)[pos[2]];
        Vec3b pixel = img.at<Vec3b>(pos[1], pos[0]);
        for (int channel = 0; channel < 3; channel++) {
            double res = pixel[channel] - avg[channel];
            var[channel] += pow(res, 2);
        }
    }

    //var = var/(double) (count - 1);
    //cout << "var:\t" << var[0] << ", " << var[1] << ", " << var[2] << endl;
    double std = sqrt(var[0] / (double) (count - 1))
        + sqrt(var[1] / (double) (count - 1))
        + sqrt(var[2] / (double) (count - 1));
    return std;
}

double cv_imgs_point_scharr(vector<Mat> *images, Scalar point) {
    // get left_up_most and right_down_most and enumerate all pixels
    int w = (*images)[0].cols;
    int h = (*images)[0].rows;
    // point position
    int x = point[0];
    int y = point[1];
    int frame = point[2];
    if (x < 1 || y < 1 || x > w - 2 || y > h - 2)
        return 0.0;
    // build brightness matrix
    Mat img = (*images)[frame];
    arma::mat A = {{(double) img.at<Vec3b>(y-1, x-1)[0],
                    (double) img.at<Vec3b>(y-1, x)[0],
                    (double) img.at<Vec3b>(y-1, x+1)[0]},
                   {(double) img.at<Vec3b>(y, x-1)[0],
                    (double) img.at<Vec3b>(y, x)[0],
                    (double) img.at<Vec3b>(y, x+1)[0]},
                   {(double) img.at<Vec3b>(y+1, x-1)[0],
                    (double) img.at<Vec3b>(y+1, x)[0],
                    (double) img.at<Vec3b>(y+1, x+1)[0]}};
    // calculate Scharr gradients in brightness channel
    arma::mat gx = {{3, 10, 3},
                    {0, 0, 0},
                    {-3, -10, -3}};
    arma::mat gy = {{3, 0, -3},
                    {10, 0, -10},
                    {3, 0, -3}};
    gx = gx/32;
    gy = gy/32;
    double Gx = gx(2,2)*A(0,0) + gx(2,1)*A(0,1) + gx(2,0)*A(0,2)
        + gx(1,2)*A(1,0) + gx(1,1)*A(1,1) + gx(1,0)*A(1,2)
        + gx(0,2)*A(2,0) + gx(0,1)*A(2,1) + gx(0,0)*A(2,2);
    double Gy = gy(2,2)*A(0,0) + gy(2,1)*A(0,1) + gy(2,0)*A(0,2)
        + gy(1,2)*A(1,0) + gy(1,1)*A(1,1) + gy(1,0)*A(1,2)
        + gy(0,2)*A(2,0) + gy(0,1)*A(2,1) + gy(0,0)*A(2,2);
    double G = sqrt(Gx*Gx + Gy*Gy);
    return G;
}

Scalar cv_imgs_point_color_loc(vector<Mat> *images, Scalar point,
                               Scalar radius) {
    // canvas size
    int w = (*images)[0].cols;
    int h = (*images)[0].rows;
    int d = images->size();
    // bounded neighborhood
    vector<Scalar> bounds = bound_scalar_3d(point, radius, Scalar(w, h, d));
    Scalar left_up_most = bounds[0];
    Scalar right_down_most = bounds[1];
    
    // collect pixels 
    vector<Scalar> point_set;
    for (auto i = left_up_most[2]; i <= right_down_most[2]; i++)
        for (auto r = left_up_most[1]; r <= right_down_most[1]; r++)
            for (auto c = left_up_most[0]; c <= right_down_most[0]; c++) {
                double p1, p2, p3;
                p1 = radius[0] > 0 ?
                    pow(((double) c - point[0])/radius[0], 2) : 0;
                p2 = radius[1] > 0 ?
                    pow(((double) r - point[1])/radius[1], 2) : 0;
                p3 = radius[2] > 0 ?
                    pow(((double) i - point[2])/radius[2], 2) : 0;
                /* debug
                cout << c << "-" << point[0] << ","
                     << r << "-" << point[1] << ","
                     << i << "-" << point[2] << "," << "\n\t";
                cout << p1 << ","
                     << p2 << ","
                     << p3 << endl;
                */
                if (p1 + p2 + p3 <= 1.0) {
                    point_set.push_back(Scalar(c, r, i));
                }
            }

    int count = point_set.size();
    Scalar avg = Scalar(.0, .0, .0);
    // compute average
    for (auto it = point_set.begin(); it != point_set.end(); ++it) {
        Scalar pos = *it; // position of the pixel
        Mat img = (*images)[pos[2]];
        Vec3b pixel = img.at<Vec3b>(pos[1], pos[0]);
        for (int channel = 0; channel < 3; channel++)
            avg[channel] += pixel[channel];

    }
    
    avg = avg/(double) count;
    return avg;
}

vector<Scalar> cv_imgs_points_color_loc(vector<Mat> *images,
                                        vector<Scalar> points,
                                        Scalar radius) {
    vector<Scalar> re;
    for (auto it = points.begin(); it != points.end(); ++it)
        re.push_back(cv_imgs_point_color_loc(images, (Scalar) *it, radius));
    return re;
}

vector<double> cv_imgs_points_var_loc(vector<Mat> *images,
                                      vector<Scalar> points,
                                      Scalar radius) {
    vector<double> re;
    for (auto it = points.begin(); it != points.end(); ++it)
        re.push_back(cv_imgs_point_var_loc(images, (Scalar) *it, radius));
    return re;
}

vector<double> cv_imgs_points_scharr(vector<Mat> *images,
                                     vector<Scalar> points) {
    vector<double> re;
    for (auto it = points.begin(); it != points.end(); ++it)
        re.push_back(cv_imgs_point_scharr(images, (Scalar) *it));
    return re;
}

vector<Scalar> bound_scalar_3d(Scalar point, Scalar radius, Scalar bound) {
    Scalar left_up_most(10000, 10000, 10000);
    Scalar right_down_most(-10000, -10000, -10000);
    // the order of Mat is "column, row, duration"
    for (auto i = 0; i < 3; i++) {
        left_up_most[i] = point[i] - radius[i] < 0 ? 0 : point[i] - radius[i];
        right_down_most[i] = point[i] + radius[i] >= bound[i] ? bound[i] - 1 : point[i] + radius[i];
    }
    vector<Scalar> re;
    re.push_back(left_up_most);
    re.push_back(right_down_most);
    return re;
}

vector<Scalar> cv_line_pts_var_geq_T(vector<Mat> *images, Scalar point,
                              Scalar direction, double var_threshold,
                              Scalar loc_radius){
    vector<Scalar> re;
    // size of the 3-d space
    Scalar bound((*images)[0].cols, (*images)[0].rows, images->size());
    // get all points on this line
    vector<Scalar> line_points = get_line_points(point, direction, bound);
    // evaluate local variance of all points
    for (auto it = line_points.begin(); it != line_points.end(); ++it) {
        Scalar pt = *it;
        double var =  cv_imgs_point_var_loc(images, pt, loc_radius);
        // debug
        cout << (*it)[0] << ","
             << (*it)[1] << ","
             << (*it)[2] << "\t";
        cout << var << endl;
        //
        if (var >= var_threshold)
            re.push_back(pt);
    }
    return re;
}

vector<Scalar> cv_line_seg_pts_var_geq_T(vector<Mat> *images, Scalar start,
                              Scalar end, double var_threshold,
                              Scalar loc_radius){
    vector<Scalar> re;
    // size of the 3-d space
    Scalar bound((*images)[0].cols, (*images)[0].rows, images->size());
    // get all points on this line
    vector<Scalar> line_points = get_line_seg_points(start, end, bound);
    // evaluate local variance of all points
    for (auto it = line_points.begin(); it != line_points.end(); ++it) {
        Scalar pt = *it;
        double var =  cv_imgs_point_var_loc(images, pt, loc_radius);
        /* debug
        cout << (*it)[0] << ","
             << (*it)[1] << ","
             << (*it)[2] << "\t";
        cout << var << endl;
        */
        if (var >= var_threshold)
            re.push_back(pt);
    }
    return re;
}

// 3D Bresenham's line generation
vector<Scalar> get_line_points(Scalar point, Scalar direction,
                               Scalar bound) {
    // REMARK: all coordinate oders as "column, row, duration"
    vector<Scalar> re; // returned point list

    if (direction[0] == 0 &&
        direction[1] == 0 &&
        direction[2] == 0)
        return re;
    
    // increment in each direction
    int c_inc = direction[0] > 0 ? 1 : -1;
    int r_inc = direction[1] > 0 ? 1 : -1;
    int d_inc = direction[2] > 0 ? 1 : -1;
    Scalar inc;
    inc[0] = c_inc;
    inc[1] = r_inc;
    inc[2] = d_inc;
    bresenham(point, direction, inc, bound, &re);

    reverse(re.begin(), re.end());
    re.push_back(point); // insert itself
    // grow in another direction
    c_inc = -1 * c_inc;
    r_inc = -1 * r_inc;
    d_inc = -1 * d_inc;
    inc[0] = c_inc;
    inc[1] = r_inc;
    inc[2] = d_inc;
    bresenham(point, direction, inc, bound, &re);
    reverse(re.begin(), re.end());

    return re;
}

vector<Scalar> get_line_seg_points(Scalar start, Scalar end, Scalar bound) {
    vector<Scalar> re; // returned point list
    
    int x = start[0];
    int y = start[1];
    int z = start[2];
    int x2 = end[0];
    int y2 = end[1];
    int z2 = end[2];

    if (x == x2 &&
        y == y2 &&
        z == z2)
        return re;
    // boundary check
    if (out_of_canvas(start, bound) && out_of_canvas(end, bound))
        return re;
    if (out_of_canvas(start, bound) && !out_of_canvas(end, bound)) {
        // if start point is out of canvas and end point is in the canvas,
        // swap the start and the end point
        SWAP(x, x2);
        SWAP(y, y2);
        SWAP(z, z2);
    }
    
    re.push_back(Scalar(x, y, z));
    // bresenham for line segment
    int dx = x2 - x;
    int dy = y2 - y;
    int dz = z2 - z;

    int x_inc = dx > 0 ? 1 : -1;
    int y_inc = dy > 0 ? 1 : -1;
    int z_inc = dz > 0 ? 1 : -1;

    int Adx = abs(dx);
    int Ady = abs(dy);
    int Adz = abs(dz);
    
    int dx2 = Adx*2;
    int dy2 = Ady*2;
    int dz2 = Adz*2;

    int err_1, err_2;

    while (!out_of_canvas(Scalar(x, y, z), bound) &&
           !(x == x2 && y == y2 && z == z2)) {
        if (Adx >= Ady && Adx >= Adz) {
            err_1 = dy2 - Adx;
            err_2 = dz2 - Adx;
            for (int cont = 0; cont < Adx; cont++) {
                if (err_1 > 0) { 
                    y += y_inc;
                    err_1 -= dx2;
                }
                if (err_2 > 0) {
                    z += z_inc;
                    err_2 -= dx2;
                }
                err_1 += dy2;
                err_2 += dz2;
                x += x_inc;
                Scalar point(x, y, z);
                if (!out_of_canvas(point, bound) &&
                    !(x == x2 && y == y2 && z == z2))
                    re.push_back(point);
                else
                    break;
            }
        }
        
        if (Ady > Adx && Ady >= Adz) {
            err_1 = dx2 - Ady;
            err_2 = dz2 - Ady;
            for (int cont = 0; cont < Ady; cont++) {
                if (err_1 > 0) {
                    x += x_inc;
                    err_1 -= dy2;
                }
                if (err_2 > 0) { 
                    z += z_inc;
                    err_2 -= dy2;
                }
                err_1 += dx2;
                err_2 += dz2;
                y += y_inc;
                Scalar point(x, y, z);
                if (!out_of_canvas(point, bound) &&
                    !(x == x2 && y == y2 && z == z2))
                    re.push_back(point);
                else
                    break;
            }
        }
        
        if (Adz > Adx && Adz > Ady) {
            err_1 = dy2 - Adz;
            err_2 = dx2 - Adz;
            for (int cont = 0; cont < Adz; cont++) {
                if (err_1 > 0) {
                    y += y_inc;
                    err_1 -= dz2;
                }
                if (err_2 > 0) {
                    x += x_inc;
                    err_2 -= dz2;
                }
                err_1 += dy2;
                err_2 += dx2;
                z += z_inc;
                Scalar point(x, y, z);
                if (!out_of_canvas(point, bound) &&
                    !(x == x2 && y == y2 && z == z2))
                    re.push_back(point);
                else
                    break;
            }
        }
    }
    re.push_back(Scalar(x2, y2, z2));
    return re;
}
/* bresenham for line, NOT segment */
void bresenham(Scalar current, Scalar direction, Scalar inc,
               Scalar bound, vector<Scalar> *points) {
    // direction
    int dx = direction[0];
    int dy = direction[1];
    int dz = direction[2];
    
    int x_inc = inc[0];
    int y_inc = inc[1];
    int z_inc = inc[2];
    
    int Adx = abs(dx);
    int Ady = abs(dy);
    int Adz = abs(dz);
    
    int dx2 = Adx*2;
    int dy2 = Ady*2;
    int dz2 = Adz*2;
    
    int x = current[0];
    int y = current[1];
    int z = current[2];

    int err_1, err_2;
    
    if (Adx >= Ady && Adx >= Adz) { 
        err_1 = dy2 - Adx;
        err_2 = dz2 - Adx;
        for (int cont = 0; cont < Adx; cont++) {
            if (err_1 > 0) { 
                y += y_inc;
                err_1 -= dx2;
            }
            if (err_2 > 0) {
                z += z_inc;
                err_2 -= dx2;
            }
            err_1 += dy2;
            err_2 += dz2;
            x += x_inc;
            Scalar point(x, y, z);
            if (!out_of_canvas(point, bound))
                points->push_back(point);
            else
                break;
        }
    }
    
    if (Ady > Adx && Ady >= Adz) {
        err_1 = dx2 - Ady;
        err_2 = dz2 - Ady;
        for (int cont = 0; cont < Ady; cont++) {
            if (err_1 > 0) {
                x += x_inc;
                err_1 -= dy2;
            }
            if (err_2 > 0) { 
                z += z_inc;
                err_2 -= dy2;
            }
            err_1 += dx2;
            err_2 += dz2;
            y += y_inc;
            Scalar point(x, y, z);
            if (!out_of_canvas(point, bound))
                points->push_back(point);
            else
                break;
            
        }
    }
   
    if (Adz > Adx && Adz > Ady) {
        err_1 = dy2 - Adz;
        err_2 = dx2 - Adz;
        for (int cont = 0; cont < Adz; cont++) {
            if (err_1 > 0) {
                y += y_inc;
                err_1 -= dz2;
            }
            if (err_2 > 0) {
                x += x_inc;
                err_2 -= dz2;
            }
            err_1 += dy2;
            err_2 += dx2;
            z += z_inc;
            Scalar point(x, y, z);
            if (!out_of_canvas(point, bound))
                points->push_back(point);
            else
                break;
        }
    }
    
    Scalar point(x, y, z);
    if (!out_of_canvas(point, bound))
        bresenham(point, direction, inc, bound, points);
}

void fit_ellipse(vector<Scalar> points, Scalar& centre, Scalar& param) {
    assert(points.size() >= 5); // must use more than 5 points
    int frame = points[0][2];
    arma::mat pts(points.size(), 2);
    for (arma::uword i = 0; i < points.size(); i++) {
        arma::rowvec p = {(double) points[i][0], (double) points[i][1]};
        pts.row(i) = p;
    }
    arma::vec x = pts.col(0);
    arma::vec y = pts.col(1);
    // get matrix D = (x^2, xy, y^2, x, y, 1)
    arma::mat D(x.size(), 6);
    D.col(0) = x % x;
    D.col(1) = x % y;
    D.col(2) = y % y;
    D.col(3) = x;
    D.col(4) = y;
    D.col(5) = arma::vec(x.size(), arma::fill::ones);
    // S = D^T * D
    arma::mat S = D.t() * D;
    // C is a matrix filled with zeros except C_{3,1}=C_{1,3}=2, C_{1,1}=-1
    arma::mat C(6, 6, arma::fill::zeros);
    C(0, 2) = 2;
    C(2, 0) = 2;
    C(1, 1) = -1;
    // solve eigen decomposition
    arma::cx_vec eigval;
    arma::cx_mat eigvec;
    arma::eig_gen(eigval, eigvec, inv(S) * C);
    // get largest eigen value and vector
    arma::uword n;
    (arma::abs(real(eigval))).max(n);
    // a is parameter of the ellipse equation axx, axy, ayy, ax, ay, a1
    arma::vec a_vec = real(eigvec.col(n));
    // transform a into other parameters
    double b = a_vec[1] / 2;
    double c = a_vec[2];
    double d = a_vec[3] / 2; 
    double f = a_vec[4] / 2;
    double g = a_vec[5];
    double a = a_vec[0];
    double num = b * b - a * c;
    int x0 = int((c * d - b * f) / num);
    int y0 = int((a * f - b * d) / num);
    centre = Scalar(x0, y0, frame); // centre of ellipse
    double up = 2*(a * f * f + c * d * d + g * b * b
                   - 2 * b * d * f - a * c * g);
    double down1 = (b * b - a * c) *
        ((c - a) * sqrt(1 + 4 * b * b / ((a - c) * (a - c))) - (c + a));
    double down2 = (b * b - a * c) *
        ((a - c) * sqrt(1 + 4 * b * b / ((a - c) * (a - c))) - (c + a));
    double res1 = sqrt(up / down1); // axis length
    double res2 = sqrt(up / down2); // axis length
    double angle; // angle of ellipse
    if (b == 0)
        if (a > c)
            angle = 0;
        else
            angle = arma::datum::pi/2;
    else
        if (a > c)
            angle = atan(2 * b / (a - c)) / 2;
        else
            angle = arma::datum::pi/2 + atan(2 * b / (a - c)) / 2;
    // save parameters
    angle = 90 + (angle * 180/arma::datum::pi); // readjust "angle" to OpenCV radius axis
    if (res1 >= res2)
        param = Scalar(round(res1), round(res2), round(angle));
    else
        param = Scalar(round(res2), round(res1), round(angle));
}

vector<Scalar> get_ellipse_points(Scalar centre, Scalar param, Scalar bound) {
    vector<Scalar> re;
    // detailness of using polylines to approximate ellipse
    int width = std::abs((int) param[0]);
    int height = std::abs((int) param[1]);
    if (width > height)
        SWAP(width, height);

    std::vector<Point> v; // vertices
    //cout << "Starting OpenCV ellipse2Poly.\n";
    ellipse2Poly(Point(centre[0], centre[1]),
                 Size(width, height),
                 param[2], 0, 360, 3, v); // the "3" before "v" is a parameter to control the accuracy of the polygon-approximation to ellipse
    if((int) v.size() <= 0)
        return re;
    //cout << "OpenCV ellipse2Poly done.\n";
    //cout << "Totally " << v.size() << " vertices.\n";

    Point p0;
    p0 = v[0];
    //cout << "Starting polygon to line segment points.\n";
    for(int i = 1; i < (int) v.size(); i++) {
        //cout << "converting point #" << i;
        Point p = v[i];
        //cout << " (" << p.x << ", " << p.y << ")";
        // points on line segment
        vector<Scalar> temp_pts = get_line_seg_points(Scalar(p0.x, p0.y, centre[2]), Scalar(p.x, p.y, centre[2]), bound);
        re += temp_pts;
        //cout << ". Points num: " << temp_pts.size() << endl;;
        p0 = p;
    }
    //cout << "polygon to line segment points done.\n";
    return re;
}

vector<Scalar> cv_line_pts_scharr_geq_T(vector<Mat> *images,
                                        Scalar point,
                                        Scalar direction,
                                        double grad_threshold) {
    vector<Scalar> re;
    // size of the 3-d space
    Scalar bound((*images)[0].cols, (*images)[0].rows, images->size());
    // get all points on this line
    vector<Scalar> line_points = get_line_points(point, direction, bound);
    re.push_back((Scalar) *(line_points.begin()));
    // evaluate Scharr gradients of all points
    Scalar tmp_last; // temporary last point
    Scalar tmp_max; // max gradient point
    double tmp_g; // max gradient intensity
    bool cont = false; // flag of continuous component existence
    for (auto it = line_points.begin(); it != line_points.end(); ++it) {
        Scalar pt = *it;
        double grad = cv_imgs_point_scharr(images, pt);
        if (grad >= grad_threshold) {
            if (!cont) {
                tmp_last = pt;
                tmp_max = pt;
                tmp_g = grad;
                cont = true;
            } else {
                if (point_cont(tmp_last, pt)) {
                    tmp_last = pt;
                    //re.push_back(pt);
                    if (grad >= tmp_g) {
                        tmp_g = grad;
                        tmp_max = pt;
                    }
                } else { // when continuous ends
                    re.push_back(tmp_max);
                    cont = false;
                }
            }
        }
    }
    re.push_back((Scalar) *line_points.rbegin());
    return re;
}

vector<Scalar> cv_line_seg_pts_scharr_geq_T(vector<Mat> *images,
                                            Scalar start,
                                            Scalar end,
                                            double grad_threshold) {
    vector<Scalar> re;
    // size of the 3-d space
    Scalar bound((*images)[0].cols, (*images)[0].rows, images->size());
    // get all points on this line
    vector<Scalar> line_points = get_line_seg_points(start, end, bound);
    re.push_back((Scalar) *(line_points.begin()));
    // evaluate Scharr gradients of all points
    Scalar tmp_last;
    Scalar tmp_max;
    double tmp_g;
    bool cont = false; // flag of continuous component existence    
    for (auto it = line_points.begin(); it != line_points.end(); ++it) {
        Scalar pt = *it;
        double grad = cv_imgs_point_scharr(images, pt);

        if (grad >= grad_threshold) {
            if (!cont) {
                tmp_last = pt;
                tmp_max = pt;
                tmp_g = grad;
                cont = true;
            } else {
                if (point_cont(tmp_last, pt)) {
                    tmp_last = pt;
                    //re.push_back(pt);
                    if (grad >= tmp_g) {
                        tmp_g = grad;
                        tmp_max = pt;
                    }
                } else { // when continuous ends
                    re.push_back(tmp_max);
                    cont = false;
                }
            }
        }
    }
    re.push_back((Scalar) *line_points.rbegin());
    return re;
}

double compare_hist(vector<Mat> *images,
                    vector<Scalar> points_1,
                    vector<Scalar> points_2) {
    // get colors
    vector<Scalar> colors_1;
    vector<Scalar> colors_2;
    for (auto it = points_1.begin(); it != points_1.end(); ++it)
        colors_1.push_back(cv_imgs_point_color_loc(images, (Scalar) *it,
                                                   Scalar(0, 0, 0)));
    for (auto it = points_2.begin(); it != points_2.end(); ++it)
        colors_2.push_back(cv_imgs_point_color_loc(images, (Scalar) *it,
                                                   Scalar(0, 0, 0)));
    // calculate histogram
    /* cout << "pts1:" << endl; */
    arma::Mat<int> freq_1(3, 32, arma::fill::zeros); // frequency
    arma::Mat<int> freq_2(3, 32, arma::fill::zeros);
    for (auto it = colors_1.begin(); it != colors_1.end(); ++it) {
        Scalar lab = *it;
        for (int channel = 0; channel < 3; channel++) {
            int f = lab[channel]/8;
            /*
            cout << channel << " .. "
                 << lab[channel] << " ~= " 
                 << f << endl;
            */
            freq_1(channel, f) = freq_1(channel, f) + 1;
        }
    }
    /* cout << "pts2:" << endl; */
    for (auto it = colors_2.begin(); it != colors_2.end(); ++it) {
        Scalar lab = *it;
        for (int channel = 0; channel < 3; channel++) {
            int f = lab[channel]/8;
            /*
            cout << channel << " .. "
                 << lab[channel] << " ~= " 
                 << f << endl;
            */
            freq_2(channel, f) = freq_2(channel, f) + 1;
        }
    }
    arma::mat hist_1(3, 32, arma::fill::zeros); // histogram
    arma::mat hist_2(3, 32, arma::fill::zeros);
    /* cout << "frequencies: " << endl; */
    for (int f = 0; f < 32; f++) {
        for (int ch = 0; ch < 3; ch++) {
            /* cout << freq_1(ch, f) << "/" << freq_2(ch, f) << ", "; */
            // smooth the distribution with Dirichlet prior
            hist_1(ch, f) =
                (freq_1(ch, f) + 0.0001)/(arma::sum(freq_1.row(ch)) + 0.0032);
            hist_2(ch, f) =
                (freq_2(ch, f) + 0.0001)/(arma::sum(freq_2.row(ch)) + 0.0032);
        }
        /* cout << endl; */
    }
    // calculate KL divergence
    Scalar kls(0.0, 0.0, 0.0);
    for (int ch = 0; ch < 3; ch++) {
        double D_1_2 = 0;
        double D_2_1 = 0;
        for (int f = 0; f < 32; f++) {
            D_1_2 += hist_1(ch, f)*log2(hist_1(ch, f)/hist_2(ch, f));
            D_2_1 += hist_2(ch, f)*log2(hist_2(ch, f)/hist_1(ch, f));
        }
        kls[ch] = (D_1_2 + D_2_1)/2;
        /* cout << kls[ch] << " "; */
    }
    // quadratic mean of 3 channels
    double kl = sqrt((kls[0]*kls[0] + kls[1]*kls[1] + kls[2]*kls[2])/3);
    /* cout << ".......... " << kl << endl; */
    return kl;
}

#endif
