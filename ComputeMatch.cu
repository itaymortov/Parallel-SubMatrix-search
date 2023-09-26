
#include "CalcMatch.h"
#include <math.h>
#include "func.h"


__global__ void calcMatchingGPU(int* pic, int picSize, int* obj, int objSize, double matchingValue, int* matchCount, int* matchPlace, int objId, int* hasMatch)
{
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    int j = blockIdx.y * blockDim.y + threadIdx.y;
    if (i <= picSize - objSize && j <= picSize - objSize)
    {
        double match = 0;
        for (int x = 0; x < objSize; x++)
        {
            for (int y = 0; y < objSize; y++)
            {
                double p = pic[(i+x)*picSize + j+y];
                double o = obj[x*objSize + y];
                double matchAdd = (p - o);
                if(matchAdd<0)
                    matchAdd = matchAdd * - 1;
                matchAdd = matchAdd/p;
                match = match + matchAdd;
            }
        }
        match = match / (objSize * objSize);
        if (match <= matchingValue && *matchCount < 3 && *hasMatch != 1)
        {
        	atomicAdd(hasMatch, 1);
            
            matchPlace[*matchCount*3] = objId;
            matchPlace[*matchCount*3+1] = i;
            matchPlace[*matchCount*3+2] = j;
        }
    }
}

void calcMatchingGPU(struct Picture* pic, struct Objects* obj, double matchingValue, int* matchCount,int* match,int* matchPlace)
{
    int* d_Pic;
    int* d_Obj;
    int* d_MatchCount;
    int* d_MatchPlace;
    int* d_hasMatch; 
	
    // Allocate memory on the device
    cudaMalloc(&d_Pic, pic->picSize * pic->picSize * sizeof(int));
    cudaMalloc(&d_Obj, obj->objSize * obj->objSize * sizeof(int));
    cudaMalloc(&d_MatchCount, sizeof(int));
    cudaMalloc(&d_MatchPlace, 3 * sizeof(int));
    cudaMalloc(&d_hasMatch, sizeof(int));
	
    // Copy data from host to device
    cudaMemcpy(d_Pic, pic->pic, pic->picSize * pic->picSize * sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpy(d_Obj, obj->obj, obj->objSize * obj->objSize * sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpy(d_MatchCount, matchCount, sizeof(int), cudaMemcpyHostToDevice);
	cudaMemset(d_hasMatch, 0, sizeof(int));
	cudaMemset(d_MatchPlace, 0, 3 * sizeof(int));
	
		
    // Launch the kernel
    dim3 block(32, 32);
    //grid makes it so their is enough threads to get from the beginning of the matrix to the end minus the object size, that so we will not go over the matrix size.
    //in the calcMatchingGPU function we have a for the size of the object size so that we can calc the border of the pictures (from picSize-objSize to picSize)
    dim3 grid((pic->picSize - obj->objSize + block.x - 1) / block.x, (pic->picSize - obj->objSize + block.y - 1) / block.y);
    
    
    calcMatchingGPU<<<grid, block>>>(d_Pic, pic->picSize, d_Obj, obj->objSize, matchingValue, d_MatchCount, d_MatchPlace, obj->objId ,d_hasMatch);
	
    // Copy data from device to host
    cudaMemcpy(matchCount, d_MatchCount, sizeof(int), cudaMemcpyDeviceToHost);
    cudaMemcpy(match, d_hasMatch, sizeof(int), cudaMemcpyDeviceToHost);
    cudaMemcpy(matchPlace, d_MatchPlace, 3 * sizeof(int), cudaMemcpyDeviceToHost);	
    //only if we have a match we are adding 1 to matchCount.
    if(*match == 1){
        *matchCount++;
    }

    // Free memory on the device
    cudaFree(d_Pic);
    cudaFree(d_Obj);
    cudaFree(d_MatchPlace);
    cudaFree(d_MatchCount);
    cudaFree(d_hasMatch);

}
