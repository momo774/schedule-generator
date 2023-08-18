/**
 * @file schedule.cpp
 * Exam scheduling using graph coloring
 */

#include <iostream>
#include <cmath>
#include <cstdlib>
#include <map>
#include <unordered_map>
#include <set>

#include "schedule.h"
#include "utils.h"
#include <algorithm>

/**
 * Given a filename to a CSV-formatted text file, create a 2D vector of strings where each row
 * in the text file is a row in the V2D and each comma-separated value is stripped of whitespace
 * and stored as its own string. 
 * 
 * Your V2D should match the exact structure of the input file -- so the first row, first column
 * in the original file should be the first row, first column of the V2D.
 *  
 * @param filename The filename of a CSV-formatted text file. 
 */
V2D file_to_V2D(const std::string & filename){
    std::vector<std::vector<std::string>> vec;

    std::string str;
    std::vector<std::string> pushVec;
    std::ifstream fin(filename);
    while (std::getline(fin, str)) {
        str = Trim(str);
        int len = SplitString(str, ',', pushVec);
        std::vector<std::string> tmp;
        for (auto& v: pushVec) {
            tmp.push_back(Trim(v));
        }
        vec.push_back(tmp);
        pushVec.clear();
    }

    return vec;
}

/**
 * Given a course roster and a list of students and their courses, 
 * perform data correction and return a course roster of valid students (and only non-empty courses).
 * 
 * A 'valid student' is a student who is both in the course roster and the student's own listing contains the course
 * A course which has no students (or all students have been removed for not being valid) should be removed
 * 
 * @param cv A 2D vector of strings where each row is a course ID followed by the students in the course
 * @param student A 2D vector of strings where each row is a student ID followed by the courses they are taking
 */
V2D clean(const V2D & cv, const V2D & student){
    std::vector<std::vector<std::string>> vec;
    for (auto const& a: cv) {
        std::vector<std::string> tmp;
        std::string cse = a[0];
        tmp.push_back(cse);
        for (unsigned i = 1; i < a.size(); i++) {
            for (auto const& b: student) {
                if ((std::find(b.begin(), b.end(), a[i]) != b.end()) && (std::find(b.begin(), b.end(), cse) != b.end())) {
                    tmp.push_back(a[i]);
                }
            }
            
        }
        if (tmp.size() > 1) {
            vec.push_back(tmp);
        }
    }
    return vec;
}

/**
 * Given a collection of courses and a list of available times, create a valid scheduling (if possible).
 * 
 * A 'valid schedule' should assign each course to a timeslot in such a way that there are no conflicts for exams
 * In other words, two courses who share a student should not share an exam time.
 * Your solution should try to minimize the total number of timeslots but should not exceed the timeslots given.
 * 
 * The output V2D should have one row for each timeslot, even if that timeslot is not used.
 * 
 * As the problem is NP-complete, your first scheduling might not result in a valid match. Your solution should 
 * continue to attempt different schedulings until 1) a valid scheduling is found or 2) you have exhausted all possible
 * starting positions. If no match is possible, return a V2D with one row with the string '-1' as the only value. 
 * 
 * @param courses A 2D vector of strings where each row is a course ID followed by the students in the course
 * @param timeslots A vector of strings giving the total number of unique timeslots
 */
V2D schedule(const V2D &courses, const std::vector<std::string> &timeslots){
    // number of colors
    unsigned int m = timeslots.size();
    // index of this vector corresponds to adj matrix index to determine course
    std::vector<std::string> course_order;

    for (auto& c: courses) {
        course_order.push_back(c[0]);
    }
    //vertices
    unsigned int v = course_order.size();

    // contains the current color of each node (index = node #; color <= m)
    std::vector<unsigned int> colors;
    for (unsigned i = 0; i < course_order.size(); i++) {
        colors.push_back(0);
    }

    std::vector<std::vector<int>> adjMatrix = buildAdjacencyMatrix(courses);

    
    for (unsigned k = 0; k < 1; k++) {
        if (graphColor(k, m, colors, adjMatrix, v)) {
            std::cout << "working!" << std::endl;
        } else {
            std::cout << "not working!" << std::endl;
            std::vector<std::vector<std::string>> tmpp;
            std::vector<std::string> tmppp;
            tmppp.push_back("-1");
            tmpp.push_back(tmppp);
            return tmpp;
            
        }
        
    }

    // print colors (which should lead us to the answer)
    for (unsigned i = 0; i < colors.size(); i++) {
        std::cout << "Node " << i << " : " << colors[i] << std::endl;
    }

    std::vector<std::vector<std::string>> ans;

    for (unsigned i = 0; i < timeslots.size(); i++) {
        std::vector<std::string> temp;
        temp.push_back(timeslots[i]);
        for (unsigned a = 0; a < colors.size(); a++) {
            if (colors[a] == (i + 1)) {
                temp.push_back(course_order[a]);
            }
        }
        ans.push_back(temp);
    }

    return ans;

}



// Part 2 Functions put here!

const std::vector<std::vector<int>> buildAdjacencyMatrix(const V2D& courses) {
    std::vector<std::string> course_order;
    for (auto& c: courses) {
        course_order.push_back(c[0]);
    }
    int n = course_order.size();
    std::vector<std::vector<int>> adj_mat;
    //initialize adj_mat
    for (int i = 0; i < n; i++) {
        std::vector<int> tmp;
        for (int i = 0; i < n; i++) {
            tmp.push_back(0);
        }
        adj_mat.push_back(tmp);
    }
    //update adj_mat
    for (unsigned i = 0; i < courses.size(); i++) {
        for (unsigned j = 1; j < courses[i].size(); j++) {
            for(unsigned k = 0; k < courses.size(); k++) {
                if (std::find(courses[k].begin(), courses[k].end(), courses[i][j]) != courses[k].end()) {
                    adj_mat[k][i] = 1;
                    adj_mat[i][k] = 1;  
                }
            }
        }
    }
    return adj_mat;
}

bool graphColor(unsigned int k, unsigned int m, std::vector<unsigned int>& colors, std::vector<std::vector<int>>& adjMatrix, unsigned int v) {
    if (k == v) {
        return true;
    }
    for (unsigned c = 1; c <= m; c++) {
        if (isSafe(k, c, adjMatrix, v, colors)) {
            colors[k] = c;
            if (graphColor(k + 1, m, colors, adjMatrix, v)) {
                return true;
            } 
            colors[k] = 0;
        }
    }
    return false;
}

bool isSafe(unsigned int k, unsigned int c, std::vector<std::vector<int>>& adjMatrix, unsigned int v, std::vector<unsigned int>& colors) {
    for (unsigned i = 0; i < v; i++) {
        if (adjMatrix[k][i] == 1 && c == colors[i]) {
            return false;
        }
    }
    return true;
}


