#include "parse_ini.h"

int main(void)
{
	unsigned char a[4] = {1, 2, 3, 4};
	unsigned char b[4] = {0};
	int value = 0;
	char buf[128] = {0};

	parse_start("config.ini");
	//add or update
	add_one_key("config", "abc1", 1234);
	add_more_key("config", "more", a, 4);
	add_key_string("config", "key", "string");
	//get
	value = get_one_value("config", "abc1");
	get_more_value("config", "more", b, 4);
	get_key_string("config", "key", buf);
	parse_end();

	//print info
	printf("value = %d, b[] = {%d, %d, %d, %d}, key = %s\n",
			value, b[0], b[1], b[2], b[3], buf);

	write_profile_string("test", "arg", "345", "config.ini");
	value = read_profile_int("test", "arg", 0, "config.ini");
	write_profile_string("NULL", "val", "250", "config.ini");
	printf("value = %d\n", value);
	return 0;
}
