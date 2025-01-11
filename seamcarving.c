# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <math.h>
# include "seamcarving.h"

void calc_energy(struct rgb_img *im, struct rgb_img **grad){
    *grad = (struct rgb_img *)malloc(sizeof(struct rgb_img));
    (*grad)->height = im->height;
    (*grad)->width = im->width;
    (*grad)->raster = (uint8_t *)malloc(3 * im->height * im->width);

    for(int i = 0; i < im->height; i++){ // row
        for(int j = 0; j < im->width; j++){ // column
            int Rx, Gx, Bx, Ry, Gy, By;
            int hor_left, hor_right, ver_up, ver_down;
            if(i == 0){
                ver_down = 1;
                ver_up = im->height - 1;
            }
            else if(i == im->height - 1){
                ver_up = i-1;
                ver_down = 0;
            }
            else{
                ver_up = i-1;
                ver_down = i+1;
            }
            if(j == 0){
                hor_right = 1;
                hor_left = im->width - 1;
            }
            else if(j == im->width - 1){
                hor_left = j-1;
                hor_right = 0;
            }
            else{
                hor_left = j-1;
                hor_right = j+1;
            }
            Rx = get_pixel(im, i, hor_right, 0) - get_pixel(im, i, hor_left, 0);
            Gx = get_pixel(im, i, hor_right, 1) - get_pixel(im, i, hor_left, 1);
            Bx = get_pixel(im, i, hor_right, 2) - get_pixel(im, i, hor_left, 2);
            Ry = get_pixel(im, ver_down, j, 0) - get_pixel(im, ver_up, j, 0);
            Gy = get_pixel(im, ver_down, j, 1) - get_pixel(im, ver_up, j, 1);
            By = get_pixel(im, ver_down, j, 2) - get_pixel(im, ver_up, j, 2);

            double energy = sqrt((pow(Rx, 2) + pow(Gx, 2) + pow(Bx, 2) + pow(Ry, 2) + pow(Gy, 2) + pow(By, 2))*1.0);
            uint8_t scaled_energy = (uint8_t)(energy / 10.0);
            set_pixel(*grad, i, j, scaled_energy, scaled_energy, scaled_energy);    
        }
    }
}

void dynamic_seam(struct rgb_img *grad, double **best_arr){
    *best_arr = (double *)malloc(grad->height * grad->width * sizeof(double));
    for(int i = 0; i < grad->width; i++){
        (*best_arr)[i] = get_pixel(grad, 0, i, 0);
    }
    for(int i = 1; i < grad->height; i++){
        for(int j = 0; j < grad->width; j++){
            double min_path;
            if(j == 0){
                if((*best_arr)[(i-1) * grad->width + j] <= (*best_arr)[(i-1) * grad->width + j+1]){
                    min_path = (*best_arr)[(i-1) * grad->width + j];
                }
                else min_path = (*best_arr)[(i-1) * grad->width + j+1];
            }
            else if(j == grad->width-1){
                if((*best_arr)[(i-1) * grad->width + j-1] <= (*best_arr)[(i-1) * grad->width + j]){
                    min_path = (*best_arr)[(i-1) * grad->width + j-1];
                }
                else min_path = (*best_arr)[(i-1) * grad->width + j];
            }
            else{
                if((*best_arr)[(i-1) * grad->width + j-1] <= (*best_arr)[(i-1) * grad->width + j] && (*best_arr)[(i-1) * grad->width + j-1] <= (*best_arr)[(i-1) * grad->width + j+1]){
                    min_path = (*best_arr)[(i-1) * grad->width + j-1];
                }
                else if((*best_arr)[(i-1) * grad->width + j] <= (*best_arr)[(i-1) * grad->width + j-1] && (*best_arr)[(i-1) * grad->width + j] <= (*best_arr)[(i-1) * grad->width + j+1]){
                    min_path = (*best_arr)[(i-1) * grad->width + j];
                }
                else min_path = (*best_arr)[(i-1) * grad->width + j+1];
            }
            (*best_arr)[i * grad->width + j] = min_path + get_pixel(grad, i, j, 0);
        }
    }
}

int find_min_index(double *best, int width, int row){
    double min = INFINITY;
    int ind = 0;
    for(int i = 0; i < width; i++){
        if(best[row * width + i] < min){
            min = best[row * width + i];
            ind = i;
        }
    }
    return ind;
}

void reverse(int **path, int length){
    int *temp = (int *)malloc(length * sizeof(int));
    for(int i = 0; i < length; i++){
        temp[i] = (*path)[length-1-i];
    }
    free(*path);
    *path = temp;
}

void recover_path(double *best, int height, int width, int **path){
    *path = (int *)malloc(height * sizeof(int));
    (*path)[0] = find_min_index(best, width, height-1); // Starting point from bottom of best
    for(int i = height-2; i >= 0; i--){
        int a, b, c = 100000000;
        if((*path)[height-2-i]-1 >= 0){
            a = best[i * width + (*path)[height-2-i]-1];
        }
        if((*path)[height-2-i]+1 < width){
            c = best[i * width + (*path)[height-2-i]+1];
        }
        b = best[i * width + (*path)[height-2-i]];
        if(a <= b && a <= c){
            (*path)[height-1-i] = (*path)[height-2-i]-1;
        }
        else if(b <= a && b <= c){
            (*path)[height-1-i] = (*path)[height-2-i];
        }
        else{
            (*path)[height-1-i] = (*path)[height-2-i]+1;
        }
    }
    reverse(path, height);
}

void remove_seam(struct rgb_img *src, struct rgb_img **dest, int *path){
    create_img(dest, src->height, src->width-1);
    for(int i = 0; i < (*dest)->height; i++){
        int target = path[i];
        for(int j = 0; j < (*dest)->width; j++){
            if(j == target){
                continue;
            }
            else set_pixel(*dest, i, j, get_pixel(src, i, j, 0), get_pixel(src, i, j, 1), get_pixel(src, i, j, 2));
        }
    }
}