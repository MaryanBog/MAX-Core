// ==============================
// File: include/maxcore/c_api.h
// ==============================
#ifndef MAXCORE_C_API_H
#define MAXCORE_C_API_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32)
  #if defined(MAXCORE_CAPI_BUILD)
    #define MAXCORE_CAPI __declspec(dllexport)
  #else
    #define MAXCORE_CAPI __declspec(dllimport)
  #endif
#else
  #define MAXCORE_CAPI
#endif

typedef struct maxcore_handle maxcore_handle;

typedef enum maxcore_event {
    MAXCORE_EVENT_NORMAL   = 0,
    MAXCORE_EVENT_COLLAPSE = 1,
    MAXCORE_EVENT_ERROR    = 2
} maxcore_event;

typedef struct maxcore_params {
    double alpha;
    double eta;
    double beta;
    double gamma;
    double rho;
    double lambda_phi;
    double lambda_m;
    double kappa_max;
} maxcore_params;

typedef struct maxcore_state {
    double phi;
    double memory;
    double kappa;
} maxcore_state;

typedef struct maxcore_lifecycle {
    uint64_t step_counter;
    int terminal;
    int collapse_emitted;
} maxcore_lifecycle;

typedef struct maxcore_derived_frame {
    double d_phi;
    double d_memory;
    double d_kappa;

    double phi_rate;
    double memory_rate;
    double kappa_rate;

    double kappa_ratio;
    double kappa_distance;

    double load_term;
    double regen_term;

    uint64_t step_counter;
    int terminal;
    int collapse_emitted;
} maxcore_derived_frame;

MAXCORE_CAPI const char* maxcore_version(void);

MAXCORE_CAPI maxcore_handle* maxcore_create(
    const maxcore_params* params,
    size_t delta_dim,
    const maxcore_state* initial_state,
    const double* delta_max_opt
);

MAXCORE_CAPI void maxcore_destroy(maxcore_handle* h);

MAXCORE_CAPI maxcore_event maxcore_step(
    maxcore_handle* h,
    const double* delta_input,
    size_t delta_len,
    double dt
);

MAXCORE_CAPI int maxcore_get_current(const maxcore_handle* h, maxcore_state* out);
MAXCORE_CAPI int maxcore_get_previous(const maxcore_handle* h, maxcore_state* out);
MAXCORE_CAPI int maxcore_get_lifecycle(const maxcore_handle* h, maxcore_lifecycle* out);

MAXCORE_CAPI int maxcore_compute_derived(
    const maxcore_handle* h,
    double dt,
    maxcore_derived_frame* out
);

MAXCORE_CAPI const char* maxcore_last_error(const maxcore_handle* h);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // MAXCORE_C_API_H
