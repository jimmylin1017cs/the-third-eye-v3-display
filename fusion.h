#ifndef FUSION_H
#define FUSION_H
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>
#include <math.h>
#include "image.h"

// define in image.h
#define MAX_SORT_NUM 500

#define PI_PATH_NUM 3
#define VI_PATH_NUM 500
#define MAX_DATA_PATH_NUM 3000
#define PERSON_ONLY 1

#define FPS 15
#define FUSION_ENABLE 1

typedef struct{
    float x, y, w, h;
} box;

typedef struct  {
	double x;
	double y;
}Data;

typedef struct  {
	int num;
	char name[200];
}Fusion_result;

typedef struct  {
	int num; 
	double x;
	double y;
}Video_data;

//void do_fusion(Fusion_result *match_result, int *match_result_size, image im, int num, float thresh, box *boxes, float **probs, char **names, image **alphabet, int classes, int *sort_ids);
void do_fusion(Fusion_result *match_result, int *match_result_size, image im, std::vector<person_box> &boxes, std::map<int, person_location> &beacon_data, char fusion_name_result[][30]);

int fusion(Video_data *all_vi_data, Fusion_result *match_result, int *match_result_size, int all_vi_data_size, char fusion_name_result[][30]);

#endif
