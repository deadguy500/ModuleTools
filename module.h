#ifndef _MODULE_H_
#define _MODULE_H_

#define MOD_SONGNAME_SIZE       20
#define SMP_NAME_SIZE           22


#define MODULE_ID_MK        "M.K."


typedef struct _patternrow
{                                   // 0x----YXXX   Y: sample number X: command
    unsigned int channel_a;         // Left channel
    unsigned int channel_b;         // Middle left channel
    unsigned int channel_c;         // Middle right channel
    unsigned int channel_d;         // 
} PatternRow;

typedef struct _patterndata
{
    PatternRow pattern_rows[64];
} PatternData;

typedef struct _sampledesc
{
    char *sample_name;              // Sample name
    unsigned short length;          // Sample length / 2. 
    char finetune;                  // 
    char volume;                    // Valid 0 - 64    
    unsigned short loop_start;      // Sample loop start / 2
    unsigned short loop_length;     // Sample loop length / 2
} SampleDesc;

typedef struct _moduledata
{
    char *song_title;
    SampleDesc *sample_desciptions; //[31];
    unsigned char song_length;
    unsigned char restart_byte;
    unsigned char *pattern_play_sequence; //[127]
    char *module_id;
    PatternData *pattern_data;
    char *sample_data;
} ModuleData;

ModuleData *get_module_data(const char *module);

void write_module(ModuleData *module_data, char *filename);

#endif
