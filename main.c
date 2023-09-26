#include <stdio.h>
#include "func.h"
#include <math.h>
#include <stdlib.h>
#include <mpi.h>
#include <omp.h>
#include "CalcMatch.h"

#define STOP 100
#define NEXT 200
#define DIE 300
#define GO 400
#define MATCH 500

//Write the results to a file called "Output.txt"
void printMatch(struct Picture* pics,int numPictures){
	FILE *fp;
	char filename[] = "Output.txt";
	fp = fopen ("Output.txt", "w");

	if (fp == NULL) {
        printf("Could not open file %s", filename);
        	return;
    	}


	for(int i = 0; i<numPictures;i++){
		if(pics[i].matchCount == 3){
		
			printf("Picture %d: found Objects: ",pics[i].picId);
			fprintf(fp,"Picture %d: found Objects: ",pics[i].picId);
			for(int j = 0;j<3;j++){
				printf("%d Position(%d,%d); ", pics[i].matchPlace[j][0], pics[i].matchPlace[j][1], pics[i].matchPlace[j][2]);
				fprintf(fp,"%d Position(%d,%d); ", pics[i].matchPlace[j][0], pics[i].matchPlace[j][1], pics[i].matchPlace[j][2]);
			}
			printf("\n");
			fprintf(fp,"\n");
		}else{
			printf("Picture %d: No three different Objects were found\n",pics[i].picId);
			fprintf(fp,"Picture %d: No three different Objects were found\n",pics[i].picId);
		
		}
	}
}

int main(int argc, char **argv)
{
    int myid,size;
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &myid);
    MPI_Comm_size(MPI_COMM_WORLD,&size);
    MPI_Status status;
	double matchingValue;
	int numPictures, numObj;
	struct Picture *pictures;
	struct Objects *objs;
	
	//Firstly the Master gets the input from the file "input.txt"
    if(myid == 0){
	
	FILE *fp;
	char filename[] = "input.txt";
	fp = fopen("input.txt", "r");

    if (fp == NULL) {
        printf("Could not open file %s", filename);
        return 1;
    }

    fscanf(fp, "%lf", &matchingValue);
    fscanf(fp, "%d", &numPictures);
    //printf("n-%d, v-%f\n",numPictures,matchingValue);

    pictures = (struct Picture*)malloc(numPictures * sizeof(struct Picture));

    for (int i = 0; i < numPictures; i++) {
        fscanf(fp, "%d", &(pictures + i)->picId);
        //printf("Picture ID: %d\n", (pictures + i)->picId);

        fscanf(fp, "%d", &(pictures + i)->picSize);
        //printf("Picture Size: %d\n", (pictures + i)->picSize);
        
        
        (pictures + i)->pic = (int *)malloc(sizeof(int) * (pictures + i)->picSize*(pictures + i)->picSize);
	
        for (int j = 0; j < (pictures + i)->picSize; j++) {
            for (int k = 0; k < (pictures + i)->picSize; k++) {
                fscanf(fp, "%d", &((pictures+i)->pic[j*(pictures+i)->picSize + k]));
                
            }
        }
        
        (pictures + i)->matchCount = 0;
    }
    
    fscanf(fp, "%d", &numObj);
    objs = (struct Objects *)malloc(numObj * sizeof(struct Objects));

    for (int i = 0; i < numObj; i++)
    {
        fscanf(fp, "%d", &(objs + i)->objId);
        //printf("Object ID: %d\n", (objs + i)->objId);

        fscanf(fp, "%d", &(objs + i)->objSize);
        //printf("Object Size: %d\n", (objs + i)->objSize);
        
   	(objs + i)->obj = (int *)malloc(sizeof(int) * (objs + i)->objSize*(objs + i)->objSize);
        
        for (int j = 0; j < (objs + i)->objSize; j++)
        {
            for (int k = 0; k < (objs + i)->objSize; k++)
            {
                fscanf(fp, "%d", &((objs + i)->obj[j*(objs + i)->objSize + k]));
                //printf("%d ", (objs + i)->obj[j][k]);
                
            }
            //printf("\n");
        }
        //printf("\n");
    }
    fclose(fp);
	}
	
	
	
	//Secondly the master send to everyone the matchingValue that was decided in the input file aswell as the number of objects and number of pictuers
	MPI_Bcast(&matchingValue,1,MPI_DOUBLE,0,MPI_COMM_WORLD);
	MPI_Bcast(&numObj,1,MPI_INT,0,MPI_COMM_WORLD);
	MPI_Bcast(&numPictures,1,MPI_INT,0,MPI_COMM_WORLD);
	
	
	//Thirdly the master start dynamicly send to every other processes the pictures and objects one by one.
	if(myid == 0){
		int picCount = 0;
		int objCount = 0;
		int recvProc = 1;

		while(picCount < numPictures){
			MPI_Send(&pictures[picCount].picSize, 1, MPI_INT, recvProc, GO, MPI_COMM_WORLD);
			MPI_Send(&pictures[picCount].picId, 1, MPI_INT, recvProc, GO, MPI_COMM_WORLD);
			MPI_Send(&objs[objCount].objSize, 1, MPI_INT, recvProc, GO, MPI_COMM_WORLD);
			MPI_Send(&objs[objCount].objId, 1, MPI_INT, recvProc, GO, MPI_COMM_WORLD);
			MPI_Send(pictures[picCount].pic,  pictures[picCount].picSize * pictures[picCount].picSize ,MPI_INT, 1, GO, MPI_COMM_WORLD);
			MPI_Send(objs[objCount].obj, objs[objCount].objSize*objs[objCount].objSize, MPI_INT, 1, GO, MPI_COMM_WORLD);
			
			MPI_Recv(pictures[picCount].matchPlace[pictures[picCount].matchCount],3,MPI_INT,MPI_ANY_SOURCE ,MPI_ANY_TAG,MPI_COMM_WORLD,&status);
			recvProc = status.MPI_SOURCE;
						
			if(status.MPI_TAG == NEXT){
				objCount++;
				if(objCount == numObj){
					objCount = 0;
					picCount++;
				}
			}else if(status.MPI_TAG == MATCH){
				pictures[picCount].matchCount++;
				objCount++;
				if(pictures[picCount].matchCount == 3){
					picCount++;
					objCount = 0;
				}
			}
			if(picCount == numPictures)
				for(int i = 1; i < size; i++){
				MPI_Send(&status.MPI_TAG,1,MPI_INT,i,DIE,MPI_COMM_WORLD);
				}
		}
		
		printMatch(pictures,numPictures);

		
		for (int i = 0; i < numPictures; i++) {
	   		free(pictures[i].pic);
		}
		free(pictures);
		
		for (int i = 0; i < numObj; i++) {
	   		free(objs[i].obj);
		}
		free(objs);
		
    }
    
    // other processes will get the object and pictures to check and while the tag the master send is GO (DIE tag ends receiving) and send the necessary variabels to the GPU to do the heavy calc. 
    if(myid != 0){
    	
        int* pic;
        int* obj;
        int picId;
        int objId;
		status.MPI_TAG = GO;
        int result = 0;
        int matchCount = 0;
        int sizeTocalc;
        int picSize;
        int objSize;
 		struct Picture tPic;
 		struct Objects tObj;
		
		while(1){
			result = 0;
			int* tMatchPlace;
			tMatchPlace = (int*)calloc(3,sizeof(int));
			MPI_Recv(&picSize, 1, MPI_INT, 0,MPI_ANY_TAG, MPI_COMM_WORLD,&status);

			if(status.MPI_TAG == DIE)
				break;
			MPI_Recv(&picId, 1, MPI_INT, 0,MPI_ANY_TAG, MPI_COMM_WORLD,&status);
			if(status.MPI_TAG == DIE)
				break;
			MPI_Recv(&objSize, 1, MPI_INT, 0,GO, MPI_COMM_WORLD,&status);

			MPI_Recv(&objId, 1, MPI_INT, 0,GO, MPI_COMM_WORLD,&status);

			
			pic = (int*)malloc(picSize*picSize*sizeof(int));
			obj = (int*)malloc(objSize*objSize*sizeof(int));
			
			MPI_Recv(pic, picSize * picSize ,MPI_INT, 0,GO, MPI_COMM_WORLD,&status);

			MPI_Recv(obj, objSize * objSize, MPI_INT, 0,GO ,MPI_COMM_WORLD,&status);

			tPic.picId = picId;
			tPic.pic = pic;
			tPic.picSize = picSize; 
			tObj.objId = objId;
			tObj.obj = obj;
			tObj.objSize = objSize; 
			calcMatchingGPU(&tPic,&tObj,matchingValue,&matchCount,&result,tMatchPlace);

			if(result == 1){
				MPI_Send(tMatchPlace,3,MPI_INT, 0, MATCH, MPI_COMM_WORLD);
				if(matchCount == 3)
					matchCount = 0;
			}
			else{
				MPI_Send(tMatchPlace,3,MPI_INT, 0, NEXT, MPI_COMM_WORLD);
			}
				
		}
	    free(pic);
	    free(obj);
    }
    
    MPI_Finalize();
    return 0;
}
