/*
 * main.hh
 *
 *  Created on: 2013-8-16
 *      Author: cxxjava@163.com
 */

#ifndef MAIN_H_
#define MAIN_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifdef __MAIN__
#define MAIN_IMPL(name) int main(int argc, const char **argv)
#else //!
#define MAIN_IMPL(name)                                                       \
	int run_main_##name(int argc, const char** argv);                         \
	int run_main_##name(int argc, const char** argv)

#define MAIN_CALL(name)                                                       \
	extern int run_main_##name(int argc, const char** argv);                  \
	run_main_##name(argc, argv)
#endif //!__MAIN__

#ifdef __cplusplus
}
#endif

#endif /* MAIN_H_ */
