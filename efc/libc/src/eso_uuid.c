/**
 * @file  eso_uuid.c
 * @brief ES UUID Library
 */

#include "eso_uuid.h"
#include "eso_md5.h"
#include "eso_libc.h"
#include "eso_datetime.h"
#include "eso_net.h"

#ifdef WIN32
#include <windows.h>
#else
#include <unistd.h>     /* for getpid, gethostname */
#include <sys/time.h>   /* gettimeofday */
#endif

void eso_uuid_format(char *buffer, const es_uuid_t *uuid)
{
    const unsigned char *d = uuid->data;

    eso_sprintf(buffer,
            "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
            d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7],
            d[8], d[9], d[10], d[11], d[12], d[13], d[14], d[15]);
}

/* convert a pair of hex digits to an integer value [0,255] */
#if 'A' == 65
static unsigned char parse_hexpair(const char *s)
{
    int result;
    int temp;

    result = s[0] - '0';
    if (result > 48)
	result = (result - 39) << 4;
    else if (result > 16)
	result = (result - 7) << 4;
    else
	result = result << 4;

    temp = s[1] - '0';
    if (temp > 48)
	result |= temp - 39;
    else if (temp > 16)
	result |= temp - 7;
    else
	result |= temp;

    return (unsigned char)result;
}
#else
static unsigned char parse_hexpair(const char *s)
{
    int result;

    if (isdigit(*s)) {
        result = (*s - '0') << 4;
    }
    else {
        if (isupper(*s)) {
            result = (*s - 'A' + 10) << 4;
        }
        else {
            result = (*s - 'a' + 10) << 4;
        }
    }

    ++s;
    if (isdigit(*s)) {
        result |= (*s - '0');
    }
    else {
        if (isupper(*s)) {
            result |= (*s - 'A' + 10);
        }
        else {
            result |= (*s - 'a' + 10);
        }
    }

    return (unsigned char)result;
}
#endif

es_status_t eso_uuid_parse(es_uuid_t *uuid,
                                         const char *uuid_str)
{
    int i;
    unsigned char *d = uuid->data;

    for (i = 0; i < 36; ++i) {
	char c = uuid_str[i];
	if (!eso_isxdigit((int)c) &&
	    !(c == '-' && (i == 8 || i == 13 || i == 18 || i == 23)))
            /* ### need a better value */
	    return ES_FAILURE;
    }
    if (uuid_str[36] != '\0') {
        /* ### need a better value */
	return ES_FAILURE;
    }

    d[0] = parse_hexpair(&uuid_str[0]);
    d[1] = parse_hexpair(&uuid_str[2]);
    d[2] = parse_hexpair(&uuid_str[4]);
    d[3] = parse_hexpair(&uuid_str[6]);

    d[4] = parse_hexpair(&uuid_str[9]);
    d[5] = parse_hexpair(&uuid_str[11]);

    d[6] = parse_hexpair(&uuid_str[14]);
    d[7] = parse_hexpair(&uuid_str[16]);

    d[8] = parse_hexpair(&uuid_str[19]);
    d[9] = parse_hexpair(&uuid_str[21]);

    for (i = 6; i--;)
	d[10 + i] = parse_hexpair(&uuid_str[i*2+24]);

    return ES_SUCCESS;
}


#define NODE_LENGTH 6

static int uuid_state_seqnum;
static unsigned char uuid_state_node[NODE_LENGTH] = { 0 };

static es_uint64_t get_accurate_time(void)
{
#ifdef WIN32
	return GetTickCount();
#elif defined(__MTK__)
	return clock();
#else
	struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000 + tv.tv_usec;
#endif
}

static void get_random_info(unsigned char node[NODE_LENGTH])
{
    unsigned char seed[ES_MD5_DIGEST_LEN];
    es_md5_ctx_t c;

    /* ### probably should revise some of this to be a bit more portable */
    struct {
        es_uint64_t t;
        char hostname[257];
        #ifdef WIN32
        
        #else
        /* Add thread id here, if applicable, when we get to pthread or apr */
        pid_t pid;
        #endif
    } r;

    eso_md5_init(&c);
    r.t = get_accurate_time();
#ifdef WIN32
	{
    	WSADATA wsaData; 
		WSAStartup(MAKEWORD(2,2), &wsaData);
		gethostname(r.hostname, 256);
		WSACleanup();
	}
#else
    r.pid = getpid();
    gethostname(r.hostname, 256);
#endif
    eso_md5_update(&c, (const unsigned char *)&r, sizeof(r));
    eso_md5_final(seed, &c);

    eso_memcpy(node, seed, NODE_LENGTH);    /* use a subset of the seed bytes */
}

/* true_random -- generate a crypto-quality random number. */
static int true_random(void)
{
    es_uint64_t time_now;
	
    time_now =  get_accurate_time();
    eso_srand((unsigned int)(((time_now >> 32) ^ time_now) & 0xffffffff));

    return eso_rand() & 0x0FFFF;
}

static void init_state(void)
{
    uuid_state_seqnum = true_random();
    
    get_random_info(uuid_state_node);
    uuid_state_node[0] |= 0x01;                    /* this designates a random node ID */
}

static void get_current_time(es_uint64_t *timestamp)
{
    /* ### this needs to be made thread-safe! */

    es_uint64_t time_now;
    static es_uint64_t time_last = 0;
    static es_uint64_t fudge = 0;

    time_now = get_accurate_time();
    
    /* if clock reading changed since last UUID generated... */
    if (time_last != time_now) {
        /* The clock reading has changed since the last UUID was generated.
           Reset the fudge factor. if we are generating them too fast, then
           the fudge may need to be reset to something greater than zero. */
        if (time_last + fudge > time_now)
            fudge = time_last + fudge - time_now + 1;
        else
            fudge = 0;
        time_last = time_now;
    }
    else {
        /* We generated two really fast. Bump the fudge factor. */
        ++fudge;
    }

    *timestamp = time_now + fudge;
}

void eso_uuid_get(es_uuid_t *uuid)
{
    es_uint64_t timestamp;
    unsigned char *d = uuid->data;

    if (!uuid_state_node[0])
        init_state();

    get_current_time(&timestamp);

    d[0] = (unsigned char)timestamp;
    d[1] = (unsigned char)(timestamp >> 8);
    d[2] = (unsigned char)(timestamp >> 16);
    d[3] = (unsigned char)(timestamp >> 24);
    d[4] = (unsigned char)(timestamp >> 32);
    d[5] = (unsigned char)(timestamp >> 40);
    d[6] = (unsigned char)(timestamp >> 48);
    d[7] = (unsigned char)(((timestamp >> 56) & 0x0F) | 0x10);

    d[8] = (unsigned char)(((uuid_state_seqnum >> 8) & 0x3F) | 0x80);
    d[9] = (unsigned char)uuid_state_seqnum;

    eso_memcpy(&d[10], uuid_state_node, NODE_LENGTH);
}
