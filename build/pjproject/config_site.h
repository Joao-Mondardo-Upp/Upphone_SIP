/* Configuracao para build do MicroSIP/Upphone (Windows, sem video, sem OpenSSL, com G.729 (bcg729)) */
#define PJ_CONFIG_MAXIMUM_SPEED
#include <pj/config_site_sample.h>

#define PJMEDIA_HAS_VIDEO               0
#define PJMEDIA_HAS_OPUS_CODEC          0
#define PJMEDIA_HAS_BCG729              1
#define BCG729_STATIC                   1   /* bcg729 linked as a static library */
#define PJMEDIA_HAS_SRTP                1
#define PJ_HAS_SSL_SOCK                 0
#define PJMEDIA_SRTP_HAS_DTLS           0

/* config_site_sample.h (PJ_CONFIG_MAXIMUM_SPEED) already sets these two: undef avoids warning C4005 */
#define PJSUA_MAX_ACC                   32
#undef  PJSUA_MAX_CALLS
#define PJSUA_MAX_CALLS                 32
#undef  PJ_IOQUEUE_MAX_HANDLES
#define PJ_IOQUEUE_MAX_HANDLES          1024
