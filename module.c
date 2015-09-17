
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "module.h"

// TODO:
// - M.K. only
// - check for endians



//If this position contains 'M.K.','8CHN',
//  '4CHN','6CHN','FLT4' or 'FLT8' the module
//  has 31 instruments.

/******************************************************************
* Sample functions
******************************************************************/
static char *sample_name(const char *module, int sample_number)
{
    char *samplename = malloc(22);
   
    unsigned int offset = 20 + (30 * sample_number);

    for(int i = 0; i < 22; i++)
    {
        samplename[i] = *(module + offset + i);
    }

    return samplename;
}

static unsigned short sample_length(const char *module, int sample_number)
{
    unsigned int offset = 20 + (30 * sample_number);

    return (unsigned char)module[0 + offset + 22] << 8 | (unsigned char)module[1 + offset + 22];
}

static char sample_finetune(const char *module, int sample_number)
{
    unsigned int offset = 20 + (30 * sample_number);

    return module[offset + 24];
}

static char sample_volume(const char *module, int sample_number)
{
    unsigned int offset = 20 + (30 * sample_number);

    return module[offset + 25];
}

static unsigned short sample_loop_start(const char *module, int sample_number)
{
    unsigned int offset = 20 + (30 * sample_number);

    return (unsigned char)module[0 + offset + 26] << 8 | (unsigned char)module[1 + offset + 26];
}

static unsigned short sample_loop_length(const char *module, int sample_number)
{
    unsigned int offset = 20 + (30 * sample_number);

    return (unsigned char)module[0 + offset + 28] << 8 | (unsigned char)module[1 + offset + 28];
}

static SampleDesc *module_sample_descriptions(const char *module)
{
    SampleDesc *samples = malloc(31 * sizeof(SampleDesc));

    for(int i = 0; i < 31; i++)
    {
        samples[i].sample_name = sample_name(module, i);
        samples[i].length = sample_length(module, i);
        samples[i].finetune = sample_finetune(module, i);
        samples[i].volume = sample_volume(module, i);
        samples[i].loop_start = sample_loop_start(module, i);   
        samples[i].loop_length = sample_loop_length(module, i); 
    }

    return samples;
}

static char *sample_sampledata(const char *module, int num_patterns, int sample_size)
{
    unsigned char *sample_module = (unsigned char*)(module + 1084 + (1024 * (num_patterns + 1)));

    char *sample_data = malloc(sample_size);

    memcpy(sample_data, sample_module, sample_size);

    return sample_data;
}

/******************************************************************
* Pattern functions
******************************************************************/

static unsigned int pattern_unique_length(const char *module)
{
    unsigned int max_pattern = 0;
    unsigned int i = 0;

    for (int i = 0; i < 128; i++) 
    {
        char current_pattern = module[952 + i];

        if (current_pattern > max_pattern)
        {
            max_pattern = module[952 + i];
        }
    }

    return max_pattern;
}

static PatternRow *pattern_row(const char *module, int pattern_number, int row_number)
{
    PatternRow *pattern_row = malloc(sizeof(PatternRow));

    int pattern_offset = 1024 * pattern_number;
    int row_offset = 16 * row_number;

    unsigned char *row_data = (unsigned char*)(module + 1084 + pattern_offset + row_offset);

    pattern_row->channel_a = row_data[0] << 24 | row_data[1] << 16 | row_data[2] << 8 | row_data[3];
    pattern_row->channel_b = row_data[4] << 24 | row_data[5] << 16 | row_data[6] << 8 | row_data[7];
    pattern_row->channel_c = row_data[8] << 24 | row_data[9] << 16 | row_data[10] << 8 | row_data[11];
    pattern_row->channel_d = row_data[12] << 24 | row_data[13] << 16 | row_data[14] << 8 | row_data[15];

    return pattern_row;
}

static PatternData *pattern_pattern(const char *module, int pattern_number)
{
    PatternData *pattern = malloc(sizeof(PatternData));

    for(int i = 0; i < 64; i++)
    {
        PatternRow *pr = pattern_row(module, pattern_number, i);
        pattern->pattern_rows[i] = *pr;
    }

    return pattern;
}

/******************************************************************
* Module functions
******************************************************************/

static char *module_songtitle(const char *module)
{
    char *songname = malloc(20);

    for(int i = 0; i < 20; i++)
    {   
        songname[i] = *(module + i);
    }

    return songname;
}

static unsigned char module_songlength(const char *module)
{
    unsigned char sl = module[950];

    return sl;
}

static unsigned char module_restartbyte(const char *module)
{
    unsigned char rb = module[951];

    return rb;
}

static unsigned char *module_patternsequences(const char *module)
{
    unsigned char *sequence = malloc(128);

    for(int i = 0; i < 128; i++)
    {
        sequence[i] = *(module + 952 + i);
    }

    return sequence;
}

static char *module_songid(const char *module)
{
    char *id = malloc(4);

    for(int i = 0; i < 4; i++)
    {
        id[i] = *(module + 1080 + i);
    }

    return id;
}

static PatternData *module_pattern_data(const char *module)
{
    unsigned int length = pattern_unique_length(module);

    PatternData *pattern_data = malloc(length * sizeof(PatternData));

    for(int i = 0; i <= length; i++)
    {
        PatternData *pd = pattern_pattern(module, i);
        pattern_data[i] = *pd;
    }

    return pattern_data;
}

/******************************************************************
* Helpers
******************************************************************/

static short change_endian_word(unsigned short value)
{
    unsigned short a = value;
    unsigned short b = value;

    return b << 8 | a >> 8;
}

static unsigned int change_endian_longword(unsigned int value)
{
    unsigned int a = value;
    unsigned int b = value;
    unsigned int c = value;
    unsigned int d = value;

    return ((d << 24) & 0xff000000) | ((c << 8) & 0x00ff0000) | ((b >> 8) & 0x0000ff00) | ((a >> 24) & 0x000000ff);
}

/******************************************************************
* Public
******************************************************************/

ModuleData *get_module_data(const char *module)
{
    ModuleData *module_data = malloc(sizeof(ModuleData));

    module_data->song_title = module_songtitle(module);
    module_data->sample_desciptions = module_sample_descriptions(module);
    module_data->song_length = module_songlength(module);
    module_data->restart_byte = module_restartbyte(module);
    module_data->pattern_play_sequence = module_patternsequences(module);
    module_data->module_id = module_songid(module);
    module_data->pattern_data = module_pattern_data(module);

    int max_pattern = 0;

    for(int i = 0; i < 128; i++)
    {    
        if(module_data->pattern_play_sequence[i] > max_pattern)
        {
            max_pattern = module_data->pattern_play_sequence[i];
        }
    }

    int sample_size = 0;

    for(int i = 0; i < 31; i++)
    {
        sample_size += module_data->sample_desciptions[i].length * 2;
    }

    module_data->sample_data = sample_sampledata(module, max_pattern, sample_size);

    if(strcmp(module_data->module_id, MODULE_ID_MK) != 0)
    {
        fprintf(stderr, "Wrong module format: %s", module_data->module_id);

        return 0;
    }

    return module_data;
}

void write_module(ModuleData *module_data, char *filename)
{
    FILE *file = fopen(filename, "wb+");

    if (file == 0) 
    {
        fprintf(stderr, "Cannot open \"%s\"!\n", filename);

        return;
    }

    int max_pattern = 0;

    for(int i = 0; i < 128; i++)
    {    
        if(module_data->pattern_play_sequence[i] > max_pattern)
        {
            max_pattern = module_data->pattern_play_sequence[i];
        }
    }

    int sample_size = 0;

    for(int i = 0; i < 31; i++)
    {
        sample_size += module_data->sample_desciptions[i].length * 2;
    }

    unsigned short word_value = 0;
    unsigned int longword_value = 0;

    fwrite(module_data->song_title, MOD_SONGNAME_SIZE, 1, file);

    for(int i = 0; i < 31; i++)
    {
        fwrite(module_data->sample_desciptions[i].sample_name, SMP_NAME_SIZE, 1, file);

        word_value = change_endian_word(module_data->sample_desciptions[i].length);
        fwrite(&word_value, 2, 1, file);

        fwrite(&module_data->sample_desciptions[i].finetune, 1, 1, file);
        fwrite(&module_data->sample_desciptions[i].volume, 1, 1, file);

        word_value = change_endian_word(module_data->sample_desciptions[i].loop_start);
        fwrite(&word_value, 2, 1, file);

        word_value = change_endian_word(module_data->sample_desciptions[i].loop_length);
        fwrite(&word_value, 2, 1, file);
    }

    fwrite(&module_data->song_length, 1, 1, file);
    fwrite(&module_data->restart_byte, 1, 1, file);
    fwrite(module_data->pattern_play_sequence, 128, 1, file);
    fwrite(module_data->module_id, 4, 1, file);

    for(int p = 0; p <= max_pattern; p++)
    {
        for(int r = 0; r < 64; r++)
        {
            longword_value = change_endian_longword(module_data->pattern_data[p].pattern_rows[r].channel_a);
            fwrite(&longword_value, 4, 1, file);

            longword_value = change_endian_longword(module_data->pattern_data[p].pattern_rows[r].channel_b);
            fwrite(&longword_value, 4, 1, file);
            
            longword_value = change_endian_longword(module_data->pattern_data[p].pattern_rows[r].channel_c);
            fwrite(&longword_value, 4, 1, file);
            
            longword_value = change_endian_longword(module_data->pattern_data[p].pattern_rows[r].channel_d);
            fwrite(&longword_value, 4, 1, file);
        }
    }

    for(int i = 0; i < sample_size; i++)
    {
        unsigned char sample_byte = module_data->sample_data[i];
        fwrite(&sample_byte, 1, 1, file);
    }

    fclose(file);
}


