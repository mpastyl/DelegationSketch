#ifndef FILTER_H
#define FILTER_H

#include "sketches.h"
#include "thread_data.h"

#define MAX_FILTER_SLACK 5

int queryFilter(unsigned int key, int * filterIndexes){
    const __m128i s_item = _mm_set1_epi32(key);
    __m128i *filter = (__m128i *)filterIndexes;
    
    __m128i f_comp = _mm_cmpeq_epi32(s_item, filter[0]);
    __m128i s_comp = _mm_cmpeq_epi32(s_item, filter[1]);
    __m128i t_comp = _mm_cmpeq_epi32(s_item, filter[2]);
    __m128i r_comp = _mm_cmpeq_epi32(s_item, filter[3]);

    f_comp = _mm_packs_epi32(f_comp, s_comp);
    t_comp = _mm_packs_epi32(t_comp, r_comp);
    f_comp = _mm_packs_epi32(f_comp, t_comp);

    int found  = _mm_movemask_epi8(f_comp);
    if (found){
        return __builtin_ctz(found);
    }
    else{
        return -1;
    }
}

void updateWithFilter(threadDataStruct * localThreadData, unsigned int key){

    FilterStruct * filter = &(localThreadData->Filter);
    int qRes = queryFilter(key,filter->filter_id);
    if (qRes == -1){
        // not in the filter but filter has space
        if (filter->filterFull < 16){
            filter->filter_id[filter->filterFull] = key;
            filter->filter_count[filter->filterFull] = 1;
            filter->filterFull++;
            //localThreadData->filterCount++;
        }
        // not in the filter and filter is full, just do the insert in the normal way?
        else{
            int owner = key % numberOfThreads; 
            localThreadData->sketchArray[owner]->Update_Sketch_Atomics(key, 1);
        }
    }
    else{ 
        filter->filter_count[qRes]++;
        filter->filterCount++;
        if (filter->filter_count[qRes] % MAX_FILTER_SLACK){
            unsigned int new_key = (unsigned int )(filter->filter_id[qRes]);
            int owner = new_key % numberOfThreads; 
            localThreadData->sketchArray[owner]->Update_Sketch_Atomics(new_key, MAX_FILTER_SLACK);
            filter->filter_count[qRes] = 0;
        }
    }
    // if (localThreadData->filterCount == 20){
    //     for (int j=0; j<16; j++){
    //         if (localThreadData->filter_count[j]){
    //             unsigned int new_key = localThreadData->filter_id[j];
    //             int owner = new_key % numberOfThreads; 
    //             localThreadData->sketchArray[owner]->Update_Sketch_Atomics(new_key,localThreadData->filter_count[j]);
    //             localThreadData->filter_count[j] = 0;
    //             localThreadData->filter_id[j] = -1;//FIXME
    //         }
    //     }
    //     localThreadData->filterCount = 0;
    //     localThreadData->filterFull = 0;
    // }
}
#endif