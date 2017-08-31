#include "es_main.h"
#include "libc.h"
#include "eso_except.h"

typedef void * volatile vvoidp;
typedef vvoidp *pvvoidp;

static void test_mstr(void)
{
	char *str1 = eso_mstrdup("XXXXXXXXXXXXXXXXXXXXX");
	printf("str1=%s\n", str1);
	eso_mfree(str1);

	str1 = NULL;
	eso_msprintf(&str1, "xxx:%d,%.2f,%s", 1, 1.92, "YYY");
	printf("str1=%s\n", str1);
	ESO_MFREE(&str1);
}

static void test_mpool(void)
{
	es_mpool_t *pool = eso_mempool_create();

	char *str2 = (char*)eso_mpmalloc(pool, 10);
	eso_strcpy(str2, "1234567890");
	printf("str2=%s\n", str2);
	str2 = (char*)eso_mprealloc(str2, 20);
	eso_strcpy(str2, "abcdefghij1234567890");
	printf("str2=%s\n", str2);
	
	eso_mempool_free(&pool);
}

static void test_bitset(void)
{
	es_bitset_t *bitset;
	const char *fmtstr = "000100010100101010000000000000111111111";
	
	printf("old str=[%s]\n", fmtstr);
	int len = strlen(fmtstr);
	bitset = eso_bitset_make2(len, (char*)fmtstr);
	
	es_string_t *str = eso_bitset_to_string(bitset);
	printf("str 1=[%s]\n", str);
	ESO_MFREE(&str);
	
	eso_bitset_xor(bitset, 32767);
	str = eso_bitset_to_string(bitset);
	printf("str 2=[%s]\n", str);
	ESO_MFREE(&str);
	
	eso_bitset_clear(bitset);
	eso_bitset_set_bit(bitset, 20);
	//eso_bitset_flip_bit(bitset, 3);
	eso_bitset_or(bitset, 32767);
	str = eso_bitset_to_string(bitset);
	printf("str 3=[%s]\n", str);
	ESO_MFREE(&str);
	
	int count = eso_bitset_count(bitset);
	printf("count=[%d]\n", count);
	
	if (eso_bitset_test_any(bitset))
		printf("any=TRUE\n");
	if (eso_bitset_test_none(bitset))
		printf("none=TRUE\n");
	
    eso_bitset_free(&bitset);
}

static void test_conf(void)
{
	es_bson_t *bson = eso_bson_create(NULL);

	eso_conf_load(bson, "sample.conf");
	char *p = eso_bson_get_str(bson, "/proc/prv_conf/http/include");
	printf("conf v1=%s\n", p);
	p = eso_bson_get_str(bson, "/proc/refork_wait");
	printf("conf v2=%s\n", p);

	eso_bson_destroy(&bson);
}

static void test_json(void)
{
	const char *end = NULL;
	//char *json_str = "{\"test\":\"\\u6211\\u662f\\u6d4b\\u8bd5\"}";
	const char *json_str = "{\"Cars\" : [{\"Honda\": [\"CRV\", \"Accord\", \"Civic\", \"Pilot\"]},{\"Toyota\":[\"Camry\", \"Corolla\", \"Sienna\", 2233]}]}";
	es_json_t *json = eso_json_make2(json_str, &end, 0);
	if (!json && end) {
		printf("end=%s\n", end);
		return;
	}

	es_string_t *out = NULL;

	char *xxx = eso_json_sprintf(json, &out, TRUE);
	printf("xxx = %s\n", xxx);
	ESO_MFREE(&out);
	char *yyy = eso_json_sprintf(json, &out, FALSE);
	printf("yyy = %s\n", yyy);
	ESO_MFREE(&out);
	
	//eso_json_free(&json);

	int numbers[3] = {111, 222, 333};
	es_json_t *json2 = eso_json_create_intArray(numbers, 3);
	eso_json_add_item_to_object(json, "xxx", json2);
	char *zzz = eso_json_sprintf(json, &out, TRUE);
	printf("zzz = %s\n", zzz);
	ESO_MFREE(&out);

	es_json_t *json3 = eso_json_get_array_item(json, 0);
	char *aaa = eso_json_sprintf(json3, &out, TRUE);
	printf("aaa = %s\n", aaa);
	ESO_MFREE(&out);
	es_json_t *json4 = eso_json_get_array_item(eso_json_get_array_item(json, 0), 1);
	char *bbb = eso_json_sprintf(json4, &out, TRUE);
	printf("bbb = %s\n", bbb);
	ESO_MFREE(&out);
	es_json_t *json5 = eso_json_get_object_item(json, "Cars");
	char *ccc = eso_json_sprintf(json5, &out, TRUE);
	printf("ccc = %s\n", ccc);
	ESO_MFREE(&out);

	//eso_json_free(&json2);
	eso_json_free(&json);
}

static void test_uuid(void)
{
	es_uuid_t uuid;
	char fmtuuid[ES_UUID_FORMATTED_LENGTH];
	
	eso_uuid_get(&uuid);
	eso_uuid_format(fmtuuid, &uuid);
	printf("uudi = %s\n", fmtuuid);

	eso_uuid_get(&uuid);
	eso_uuid_format(fmtuuid, &uuid);
	printf("uudi = %s\n", fmtuuid);

	eso_uuid_get(&uuid);
	eso_uuid_format(fmtuuid, &uuid);
	printf("uudi = %s\n", fmtuuid);

	eso_uuid_get(&uuid);
	eso_uuid_format(fmtuuid, &uuid);
	printf("uudi = %s\n", fmtuuid);

	eso_uuid_get(&uuid);
	eso_uuid_format(fmtuuid, &uuid);
	printf("uudi = %s\n", fmtuuid);

	eso_uuid_get(&uuid);
	eso_uuid_format(fmtuuid, &uuid);
	printf("uudi = %s\n", fmtuuid);

	eso_uuid_get(&uuid);
	eso_uuid_format(fmtuuid, &uuid);
	printf("uudi = %s\n", fmtuuid);

	eso_uuid_get(&uuid);
	eso_uuid_format(fmtuuid, &uuid);
	printf("uudi = %s\n", fmtuuid);
}

static void test_crc32(void)
{
	es_file_t *pf = eso_fopen("xxx.dat", "rb");
	if (!pf) {
		printf("open xxx.dat fail.\n");
		return;
	}

	int length = eso_fsize(pf);
	es_byte_t *buffer = (es_byte_t*)eso_malloc(length);
	eso_fread(buffer, length, pf);

	es_uint32_t crc32 = eso_crc32_calc(buffer, length);
	printf("crc32=0x%x\n", crc32);

	eso_fclose(pf);
}

static void test_datetime(void)
{
	es_tm_t tm, result;
	es_string_t *strdate = NULL;

	//eso_dt_last_day_of_mon
	eso_log("eso_dt_last_day_of_mon(11, 2011)=%d\n", eso_dt_last_day_of_mon(11, 2011));
	eso_log("eso_dt_last_day_of_mon(2, 2011)=%d\n", eso_dt_last_day_of_mon(2, 2011));

	//eso_dt_dow
	eso_log("eso_dt_dow(2011, 11, 30)=%d\n", eso_dt_dow(2011, 11, 30));
	eso_log("eso_dt_dow(2010, 2, 2)=%d\n", eso_dt_dow(2010, 2, 2));
	eso_log("eso_dt_dow(2010, 2, 30)=%d\n", eso_dt_dow(2010, 2, 30));

	//eso_dt_get_week_number
	eso_dt_now(&tm);
	eso_log("eso_get_date_time(&tm)=%s\n", eso_fmt_time(&strdate, "%Y%m%d %H:%M:%S", &tm));
	eso_log("eso_dt_get_week_number()=%d\n", eso_dt_get_week_number(&tm));

	//eso_dt_cal_time
	eso_dt_cal_time(&tm, 10, 20, &result);
	eso_log("eso_dt_cal_time(&tm, 10, 20, &result)=%s\n", eso_fmt_time(&strdate, "%Y%m%d %H:%M:%S", &result));
	eso_dt_cal_time(&tm, -10, -20, &result);
	eso_log("eso_dt_cal_time(&tm, -10, -20, &result)=%s\n", eso_fmt_time(&strdate, "%Y%m%d %H:%M:%S", &result));
	eso_dt_cal_time(&tm, -10, 20, &result);
	eso_log("eso_dt_cal_time(&tm, -10, 20, &result)=%s\n", eso_fmt_time(&strdate, "%Y%m%d %H:%M:%S", &result));
	eso_dt_cal_time(&tm, 500, 20, &result);
	eso_log("eso_dt_cal_time(&tm, 500, 20, &result)=%s\n", eso_fmt_time(&strdate, "%Y%m%d %H:%M:%S", &result));

	eso_mfree(strdate);
}

static void test_atomic(void)
{
	es_int32_t m = 1, n = 100;
	//es_int64 lm = 1LL, ln = 100LL;
	const char* m2 = "hello";
	char* n2;

	printf("sizeof(int32_t)==%d   sizeof(void *)==%d\n", (int) sizeof(es_int32_t),
			(int) sizeof(void *));

	printf("CAS: %d == 1  then  2\n", m);
	n = eso_atomic_compare_and_swap32(&m, 1, 2);
	printf("changed to: %d,  result=%s\n", m, n ? "true" : "false");

	printf("CAS: %d == 1  then  3\n", m);
	n = eso_atomic_compare_and_swap32(&m, 1, 3);
	printf("changed to: %d,  result=%s\n", m, n ? "true" : "false");

	printf("CAS pointer: '%s' == 'hello'  then  'new'\n", m2);
	n = eso_atomic_compare_and_swapptr((pvvoidp) & m2, (char *) "hello", (char *) "new");
	printf("changed to: %s, result=%s\n", m2,
			n ? (char *) "true" : (char *) "false");

	printf("CAS pointer: '%s' == 'hello'  then  'new2'\n", m2);
	n = eso_atomic_compare_and_swapptr((pvvoidp) & m2, (char*) m2, (char *) "new2");
	printf("changed to: %s, result=%s\n", m2, n ? "true" : "false");

	n = 100;
	printf("Atomic Inc of %d\n", n);
	m = eso_atomic_fetch_and_sub32((int *) &n, 1);
	printf("changed to: %d,  result=%d\n", n, m);

	n = 100;
	printf("Atomic Inc of %d\n", n);
	m = eso_atomic_sub_and_fetch32((int *) &n, 1);
	printf("changed to: %d,  result=%d\n", n, m);

	n = 2;
	printf("Atomic Set of %d\n", n);
	m = eso_atomic_test_and_set32((int *) &n, 10);
	printf("changed to: %d,  result=%d\n", n, m);

	printf("Atomic Fetch-and-Add 2 to pointer to '%s'\n", m2);
	n2 = (char *) eso_atomic_fetch_and_add64((volatile es_int64_t *) & m2, 2);
	printf("changed to: %s,  result=%s\n", m2, n2);
}

static void test_file(void) {
	es_file_t* f = eso_fopen("./test_file.txt", "wb");
	eso_fwrite("12345", 5, f);
	eso_fflush(f);

	es_file_t* fdup = eso_fdopen(eso_fileno(f), "wb");
	int r1 = eso_fclose(f);
	eso_fwrite("67890", 5, fdup);

	int r0 = eso_fclose(fdup);
//	int r1 = eso_fclose(f);

	printf("r0=%d, r1=%d\n", r0, r1);
}

static void test_file2(void)
{
	es_file_t *pf;
	char buffer[512] = {0};
	const char *filename = "a.bin";

	pf = eso_fopen(filename, "wb");
	if (pf) {
		eso_fwrite("012345678901234567", 10, pf);
		eso_fflush(pf);
		eso_ftruncate(pf, 4096);
		eso_fclose(pf);
	}
	
	pf = eso_fopen(filename, "rb");
	if (pf) {
		eso_fread(buffer, sizeof(buffer), pf);
		eso_log("buffer=[%s]\n", buffer);

		eso_fclose(pf);
	}

	pf = eso_fopen(filename, "r+b");
	if (pf) {
		eso_fseek(pf, 8, ES_SEEK_SET);
		eso_ftruncate(pf, 5);
		int n = eso_ftell(pf);
 		eso_log("n=%d\n", n);

		eso_fclose(pf);
	}

	pf = eso_fopen(filename, "rb");
	if (pf) {
		eso_memset(buffer, 0, sizeof(buffer));
		eso_fread(buffer, sizeof(buffer), pf);
		eso_log("buffer=[%s]\n", buffer);
		
		eso_log("fsize=%ld\n", eso_fsize(pf));

		eso_fclose(pf);
	}

	eso_log("fexist=%d, fsize=%ld\n", eso_fexist(filename), eso_fsize2(filename));

	eso_log("fexist=%d\n", eso_fexist("xbxb"));

	//eso_unlink(filename);

	//eso_frename(filename, "../b.bin", FALSE);

	//eso_fcopy(filename, "../b.bin", FALSE);

	//eso_mkdir("./a/b/c");

}

static void test_lock(void) {
	es_thread_mutex_t* lock = eso_thread_mutex_create(0);

	eso_thread_mutex_lock(lock);

	printf("wait for interrupt...\n");
	es_status_t stat = eso_thread_mutex_lock(lock);
	printf("stat=%d\n", stat);

	eso_thread_mutex_destroy(&lock);
}

static void test_util(void) {
	char* hexstr = eso_new_bytes2hexstr((es_uint8_t*)"3413", 4);
	printf("hexstr=%s\n", hexstr);
	eso_free(hexstr);

	char* bytes = (char*)eso_new_hexstr2bytes(hexstr);
	printf("bytes=%s\n", bytes);
	eso_free(bytes);

	//===

	int n = 123;
#if (SIZEOF_LONG > 4)
	long m = 1234567890333;
#else
	long m = 1234567890;
#endif
	es_byte_t b[30];
	
	eso_log("sizeof long=%d\n", sizeof(long));
	
	int l = (int)eso_llong2array(n, b, sizeof(b));
	int x = (int)eso_array2llong(b, l);
	eso_log("l=%d, x=%d\n", l, x);

	l = eso_llong2array(m, b, sizeof(b));
	long y = eso_array2llong(b, l);
	eso_log("l=%d, x=%ld\n", l, y);
}

static void test_time(void) {
	es_int64_t t = eso_dt_nano();

	printf("nano time1=%lld\n", t);

	t = eso_dt_nano();
	printf("nano time2=%lld\n", t);

	t = eso_dt_nano();
	printf("nano time3=%lld\n", t);

	t = eso_dt_nano();
	printf("nano time4=%lld\n", t);

	t = eso_dt_nano();
	es_int64_t t2 = eso_dt_nano();
	printf("diff nano time=%lld\n", t2-t);

	//============================

	es_tm_t now;
	eso_dt_now(&now);
	printf("now->zone=%d\n", now.tm_zone);

}

static void test_signal(void)
{
	printf("signal 22 [%s]\n", eso_signal_description(22));
}

static void test_sys(void)
{
	es_string_t *trace = NULL;
	eso_backtrace_get(&trace);
	printf("trace=[%s]\n", trace);
	eso_mfree(trace);
	
	eso_setup_sigsegv(NULL);
	//eso_setup_sigsegv("dump.txt");
	char *p = NULL;
	strcpy(p, "");

	es_int64_t n = LLONG(2222222222222222);
	printf("n=%I64d\n", n);
}

static void test_dso(void)
{
	es_dso_t *handle = NULL;
	handle = eso_dso_load("E:\\WORK\\MEC\\libc\\test\\Debug\\uninst.dll");
	void *pfunc = eso_dso_sym(handle, "UninstInitialize");
	pfunc = eso_dso_sym(handle, "UninstInitializexx");
	eso_dso_unload(&handle);
}

static void test_mmap(void)
{
	es_mmap_t *mmap = eso_mmap_create(".\\mmap.dat", ES_MMAP_WRITE, 0, 11);
//	es_mmap_t *mmap = eso_mmap_create(NULL, ES_MMAP_WRITE, 0, 100);
	void *paddr = eso_mmap_offset(mmap, 0);
	memcpy(paddr, "gggggggggg", 11);
	eso_mmap_flush(mmap, ES_MMAP_SYNC);
//	printf("paddr=%s\n", paddr);
	eso_mmap_delete(&mmap);
}

static void test_shm(void)
{
	es_shm_t *shm, *shm2=NULL;
	shm = eso_shm_create(100, ".\\shm.dat");

	void *base = eso_shm_baseaddr_get(shm);
	if (base) {
		eso_memcpy(base, "1234567890", 10);
	}

	eso_shm_attach(&shm2, ".\\shm.dat");
	if (shm2) {
		base = eso_shm_baseaddr_get(shm2);
		if (base) {
			eso_memcpy(base, "rrrrr", 5);
		}
		eso_shm_detach(&shm2);
	}

	eso_shm_destroy(&shm);
}

static void *thread_func_pipe_test(es_thread_t* handle)
{
	es_pipe_t *pipe = (es_pipe_t *)handle->data;

	for (int i=0; i<2; i++) {
		eso_fwrite("123\0x1234", 8, pipe->out);
		eso_thread_sleep(1000);
	}
	eso_fclose(pipe->out);
	pipe->out = NULL;

	return NULL;
}
static void test_pipe()
{
	char buffer[10];

	es_pipe_t *pipe = eso_pipe_create();

	es_status_t retval;
	es_thread_t* thread = eso_thread_create(NULL, thread_func_pipe_test, pipe);

	for (int i=0; i<10; i++) {
		int n = eso_fread(buffer, sizeof(buffer)-1, pipe->in);
		buffer[n] = 0;
		printf("n=%d, buffer=[%s], eof=%d\n", n, buffer, eso_feof(pipe->in));
	}

	eso_thread_join(&retval, thread);
	eso_thread_destroy(&thread);

	eso_pipe_destroy(&pipe);
}

static void test_proc()
{
	es_proc_t proc;
	es_proc_mutex_t *mutex = eso_proc_mutex_create();
	es_pipe_t *pipe = eso_pipe_create();
	char buffer[33];

	ES_PROC_INIT(&proc);

	es_status_t st = eso_proc_fork(&proc);
	if (st == ES_INCHILD) {
		printf("in child 1\n");

		eso_thread_sleep(1000);
		eso_fwrite("OK", 2, pipe->out);

		int readn;
		es_file_t *pf = eso_fopen("D:\\Downloads\\aa.ppt", "rb");
		while ((readn = (int)eso_fread(buffer, sizeof(buffer), pf))) {
			eso_fwriten(buffer, readn, pipe->out);
		}
		eso_fclose(pf);

		for (int i=0; i<9999; i++) {
			eso_proc_mutex_lock(mutex);
			printf("in child n=%d\n", i);
			eso_proc_mutex_unlock(mutex);
		}

		printf("in child 2\n");

		exit(0);
	}
	else if (st == ES_INPARENT) {
		printf("in parent 1\n");

		eso_fclose(pipe->out);
		pipe->out = NULL;

		int n = (int)eso_fread(buffer, 2, pipe->in);
		buffer[n] = 0;
		if (eso_strcmp(buffer, "OK") == 0) {
			printf("child is ready!\n");

			int writen;
			es_file_t *pf = eso_fopen("D:\\Downloads\\bb.ppt", "wb");
			while ((writen = (int)eso_fread(buffer, sizeof(buffer), pipe->in))) {
				eso_fwrite(buffer, writen, pf);
			}
			eso_fclose(pf);
		}

		for (int i = 0; i < 9999; i++) {
			eso_proc_mutex_lock(mutex);
			printf("in parent n=%d\n", i);
			eso_proc_mutex_unlock(mutex);
		}
		eso_thread_sleep(3000);
		eso_proc_kill(&proc, SIGTERM);
		eso_proc_wait(&proc, NULL, NULL, ES_WAIT);

		printf("in parent 2\n");

		eso_proc_mutex_destroy(&mutex);
	}
	else {
		//error.
	}
	printf("xxxxxxxxxxx\n");
}

static void test_proc2()
{
	es_proc_t proc;
	const char *args[2];

	ES_PROC_INIT(&proc);

	args[0] = "param";
	args[1] = NULL;

	es_status_t rv = eso_proc_create(&proc, "progname", NULL, NULL, NULL, TRUE);
	eso_thread_sleep(10000);
	eso_proc_kill(&proc, SIGTERM);
	eso_proc_wait(&proc, NULL, NULL, ES_WAIT);
}

static void* thread_func(es_thread_t* t) {
	printf("handle->data = %ld\n", (long)t->data);

//	eso_thread_detach(t);

	eso_thread_sleep(1000);

	printf("thread_func...%ld\n", eso_os_thread_current_id());
	printf("thread_func...%ld\n", eso_os_thread_current_id());
	printf("thread_func...%ld\n", eso_os_thread_current_id());

//	eso_thread_destroy(&t); //error!

	return NULL;
}
static void test_thread(void) {
#if 0
	es_thread_t* t = eso_thread_create(NULL, thread_func, NULL);
	eso_thread_join(NULL, t);
	eso_thread_destroy(&t);
#endif

	es_threadattr_t attr = {0};
	es_status_t st;
	es_status_t retval;

	printf("&attr = %ld\n", (long)&attr);
	st = eso_threadattr_init(&attr);
	printf("st1 = %d\n", st);
	st = eso_threadattr_detach_set(&attr, FALSE);
	printf("st2 = %d\n", st);
	printf("detach = %d\n", eso_threadattr_detach_get(&attr));
	st = eso_threadattr_stacksize_set(&attr, 4096);
	printf("st3 = %d\n", st);

	es_thread_t* thread = eso_thread_create(&attr, thread_func, &attr);
	st = eso_thread_join(&retval, thread);
	printf("st4 = %d\n", st);
	eso_thread_destroy(&thread);
}

static void test_alogger(void) {
	es_alogger_t* logger = eso_alogger_create("alog.txt", 1024000);

	for (int i=0; i<10000; i++) {
		eso_alogger_logfmt(logger, "i = %d", i);
	}

	eso_thread_sleep(1000);

	eso_alogger_free(&logger);
}

static void test_exception(void) {
	try_begin
		try_begin
			printf("block one\n");
			throw(-1, 1001, "1001 error.");
		catch_one(-1)
			printf("catched error: err_type=%d, err_code=%d, err_msg=%s\n",
				  eso_get_except_errtype(), eso_get_except_errcode(), eso_get_except_errmsg());
			rethrow;
		try_end;
	catch_all()
		printf("catched -1 error\n");
	try_end;
}

static void test_md4(void) {
	MD4_CTX ctx;
	MD4_Init(&ctx);
	MD4_Update(&ctx, (const es_uint8_t*)"abcdefg", 7);
	es_uint8_t hash[MD4_DIGEST_LENGTH] = {0};
	MD4_Final(hash, &ctx);
	char* s = eso_new_bytes2hexstr(hash, MD4_DIGEST_LENGTH);
	printf("hash=%s\n", s);
	eso_free(s);
}

static void test_pcre(void)
{
	es_pcre_t *pcre;
	int v;
	int ovector[15];
	
	//pcre = eso_pcre_make("(\\w+)  ***&&&:(\\d+)", 0, &v);
	pcre = eso_pcre_make("(\\w+):(\\d+)", 0, &v);
	if (!pcre) {
		printf("pcre make off=%d\n", v);
		return;
	}
	v = eso_pcre_exec(pcre, "123 hello:1234", 14, 0, 0, ovector, 15);
	printf("pcre exec v=%d\n", v);
	if (v >= 0) {
		printf("matched start pos=%d\n", ovector[0]);
	}
	eso_pcre_free(&pcre);
}

static void test_net(void)
{
	int fd = eso_net_socket(PF_INET, SOCK_STREAM, 0);
	int ret = eso_net_bind(fd, "192.168.1.107", 8897);
	ret = eso_net_connect(fd, "115.239.210.27", 80, 1000);
	printf("connected socket fd=%d, ret=%d, errno=%d", fd, ret, errno);
	if (fd != -1) {
		eso_net_close(fd);
	}
}

//=============================================================================


typedef struct {
	es_istream_t s;
	es_file_t *file;
} CFileInStreamLZMA;

typedef struct {
	es_ostream_t s;
	es_file_t *file;
} CFileOutStreamLZMA;

static es_int32_t lzma_stream_read_file(void *p, void *buf, es_size_t *size)
{
	CFileInStreamLZMA *inStream = (CFileInStreamLZMA*)p;
	es_size_t ret = eso_fread(buf, *size, inStream->file);
	*size = ret;
	return ret >= 0 ? 0 : -1;
}

static es_size_t lzma_stream_write_file(void *p, const void *buf, es_size_t size)
{
	CFileOutStreamLZMA *outStream = (CFileOutStreamLZMA*)p;
// 	return eso_fwrite(buf, size, outStream->file);
	es_size_t n = eso_fwrite(buf, size, outStream->file);
	eso_fflush(outStream->file);
	return n;
}


typedef struct {
	es_istream_t s;
	es_byte_t *ptr;
	es_int32_t size;
	es_int32_t pos;
} CMemInStreamLZMA;

typedef struct {
	es_ostream_t s;
	es_byte_t *ptr;
	es_int32_t size;
	es_int32_t pos;
} CMemOutStreamLZMA;

static es_int32_t lzma_stream_read_mem(void *p, void *buf, es_size_t *size)
{
	CMemInStreamLZMA *inStream = (CMemInStreamLZMA*)p;
	
	if (inStream->pos > inStream->size) {
		return -1;
	}
	
	*size = ES_MIN(*size, (es_size_t)(inStream->size - inStream->pos));
	eso_memcpy(buf, inStream->ptr+inStream->pos, *size);
	inStream->pos += *size;
	
	return 0;
}

static es_size_t lzma_stream_write_mem(void *p, const void *buf, es_size_t size)
{
	CMemOutStreamLZMA *outStream = (CMemOutStreamLZMA*)p;
	
	//...
	
	return size;
}

void test_lzma_unzip(void)
{
#if 0
	CFileInStreamLZMA inStream;
	CFileOutStreamLZMA outStream;
	
	inStream.s.read = lzma_stream_read_file;
	outStream.s.write = lzma_stream_write_file;

	inStream.file = eso_fopen(".\\tst.7z", "rb");
	outStream.file = eso_fopen(".\\tst.7u", "wb");
	
	eso_log("eso_unzip begin....");
	eso_lzma_unzip((es_ostream_t*)&outStream, (es_istream_t*)&inStream);
	eso_log("eso_unzip end....");
	
	eso_fclose(outStream.file);
	eso_fclose(inStream.file);
#else
	es_file_t *pfI, *pfO;
	CMemInStreamLZMA inStream;
	CMemOutStreamLZMA outStream;
	
	eso_memset(&inStream, 0, sizeof(CMemInStreamLZMA));
	eso_memset(&outStream, 0, sizeof(CMemOutStreamLZMA));
	
	inStream.s.read = lzma_stream_read_mem;
	outStream.s.write = lzma_stream_write_mem;
	
	pfI = eso_fopen(".\\tst.7z", "rb");
	inStream.size = eso_fsize(pfI);
	inStream.ptr = (es_byte_t *)eso_malloc(inStream.size);
	eso_fread(inStream.ptr, inStream.size, pfI);
	eso_fclose(pfI);
	
	eso_lzma_unzip((es_ostream_t*)&outStream, (es_istream_t*)&inStream);
	
	pfO = eso_fopen(".\\tst.7u", "wb");
	eso_fwrite(outStream.ptr, outStream.size, pfO);
	eso_free(outStream.ptr);
	eso_fclose(pfO);
#endif
}

void test_lzma_zip(void)
{
	CFileInStreamLZMA inStream;
	CFileOutStreamLZMA outStream;
	
	inStream.s.read = lzma_stream_read_file;
	outStream.s.write = lzma_stream_write_file;
	
	inStream.file = eso_fopen(".\\tst.7u", "rb");
	outStream.file = eso_fopen(".\\tst.z7", "wb");
	
	eso_log("eso_unzip begin....");
	eso_lzma_zip((es_ostream_t*)&outStream, (es_istream_t*)&inStream, eso_fsize(inStream.file));
	eso_log("eso_unzip end....");
	
	eso_fclose(outStream.file);
	eso_fclose(inStream.file);
}

//=============================================================================

typedef struct {
	es_istream_t s;
	es_file_t *file;
} CFileInStreamLZ77;

typedef struct {
	es_ostream_t s;
	es_file_t *file;
} CFileOutStreamLZ77;

static es_int32_t lz77_stream_read_file(void *p, void *buf, es_size_t *size)
{
	CFileInStreamLZ77 *inStream = (CFileInStreamLZ77*)p;
	return (*size = eso_fread(buf, *size, inStream->file)) >= 0 ? 0 : -1;
}

static es_size_t lz77_stream_write_file(void *p, const void *buf, es_size_t size)
{
	CFileOutStreamLZ77 *outStream = (CFileOutStreamLZ77*)p;
	return eso_fwrite(buf, size, outStream->file);
}

typedef struct {
	es_istream_t s;
	es_byte_t *ptr;
	es_int32_t size;
	es_int32_t pos;
} CMemInStreamLZ77;

typedef struct {
	es_ostream_t s;
	es_byte_t *ptr;
	es_int32_t size;
	es_int32_t pos;
} CMemOutStreamLZ77;

static es_int32_t lz77_stream_read_mem(void *p, void *buf, es_size_t *size)
{
	CMemInStreamLZ77 *inStream = (CMemInStreamLZ77*)p;
	
	if (inStream->pos > inStream->size) {
		return -1;
	}
	
	*size = ES_MIN(*size, inStream->size - inStream->pos);
	memcpy(buf, inStream->ptr+inStream->pos, *size);
	inStream->pos += *size;
	
	return 0;
}

static es_size_t lz77_stream_write_mem(void *p, const void *buf, es_size_t size)
{
	CMemOutStreamLZ77 *outStream = (CMemOutStreamLZ77*)p;
	
	if (!outStream->ptr) {
		outStream->ptr = (es_byte_t *)eso_malloc(size);
		if (!outStream->ptr) {
			return -1;
		}
		outStream->size = size;
	}
	else {
		//realloc
		es_byte_t *p = outStream->ptr;
		outStream->ptr = (es_byte_t *)eso_malloc(outStream->size + size);
		memcpy(outStream->ptr, p, outStream->size);
		eso_free(p);
		outStream->size += size;
	}
	memcpy(outStream->ptr + outStream->pos, buf, size);
	outStream->pos += size;
	
	return size;
}

static void test_lz77_unzip(void)
{
#if 0
	CFileInStreamLZ77 inStream;
	CFileOutStreamLZ77 outStream;
	
	inStream.s.read = lz77_stream_read_file;
	outStream.s.write = lz77_stream_write_file;

	inStream.file = eso_fopen("e:\\bmw\\tst.z7", "rb");
	outStream.file = eso_fopen("e:\\bmw\\tst.u7", "wb");
	
	eso_lz77_unzip((es_ostream_t*)&outStream, (es_istream_t*)&inStream);
	
	eso_fclose(outStream.file);
	eso_fclose(inStream.file);
#else
	es_file_t *pfI, *pfO;
	CMemInStreamLZ77 inStream;
	CMemOutStreamLZ77 outStream;
	
	memset(&inStream, 0, sizeof(CMemInStreamLZ77));
	memset(&outStream, 0, sizeof(CMemOutStreamLZ77));
	
	inStream.s.read = lz77_stream_read_mem;
	outStream.s.write = lz77_stream_write_mem;
	
	pfI = eso_fopen("e:\\bmw\\tst.z7", "rb");
	inStream.size = eso_fsize(pfI);
	inStream.ptr = (es_byte_t *)eso_malloc(inStream.size);
	eso_fread(inStream.ptr, inStream.size, pfI);
	eso_fclose(pfI);
	
	eso_lz77_unzip((es_ostream_t*)&outStream, (es_istream_t*)&inStream);
	
	pfO = eso_fopen("e:\\bmw\\tst.u7", "wb");
	eso_fwrite(outStream.ptr, outStream.size, pfO);
	eso_free(outStream.ptr);
	eso_fclose(pfO);
#endif
}

static void test_lz77_zip(void)
{
#if 0
	CFileInStreamLZ77 inStream;
	CFileOutStreamLZ77 outStream;
	
	inStream.s.read = lz77_stream_read_file;
	outStream.s.write = lz77_stream_write_file;

	inStream.file = eso_fopen("e:\\bmw\\tst.u7", "rb");
	outStream.file = eso_fopen("e:\\bmw\\tst.z7", "wb");
	
	eso_lz77_zip((es_ostream_t*)&outStream, (es_istream_t*)&inStream);
	
	eso_fclose(outStream.file);
	eso_fclose(inStream.file);
#else
	es_file_t *pfI, *pfO;
	CMemInStreamLZ77 inStream;
	CMemOutStreamLZ77 outStream;
	
	memset(&inStream, 0, sizeof(CMemInStreamLZ77));
	memset(&outStream, 0, sizeof(CMemOutStreamLZ77));
	
	inStream.s.read = lz77_stream_read_mem;
	outStream.s.write = lz77_stream_write_mem;
	
	pfI = eso_fopen("e:\\bmw\\tst.u7", "rb");
	inStream.size = eso_fsize(pfI);
	inStream.ptr = (es_byte_t *)eso_malloc(inStream.size);
	eso_fread(inStream.ptr, inStream.size, pfI);
	eso_fclose(pfI);
	
	eso_lz77_zip((es_ostream_t*)&outStream, (es_istream_t*)&inStream);
	
	pfO = eso_fopen("e:\\bmw\\tst.z7", "wb");
	eso_fwrite(outStream.ptr, outStream.size, pfO);
	eso_free(outStream.ptr);
	eso_fclose(pfO);
#endif
}

//=============================================================================

typedef struct {
	es_istream_t s;
	es_file_t *file;
} CFileInStreamZLIB;

typedef struct {
	es_ostream_t s;
	es_file_t *file;
} CFileOutStreamZLIB;

static es_int32_t zlib_stream_read_file(void *p, void *buf, es_size_t *size)
{
	CFileInStreamZLIB *inStream = (CFileInStreamZLIB*)p;
	return (*size = eso_fread(buf, *size, inStream->file)) >= 0 ? 0 : -1;
}

static es_size_t zlib_stream_write_file(void *p, const void *buf, es_size_t size)
{
	CFileOutStreamZLIB *outStream = (CFileOutStreamZLIB*)p;
	return eso_fwrite(buf, size, outStream->file);
}


typedef struct {
	es_istream_t s;
	es_byte_t *ptr;
	es_int32_t size;
	es_int32_t pos;
} CMemInStreamZLIB;

typedef struct {
	es_ostream_t s;
	es_byte_t *ptr;
	es_int32_t size;
	es_int32_t pos;
} CMemOutStreamZLIB;

static es_int32_t zlib_stream_read_mem(void *p, void *buf, es_size_t *size)
{
	CMemInStreamZLIB *inStream = (CMemInStreamZLIB*)p;
	
	if (inStream->pos > inStream->size) {
		return -1;
	}
	
	*size = ES_MIN(*size, inStream->size - inStream->pos);
	memcpy(buf, inStream->ptr+inStream->pos, *size);
	inStream->pos += *size;
	
	return 0;
}

static es_size_t zlib_stream_write_mem(void *p, const void *buf, es_size_t size)
{
	CMemOutStreamZLIB *outStream = (CMemOutStreamZLIB*)p;
	
	if (!outStream->ptr) {
		outStream->ptr = (es_byte_t *)eso_malloc(size);
		if (!outStream->ptr) {
			return -1;
		}
		outStream->size = size;
	}
	else {
		//realloc
		es_byte_t *p = outStream->ptr;
		outStream->ptr = (es_byte_t *)eso_malloc(outStream->size + size);
		memcpy(outStream->ptr, p, outStream->size);
		eso_free(p);
		outStream->size += size;
	}
	memcpy(outStream->ptr + outStream->pos, buf, size);
	outStream->pos += size;
	
	return size;
}

static void test_zlib_unzip(void)
{
#if 0
	CFileInStreamZLIB inStream;
	CFileOutStreamZLIB outStream;
	
	inStream.s.read = zlib_stream_read_file;
	outStream.s.write = zlib_stream_write_file;

	inStream.file = eso_fopen("d:\\downloads\\tst.z", "rb");
	outStream.file = eso_fopen("d:\\downloads\\tst.u", "wb");
	
	eso_zlib_inflate((es_ostream_t*)&outStream, (es_istream_t*)&inStream);
	
	eso_fclose(outStream.file);
	eso_fclose(inStream.file);
#else
	es_file_t *pfI, *pfO;
	CMemInStreamZLIB inStream;
	CMemOutStreamZLIB outStream;
	
	memset(&inStream, 0, sizeof(CMemInStreamZLIB));
	memset(&outStream, 0, sizeof(CMemOutStreamZLIB));
	
	inStream.s.read = zlib_stream_read_mem;
	outStream.s.write = zlib_stream_write_mem;
	
	pfI = eso_fopen("d:\\downloads\\tst.z", "rb");
	inStream.size = eso_fsize(pfI);
	inStream.ptr = (es_byte_t *)eso_malloc(inStream.size);
	eso_fread(inStream.ptr, inStream.size, pfI);
	eso_fclose(pfI);
	
	eso_zlib_inflate((es_ostream_t*)&outStream, (es_istream_t*)&inStream);
	
	pfO = eso_fopen("d:\\downloads\\tst.u", "wb");
	eso_fwrite(outStream.ptr, outStream.size, pfO);
	eso_free(outStream.ptr);
	eso_fclose(pfO);
#endif
}

static void test_zlib_zip(void)
{
#if 0
	CFileInStreamZLIB inStream;
	CFileOutStreamZLIB outStream;
	
	inStream.s.read = zlib_stream_read_file;
	outStream.s.write = zlib_stream_write_file;

	inStream.file = eso_fopen("d:\\downloads\\tst.u", "rb");
	outStream.file = eso_fopen("d:\\downloads\\tst.z", "wb");
	
	eso_zlib_deflate((es_ostream_t*)&outStream, (es_istream_t*)&inStream);
	
	eso_fclose(outStream.file);
	eso_fclose(inStream.file);
#else
	es_file_t *pfI, *pfO;
	CMemInStreamZLIB inStream;
	CMemOutStreamZLIB outStream;
	
	memset(&inStream, 0, sizeof(CMemInStreamZLIB));
	memset(&outStream, 0, sizeof(CMemOutStreamZLIB));
	
	inStream.s.read = zlib_stream_read_mem;
	outStream.s.write = zlib_stream_write_mem;
	
	pfI = eso_fopen("d:\\downloads\\tst.u", "rb");
	inStream.size = eso_fsize(pfI);
	inStream.ptr = (es_byte_t *)eso_malloc(inStream.size);
	eso_fread(inStream.ptr, inStream.size, pfI);
	eso_fclose(pfI);
	
	eso_zlib_deflate((es_ostream_t*)&outStream, (es_istream_t*)&inStream);
	
	pfO = eso_fopen("d:\\downloads\\tst.z", "wb");
	eso_fwrite(outStream.ptr, outStream.size, pfO);
	eso_free(outStream.ptr);
	eso_fclose(pfO);
#endif
}

//=============================================================================

typedef struct {
	es_bson_stream_t s;
	es_file_t *file;
} CFileInStreamBSON;

static es_int32_t bson_stream_read_file(void *s, void *buf, es_size_t *size)
{
	CFileInStreamBSON *inStream = (CFileInStreamBSON*)s;
	es_size_t ret = eso_fread(buf, *size, inStream->file);
	*size = ret;
	return ret > 0 ?  ES_SUCCESS : BSON_EHEAD;
}

static void bson_stream_parsed_node(void *p, es_bson_t *bson, es_bson_node_t *node)
{
	printf("name=[%s]\n", node->name);
}

static void test_bson(void)
{
	int stat;
	es_bson_t *bson;
	CFileInStreamBSON inStream;
	
	bson = eso_bson_create("UTF-8");
	
#if 1
	inStream.s.read = bson_stream_read_file;
	inStream.s.parsed = bson_stream_parsed_node;
	inStream.file = eso_fopen("bson.dat", "rb");
	if (!inStream.file) {
		return;
	}
	stat = eso_bson_parse(bson, (es_bson_stream_t*)&inStream);
	eso_fclose(inStream.file);
	
 	es_bson_node_t * node = eso_bson_add_str(bson, "/node/xxxx", "XXXXXXXXXXXXX");
 	es_bson_node_t * attr_node1 = eso_bson_attr_add_str(node, "attr1", "attr 1");
 	es_bson_node_t * attr_node2 = eso_bson_attr_add_str(node, "attr2", "attr 2");
 	eso_bson_add_str(bson, "/node/yyyy", "YYYYYYYYYYYY");
 	eso_bson_add_fmt(bson, "/node/xxxx", "==%s", "%%%%%%%%%%%%%%%%%%%");
 	eso_bson_add_str(bson, "/node/xxxx|1", "11111111111111111111");
 	eso_bson_add_str(bson, "/root", "root");
#else
	//load from bson file
 	stat = eso_bson_load(bson, "bson.dat");
 	if (stat != ES_SUCCESS) {
 		printf("bson[%s] --> xml[%s] fail(%d).\n", argv[1], argv[2], stat);
 		eso_bson_destroy(&bson);
 		return stat;
 	}
#endif

	eso_bson_save(bson, "s0.dat", NULL);
	eso_bson_save(bson, "s1.dat", "/node");
	eso_bson_save(bson, "s2.dat", "/node/so");
	eso_bson_save(bson, "s3.dat", "/node/t");

	eso_bson_destroy(&bson);
}

//=============================================================================

MAIN_IMPL(testlibc) {
	eso_initialize();

	do {
//		test_mstr();
//		test_mpool();
//		test_bitset();
//		test_conf();
//		test_json();
//		test_uuid();
//		test_crc32();
		test_datetime();
//		test_atomic();
//		test_file();
//		test_file2();
//		test_lock();
//		test_util();
//		test_time();
//		test_signal();
//		test_sys();
//		test_dso();
//		test_mmap();
//		test_shm();
//		test_pipe();
//		test_proc();
//		test_proc2();
//		test_thread();
//		test_alogger();
//		test_exception();
//		test_md4();
//		test_pcre();
//		test_net();
//		test_lzma_zip();
//		test_lzma_unzip();
//		test_lz77_zip();
//		test_lz77_unzip();
//		test_zlib_zip();
//		test_zlib_unzip();
//		test_bson();
	} while(0);

	eso_terminate();

	return 0;
}
