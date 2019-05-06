#ifndef PARAMS_H
#define PARAMS_H

#define RUN_WITH_SCRIPT 0

#if RUN_WITH_SCRIPT
   #define LOCAL_COPIES  REPLACE_LOCAL_COPIES
   #define FIXED_DURATION REPLACE_FIXED_DURATION //sec
   #define HYBRID REPLACE_HYBRID
   #define REMOTE_INSERTS REPLACE_REMOTE_INSERTS
#else
    #define LOCAL_COPIES 0
    #define FIXED_DURATION 0 //sec
    #define HYBRID 0 //home many insertions can be kept localy per counter
    #define REMOTE_INSERTS 0
#endif


#endif