/*
*
* Paweł Gędłek
* lab 8, zad 1
*
*/

#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/times.h>
#include <math.h>
#include <pthread.h>
#include <zconf.h>

const int size = 1024;
int **I, **J;
double **K;
int w,h,c, gray_scale;

typedef struct division_props {
  int start;
  int end;
} division_props;

void set_matrix(char *buff, int **output, FILE *fp, int heigth, int width) {
  int i=0, j=0;
  while (fgets(buff, size, fp) != NULL) {
    for (char *num = strtok(buff, " \n\t\r"); num != NULL; num = strtok(NULL, " \t\n\r")) {
      output[i][j] = strtol(num, NULL, 10);
      j++;
      i = (j < width) ? i : i + 1;
      j = (j < width) ? j : 0;
    }
  }
}

void set_filter(char *buff, double **output, FILE *fp, int heigth, int width) {
  int i = 0, j = 0;
  while (fgets(buff, size, fp) != NULL) {
    for (char *num = strtok(buff, " \n\t\r"); num != NULL; num = strtok(NULL, " \t\n\r")) {
      output[i][j] = strtod(num, NULL);
      j++;
      i = (j < width) ? i : i + 1;
      j = (j < width) ? j : 0;
    }
  }
}

int evaluate(int x, int y, int width, int height, int c, int **I, double **K) {
 double pixel_val = 0;
  int a,b;
  for (int i = 0; i < c; i++) {
    a = round(fmax(0, y - ceil(c / 2) + i));
    a = a < height ? a : height - 1;
    for (int j = 0; j < c; j++) {
      b = round(fmax(0, x - ceil(c / 2) + j));
      b = b < width ? b : width - 1;
      double v = I[a][b] * K[i][j];
      pixel_val += v;
    }
  }
  pixel_val = pixel_val < 0 ? 0 : pixel_val;

return round(pixel_val);
}

void filter_image(int start, int end, int width, int heigth, int c, int **input, double **filter, int **output) {
    for (int x = start; x < end; x++)   
        for (int y = 0; y < heigth; y++)
            output[y][x] = evaluate(x, y, width, heigth, c, input, filter);
}

void *wrapped(void *props) {
  division_props *d_props = (division_props *)props;
  //printf("Hello from thread: %lu\n", pthread_self());
  //printf("Now, we're filtering pixels: (%d, %d)\n", d_props->start, d_props->end);
  filter_image(d_props->start, d_props->end, w, h, c, I, K, J);
  return (void *)0;
}

void save_picture(int width, int heigth, int **output, FILE *fp) {
  char buff[1024];
  fprintf(fp, "P2\n");
  fprintf(fp, "%d %d\n", width, heigth);
  fprintf(fp, "%d\n", gray_scale);
  for (int i = 0; i < heigth; i++) {
    for (int j = 0; j < width; j++) {
      if (j < width - 1) {
        sprintf(buff, "%d ", output[i][j]);
      } else {
        sprintf(buff, "%d\n", output[i][j]);
      }
      fputs(buff, fp);
    }
  }
}

double time_diff(clock_t start, clock_t end) {
    return (double) (end - start) / sysconf(_SC_CLK_TCK);
}

void save_time_res(clock_t r_time[2], struct tms tms_time[2], int threads, int width, int heigth, int filter_size) {
  FILE *fp = fopen("time_res.txt", "a");
  fprintf(fp, "Number of threads: %d\n", threads);
  fprintf(fp, "Input image size: %dx%d\n", width, heigth);
  fprintf(fp, "Filter size: %dx%d\n", filter_size, filter_size);
  fprintf(fp, "Real time:   %.2lf s   ", time_diff(r_time[0], r_time[1]));
  fprintf(fp, "User time:   %.2lf s   ", time_diff(tms_time[0].tms_utime, tms_time[1].tms_utime));
  fprintf(fp, "System time: %.2lf s\n",  time_diff(tms_time[0].tms_stime, tms_time[1].tms_stime));
  fprintf(fp, "\n\n");
  fclose(fp);
}

int main(int argc, char **argv) {
  if (argc < 5) {
    printf("threads count | in file path | filter path | out file name\n");
    exit(1);
  };
  char buff[size];
  char *token, *line;
  int threads_number;

  clock_t r_time[2] = {0, 0};
  struct tms tms_time[2];

  threads_number = strtol(argv[1], NULL, 10);

  FILE *f_in   = fopen(argv[2], "r+");
  FILE *f_fltr = fopen(argv[3], "r+");
  FILE *f_out  = fopen(argv[4], "w+");

  if (f_in == NULL) {
    perror("opening the input");
    exit(1);
  }
  if (f_out == NULL) {
    perror("creating the output");
    exit(1);
  }
  if (f_fltr == NULL) {
    perror("opening the filter");
    exit(1);
  }

  fgets(buff, size, f_in); // to solve problem with tag as "P2"
  fgets(buff, size, f_in);

  line = strdup(buff);
  //reading width of image
  token = strsep(&line, " \t");
  w = strtol(token, NULL, 10);
  //reading height of image
  token = strsep(&line, " \t");
  h = strtol(token, NULL, 10);

  //allocate matrixs
  I = calloc(h, sizeof(int *));
  for (int i = 0; i < h; i++) {
    I[i] = calloc(w, sizeof(int));
  }
  J = calloc(h, sizeof(int *));
  for (int i = 0; i < h; i++) {
    J[i] = calloc(w, sizeof(int));
  }

  fgets(buff, size, f_in); // to set gray scale value
  line = strdup(buff);
  token = strsep(&line, " \t");
  gray_scale = strtol(token, NULL, 10);   

  set_matrix(buff, I, f_in, h, w);

  fgets(buff, size, f_fltr);
  c = strtol(buff, NULL, 10);
  K = calloc(c, sizeof(double *));
  for (int i = 0; i < c; i++) {
    K[i] = calloc(c, sizeof(double));
  }

  set_filter(buff, K, f_fltr, c, c);

  pthread_t *thread = calloc(threads_number, sizeof(pthread_t));
  division_props **props = malloc(threads_number * sizeof(division_props *));

  r_time[0] = times(&tms_time[0]);
  for (int i = 0; i < threads_number; i++) {
    props[i] = malloc(sizeof(division_props));
      
    props[i]->start = i * w / threads_number;
    //printf("%d\t", props[i]->start);
    props[i]->end = (i + 1) * w / threads_number;
    //printf("%d\n", props[i]->end);
    
    pthread_create(&thread[i], NULL, wrapped, (void *)props[i]);
  }

  for (int i = 0; i < threads_number; i++) {
    pthread_join(thread[i], NULL);
    free(props[i]);
  }
  free(props);

  r_time[1] = times(&tms_time[1]);

  save_picture(w, h, J, f_out);
  save_time_res(r_time, tms_time, threads_number, w, h, c);

  // free matrixs
  for (int i = 0; i < h; i++) {
    free(I[i]);
  }
  free(I);
  for (int i = 0; i < h; i++) {
    free(J[i]);
  }
  free(J);
  for (int i = 0; i < c; i++) {
    free(K[i]);
  }
  free(K);

  //close files
  fclose(f_in);
  fclose(f_out);
  fclose(f_fltr);
}
