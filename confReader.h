/** 
* @file confReader.h
* @brief �����ļ���ȡ���ͷ�ļ�
*/

/* confReader.h */

#ifndef _confReader_H
#define _confReader_H


/** 
* @example confReader����
  
  confReader ini("/where/is/config.conf");\n
  ini.setSection("MysqlDB");\n
  ini.readStr("mysqlServer", "");   // On error,"" is returned.\n
  ini.readInt("mysqlPort", 3306);   // On error,3306 is returned.\n
*/


#include <string>
using namespace std;

#ifdef _MSC_VER
#define strcasecmp stricmp
#define strncasecmp  _strnicmp 
#endif
//////////////////////////////////////////////////////////////////////////

/**
* @class confReader
* @brief ���ö�ȡ��
*/
class confReader
{
public:
  confReader(const string & fileName);
public:
  virtual ~confReader(void);
  const string & getFileName() const
  {
	  return m_fileName;
  }

  //  const string &getSection() const;  
  const string &getSection() const
  {
    return m_section;
  }

  void setSection(const string &section);
  
  bool write(const string &key, const string & value) const ;
  bool write(const string &key, int value) const ;

  string readStr(const string &key,const string &default_value) const;
  int readInt(const string &key, int default_value) const;

public:
  static int read_profile_string( const char *section, const char *key,char *value, 
    int size, const char *default_value, const char *file);
  static int read_profile_int( const char *section, const char *key,int default_value, 
    const char *file);
  static int write_profile_string(const char *section, const char *key,
    const char *value, const char *file);

private:
  static int load_ini_file(const char *file, char *buf,int *file_size);
  static int newline(char c);
  static int end_of_string(char c);
  static int left_barce(char c);
  static int right_brace(char c );
  static int parse_file(const char *section, const char *key, const char *buf,int *sec_s,int *sec_e,
    int *key_s,int *key_e, int *value_s, int *value_e);

private:
  /// �����ļ�����
  string m_fileName;
  /// �����ļ��ڵ�
  string m_section;
};

#endif //_confReader_H
