#include "fusion.h"

#ifdef OPENCV
#include "opencv2/highgui/highgui_c.h"
#include "opencv2/imgproc/imgproc_c.h"
#include "opencv2/videoio/videoio_c.h"
#endif

//int pi_time=0; // use for give many pi data.

static double record_euclidean[PI_PATH_NUM][VI_PATH_NUM][MAX_DATA_PATH_NUM];
static int segment[PI_PATH_NUM][VI_PATH_NUM];

static std::map<int, person_location> all_beacon_data;

void initial( int pi_compare[], int vi_compare[], Data pi_data[], 
             Data vi_data[], double current_ans[][VI_PATH_NUM], Fusion_result match_result[] ) {
  int i,j;
  for( i=0; i<PI_PATH_NUM; i++ ) {
    pi_compare[i]=0;
    pi_data[i].x=0;
    pi_data[i].y=0;
    for( j=0; j<VI_PATH_NUM; j++ )
      current_ans[i][j]=0;
  } // for
  
  for( i=0; i<VI_PATH_NUM; i++ ){
    vi_compare[i]=0;
    vi_data[i].x=0;
    vi_data[i].y=0;
  } // for
  
  
  for( i=0; i<MAX_DATA_PATH_NUM; i++) {
    match_result[i].num=0;
    strcpy(match_result[i].name,"");
  } // for
} // initial()

void get_next_string( int *count, char *data, char *pi_data ) {
  int i,j=0;
  for( i = 0 ; i < 20 ; i++ )
  pi_data[i]=0;
  while( data[(*count)] != ' '  && data[(*count)] != 0 ) {
    pi_data[j]=data[(*count)];
  (*count)++;
  j++;
  } // while
} // get_next_string()

void get_pi_data( Data data[], int pi_compare[], int *pi_compare_size) {

  for( int i = 1 ; i < PI_PATH_NUM + 1 ; i++ )
  {
    if(all_beacon_data.find(i) != all_beacon_data.end())
    {
      double x = all_beacon_data[i].x;
      double y = all_beacon_data[i].y;

      if (!( x ==  data[i-1].x && y == data[i-1].y ))
      {
        pi_compare[(*pi_compare_size)]=i-1;
        (*pi_compare_size)++;
      } // if

      data[i-1].x = x;
      data[i-1].y = y;
    }
  }
} // get_pi_data()

void get_vi_data( Video_data all_vi_data[], int all_vi_data_size, Data data[], 
                  int vi_compare[], int *vi_compare_size ) {
  int i,j;
  //printf("all_vi_data_size=%d\n",all_vi_data_size);
  for( i = 0 ; i <  all_vi_data_size ; i++ ) {
    double x = all_vi_data[i].x/100;
    double y = all_vi_data[i].y/100;
    if (!( x ==  data[(all_vi_data[i].num)-1].x && y == data[(all_vi_data[i].num)-1].y )) {
      vi_compare[(*vi_compare_size)]=all_vi_data[i].num-1;
    (*vi_compare_size)++;
  } // if
  data[(all_vi_data[i].num)-1].x = x;
  data[(all_vi_data[i].num)-1].y = y;
  } // for  
} // get_vi_data()

double cal_distance_euclid( double a,double b, double c, double d ) {
    return sqrt(pow(a - c, 2) + pow(b - d, 2));
} // cal_distance_euclid()

void calc_euclidean( Data pi_data[], Data vi_data[], int pi_compare[], int vi_compare[], 
                     int pi_compare_size, int vi_compare_size, double current_ans[PI_PATH_NUM][VI_PATH_NUM] ) {
  int i,j,k;
  for( i = 0 ; i < pi_compare_size ; i++ )
    for( j = 0 ; j < vi_compare_size ; j++ ) {
      segment[pi_compare[i]][vi_compare[j]]++;
      double euclid = cal_distance_euclid( pi_data[pi_compare[i]].x, pi_data[pi_compare[i]].y, 
                                         vi_data[vi_compare[j]].x, vi_data[vi_compare[j]].y); 
      record_euclidean[pi_compare[i]][vi_compare[j]][(segment[pi_compare[i]][vi_compare[j]])-1] = euclid;
      double euclid_sum=0;
      for(k = 0 ; k < segment[pi_compare[i]][vi_compare[j]] ; k++ ) 
        euclid_sum += record_euclidean[pi_compare[i]][vi_compare[j]][k];
      current_ans[pi_compare[i]][vi_compare[j]] = euclid_sum / 
                                                    segment[pi_compare[i]][vi_compare[j]];
  } // for
} // calc_euclidean()

int equal_max( int a, int b ) {
  return (a >= b) ? a : b;
} // equal_max()

int equal_min( int a, int b ) {
  return (a <= b) ? a : b;
} // equal_min()

int factorial( int n ) {
  int fac=1;
  while( n > 0 ) {
    fac *= n;
    n--;
  } // while
  return fac;
} // factorial()

void backtrack( int n, int N, int a[], int used[], int permutation[][VI_PATH_NUM], int *permutation_count ) {
  int i;
  if (n == N) {
    for ( i = 0 ; i < N ; ++i ) 
    permutation[(*permutation_count)][i]=(a[i]);
  (*permutation_count)++;
  return;
  } // if
 
  for ( i=0 ; i<N ; i++ )
    if ( !used[i] ) {
      used[i] = 1;
      a[n] = i;
      backtrack(n+1, N, a, used, permutation, permutation_count);
      used[i] = 0;
    } // if
} // backtrack()

void enumerate_permutations( int n, int a[], int used[], int permutation[][VI_PATH_NUM], int permutation_count ) {
  int i;
    for ( i = 0 ; i < n ; i++ ) 
    used[i] = 0;
    backtrack(0, n, a, used, permutation, &permutation_count);
} // enumerate_permutations()

void find_real_name(int n, char* result) {
  if(n == 1)
  {
    sprintf(result, "ID:01 hung");
  }
  if(n == 2)
  {
    sprintf(result, "ID:02 chiu");
  }
} // find_real_name()



void min_match( int pi_compare[], int vi_compare[], int pi_compare_size, int vi_compare_size, 
                int pi_vi_small_size, double current_ans[PI_PATH_NUM][VI_PATH_NUM], int permutation[][VI_PATH_NUM], 
        int permutation_size, Fusion_result match_result[], int *match_result_size ) {
  double min_map = 1000;
  int match_result_tmp[pi_vi_small_size][2];
  int i,j;
  //printf("permutation_size=%d\n",permutation_size);
  printf("pi_compare_size=%d\n",pi_compare_size);
  printf("vi_compare_size=%d\n",vi_compare_size);
  //printf("pi_vi_small_size=%d\n",pi_vi_small_size);
  for( i = 0 ; i < permutation_size ; i++ ) {
    double current_min_map = 0;
    for(  j = 0 ; j < pi_vi_small_size ; j++ ) {
      if( pi_compare_size > vi_compare_size ){
        if (i%factorial(pi_compare_size-vi_compare_size)==0){
          current_min_map += current_ans[pi_compare[permutation[i][j]]][vi_compare[j]];
          match_result_tmp[j][0] = vi_compare[j];
          match_result_tmp[j][1] = pi_compare[permutation[i][j]]; 
        }//if
    } // if
    else{
      if (i%factorial(vi_compare_size-pi_compare_size)==0){
        current_min_map += current_ans[pi_compare[j]][vi_compare[permutation[i][j]]];
        match_result_tmp[j][0] = pi_compare[j];
        match_result_tmp[j][1] = vi_compare[permutation[i][j]];
      }//if
    } // else
  } // for
  
    //printf("current_min_map=%f\n",current_min_map);
    //printf("min_map=%f\n",min_map);
    if( current_min_map < min_map ) {
      printf("--------");
      
      (*match_result_size)=0;
      min_map = current_min_map;
      
      for( j = 0 ; j < pi_vi_small_size ; j++ ) {
        char name[30];
        int tag_id;
        if( pi_compare_size > vi_compare_size ) {
          tag_id = match_result_tmp[j][1]+1;
          find_real_name( tag_id, name);
          match_result[(*match_result_size)].num = match_result_tmp[j][0]+1;
          strcpy( match_result[(*match_result_size)].name, name);
          
        } // if 
        else {
          tag_id = match_result_tmp[j][0]+1;
          find_real_name( tag_id, name);
          match_result[(*match_result_size)].num = match_result_tmp[j][1]+1;
          strcpy( match_result[(*match_result_size)].name, name);
        } //else
        /***** UNDO: hash the tag color****/
        // if (tag_color[match_result_tmp[j][1]]==1){

        // }
        (*match_result_size)++;
        
      } // for
    } // if
  } // for
} // min_match()

void match( int pi_compare[], int vi_compare[], int pi_compare_size, int vi_compare_size,
            double current_ans[PI_PATH_NUM][VI_PATH_NUM], Fusion_result match_result[], int *match_result_size,
        int type ) {
  int pi_vi_big_size = equal_max( pi_compare_size, vi_compare_size );
  int pi_vi_small_size = equal_min( pi_compare_size, vi_compare_size );
  int used[pi_vi_big_size];
  int a[pi_vi_big_size];
  int permutation_count = 0;
  int permutation_size = factorial(pi_vi_big_size);
  int permutation[permutation_size][VI_PATH_NUM];
  enumerate_permutations( pi_vi_big_size, a, used, permutation, permutation_count);
  if( type == 0 )
    min_match( pi_compare, vi_compare, pi_compare_size, vi_compare_size, pi_vi_small_size,
             current_ans, permutation, permutation_size, match_result, match_result_size );
} // match

void run_euclidean( Data pi_data[], Data vi_data[], int pi_compare[], int vi_compare[],
                    int pi_compare_size, int vi_compare_size, double current_ans[PI_PATH_NUM][VI_PATH_NUM], 
          Fusion_result match_result[], int *match_result_size, int type ) {
  calc_euclidean( pi_data, vi_data, pi_compare, vi_compare, pi_compare_size, vi_compare_size, current_ans );
  match( pi_compare, vi_compare, pi_compare_size, vi_compare_size, current_ans, match_result, match_result_size, type );
} // run_euclidean()

void sensor_fusion( Video_data *all_vi_data, int all_vi_data_size, Fusion_result *match_result, 
            int *match_result_size , int algorithm ) {
  printf("############ sensor_fusion");
  int pi_compare[PI_PATH_NUM];
  int vi_compare[VI_PATH_NUM];
  Data pi_data[PI_PATH_NUM];
  Data vi_data[VI_PATH_NUM];
  double current_ans[PI_PATH_NUM][VI_PATH_NUM];
  initial(pi_compare, vi_compare, pi_data, vi_data, current_ans, match_result);
  int pi_compare_size=0;
  int vi_compare_size=0;
  get_pi_data(pi_data, pi_compare, &pi_compare_size);
  get_vi_data(all_vi_data, all_vi_data_size, vi_data, vi_compare, &vi_compare_size);
  printf("############get data success\n");
  if(vi_compare_size<7)
    if( algorithm == 1 )
      run_euclidean( pi_data, vi_data, pi_compare, vi_compare, pi_compare_size, vi_compare_size ,
                 current_ans, match_result, match_result_size, 0 );
//  pi_time++;
} // sensor_fusion()

void tmp_sensor_fusion( Video_data *all_vi_data, int all_vi_data_size, Fusion_result *match_result, 
            int *match_result_size , int algorithm ) {
  printf("############ tmp_sensor_fusion");
  
} // sensor_fusion()

int fusion(Video_data *all_vi_data, Fusion_result *match_result, int *match_result_size, int all_vi_data_size, char fusion_name_result[][30]){
  int i;
  printf("############# fusion\n");
  for (i=0; i<all_vi_data_size; i++){
    printf(">>>>>>>>>> %d: (%f, %f)\n", all_vi_data[i].num, all_vi_data[i].x, all_vi_data[i].y);
  }
  printf("############# before sensor_fusion\n");
  printf("%d %d\n", all_vi_data_size, *match_result_size);

  sensor_fusion(all_vi_data, all_vi_data_size, match_result, match_result_size, 1);

  for(i=0; i<all_vi_data_size; i++ )
    strcpy(fusion_name_result[all_vi_data[i].num], "unknown");
  for(i=0; i<*match_result_size; i++ ){
    printf("v%d\t%s\n",match_result[i].num,match_result[i].name);
    strcpy(fusion_name_result[match_result[i].num], match_result[i].name);
    // printf("#############2\n");
  }

  /*sensor_fusion(all_vi_data, all_vi_data_size, match_result, match_result_size, 1);
  for(i=0; i<*match_result_size; i++ ){
    printf("v%d\t",match_result[i].num);
    printf("%s\n",match_result[i].name);
  }
  sensor_fusion(all_vi_data, all_vi_data_size, match_result, match_result_size, 1);
  for(i=0; i<*match_result_size; i++ ){
    printf("v%d\t",match_result[i].num);
    printf("%s\n",match_result[i].name);
  }*/
}

void convertCoordinate(CvMat  *srcPts, CvMat  *dstPts, image im){
    CvPoint2D32f  src_coordinate[4] = {cvPoint2D32f(813, 934), cvPoint2D32f(783, 358), cvPoint2D32f(245, 362), cvPoint2D32f(256, 927)};
    CvPoint2D32f  dst_coordinate[4] = {cvPoint2D32f(540, 1100),cvPoint2D32f(540, 100),cvPoint2D32f(10, 100),cvPoint2D32f(10, 1100)};
    //CvMat* warp_matrix = cvCreateMat(im.h, im.w, CV_32FC1);
    CvMat* warp_mat = cvCreateMat( 3, 3, CV_32FC1 );
    cvGetPerspectiveTransform(src_coordinate, dst_coordinate, warp_mat);
    //printf("%f %f\n", srcPts->data.fl[0], srcPts->data.fl[1]);
    cvPerspectiveTransform(srcPts, dstPts, warp_mat);
    printf("%f %f\n", dstPts->data.fl[0], dstPts->data.fl[1]);
    //cvWarpPerspective(srcPts, dstPts, warp_matrix, CV_INTER_LINEAR+CV_WARP_FILL_OUTLIERS, cvScalarAll(0));
}

//void do_fusion(Fusion_result *match_result, int *match_result_size, image im, int num, float thresh, 
//    box *boxes, float **probs, char **names, image **alphabet, int classes, int *sort_ids)
void do_fusion(Fusion_result *match_result, int *match_result_size, image im, std::vector<person_box> &boxes, std::map<int, person_location> &beacon_data, char fusion_name_result[][30])
{
    all_beacon_data = beacon_data;

    Video_data all_vi_data[VI_PATH_NUM];
    int all_vi_data_size = 0;
    int i;
    printf("########## do_fusion\n");

    for(i = 0; i < boxes.size(); ++i)
    {
        all_vi_data[all_vi_data_size].num = boxes[i].id;

        int left, right, top, bot;
        left = boxes[i].x1;
        top = boxes[i].y1;
        right = boxes[i].x2;
        bot = boxes[i].y2;

        float points[] = {(left+right)/2.0 , (top+bot)/2.0} ;
        CvMat *src_pts = cvCreateMat(1 , 1 , CV_32FC2) ;
        cvInitMatHeader(src_pts , 1 , 1 , CV_32FC2 , points, CV_AUTOSTEP) ;
        CvMat *dst_pts = cvCreateMat(1 , 1 , CV_32FC2) ;

        convertCoordinate(src_pts, dst_pts, im);
        all_vi_data[all_vi_data_size].x = dst_pts->data.fl[0];
        all_vi_data[all_vi_data_size].y = dst_pts->data.fl[1];
        all_vi_data_size++;
    }
    
/*
    for(i = 0; i < num; ++i){

        int klass = max_index(probs[i], classes);
        if (PERSON_ONLY && klass!=0) continue;

        float prob = probs[i][klass];
        if(prob > thresh){

            int width = im.h * .012;

            if(0){
                width = pow(prob, 1./2.)*10+1;
                alphabet = 0;
            }

            box b = boxes[i];

            int left, right, top, bot;
            left = b.x;
            right = b.x+b.w;
            top = b.y;
            bot = b.y+b.h;
            
            if(left < 0) left = 0;
            if(right > im.w-1) right = im.w-1;
            if(top < 0) top = 0;
            if(bot > im.h-1) bot = im.h-1;

            all_vi_data[all_vi_data_size].num = sort_ids[i];
            printf("########all_vi_data[%d]=%d\n", all_vi_data_size, all_vi_data[all_vi_data_size].num);
            
            float points[] = {(left+right)/2.0 , (top+bot)/2.0} ;
            CvMat *src_pts = cvCreateMat(1 , 1 , CV_32FC2) ;
            cvInitMatHeader(src_pts , 1 , 1 , CV_32FC2 , points, CV_AUTOSTEP) ;
            CvMat *dst_pts = cvCreateMat(1 , 1 , CV_32FC2) ;

            convertCoordinate(src_pts, dst_pts, im);
            all_vi_data[all_vi_data_size].x = dst_pts->data.fl[0];
            all_vi_data[all_vi_data_size].y = dst_pts->data.fl[1];
            all_vi_data_size++;

            printf("@@@@@@@@@@@@@@@@@@@@@@@@@@ %f, %f", dst_pts->data.fl[0], dst_pts->data.fl[1]);

        }
    }
*/

    printf("########## berfor_fusion\n");


    fusion(all_vi_data, match_result, match_result_size, all_vi_data_size, fusion_name_result);
}


void record_convert_log(double x, double y)
{

}