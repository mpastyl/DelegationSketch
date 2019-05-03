#ifndef PARAMS_H
#define PARAMS_H

#define RUN_WITH_SCRIPT 1

#if RUN_WITH_SCRIPT
   #define UPDATE_ONLY_MINIMUM REPLACE_UPDATE_MIN
   #define ATOMIC_INCREMENTS REPLACE_ATOMIC_INC
   #define LOCAL_COPIES  REPLACE_LOCAL_COPIES
   #define FIXED_DURATION REPLACE_FIXED_DURATION //sec
   #define HYBRID REPLACE_HYBRID
#else
    #define UPDATE_ONLY_MINIMUM 0
    #define ATOMIC_INCREMENTS 0
    #define LOCAL_COPIES 1
    #define FIXED_DURATION 0 //sec
    #define HYBRID 5 //home many insertions can be kept localy per counter
#endif


#endif