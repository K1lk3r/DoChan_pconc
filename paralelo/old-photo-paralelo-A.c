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

#include <gd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <time.h>
#include "image-lib.h"
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>





/* the directories wher output files will be placed */
#define OLD_IMAGE_DIR "./Old-image-dir/"

struct ThreadArgs {
	int thread_id;
    char c_start;
    char c_end; 
};

char **images_list;  
char *imageDirectory;
gdImagePtr in_texture_img;
int num_threads;



void *old_image_thread(void *arg) {

    struct ThreadArgs *args = (struct ThreadArgs *)arg;
	int thread_id = args->thread_id;
    char start = args->c_start;
    char end = args->c_end;
    char filepath[256];
    char writepath[100];

    gdImagePtr in_img;
    gdImagePtr out_smoothed_img;
    gdImagePtr out_contrast_img;
    gdImagePtr out_textured_img;
    gdImagePtr out_sepia_img;

    int j = start;

	struct timespec start_time, end_time;
	clock_gettime(CLOCK_MONOTONIC, &start_time);


    while (j <= end) {
        sprintf(writepath,"%s%s",OLD_IMAGE_DIR,images_list[j]);
        if(access(writepath,F_OK) != -1)
        {
            printf("%s ficheiro encontrado\n",writepath);
            j++;
        }
        else
        {
            sprintf(filepath, "%s/%s", imageDirectory, images_list[j]);
            in_img = read_jpeg_file(filepath);
            if (in_img == NULL) {
                fprintf(stderr, "Impossible to read %s image\n", images_list[j]);
                continue;
            }
            printf("Number Image: %d\n", j);
            out_contrast_img = contrast_image(in_img);
            out_smoothed_img = smooth_image(out_contrast_img);
            out_textured_img = texture_image(out_smoothed_img, in_texture_img);
            out_sepia_img = sepia_image(out_textured_img);

            sprintf(filepath, "%s%s", OLD_IMAGE_DIR, images_list[j]);
            write_jpeg_file(out_sepia_img, filepath);

            gdImageDestroy(out_smoothed_img);
            gdImageDestroy(out_sepia_img);
            gdImageDestroy(out_contrast_img);
            gdImageDestroy(in_img);

            j++;
        }
    }

	clock_gettime(CLOCK_MONOTONIC, &end_time);
	struct timespec time = diff_timespec(&end_time, &start_time);
	char output_filename[256];
    sprintf(output_filename, "timing_%d.txt", num_threads);
    FILE *output_file = fopen(output_filename, "a");
    if (output_file != NULL) {
        fprintf(output_file, "Thread_%d %ld.%09ld\n", thread_id, time.tv_sec, time.tv_nsec);
        fclose(output_file);
	}

    pthread_exit(NULL);
}

/******************************************************************************
 * main()
 *
 * Arguments: (none)
 * Returns: 0 in case of sucess, positive number in case of failure
 * Side-Effects: creates thumbnail, resized copy and watermarked copies
 *               of images
 *
 * Description: implementation of the parallel version 
 *              
 *****************************************************************************/
int main(int argc, char *argv[]) {


	//  Prepare the timer variables
	
    struct timespec start_time_total, end_time_total;
    struct timespec start_time_seq, end_time_seq;
    struct timespec start_time_par, end_time_par;

	clock_gettime(CLOCK_MONOTONIC, &start_time_total);
	clock_gettime(CLOCK_MONOTONIC, &start_time_seq);

	// Get arguments: Directory of the images and number of threads

    if (argc != 3) {
        printf("Usage: %s <image_directory> <num_threads>\n", argv[0]);
    }

	imageDirectory = argv[1];
	char filepath[256];


	// Get image lists from the folder 

	sprintf(filepath, "%s/image-list.txt", imageDirectory);
	printf("Filepath: %s\n", filepath);
	FILE *file = fopen( filepath, "r");
	printf("Filepath: %s\n", filepath);
	
    if (file == NULL) {
        printf("Error opening file");
        return EXIT_FAILURE;
    }
	
    // Get the file size

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    printf("File size: %ld\n", file_size);

    // Read the entire file "image-list.txt" into a buffer

    char *file_buffer = malloc(file_size + 1);
    if (file_buffer == NULL) {
        printf("Memory allocation error");
        fclose(file);
        return EXIT_FAILURE;
    }
    

    fread(file_buffer, 1, file_size, file);
    char *token_buffer = strdup(file_buffer);
    file_buffer[file_size] = '\0';     

    // Count the number of images in the file "image-list.txt"

    int num_images = 0;
    char *token = strtok(file_buffer,"\n");
    while (token != NULL) {
        if (token != NULL)
        {
            //verificar aqui extensao das imagens
            char * ext = strrchr(token,'.');
            printf("ext = %s\n", ext);
            if(strcmp(ext,".jpeg") != 0)
            {
                num_images--;
                printf("cheguei aqui\n");
            }
            
        }
        num_images++;
        token = strtok(NULL, "\n");
    }
    printf("num imagens = %d\n",num_images);
	
    // Allocate memory for the array of image names                                         //pode ser aqui feita a verificação .jpeg
    images_list = (char **)malloc(num_images * sizeof(char *));
    if (images_list == NULL) {
        printf("Memory allocation error");
        fclose(file);
        free(file_buffer);
        return EXIT_FAILURE;
    }

	// Create a pointer to each name of the images in "image-list.txt"

    int i = 0;
    char *token1 = strtok(token_buffer, "\n");
    while (token1 != NULL) {
        char * ext = strrchr(token1,'.');
        if(strcmp(ext,".jpeg") == 0)
        {
            images_list[i] = strdup(token1);  // Allocate memory for each word
            printf("Imagem = %s\n", images_list[i]);
            i++;
            token1 = strtok(NULL, "\n");
        }
        else
        {
            printf("Imagem nao jpeg = %s\n", token1);
            token1 = strtok(NULL,"\n");
        }
        
        
    }

	// Iniatlizing the Threads

	num_threads = atoi(argv[2]);
	struct ThreadArgs thread_args[num_threads];
    pthread_t thread_ids[num_threads];

	// Calculate the number of images per thread

	printf("num_images: %d\n", num_images);
	printf("num_threads: %d\n", num_threads);
	
	int window_image = (num_images ) / num_threads;
	int remaining_images = (num_images ) % num_threads;
	int aux = 0;

	//printf("window_image: %d\n", window_image);
	//printf("remaining_images: %d\n", remaining_images);

	// Create the output directory

	if (create_directory(OLD_IMAGE_DIR) == 0){
		fprintf(stderr, "Impossible to create %s directory\n", OLD_IMAGE_DIR);
		exit(-1);
	}

    // Read the texture image  

    in_texture_img = read_png_file("./paper-texture.png");
    if (in_texture_img == NULL) {
        fprintf(stderr, "Impossible to read texture image\n");
        exit(-1);
    }


	for (int i = 0; i < num_threads; i++) {

		thread_args[i].thread_id = i;
		thread_args[i].c_start = aux;
		thread_args[i].c_end = aux + window_image - 1 + (remaining_images > 0 ? 1 : 0);

		printf("Thread %d: %d - %d\n", i, thread_args[i].c_start, thread_args[i].c_end);
		pthread_create(&thread_ids[i], NULL, old_image_thread, &thread_args[i]);

		aux = thread_args[i].c_end +1 ;
		remaining_images--;
	}

	for (int i = 0; i < num_threads; i++) {
		pthread_join(thread_ids[i], NULL);

	}

	/* creation of output directories */

	clock_gettime(CLOCK_MONOTONIC, &end_time_seq);
	clock_gettime(CLOCK_MONOTONIC, &start_time_par);
	
	//Create Threads

	clock_gettime(CLOCK_MONOTONIC, &end_time_par);
	clock_gettime(CLOCK_MONOTONIC, &end_time_total);


    struct timespec par_time = diff_timespec(&end_time_par, &start_time_par);
    struct timespec seq_time = diff_timespec(&end_time_seq, &start_time_seq);
    struct timespec total_time = diff_timespec(&end_time_total, &start_time_total);
    printf("\tseq \t %10jd.%09ld\n", seq_time.tv_sec, seq_time.tv_nsec);
    printf("\tpar \t %10jd.%09ld\n", par_time.tv_sec, par_time.tv_nsec);
    printf("total \t %10jd.%09ld\n", total_time.tv_sec, total_time.tv_nsec);


	exit(0);
}

