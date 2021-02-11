#ifndef _FAS_FAUST_H_
#define _FAS_FAUST_H_

#include <stdio.h>

#include "faust/dsp/llvm-c-dsp.h"

struct _fas_faust_ui_control {
    FAUSTFLOAT *zone;
    char label[34];
    FAUSTFLOAT min;
    FAUSTFLOAT max;
    struct _fas_faust_ui_control *next;
};

extern void addFaustControl(struct _fas_faust_ui_control *ctrl, const char *label, FAUSTFLOAT *zone, FAUSTFLOAT min, FAUSTFLOAT max);
struct _fas_faust_ui_control *getFaustControl(struct _fas_faust_ui_control *ctrl, char *label);
extern void freeFaustControls(struct _fas_faust_ui_control *ctrl);

static void ui_open_tab_box(void* iface, const char* label){

}

static void ui_open_horizontal_box(void* iface, const char* label) {

}

static void ui_open_vertical_box(void* iface, const char* label){

}

static void ui_close_box(void* iface) {

}

static void ui_add_button(void* iface, const char* label, FAUSTFLOAT* zone){

}

static void ui_add_check_button(void* iface, const char* label, FAUSTFLOAT* zone) {

}

static void ui_add_vertical_slider(void* iface, const char* label,
                                   FAUSTFLOAT* zone, FAUSTFLOAT init, FAUSTFLOAT min,
                                   FAUSTFLOAT max, FAUSTFLOAT step) {

}

static void ui_add_horizontal_slider(void *iface, const char *label,
                                     FAUSTFLOAT *zone, FAUSTFLOAT init, FAUSTFLOAT min,
                                     FAUSTFLOAT max, FAUSTFLOAT step) {

}

static void ui_add_num_entry(void* iface, const char* label,
                             FAUSTFLOAT* zone, FAUSTFLOAT init, FAUSTFLOAT min,
                             FAUSTFLOAT max, FAUSTFLOAT step) {
    addFaustControl((struct _fas_faust_ui_control *)iface, label, zone, min, max);
}

static void ui_add_horizontal_bargraph(void* iface, const char* label, FAUSTFLOAT* zone, FAUSTFLOAT min, FAUSTFLOAT max) {

}

static void ui_add_vertical_bargraph(void* iface, const char* label, FAUSTFLOAT* zone, FAUSTFLOAT min, FAUSTFLOAT max) {

}

static void ui_add_sound_file(void* iface, const char* label, const char* filename, struct Soundfile** sf_zone) {

}

static void ui_declare(void* iface, FAUSTFLOAT *zone, const char *key, const char *val) {

}

struct _fas_faust_dsp {
    llvm_dsp *dsp;
    UIGlue *ui;
    struct _fas_faust_ui_control *controls;
};

struct _faust_factories {
    llvm_dsp_factory **factories;
    size_t len;
};

extern struct _faust_factories *createFaustFactories(char *directory, char *libs_path);
extern void freeFaustFactories(struct _faust_factories *fl);

#endif