#include "ESystem.hh"
#include "ERuntime.hh"
#include "EOS.hh"
#include "EMath.hh"
#include "EThread.hh"
#include "ETimer.hh"
#include "EFileOutputStream.hh"
#include "EFileInputStream.hh"
#include "ENullPointerException.hh"
#include "../inc/concurrent/EAtomic.hh"
#include "../inc/concurrent/EThreadPoolExecutor.hh"
#include "../inc/concurrent/EThreadLocalRandom.hh"

#ifdef WIN32
#include <windows.h>
#include <direct.h>
#include <locale.h>
#else
#include <sys/utsname.h>
#include <unistd.h>
#include <pwd.h>
#include <locale.h>
#include <langinfo.h>
#endif

namespace efc {

EInputStream* ESystem::in;
EPrintStream* ESystem::out;
EPrintStream* ESystem::err;
EInputStream* ESystem::m_stdin;
EFileOutputStream* ESystem::m_stdout0;
EPrintStream* ESystem::m_stdout;
EFileOutputStream* ESystem::m_stderr0;
EPrintStream* ESystem::m_stderr;
EString* ESystem::m_FileCfg;
EConfig* ESystem::m_Cfg;
ESimpleMap* ESystem::m_Arg;
ESimpleMap* ESystem::m_Prop;
es_string_t* ESystem::m_CurrPath;
es_string_t* ESystem::m_ExecPath;
es_string_t* ESystem::m_TempPath;

DEFINE_STATIC_INITZZ_BEGIN(ESystem)
EOS::_initzz_();
EMath::_initzz_();
EAtomic::_initzz_();
ERuntime::_initzz_();
EThreadLocalRandom::_initzz_();
EThread::_initzz_();
ETimer::_initzz_();
EThreadPoolExecutor::_initzz_();
ESystem::initSysProp();
in = ESystem::getInput();
out = ESystem::getOutput();
err = ESystem::getErrput();
DEFINE_STATIC_INITZZ_END

//=============================================================================

int ESystem::localTimeZone()
{
	es_tm_t tm;
	eso_dt_now(&tm);
	return tm.tm_zone;
}

llong ESystem::currentTimeMillis() {
	es_tm_t tm;
	eso_dt_now(&tm);
	return (llong) (tm.tm_time_sec * (llong)1000 + tm.tm_time_usec / (llong)1000);
}

uint ESystem::currentTimeSeconds()
{
	es_tm_t tm;
	eso_dt_now(&tm);
	return tm.tm_time_sec;
}

llong ESystem::nanoTime() {
	return eso_dt_nano();
}

es_status_t ESystem::init(char *config)
{
	return ESystem::init(0, null, config);
}

static void parseArgs(ESimpleMap *map, int argc, char **argv)
{
	if (argc < 2) return; //first 1 is exec name

	for (int i=1; i < argc;) {
		if (*argv[i] == '-') {
			char *key = argv[i]+1;
			char *val = NULL;
			i++;
			if (i < argc) {
				val = argv[i];
				i++;
			}
			map->put(key, new EString(val));
		}
		else {
			char *key = argv[i];
			map->put(key, NULL);
			i++;
		}
	}
}

#ifdef WIN32

//@see: openjdk-7/jdk/src/windows/native/java/lang/java_props_md.c

#define PROPSIZE 9      // eight-letter + null terminator
#define SNAMESIZE 86    // max number of chars for LOCALE_SNAME is 85

static char *
getEncodingInternal(LCID lcid)
{
    char * ret = (char*)malloc(16);
    int codepage;

    if (GetLocaleInfo(lcid,
                      LOCALE_IDEFAULTANSICODEPAGE,
                      ret+2, 14) == 0) {
        codepage = 1252;
    } else {
        codepage = eso_atol(ret+2);
    }

    switch (codepage) {
    case 0:
    	eso_strcpy(ret, "UTF-8");
        break;
    case 874:     /*  9:Thai     */
    case 932:     /* 10:Japanese */
    case 949:     /* 12:Korean Extended Wansung */
    case 950:     /* 13:Chinese (Taiwan, Hongkong, Macau) */
    case 1361:    /* 15:Korean Johab */
        ret[0] = 'M';
        ret[1] = 'S';
        break;
    case 936:
    	eso_strcpy(ret, "GBK");
        break;
    case 54936:
    	eso_strcpy(ret, "GB18030");
        break;
    default:
        ret[0] = 'C';
        ret[1] = 'p';
        break;
    }

    //Traditional Chinese Windows should use MS950_HKSCS_XP as the
    //default encoding, if HKSCS patch has been installed.
    // "old" MS950 0xfa41 -> u+e001
    // "new" MS950 0xfa41 -> u+92db
    if (eso_strcmp(ret, "MS950") == 0) {
        TCHAR  mbChar[2] = {(char)0xfa, (char)0x41};
        WCHAR  unicodeChar;
        MultiByteToWideChar(CP_ACP, 0, mbChar, 2, &unicodeChar, 1);
        if (unicodeChar == 0x92db) {
        	eso_strcpy(ret, "MS950_HKSCS_XP");
        }
    } else {
        //SimpChinese Windows should use GB18030 as the default
        //encoding, if gb18030 patch has been installed (on windows
        //2000/XP, (1)Codepage 54936 will be available
        //(2)simsun18030.ttc will exist under system fonts dir )
        if (eso_strcmp(ret, "GBK") == 0 && IsValidCodePage(54936)) {
            char systemPath[MAX_PATH + 1];
            char* gb18030Font = "\\FONTS\\SimSun18030.ttc";
            FILE *f = NULL;
            if (GetWindowsDirectory(systemPath, MAX_PATH + 1) != 0 &&
            		eso_strlen(systemPath) + eso_strlen(gb18030Font) < MAX_PATH + 1) {
            	eso_strcat(systemPath, "\\FONTS\\SimSun18030.ttc");
                if ((f = eso_fopen(systemPath, "r")) != NULL) {
                	eso_fclose(f);
                	eso_strcpy(ret, "GB18030");
                }
            }
        }
    }

    return ret;
}

static void
SetupI18nProps(LCID lcid, char** language, char** script, char** country,
               char** variant, char** encoding) {
    /* script */
    char tmp[SNAMESIZE];

    if (script != NULL && *script == NULL) {
		char* _script = (char*)eso_malloc(PROPSIZE);
		if (GetLocaleInfo(lcid,
						  LOCALE_SNAME, tmp, SNAMESIZE) == 0 ||
				eso_sscanf(tmp, "%*[a-z\\-]%1[A-Z]%[a-z]", _script, &(_script[1])) == 0 ||
				eso_strlen(_script) != 4) {
			_script[0] = '\0';
		}
		if (*_script) {
			eso_mstrcpy(script, _script);
		}
		eso_free(_script);
    }

    /* country */
    if (country != NULL && *country == NULL) {
    	char* _country = (char*)eso_malloc(PROPSIZE);
		if (GetLocaleInfo(lcid,
						  LOCALE_SISO3166CTRYNAME, _country, PROPSIZE) == 0 &&
			GetLocaleInfo(lcid,
						  LOCALE_SISO3166CTRYNAME2, _country, PROPSIZE) == 0) {
			_country[0] = '\0';
		}
		if (*_country) {
			eso_mstrcpy(country, _country);
		}
		eso_free(_country);
    }

    /* language */
    if (language != NULL && *language == NULL) {
    	char* _language = (char*)eso_malloc(PROPSIZE);
    	if (GetLocaleInfo(lcid,
                      LOCALE_SISO639LANGNAME, _language, PROPSIZE) == 0 &&
    		GetLocaleInfo(lcid,
                      LOCALE_SISO639LANGNAME2, _language, PROPSIZE) == 0) {
            /* defaults to en_US */
            eso_mstrcpy(language, "en");
            eso_mstrcpy(country, "US");
        }
    	if (*_language) {
			eso_mstrcpy(language, _language);
		}
		eso_free(_language);
    }

    /* variant */
    //=0

    /* handling for Norwegian */
    //...

    /* encoding */
    if (encoding != NULL && *encoding == NULL) {
    	char* _encoding = getEncodingInternal(lcid);
    	if (*_encoding) {
			eso_mstrcpy(encoding, _encoding);
		}
		eso_free(_encoding);
    }
}

static void parseLocale(OSVERSIONINFOEXA & ver, es_string_t **language, es_string_t **country,
		es_string_t **variant, es_string_t **script, es_string_t **encoding)
{
	/*
	 * query the system for the current system default locale
	 * (which is a Windows LCID value),
	 */
	LCID userDefaultLCID = GetUserDefaultLCID();
	LCID systemDefaultLCID = GetSystemDefaultLCID();
	LCID userDefaultUILang = GetUserDefaultUILanguage();

	{
		// Windows UI Language selection list only cares "language"
		// information of the UI Language. For example, the list
		// just lists "English" but it actually means "en_US", and
		// the user cannot select "en_GB" (if exists) in the list.
		// So, this hack is to use the user LCID region information
		// for the UI Language, if the "language" portion of those
		// two locales are the same.
		if (PRIMARYLANGID(LANGIDFROMLCID(userDefaultLCID)) ==
			PRIMARYLANGID(LANGIDFROMLCID(userDefaultUILang))) {
			userDefaultUILang = userDefaultLCID;
		}

		SetupI18nProps(userDefaultUILang,
					   language,
					   script,
					   country,
					   variant,
					   encoding);
		SetupI18nProps(userDefaultLCID,
					   language,
					   script,
					   country,
					   variant,
					   encoding);
		SetupI18nProps(userDefaultUILang,
					   language,
					   script,
					   country,
					   variant,
					   encoding);

		if (LANGIDFROMLCID(userDefaultLCID) == 0x0c04 && ver.dwMajorVersion == 6) {
			// MS claims "Vista has built-in support for HKSCS-2004.
			// All of the HKSCS-2004 characters have Unicode 4.1.
			// PUA code point assignments". But what it really means
			// is that the HKSCS-2004 is ONLY supported in Unicode.
			// Test indicates the MS950 in its zh_HK locale is a
			// "regular" MS950 which does not handle HKSCS-2004 at
			// all. Set encoding to MS950_HKSCS.
			eso_mstrcpy(encoding, "MS950_HKSCS");
		}
	}
}

#else //!

//@see: openjdk-7/jdk/src/solaris/native/java/lang/java_props_md.c

/* Take an array of string pairs (map of key->value) and a string (key).
 * Examine each pair in the map to see if the first string (key) matches the
 * string.  If so, store the second string of the pair (value) in the value and
 * return 1.  Otherwise do nothing and return 0.  The end of the map is
 * indicated by an empty string at the start of a pair (key of "").
 */
static int
mapLookup(const char* map[], const char* key, es_string_t** value) {
    int i;
    for (i = 0; eso_strcmp(map[i], ""); i += 2){
        if (!eso_strcmp(key, map[i])){
            eso_mstrcpy(value, map[i + 1]);
            return 1;
        }
    }
    return 0;
}

static int ParseLocale(int cat, es_string_t ** std_language, es_string_t ** std_script,
		es_string_t ** std_country, es_string_t ** std_variant, es_string_t ** std_encoding) {
    char temp[64];
    char *language = NULL, *country = NULL, *variant = NULL,
         *encoding = NULL;
    char *p, encoding_variant[64];
    char *lc;

    /* Query the locale set for the category */
    lc = setlocale(cat, NULL);
    if (lc == NULL || !eso_strcmp(lc, "C") || !eso_strcmp(lc, "POSIX")) {
        lc = "en_US";
    }

    /*
     * locale string format in Solaris is
     * <language name>_<country name>.<encoding name>@<variant name>
     * <country name>, <encoding name>, and <variant name> are optional.
     */

    eso_strcpy(temp, lc);

    /* Parse the language, country, encoding, and variant from the
     * locale.  Any of the elements may be missing, but they must occur
     * in the order language_country.encoding@variant, and must be
     * preceded by their delimiter (except for language).
     *
     * If the locale name (without .encoding@variant, if any) matches
     * any of the names in the locale_aliases list, map it to the
     * corresponding full locale name.  Most of the entries in the
     * locale_aliases list are locales that include a language name but
     * no country name, and this facility is used to map each language
     * to a default country if that's possible.  It's also used to map
     * the Solaris locale aliases to their proper Java locale IDs.
     */
    if ((p = eso_strchr(temp, '.')) != NULL) {
    	eso_strcpy(encoding_variant, p); /* Copy the leading '.' */
        *p = '\0';
    } else if ((p = eso_strchr(temp, '@')) != NULL) {
    	eso_strcpy(encoding_variant, p); /* Copy the leading '@' */
        *p = '\0';
    } else {
        *encoding_variant = '\0';
    }

    if (mapLookup(eso_locale_aliases, temp, &p)) {
        eso_strcpy(temp, p);
        // check the "encoding_variant" again, if any.
        if ((p = eso_strchr(temp, '.')) != NULL) {
        	eso_strcpy(encoding_variant, p); /* Copy the leading '.' */
            *p = '\0';
        } else if ((p = eso_strchr(temp, '@')) != NULL) {
        	eso_strcpy(encoding_variant, p); /* Copy the leading '@' */
            *p = '\0';
        }
    }

    language = temp;
    if ((country = eso_strchr(temp, '_')) != NULL) {
        *country++ = '\0';
    }

    p = encoding_variant;
    if ((encoding = eso_strchr(p, '.')) != NULL) {
        p[encoding++ - p] = '\0';
        p = encoding;
    }
    if ((variant = eso_strchr(p, '@')) != NULL) {
        p[variant++ - p] = '\0';
    }

    /* Normalize the language name */
    if (std_language != NULL && *std_language == NULL) {
    	eso_mstrcpy(std_language, "en");
        if (language != NULL && mapLookup(eso_language_names, language, std_language) == 0) {
            eso_mstrcpy(std_language, language);
        }
    }

    /* Normalize the country name */
    if (std_country != NULL && country != NULL) {
        if (mapLookup(eso_country_names, country, std_country) == 0) {
            eso_mstrcpy(std_country, country);
        }
    }

    /* Normalize the script and variant name.  Note that we only use
     * variants listed in the mapping array; others are ignored.
     */
    if (variant != NULL) {
        if (std_script != NULL && *std_script == NULL) {
            mapLookup(eso_script_names, variant, std_script);
        }

        if (std_variant != NULL && *std_variant == NULL) {
            mapLookup(eso_variant_names, variant, std_variant);
        }
    }

    /* Normalize the encoding name.  Note that we IGNORE the string
     * 'encoding' extracted from the locale name above.  Instead, we use the
     * more reliable method of calling nl_langinfo(CODESET).  This function
     * returns an empty string if no encoding is set for the given locale
     * (e.g., the C or POSIX locales); we use the default ISO 8859-1
     * converter for such locales.
     */
    if (std_encoding != NULL && *std_encoding == NULL) {
        /* OK, not so reliable - nl_langinfo() gives wrong answers on
         * Euro locales, in particular. */
        if (eso_strcmp(p, "ISO8859-15") == 0)
            p = "ISO8859-15";
        else
            p = nl_langinfo(CODESET);

        /* Convert the bare "646" used on Solaris to a proper IANA name */
        if (eso_strcmp(p, "646") == 0)
            p = "ISO646-US";

        /* return same result nl_langinfo would return for en_UK,
         * in order to use optimizations. */
        eso_mstrcpy(std_encoding, (char*)((*p != '\0') ? p : "ISO8859-1"));

#ifdef __linux__
        /*
         * Remap the encoding string to a different value for japanese
         * locales on linux so that customized converters are used instead
         * of the default converter for "EUC-JP". The customized converters
         * omit support for the JIS0212 encoding which is not supported by
         * the variant of "EUC-JP" encoding used on linux
         */
        if (eso_strcmp(p, "EUC-JP") == 0) {
        	eso_mstrcpy(std_encoding, "EUC-JP-LINUX");
        }
#else
        if (eso_strcmp(p,"eucJP") == 0) {
            /* For Solaris use customized vendor defined character
             * customized EUC-JP converter
             */
        	eso_mstrcpy(std_encoding, "eucJP-open");
        } else if (eso_strcmp(p, "Big5") == 0 || eso_strcmp(p, "BIG5") == 0) {
            /*
             * Remap the encoding string to Big5_Solaris which augments
             * the default converter for Solaris Big5 locales to include
             * seven additional ideographic characters beyond those included
             * in the Java "Big5" converter.
             */
        	eso_mstrcpy(std_encoding, "Big5_Solaris");
        } else if (eso_strcmp(p, "Big5-HKSCS") == 0) {
            /*
             * Solaris uses HKSCS2001
             */
        	eso_mstrcpy(std_encoding, "Big5-HKSCS-2001");
        }
#endif
    }

    return 1;
}

static void parseLocale(es_string_t **language, es_string_t **country,
		es_string_t **variant, es_string_t **script, es_string_t **encoding)
{
	setlocale(LC_ALL, "");

	if (ParseLocale(LC_CTYPE, language, script, country, variant, encoding)) {
		ParseLocale(LC_MESSAGES, language, script, country, variant, NULL);
	} else {
		eso_mstrcpy(language, "en");
		eso_mstrcpy(encoding, "ISO8859-1");
	}
}

#endif //!WIN32

static void initProps(ESimpleMap *map)
{
	/**
	 * os.name Operating system name
	 * os.arch Operating system architecture
	 * os.version Operating system version
	 * file.separator File separator ("/" on UNIX and "\" on Windows)
	 * path.separator Path separator (":" on UNIX and ";" on Windows)
	 * line.separator Line separator ("\n" on UNIX and "\r\n" on Windows)
	 * user.name User's account name
	 * user.home User's home directory
	 * user.dir User's current working directory
	 */
#ifdef WIN32
	typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);

//	//vc6
// 	typedef struct _OSVERSIONINFOEXA {
// 		DWORD dwOSVersionInfoSize;
// 		DWORD dwMajorVersion;
// 		DWORD dwMinorVersion;
// 		DWORD dwBuildNumber;
// 		DWORD dwPlatformId;
// 		TCHAR szCSDVersion[ 128 ];
// 		WORD wServicePackMajor;
// 		WORD wServicePackMinor;
// 		WORD wReserved[2];
// 	} OSVERSIONINFOEXA;
//	//vc7
// 	typedef struct _OSVERSIONINFOEXW {
// 		DWORD dwOSVersionInfoSize;
// 		DWORD dwMajorVersion;
// 		DWORD dwMinorVersion;
// 		DWORD dwBuildNumber;
// 		DWORD dwPlatformId;
// 		WCHAR  szCSDVersion[ 128 ];     // Maintenance string for PSS usage
// 		WORD   wServicePackMajor;
// 		WORD   wServicePackMinor;
// 		WORD   wSuiteMask;
// 		BYTE  wProductType;
// 		BYTE  wReserved;
// 	} OSVERSIONINFOEXW;
// 
// 	#define PROCESSOR_ARCHITECTURE_AMD64 9 //x64 (AMD or Intel)
// 	#define PROCESSOR_ARCHITECTURE_IA64  6 //Intel Itanium-based
// 	#define PROCESSOR_ARCHITECTURE_INTEL 0 //x86
// 
// 	#define VER_NT_WORKSTATION  0x0000001

	/* OS properties */
	//{
		char buf[100];
		SYSTEM_INFO si;
		PGNSI pGNSI;
		OSVERSIONINFOEXA ver;

		ver.dwOSVersionInfoSize = sizeof(ver);
		GetVersionEx((OSVERSIONINFO *) &ver);

		ZeroMemory(&si, sizeof(SYSTEM_INFO));
		// Call GetNativeSystemInfo if supported or GetSystemInfo otherwise.
		pGNSI = (PGNSI) GetProcAddress(
				GetModuleHandle(TEXT("kernel32.dll")),
				"GetNativeSystemInfo");
		if(NULL != pGNSI)
		pGNSI(&si);
		else
		GetSystemInfo(&si);

		/*
		 * From msdn page on OSVERSIONINFOEX, current as of this
		 * writing, decoding of dwMajorVersion and dwMinorVersion.
		 *
		 *  Operating system            dwMajorVersion  dwMinorVersion
		 * ==================           ==============  ==============
		 *
		 * Windows 95                   4               0
		 * Windows 98                   4               10
		 * Windows ME                   4               90
		 * Windows 3.51                 3               51
		 * Windows NT 4.0               4               0
		 * Windows 2000                 5               0
		 * Windows XP 32 bit            5               1
		 * Windows Server 2003 family   5               2
		 * Windows XP 64 bit            5               2
		 *       where ((&ver.wServicePackMinor) + 2) = 1
		 *       and  si.wProcessorArchitecture = 9
		 * Windows Vista family         6               0  (VER_NT_WORKSTATION)
		 * Windows Server 2008          6               0  (!VER_NT_WORKSTATION)
		 * Windows 7                    6               1  (VER_NT_WORKSTATION)
		 * Windows Server 2008 R2       6               1  (!VER_NT_WORKSTATION)
		 *
		 * This mapping will presumably be augmented as new Windows
		 * versions are released.
		 */
		char *os_name;
		switch (ver.dwPlatformId) {
			case VER_PLATFORM_WIN32s:
			os_name = "Windows 3.1";
			break;
			case VER_PLATFORM_WIN32_WINDOWS:
			if (ver.dwMajorVersion == 4) {
				switch (ver.dwMinorVersion) {
					case 0: os_name = "Windows 95"; break;
					case 10: os_name = "Windows 98"; break;
					case 90: os_name = "Windows Me"; break;
					default: os_name = "Windows 9X (unknown)"; break;
				}
			} else {
				os_name = "Windows 9X (unknown)";
			}
			break;
			case VER_PLATFORM_WIN32_NT:
			if (ver.dwMajorVersion <= 4) {
				os_name = "Windows NT";
			} else if (ver.dwMajorVersion == 5) {
				switch (ver.dwMinorVersion) {
					case 0: os_name = "Windows 2000"; break;
					case 1: os_name = "Windows XP"; break;
// 					case 2:
// 					/*
// 					 * From MSDN OSVERSIONINFOEX and SYSTEM_INFO documentation:
// 					 *
// 					 * "Because the version numbers for Windows Server 2003
// 					 * and Windows XP 6u4 bit are identical, you must also test
// 					 * whether the wProductType member is VER_NT_WORKSTATION.
// 					 * and si.wProcessorArchitecture is
// 					 * PROCESSOR_ARCHITECTURE_AMD64 (which is 9)
// 					 * If it is, the operating system is Windows XP 64 bit;
// 					 * otherwise, it is Windows Server 2003."
// 					 */
// 					if(ver.wProductType == VER_NT_WORKSTATION &&
// 							si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64) {
// 						os_name = "Windows XP"; /* 64 bit */
// 					} else {
// 						os_name = "Windows 2003";
// 					}
// 					break;
					default: os_name = "Windows NT (unknown)"; break;
				}
			} else if (ver.dwMajorVersion == 6) {
				/*
				 * See table in MSDN OSVERSIONINFOEX documentation.
				 */
// 				if (ver.wProductType == VER_NT_WORKSTATION) {
// 					switch (ver.dwMinorVersion) {
// 						case 0: os_name = "Windows Vista"; break;
// 						case 1: os_name = "Windows 7"; break;
// 						default: os_name = "Windows NT (unknown)";
// 					}
// 				} else {
					switch (ver.dwMinorVersion) {
						case 0: os_name = "Windows Server 2008"; break;
						case 1: os_name = "Windows Server 2008 R2"; break;
						default: os_name = "Windows NT (unknown)";
//					}
				}
			} else {
				os_name = "Windows NT (unknown)";
			}
			break;
			default:
			os_name = "Windows (unknown)";
			break;
		}
		map->put("os.name", new EString(os_name));

		eso_sprintf(buf, "%d.%d.%d", ver.dwMajorVersion, ver.dwMinorVersion, ver.dwBuildNumber);
		map->put("os.version", new EString(buf));

		char *os_arch;
#if defined(_M_IA64)
		os_arch = "ia64";
#elif defined(_M_AMD64)
		os_arch = "amd64";
#elif defined(_X86_)
		os_arch = "x86";
#else
		os_arch = "unknown";
#endif
		map->put("os.arch", new EString(os_arch));
	//}

	map->put("file.separator", new EString("\\"));
	map->put("path.separator", new EString(";"));
	map->put("line.separator", new EString("\r\n"));

	char *uname = getenv("USERNAME");
	map->put("user.name", new EString(uname));

	map->put("user.home", new EString("?"));

	char sbuf[ES_PATH_MAX] = {0};
	_getcwd(sbuf, ES_PATH_MAX);
	map->put("user.dir", new EString(sbuf));

	/*
	 *  user.language
	 *  user.script, user.country, user.variant (if user's environment specifies them)
	 *  file.encoding
	 */
	es_string_t *language = NULL;
	es_string_t *country = NULL;
	es_string_t *variant = NULL;
	es_string_t *script = NULL;
	es_string_t *encoding = NULL;
	parseLocale(ver, &language, &country, &variant, &script, &encoding);
	if (language) {
		map->put("user.language", new EString(language));
	}
	if (country) {
		map->put("user.country", new EString(country));
	}
	if (variant) {
		map->put("user.variant", new EString(variant));
	}
	if (script) {
		map->put("user.script", new EString(script));
	}
	if (encoding) {
		map->put("user.script", new EString(encoding));
	}
	ESO_MFREE(&language);
	ESO_MFREE(&country);
	ESO_MFREE(&variant);
	ESO_MFREE(&script);
	ESO_MFREE(&encoding);

#else //linux

	struct utsname name;
	uname(&name);
	map->put("os.name", new EString(name.sysname));
	map->put("os.version", new EString(name.release));

	FILE* pf = popen("uname -m", "r");
	if (!pf) {
		map->put("os.arch", new EString("unknown"));
	}
	else {
		char arch[64];
		char *p = eso_fgets(arch, sizeof(arch), pf);
		if (p) {
			char *q = eso_strchr(p, '\r');
			if (q) *q = 0;
			q = eso_strchr(p, '\n');
			if (q) *q = 0;
		}
		map->put("os.arch", new EString(p ? p : "unknown"));
		pclose(pf);
	}

	map->put("file.separator", new EString("/"));
	map->put("path.separator", new EString(":"));
	map->put("line.separator", new EString("\n"));

	struct passwd *pwent = getpwuid(getuid());
	map->put("user.name", new EString(pwent ? pwent->pw_name : "?"));
	map->put("user.home", new EString(pwent ? pwent->pw_dir : "?"));

	char sbuf[ES_PATH_MAX] = {0};
	getcwd(sbuf, ES_PATH_MAX);
	map->put("user.dir", new EString(sbuf));

	/*
	 *  user.language
	 *  user.script, user.country, user.variant (if user's environment specifies them)
     *  file.encoding
	 */
	es_string_t *language = NULL;
	es_string_t *country = NULL;
	es_string_t *variant = NULL;
	es_string_t *script = NULL;
	es_string_t *encoding = NULL;
	parseLocale(&language, &country, &variant, &script, &encoding);
	if (language) {
		map->put("user.language", new EString(language));
	}
	if (country) {
		map->put("user.country", new EString(country));
	}
	if (variant) {
		map->put("user.variant", new EString(variant));
	}
	if (script) {
		map->put("user.script", new EString(script));
	}
	if (encoding) {
		map->put("user.script", new EString(encoding));
	}
	ESO_MFREE(&language);
	ESO_MFREE(&country);
	ESO_MFREE(&variant);
	ESO_MFREE(&script);
	ESO_MFREE(&encoding);

#endif
}

es_status_t ESystem::init(int argc, const char **argv, const char *config)
{
	_initzz_(); //!!!

	// m_Arg
	if (argc > 0 && argv) {
		m_Arg = new ESimpleMap(true);
		parseArgs(m_Arg, argc, (char**)argv);
	}

	// m_Cfg
	if (config && *config) {
		const char *_currpath = getCurrentWorkPath();
		EString ss;

		m_Cfg = new EConfig();

		char resolved_path[ES_PATH_MAX] = {0};
		try {
			if (eso_pathrooted(config)) {
				ss = config;
			}
			else {
				ss.format("%s%c%s", _currpath, ES_FILE_PATH_SEPARATOR_CHAR, config);
			}
			m_Cfg->load(ss.c_str());
			m_FileCfg = new EString(eso_realpath(ss.c_str(), resolved_path, sizeof(resolved_path)));
		} catch (...) {
			if (!eso_pathrooted(config)) {
				ss.format("%s%c%s%c%s", _currpath, ES_FILE_PATH_SEPARATOR_CHAR,
						"conf", ES_FILE_PATH_SEPARATOR_CHAR, config);
				try {
					m_Cfg->load(ss.c_str());
					m_FileCfg =  new EString(eso_realpath(ss.c_str(), resolved_path, sizeof(resolved_path)));
					goto SUCC;
				} catch (...) {
				}
			}
			return ES_FAILURE;
		}
	}

	//TODO...

SUCC:
	return ES_SUCCESS;
}

void ESystem::detach(boolean daemonize, int in, int out, int err)
{
	eso_proc_detach(daemonize, in, out, err);
}

void ESystem::exit(int status)
{
	setIn(null);
	setOut(null);
	setErr(null);

	if (m_Arg) {
		delete m_Arg;
	}

	if (m_Cfg) {
		delete m_Cfg;
	}

	if (m_FileCfg) {
		delete m_FileCfg;
	}

	ESO_MFREE(&m_CurrPath);
	ESO_MFREE(&m_ExecPath);
	ESO_MFREE(&m_TempPath);

	//TODO...

	eso_terminate();

	::exit(status);
}

void ESystem::setIn(EInputStream* in) {
	if (m_stdin) {
		m_stdin->close();
		delete m_stdin;
	}
	m_stdin = in;
}

void ESystem::setOut(EPrintStream* out) {
	if (m_stdout) {
		m_stdout->close();
		delete m_stdout;
		delete m_stdout0;
		m_stdout0 = null;
	}
	m_stdout = out;
}

void ESystem::setErr(EPrintStream* err) {
	if (m_stderr) {
		m_stderr->close();
		delete m_stderr;
		delete m_stderr0;
		m_stderr0 = null;
	}
	m_stderr = err;
}

EInputStream* ESystem::getInput() {
	if (!m_stdin) {
		m_stdin = new EFileInputStream(stdin);
	}
	return m_stdin;
}

EPrintStream* ESystem::getOutput()
{
	if (!m_stdout) {
		m_stdout0 = new EFileOutputStream(stdout);
		m_stdout = new EPrintStream(m_stdout0);
	}
	return m_stdout;
}

EPrintStream* ESystem::getErrput()
{
	if (!m_stderr) {
		m_stderr0 = new EFileOutputStream(stderr);
		m_stderr = new EPrintStream(m_stderr0);
	}
	return m_stderr;
}

void ESystem::initSysProp()
{
	static ESimpleMap sysProp(true);
	m_Prop = &sysProp;
	initProps(m_Prop);
}

const char* ESystem::getCurrentWorkPath()
{
	if (!m_CurrPath) {
		eso_current_workpath(&m_CurrPath);
	}
	return m_CurrPath;
}

const char* ESystem::getExecuteFilename()
{
	if (!m_ExecPath) {
		eso_execute_filename(&m_ExecPath);
	}
	return m_ExecPath;
}

const char* ESystem::getTempPath()
{
	if (!m_TempPath) {
#ifdef WIN32
		char OutPath[ES_PATH_MAX];
		int len = GetTempPath(sizeof(OutPath), OutPath);
		eso_mstrcpy(&m_TempPath, OutPath);
#else
		eso_mstrcpy(&m_TempPath, "/tmp");
#endif
	}
	return m_TempPath;
}

const char* ESystem::getProperty(const char* key, const char* defaultValue)
{
	if (!m_Prop) return defaultValue;
	EString *obj = dynamic_cast<EString*>(m_Prop->get(key));
	return obj ? obj->c_str() : defaultValue;
}

sp<EString> ESystem::setProperty(const char* key, const char* value)
{
	EString *val = dynamic_cast<EString*>(m_Prop->get(key));
	sp<EString> old = val ? new EString(val) : null;
	m_Prop->put(key, new EString(value));
	return old;
}

const char* ESystem::getConfigParameter(const char* key, const char* defaultValue)
{
	return m_Cfg ? m_Cfg->getString(key, defaultValue) : defaultValue;
}

EConfig* ESystem::getConfig() {
	return m_Cfg;
}

const char* ESystem::getConfigFilename()
{
	return m_FileCfg ? m_FileCfg->c_str() : null;
}

boolean ESystem::containsProgramArgument(const char* key) {
	if (!m_Arg) return false;
	return (m_Arg->indexOf(key) >= 0);
}

const char* ESystem::getProgramArgument(const char* key, const char* defaultValue)
{
	if (!m_Arg) return defaultValue;
	EString *obj = dynamic_cast<EString*>(m_Arg->get(key));
	return obj ? obj->c_str() : defaultValue;
}

const char* ESystem::getEnv(const char* name)
{
	return getenv(name);
}

void ESystem::arraycopy(void* src, int srcPos,
                          void* dest, int destPos,
                          int length)
{
	if (!src) {
		throw ENullPointerException(__FILE__, __LINE__);
	}

	eso_memmove((char*)dest + destPos, (char*)src + srcPos, length);
}

int ESystem::identityHashCode(EObject* x)
{
	es_size_t v = (es_size_t)x;
	return (int)(v ^ (v >> sizeof(es_size_t)));
}

} /* namespace efc */

