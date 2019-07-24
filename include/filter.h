#ifndef FILTER_H
#define FILTER_H

#include "sketches.h"
#include "thread_data.h"
#include "cm_benchmark.h"

#define MAX_FILTER_SLACK 5

void printFilter(FilterStruct filter){
    int i=0;
    for (i=0;i<FILTER_SIZE;i++){
        printf("%d ", filter.filter_count[i]);
    }
    printf("\n");
}

int findMinIndex(FilterStruct* filter){
    int min = filter->filter_count[0];
    int index = 0;
    int i=1;
    for (i=1; i<FILTER_SIZE; i++){
        if (filter->filter_count[i] < min){
            index = i;
            min = filter->filter_count[i];
        }
    }
    return index;
}

//Checks if key is in the filter.
//Return: the index if key is in the filter, otherwise -1
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
        if (filter->filterFull < FILTER_SIZE){
            filter->filter_id[filter->filterFull] = key;
            filter->filter_count[filter->filterFull] = 1;
            filter->filterFull++;
            //localThreadData->filterCount++;
        }
        // not in the filter and filter is full, just do the insert in the normal way?
        // OR evict the old minimum and place the new element there
        // Note: when evicting an item might have been pushed in the sketch already in the past
        else{
            int minIndex = findMinIndex(filter);
            //evict the old minimum
            if (filter->filter_count[minIndex] % MAX_FILTER_SLACK){
                insert(localThreadData, filter->filter_id[minIndex], filter->filter_count[minIndex] % MAX_FILTER_SLACK);
            }
            //place the new element there
            filter->filter_id[minIndex] = key;
            filter->filter_count[minIndex] = filter->filter_count[minIndex] - filter->filter_count[minIndex] % MAX_FILTER_SLACK;
        }
    }
    else{ 
        filter->filter_count[qRes]++;
        filter->filterCount++;
        if (!(filter->filter_count[qRes] % MAX_FILTER_SLACK)){
            unsigned int new_key = (unsigned int )(filter->filter_id[qRes]);
            insert(localThreadData, key, MAX_FILTER_SLACK);
        }
    }
}
#endif
