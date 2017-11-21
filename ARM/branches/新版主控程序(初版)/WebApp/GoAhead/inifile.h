/**
 * @file
 * @brief initialization file read and write API 
 * @author Deng Yangjun
 * @date 2007-1-9
 * @version 0.1
 */
 
#ifndef INI_FILE_H_
#define INI_FILE_H_

#ifdef __cplusplus
extern "C"
{
#endif

int read_profile_string(const char *section_pass, const char *key_pass_ini,char *value_pass, int size,const char *default_value,const char *file_pass);
int read_profile_int( const char *section_pass, const char *key_pass_ini,int default_value, const char *file_pass);
int write_profile_string( const char *section_pass, const char *key_pass_ini,const char *value_pass, const char *file_pass);

#ifdef __cplusplus
}; //end of extern "C" {
#endif

#endif //end of INI_FILE_H_

