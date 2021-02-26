typedef struct{
    SPFLOAT freq, atk, dec, istor;
    SPFLOAT tpidsr;
    SPFLOAT sr;
    SPFLOAT delay[4];

    SPFLOAT tatk, tdec;
    SPFLOAT patk, pdec;
}sp_fofilt;

int sp_fofilt_create(sp_fofilt **t);
int sp_fofilt_destroy(sp_fofilt **t);
int sp_fofilt_init(sp_data *sp, sp_fofilt *p);
int sp_fofilt_compute(sp_data *sp, sp_fofilt *p, SPFLOAT *in, SPFLOAT *out);
void sp_fofilt_reset(sp_data *sp, sp_fofilt *p);
