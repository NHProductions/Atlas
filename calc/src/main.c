#include <tice.h>
#include <fileioc.h> 
#include <graphx.h> 
#include <keypadc.h>
#include <debug.h>   
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <ti/vars.h>
#include <Tundora.h> 
#define FRAME_W 100
#define FRAME_H 80
#define MAX_FRAME_SIZE (FRAME_W*FRAME_H)
#define X0 0
#define Y0 0
uint8_t frame[FRAME_W*FRAME_H];
uint16_t palette[256];
int currentSelected = -1; // Index of currently selected video. -1 is uninitialized.
List* Videos = NULL; // List of video structs representing all videos found in memory.
typedef struct {
    char name[6];
    uint16_t frame_count;
    uint8_t frame_bin_amount;
    uint16_t frame_rate;
} video;
void draw_frame(uint8_t *f) {
    for (int y = 0; y < FRAME_H; y++) {
        uint16_t *row = &gfx_vram[(Y0 + y) * LCD_WIDTH + X0];
        for (int x = 0; x < FRAME_W; x++) {
            row[x] = palette[f[y * FRAME_W + x]];
        }
    }
}
void play_video(const char* varname, video* vid) {
    ti_var_t var = ti_Open(varname, "r");
    if (!var) {
        dbg_printf("Failed to open %s\n", varname);
        return;
    }

    uint16_t frame_count;
    ti_Read(&frame_count, sizeof(frame_count), 1, var);

        for (int i = 0; i < frame_count; i++) {
            uint32_t size;
            if (ti_Read(&size, sizeof(size), 1, var) != 1) {
                dbg_printf("Failed to read size for frame %d\n", i);
                break;
            }

            uint32_t expected = FRAME_W * FRAME_H;
            if (size < expected) {
                dbg_printf("Frame %d smaller than expected (%u < %u)\n", i, size, expected);
            }

            uint32_t to_read = expected;
            if (to_read > sizeof(frame)) to_read = sizeof(frame);

            if ((uint32_t)ti_Read(frame, 1, to_read, var) != to_read) {
                dbg_printf("Failed to read frame %d data\n", i);
                break;
            }
            
            draw_frame(frame);
            delay(vid->frame_rate);
        }

    ti_Close(var);
}
void playFullVideo(video* vid) {
    
    dbg_printf("Staring load pallete");
    char name_copy[12];
    strcpy(name_copy, vid->name);
    ti_var_t paletteVar = ti_Open(strncat(name_copy, "PAL", 12), "r");
    
    dbg_printf("Palette opened");
    if (!paletteVar) {
        dbg_sprintf("Failed to open palette ");
        dbg_printf("%s", name_copy);
        gfx_End();
        return;
    }
    if (ti_Read(palette, sizeof(palette), 1, paletteVar) != 1) {
        dbg_printf(dbgout, "Failed to read full palette data\n");
    }
    ti_Close(paletteVar);
    dbg_printf(dbgout, "Palette loaded\n");
    gfx_FillScreen(0); 
    delay(1000);
    for (int i = 0; i < vid->frame_bin_amount; i++) {
        char varname[12];
        char name_copy2[12];
        strcpy(name_copy2, vid->name);
        sprintf(varname, "%s%03d", name_copy2, i);
        play_video(varname, vid);
    }
    gfx_End();
}

video* getVideoFromVarName(char* name) {
    video* vid = malloc(sizeof(video));
    ti_var_t var = ti_Open(name, "r");

    // Formatting: 
    /* [3 Bytes: "DAT"], [5 Bytes: name, no null terminator.], [2 Bytes: frame_count], [1 Byte: frame_bin_amount], [2 Bytes: frame_rate] */
    if (!var) {
        dbg_printf(dbgout, "Failed to open %s\n", name);
        return vid;
    }
    char header[3];
    ti_Read(header, sizeof(header), 1, var);
    ti_Read(vid->name, 5, 1, var);
    vid->name[5] = '\0';
    ti_Read(&vid->frame_count, sizeof(vid->frame_count), 1, var);
    ti_Read(&vid->frame_bin_amount, sizeof(vid->frame_bin_amount), 1, var);
    ti_Read(&vid->frame_rate, sizeof(vid->frame_rate), 1, var);
    ti_Close(var);
    return vid;
}
void getVideos() {


    char *var_name;
    char *vat_ptr = NULL;
    uint8_t var_type;
    while ((var_name = ti_DetectAny(&vat_ptr, "DAT", &var_type) ) ) {
        if (var_type == OS_TYPE_APPVAR) {
            List_AppendElement(Videos, getVideoFromVarName(var_name));
        }
    }
}
int main(void)
{
    Videos = malloc(sizeof(List));
    *Videos = NewList();
    dbg_printf("STARTING");
    gfx_Begin();
    getVideos();
    for (int i = 0; i < Videos->length; i++) {
        video vid = *(video*)List_GetElement(Videos, i)->data;
        dbg_printf("\nVideo %d: Name: %s, Frames: %d, Bins: %d, Frame Rate: %d\n", i, vid.name, vid.frame_count, vid.frame_bin_amount, vid.frame_rate);
    }
    if (Videos->length == 0) {
        dbg_printf("No videos found!");
        return 0;
    }    
    else {
        currentSelected = 0;
        while (1==1) {
            kb_Scan();
            if (kb_Data[6] & kb_Enter) {
                dbg_printf("Selected: %s", ((video*)List_GetElement(Videos, currentSelected)->data)->name);
                playFullVideo((video*)List_GetElement(Videos, currentSelected)->data);
                break;
            }
            if (kb_Data[7] & kb_Up) {
                currentSelected++;
                if (currentSelected >= Videos->length) currentSelected = 0;
                dbg_printf("Selected: %s", ((video*)List_GetElement(Videos, currentSelected)->data)->name);
                gfx_FillScreen(255);
                gfx_SetTextXY(0, 230);
                gfx_PrintString(((video*)List_GetElement(Videos, currentSelected)->data)->name);
                delay(200);
            }
            if (kb_Data[7] & kb_Down) {
                currentSelected--;
                if (currentSelected < 0) currentSelected = Videos->length - 1;
                dbg_printf("Selected: %s", ((video*)List_GetElement(Videos, currentSelected)->data)->name);
                gfx_FillScreen(255);
                gfx_SetTextXY(0, 230);
                gfx_PrintString(((video*)List_GetElement(Videos, currentSelected)->data)->name);
                delay(200);
            }
            delay(100);
        }
    }



    gfx_End();
    return 0;
}
