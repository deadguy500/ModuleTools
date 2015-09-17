
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "module.h"

static void print_module_data(ModuleData *module_data)
{
    printf("SongTitle: %s \n", module_data->song_title);
    
    for(int i = 0; i < 31; i++)
    {
        printf("\n--- %u --------------------------------\n", (i + 1)); 
        printf("\tSample.Name: %s\n", module_data->sample_desciptions[i].sample_name);
        printf("\tSample.HalfLength: %u\n", module_data->sample_desciptions[i].length);
        printf("\tSample.Finetune: %u\n", module_data->sample_desciptions[i].finetune);
        printf("\tSample.Volume: %u\n", module_data->sample_desciptions[i].volume);
        printf("\tSample.LoopStart: %u\n", module_data->sample_desciptions[i].loop_start);
        printf("\tSample.LoopLength: %u\n", module_data->sample_desciptions[i].loop_length);
    }

    printf("SongLength: %u \n", module_data->song_length);
    printf("RestartByte: %u \n", module_data->restart_byte);

    printf("PatternPlaySequence: ");

    unsigned int max_pattern = 0;

    for(int i = 0; i < 128; i++)
    {
        printf(" %u, ", module_data->pattern_play_sequence[i]);
    
        if(module_data->pattern_play_sequence[i] > max_pattern)
        {
            max_pattern = module_data->pattern_play_sequence[i];
        }
    }

    printf("\n");

    printf("ModuleId: %s \n", module_data->module_id);

    printf("PatternData:\n");

    for(int p = 0; p <= max_pattern; p++)
    {
        printf("\n--- %u ----------------------------------\n", p); 

        for(int r = 0; r < 64; r++)
        {
            printf("\t%.8x %.8x %.8x %.8x\n", 
                module_data->pattern_data[p].pattern_rows[r].channel_a,
                module_data->pattern_data[p].pattern_rows[r].channel_b,
                module_data->pattern_data[p].pattern_rows[r].channel_c,
                module_data->pattern_data[p].pattern_rows[r].channel_d);            
        }
    }
}

static int remove_sync_8xx(unsigned int *channel_data)
{
    int value = *(channel_data);

    if ((value & 0x00000F00) == 0x800)
    {
        value = value & 0xFFFFF000;
        *(channel_data) = value;

        return 1;
    }    

    return 0;
}

static int remove_sync_e8x(unsigned int *channel_data)
{
    int value = *(channel_data);

    if((value & 0x00000FF0) == 0xE80)
    {
        value = value & 0xFFFFF000;
        *(channel_data) = value;
    
        return 1;
    }

    return 0;
}

static void remove_sync_commands(ModuleData *module_data)
{
    unsigned int max_pattern = 0;

    for(int i = 0; i < 128; i++)
    {
        if(module_data->pattern_play_sequence[i] > max_pattern)
        {
            max_pattern = module_data->pattern_play_sequence[i];
        }
    }

    int channel_a_e8 = 0;
    int channel_b_e8 = 0;
    int channel_c_e8 = 0;
    int channel_d_e8 = 0;

    int channel_a_8 = 0;
    int channel_b_8 = 0;
    int channel_c_8 = 0;
    int channel_d_8 = 0;

    for(int p = 0; p <= max_pattern; p++)
    {
        for(int r = 0; r < 64; r++)
        {
            int ae8 = remove_sync_e8x(&module_data->pattern_data[p].pattern_rows[r].channel_a);
            int a8 = remove_sync_8xx(&module_data->pattern_data[p].pattern_rows[r].channel_a);

            int be8 = remove_sync_e8x(&module_data->pattern_data[p].pattern_rows[r].channel_b);
            int b8 = remove_sync_8xx(&module_data->pattern_data[p].pattern_rows[r].channel_b);

            int ce8 = remove_sync_e8x(&module_data->pattern_data[p].pattern_rows[r].channel_c);
            int c8 = remove_sync_8xx(&module_data->pattern_data[p].pattern_rows[r].channel_c);

            int de8 = remove_sync_e8x(&module_data->pattern_data[p].pattern_rows[r].channel_d);
            int d8 = remove_sync_8xx(&module_data->pattern_data[p].pattern_rows[r].channel_d);
        
            channel_a_e8 += ae8;
            channel_b_e8 += be8;
            channel_c_e8 += ce8;
            channel_d_e8 += de8;

            channel_a_8 += a8;
            channel_b_8 += b8;
            channel_c_8 += c8;
            channel_d_8 += d8;

            if(ae8 != 0 | be8 != 0 | ce8 != 0 | de8 != 0)
            {
                printf("E8x hit on pattern %.2d and row %.2d\n", p, r);
            }

            if(a8 != 0 | b8 != 0 | c8 != 0 | d8 != 0)
            {
                printf("8xx hit on pattern %.2d and row %.2d\n", p, r);
            }
        }
    }

    printf("Number of E8x: %u + %u + %u + %u = %u\n", 
        channel_a_e8, 
        channel_b_e8, 
        channel_c_e8, 
        channel_d_e8, 
        channel_a_e8 + channel_b_e8 + channel_c_e8 + channel_d_e8);

    printf("Number of 8xx: %u + %u + %u + %u = %u\n", 
        channel_a_8, 
        channel_b_8, 
        channel_c_8, 
        channel_d_8, 
        channel_a_8 + channel_b_8 + channel_c_8 + channel_d_8);    
}

static size_t filesize(FILE *file, const char *name)
{
    long current_position;
    long size;

    if ((current_position = ftell(file)) >= 0)
    {
        if (fseek(file, 0, SEEK_END) >= 0)
        {
            if ((size = ftell(file)) >= 0)
            {
                if (fseek(file, current_position, SEEK_SET) >= 0)
                {
                    return (size_t)size;        
                }
            }
        }
    }

    fprintf(stderr,"Cannot determine file size of \"%s\"!\n", name);
  
    return 0;
}

int main(int argc,char *argv[])
{
    int rc = 1;

    if (argc != 2) 
    {
        printf("Usage: %s [OPTION] <INPUT MODULE> <OUTPUT MODULE>\n", argv[0]);
        printf("Removed E8x and 8xx Protracker commands.\n\n");
        //printf("   -p  Shows where the sync command are set\n\n");
        printf("Report no bugs to deadguy / pacif!c\n");

        return 1;
    }

    FILE *file = fopen(argv[1],"rb");

    if (file != 0) 
    {
        size_t file_length = filesize(file, argv[1]);

        if (file_length > 0) 
        {
            char *module = malloc(file_length);
            
            if (module) 
            {
                if (fread(module, 1, file_length, file) == file_length)
                {
                    ModuleData *module_data = get_module_data(module);

                    if(module_data)
                    {
                        remove_sync_commands(module_data);
                        write_module(module_data, "/Users/deadguy/Desktop/output.mod");
                    }
                }
                else
                {
                    fprintf(stderr, "Read error (\"%s\")!\n", argv[1]);                
                }
            }
            else
            {
                fprintf(stderr, "Out of memory!\n");            
            }
        }

        fclose(file);
    }
    else
    {
        fprintf(stderr, "Cannot open \"%s\"!\n",argv[1]);
    }

    return rc;
}