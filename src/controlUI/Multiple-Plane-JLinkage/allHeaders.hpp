/*
 *   File Name: allHeaders.hpp
 *     Project: Multiple Plane JLinkage
 *  Created on: 22-Mar-2016
 *      Author: Sona Praneeth Akula
 *     Details:
 *   TodoNotes: TODO
 */


// OpenCV headers
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>


// C++ Headers
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <istream>
#include <ostream>
#include <map>
#include <cmath>
#include <algorithm>
#include <set>
#include <list>

#include "../DebugUtility/DebugUtility.hpp"
#include "../LogUtility/LogUtility.hpp"

using namespace cv;
using namespace std;

int findMultiplePlanes(const vector<Point3f> &points, vector< vector<float> > &sortedPlaneParameters,  vector< vector<Point3f> > & continuousBoundingBoxPoints);

#define LLI long long int

