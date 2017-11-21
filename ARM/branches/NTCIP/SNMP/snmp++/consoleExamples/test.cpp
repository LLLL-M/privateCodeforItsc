#include <iostream>  
#include <snmp_pp/snmp_pp.h>  
  
using namespace std;  
  
int main()  
{  
    Oid id("1.2.3.4.5.6");  
    cout << id.get_printable() << endl;  
    return 0;  
}  