#include <omp.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

struct Point {
    double x , y;
};

double dist (struct Point a , struct Point b) {
    double dx = a.x - b.x;
    double dy = a.y - b.y;
    return sqrt(dx*dx + dy*dy);
}

int main (int argc, char** argv)
{	srand(time(0));
    FILE* file = fopen("/shared/points.txt" , "r");
    int pointsCount=0;
	char line[100];
	while(!feof(file))
    {
		pointsCount+=1;
		fgets(line, 100, file);
	}

	rewind(file); //to make read pointer at the begin of the file again
    int threads = atoi(argv[1]); // no of threads
    int iterations = atoi(argv[2]); //no of iteration
    int it , i , j;
    omp_set_num_threads(threads);
    struct Point points[pointsCount];
    struct Point centers[threads];
    double distance[threads][pointsCount];
    double mn[pointsCount];
    int nearest[pointsCount];
    double sumX[threads];
    double sumY[threads];
    int cnt[threads];
	
    // Read The data file 
    for (i = 0 ;i < pointsCount ;i++) {
        fscanf(file , "%lf %lf" , &points[i].x , &points[i].y);
    }
	
	// Initiate 2 random numbers for each thread
    for (i = 0 ;i < threads ;i++) {
        centers[i].x = rand() % 101 + (double)rand()/RAND_MAX;
        centers[i].y = rand() % 101 + (double)rand()/RAND_MAX;
		//printf("centroid %d generated x= %0.2lf y= %0.2lf \n",i,centers[i].x,centers[i].y);
    }

	
    for (it = 0 ;it < iterations ;it++) {
		
		// Calculate the distance between each point and cluster centroid
		#pragma omp parallel private(i , j)shared(distance)
		{
			i = omp_get_thread_num();
			//printf(" no of thread working now %d \n",i);
			sumX[i] = sumY[i] = cnt[i] = 0;
			for (j = 0 ;j < pointsCount ;j++) {
				distance[i][j] = dist(centers[i] , points[j]);
			}
		}

		// Filter each point distances depending on minimum value
		for (j = 0 ;j < pointsCount ;j++) {
			mn[j] = 3e9;
			for (i = 0 ;i < threads ;i++) {
				if (distance[i][j] < mn[j]) {
					mn[j] = distance[i][j];
					nearest[j] = i;
				}
			}
		}
		
		
		//  Calculate the mean for each cluster as new cluster centroid
		#pragma omp parallel for private(j) shared(cnt,sumX,sumY) schedule(static)
		//printf(" no of thread working now %d \n",omp_get_thread_num());		
		for (j = 0 ;j < pointsCount ;j++) {
			cnt[nearest[j]]++;
			sumX[nearest[j]] += points[j].x;
			sumY[nearest[j]] += points[j].y;
		 }	
		
		
		#pragma omp parallel private(i)shared(centers)
		{
			i = omp_get_thread_num();
			//printf(" no of thread working now %d \n",i);
			if (cnt[i] != 0) {
				centers[i].x = sumX[i] / cnt[i];
				centers[i].y = sumY[i] / cnt[i];
			}
			// if the cluster doesn't have any points that belong to it
			else{
				centers[i].x = 0;
				centers[i].y = 0;
			}
		}
    }

    for (i = 0 ;i < threads ;i++) {
        printf("Cluster %d:\n" , i);
        for (j = 0 ;j < pointsCount ;j++) {
            if (nearest[j] == i) {
                printf("(%0.2lf,%0.2lf)\n" , points[j].x , points[j].y);
            }
        }
    }
	fclose(file);
}
