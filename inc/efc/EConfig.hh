/*
 * EConfig.hh
 *
 *  Created on: 2013-3-23
 *      Author: cxxjava@163.com
 */

#ifndef ECONFIG_HH_
#define ECONFIG_HH_

#include "EFile.hh"
#include "EArray.hh"
#include "ESimpleMap.hh"
#include "EInputStream.hh"
#include "EIOException.hh"

namespace efc {

class EConfig : public EObject {
public:
	virtual ~EConfig();

	/**
	 *
	 */
	EConfig();

	/**
	 * Default to load from config file.
	 */
	EConfig(const char *file) THROWS(EIOException);

	// unsupported.
	EConfig(const EConfig& that);
	EConfig& operator= (const EConfig& that);

	/**
	 * Load from config file or stream.
	 */
	void load(const char *file) THROWS(EIOException);
	void load(EFile *file) THROWS(EIOException);
	void load(EInputStream* in) THROWS(EIOException);

	/**
	 * Load from ini file or stream.
	 */
	void loadFromINI(const char* file) THROWS(EIOException);
	void loadFromINI(EFile* file) THROWS(EIOException);
	void loadFromINI(EInputStream* in) THROWS(EIOException);

	/**
	 * Clear
	 */
	void clear();

	/**
	 * Get value with path like "/root/node|0"
	 */
	const char* getString(const char *path, const char* defval=NULL);
	int getInteger(const char *path, int defval=0);
	llong getLLong(const char *path, llong defval=0L);
	boolean getBoolean(const char *path, boolean defval=false);
	float getFloat(const char *path, float defval=0.0);
	double getFloat(const char *path, double defval=0.0);
	EConfig* getConfig(const char *path);
	EArray<EConfig*> getConfigs(const char *path);

	/**
	 * Returns a set of keys in this property list where
	 * the key and its corresponding value are strings,
	 * including distinct keys in the default property list if a key
	 * of the same name has not already been found from the main
	 * properties list.  Properties whose key or value is not
	 * of type <tt>String</tt> are omitted.
	 * <p>
	 * The returned set is not backed by the <tt>Properties</tt> object.
	 * Changes to this <tt>Properties</tt> are not reflected in the set,
	 * or vice versa.
	 *
	 * @param	type of the key.
	 * @return  a set of keys in this property list where
	 *          the key and its corresponding value are strings,
	 *          including the keys in the default property list.
	 * @see     java.util.Properties#defaults
	 * @since   1.6
	 */
	enum KeyType {
		ALL = 0,
		KV, //key = values
		KC  //key = config
	};
	EArray<EString*> keyNames(KeyType type=ALL);

	/**
	 * Get config name.
	 */
	const char* getName();

	EStringBase toString();

private:
	ESimpleMap* m_vmap; //key = values
	ESimpleMap* m_cmap; //key = config
	EString m_name;
	EConfig* m_parent;

	EConfig(const char* name, EConfig* parent);
	EString* findString(const char* path);
	EConfig* findConfig(const char* element);
};

} /* namespace efc */
#endif /* ECONFIG_HH_ */
