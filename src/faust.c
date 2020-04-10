#include "tinydir/tinydir.h"

#include "tools.h"
#include "faust.h"

struct _faust_factories *createFaustFactories(char *directory) {
    struct _faust_factories *fl = (struct _faust_factories *)malloc(sizeof(struct _faust_factories));

    tinydir_dir dir;
    int ret = tinydir_open_sorted(&dir, directory);

    if (ret == -1) {
        printf("tinydir_open failed for directory '%s'.\n", directory);

        return 0;
    }

    size_t dir_length = strlen(directory);
    char *current_dir = (char *)malloc(sizeof(char) * (dir_length + 1));
    memcpy(current_dir, directory, dir_length + 1);

    char error_msg[4096];

    tinydir_file file;

    unsigned int f = 0;

    int n_reg = 0;
    for (f = 0; f <= dir.n_files; f++) {
        tinydir_readfile_n(&dir, &file, f);

        if (file.is_reg) {
            n_reg += 1;
        }
    }

    fl->factories = (llvm_dsp_factory **)malloc(sizeof(llvm_dsp_factory *) * n_reg);
    fl->len = 0;

    fprintf(stdout, "loading Faust DSP from '%s'\n", directory);

    for (f = 0; f < dir.n_files; f++) {
        tinydir_readfile_n(&dir, &file, f);

        if (file.is_reg) {
            char *filepath = create_filepath(current_dir, file.name);

            if (!filepath) {
                continue;
            }

            //
            const char* argv1[1] = { 0 };

            llvm_dsp_factory *dsp_factory = createCDSPFactoryFromFile(filepath, 0, argv1, "", error_msg, -1);
            if (dsp_factory) {
                fl->factories[fl->len] = dsp_factory;

                fprintf(stdout, "DSP code %lu '%s' loaded.\n", fl->len, file.name);

                fl->len += 1;
            } else {
                fprintf(stdout, "%s", error_msg);
                fflush(stdout);
            }

            free(filepath);
        }
    }

    free(current_dir);

    tinydir_close(&dir);

    fflush(stdout);

    return fl;
}

void freeFaustFactories(struct _faust_factories *fl) {
    if (!fl) {
        return;
    }

    unsigned int i = 0;
    for (i = 0; i < fl->len; i += 1) {
        deleteCDSPFactory(fl->factories[i]);
    }

    free(fl->factories);
    free(fl);
}

void addFaustControl(struct _fas_faust_ui_control *ctrl, const char *label, FAUSTFLOAT *zone, FAUSTFLOAT min, FAUSTFLOAT max) {
    while (ctrl->next) {
        ctrl = ctrl->next;
    }

    ctrl->next = (struct _fas_faust_ui_control *)calloc(1, sizeof(struct _fas_faust_ui_control));

    strncpy(ctrl->label, label, 32);
    ctrl->label[32] = '\n';
    ctrl->label[33] = '\0';

    ctrl->zone = zone;
    ctrl->min = min;
    ctrl->max = max;
}

struct _fas_faust_ui_control *getFaustControl(struct _fas_faust_ui_control *ctrl, char *label) {
    while (ctrl) {
        if (strncmp(ctrl->label, label, 32) == 0) {
            return ctrl;
        }
        ctrl = ctrl->next;
    }

    return NULL;
}

void freeFaustControls(struct _fas_faust_ui_control *ctrl) {
    struct _fas_faust_ui_control *tmp;

    while (ctrl) {
        tmp = ctrl;
        ctrl = ctrl->next;
        free(tmp);
    }
}