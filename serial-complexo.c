/******************************************************************************
 * Programacao Concorrente
 * MEEC 21/22
 *
 * Projecto - Parte1
 *                           serial-complexo.c
 * 
 * Compilacao: gcc serial-complexo -o serial-complex -lgd
 *           
 *****************************************************************************/

#include "gd.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>

/* the directories wher output files will be placed */
#define RESIZE_DIR "./Resize/"
#define THUMB_DIR "./Thumbnail/"
#define WATER_DIR "./Watermark/"


/******************************************************************************
 * add_watermark()
 *
 * Arguments: in - pointer to image
 *            watermark - pointer to image watermark
 * Returns: out - pointer to image with watermark, or NULL in case of failure
 * Side-Effects: none
 *
 * Description: creates clone of image and puts a watermark on it
 *
 *****************************************************************************/
gdImagePtr  add_watermark(gdImagePtr in_img, gdImagePtr watermark){
	
	gdImagePtr out_img;

	int width,heigth;

	width = watermark->sx;
	heigth = watermark->sy;

	out_img =  gdImageClone (in_img);

	gdImageCopy(out_img, watermark, 0, 0, 0, 0, width, heigth);
	
	return(out_img);		
} 

/******************************************************************************
 * resize_image()
 *
 * Arguments: in - pointer to image
 *            new_width - new width to use in resizing
 * Returns: out - pointer to resized image, or NULL in case of failure
 * Side-Effects: none
 *
 * Description: creates clone of image with width and height scaled to target
 *              width
 *
 *****************************************************************************/
gdImagePtr  resize_image(gdImagePtr in_img ,int new_width){
	
	gdImagePtr out_img;
	int width,heigth;
	
	// Get the image's width and height
	
	width = in_img->sx;
	heigth = in_img->sy;
	int new_heigth = (int) new_width *1.0/width * heigth;
		

	gdImageSetInterpolationMethod(in_img, GD_BILINEAR_FIXED);
    out_img = gdImageScale(in_img, new_width, new_heigth);
	if (!out_img) {
		return NULL;
	}

	return(out_img);		
} 

/******************************************************************************
 * make_thumb()
 *
 * Arguments: in - pointer to image
 *            size - target size for thumbnail
 * Returns: out - pointer to thumbnail image, or NULL in case of failure
 * Side-Effects: none
 *
 * Description: thumbnail of image with smalles dimension equal to target size
 *
 *****************************************************************************/
gdImagePtr  make_thumb(gdImagePtr in_img, int size){
	
	gdImagePtr out_img, aux_img;


	int width,heigth;
	int new_heigth, new_width;
	// Get the image's width and height

	width = in_img->sx;
	heigth = in_img->sy;

	if (heigth > width){
		new_width = size;
		new_heigth = (int) new_width *1.0/width * heigth;
	}else{
		new_heigth = size;
		new_width = (int) new_heigth * 1.0 / heigth * width;
	}
		

	gdImageSetInterpolationMethod(in_img, GD_BILINEAR_FIXED);
    aux_img = gdImageScale(in_img, new_width, new_heigth);
	if (!aux_img) {
		return NULL;
    }


	gdRect crop_area;
	crop_area.height = size;
	crop_area.width = size;
	crop_area.x = 0;
	crop_area.y = 0;

	out_img = gdImageCrop(aux_img, & crop_area);
	gdImageDestroy(aux_img);

	if (!out_img) {
		return NULL;
    }

	return(out_img);		
} 

/******************************************************************************
 * read_png_file()
 *
 * Arguments: file_name - name of file with data for PNG image
 * Returns: img - the image read from file or NULL if failure to read
 * Side-Effects: none
 *
 * Description: reads a PNG image from a file
 *
 *****************************************************************************/
gdImagePtr read_png_file(char * file_name){

	FILE * fp;
	gdImagePtr read_img;

	fp = fopen(file_name, "rb");
   	if (!fp) {
        fprintf(stderr, "Can't read image %s\n", file_name);
        return NULL;
    }
    read_img = gdImageCreateFromPng(fp);
    fclose(fp);
  	if (read_img == NULL) {
    	return NULL;
    }

	return read_img;
}

/******************************************************************************
 * write_png_file()
 *
 * Arguments: img - pointer to image to be written
 *            file_name - name of file where to save PNG image
 * Returns: (bool) 1 in case of success, 0 in case of failure to write
 * Side-Effects: none
 *
 * Description: writes a PNG image to a file
 *
 *****************************************************************************/
int write_png_file(gdImagePtr write_img, char * file_name){
	FILE * fp;

	fp = fopen(file_name, "wb");
	if (fp == NULL) {
		return 0;
	}
	gdImagePng(write_img, fp);
	fclose(fp);

	return 1;
}

/******************************************************************************
 * create_directory()
 *
 * Arguments: file_name - name of directory to be created
 * Returns: (bool) 1 if the directory already exists or succesfully created
 *                 0 in case of failure to create
 * Side-Effects: none
 *
 * Description: Create a directory. 
 *
 *****************************************************************************/
int create_directory(char * dir_name){

	DIR * d = opendir(dir_name);
	if (d == NULL){
		if (mkdir(dir_name, 0777)!=0){
			return 0;
		}
	}else{
		fprintf(stderr, "%s directory already existent\n", dir_name);
		closedir(d);
	}
	return 1;
}

/******************************************************************************
 * main()
 *
 * Arguments: (none)
 * Returns: 0 in case of sucess, positive number in case of failure
 * Side-Effects: creates thumbnail, resized copy and watermarked copies
 *               of images
 *
 * Description: implementation of the complex serial version 
 *              This application only works for a fixed pre-defined set of files
 *
 *****************************************************************************/
int main(){

	/* array containg the names of files to be processed	 */
	char * files [] =  {"Lisboa-1.png", "IST-1.png", "IST-2.png", "IST-3.png", "00841.png", "00844.png", "00846.png", "00849.png" }; 
	/* length of the files array (number of files to be processed	 */
	int nn_files = 8;

	/* file name of the image to be processed	 */
	char original_name[100];
	/* file name of the image created and to be saved on disk	 */
	char out_file_name[100];

	/* input images */
	gdImagePtr in_img,  watermark_img;
	/* output images */
	gdImagePtr out_thumb_img, out_resized_img, out_watermark_img;

	/* creation of output directories */
	if (create_directory(RESIZE_DIR) == 0){
		fprintf(stderr, "Impossible to create %s directory\n", RESIZE_DIR);
		exit(-1);
	}
	if (create_directory(THUMB_DIR) == 0){
		fprintf(stderr, "Impossible to create %s directory\n", THUMB_DIR);
		exit(-1);
	}
	if (create_directory(WATER_DIR) == 0){
		fprintf(stderr, "Impossible to create %s directory\n", WATER_DIR);
		exit(-1);
	}

    watermark_img = read_png_file("watermark.png");
	if(watermark_img == NULL){
		fprintf(stderr, "Impossible to read %s image\n", "watermark.png");
		exit(-1);
	}

	/* 1st iteration over all the files 
	 * To resize images
	 */
	for (int i = 0; i < nn_files; i++){	

		printf("resize %s\n", files[i]);
		/* load of the input file */
	    in_img = read_png_file(files[i]);
		if (in_img == NULL){
			fprintf(stderr, "Impossible to read %s image\n", files[i]);
			continue;
		}
		/* resizes of each image */
		out_resized_img = resize_image(in_img, 640);
  		if (out_resized_img == NULL) {
            fprintf(stderr, "Impossible to resize %s image\n", files[i]);
        }else{
			/* save resized */
			sprintf(out_file_name, "%s%s", RESIZE_DIR, files[i]);
			if(write_png_file(out_resized_img, out_file_name) == 0){
	            fprintf(stderr, "Impossible to write %s image\n", out_file_name);
			}
			gdImageDestroy(out_resized_img);
		}
		gdImageDestroy(in_img);
	}


	/* 2nd iteration over all the files 
	 * To create the thumbnails
	 */
	for (int i = 0; i < nn_files; i++){	

	   	printf("thumbnail %s\n", files[i]);
		/* load of the input file */
	    in_img = read_png_file(files[i]);
		if (in_img == NULL){
			fprintf(stderr, "Impossible to read %s image\n", files[i]);
			continue;
		}

		/* creation of thumbnail image */
		out_thumb_img = make_thumb(in_img, 200);
  		if (out_thumb_img == NULL) {
            fprintf(stderr, "Impossible to creat thumbnail of %s image\n", files[i]);
        }else{
			/* save thumbnail image */
			sprintf(out_file_name, "%s%s", THUMB_DIR, files[i]);
			if(write_png_file(out_thumb_img, out_file_name) == 0){
	            fprintf(stderr, "Impossible to write %s image\n", out_file_name);
			}
			gdImageDestroy(out_thumb_img);
		}
		gdImageDestroy(in_img);

	}

	/* 3rd iteration over all the files 
	 * To add the watermarks
	 */
	for (int i = 0; i < nn_files; i++){	

	    printf("watermark  %s\n", files[i]);
		/* load of the input file */
	    in_img = read_png_file(files[i]);
		if (in_img == NULL){
			fprintf(stderr, "Impossible to read %s image\n", files[i]);
			continue;
		}

		/* add watermark */
		out_watermark_img = add_watermark(in_img, watermark_img);
  		if (out_watermark_img == NULL) {
            fprintf(stderr, "Impossible to creat thumbnail of %s image\n", files[i]);
        }else{
			/* save watermark */
			sprintf(out_file_name, "%s%s", WATER_DIR, files[i]);
			if(write_png_file(out_watermark_img, out_file_name) == 0){
	            fprintf(stderr, "Impossible to write %s image\n", out_file_name);
			}
			gdImageDestroy(out_watermark_img);
		}
		gdImageDestroy(in_img);

	}

	gdImageDestroy(watermark_img);
	exit(0);
}

