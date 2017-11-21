#ifdef __WIN32__
#include <QCoreApplication>
#endif

#include <time.h>
#include <stdlib.h>
#include <cstring>

#include "communication.h"
#include "its.h"
#include "config.h"
#include "sqlite_conf.h"
#include "custom.h"
#include "desc.h"
#include "hikconf.h"
#include "hiktsc.h"
#include "protocol.h"


using namespace HikIts;
using HikIts::Protocol;


int main(int argc, char *argv[])
{
#ifdef __WIN32__
    QCoreApplication a(argc, argv);
#endif
    Sqliteconf sqlite_conf;
	sqlite_conf.init_db_file_tables();
	
    CountDown countdown;
	Configfile config(&sqlite_conf);
	Customfile custom(&sqlite_conf);
	Descfile desc(&sqlite_conf);
    ChanLockControl chlock(&config, &custom);
    Hiktsc hiktsc(&countdown, &chlock, static_cast<McastInfo*>(NULL), static_cast<PerSecTimes>(4));
	Hikconf hikconf(&sqlite_conf, &hiktsc);
	Its its(&hiktsc);

    int port = 20000;
    if (argc == 2)
    {
        port = atoi(argv[1]);
        if (port > 0xffff)
        {
            cout << "port " << port << " config error, use default 20000" << endl;
            port = 20000;
        }
    }

    HikCommunication communication((unsigned short)port, &its, &hiktsc, config, custom, desc, hikconf);
	
    if (false == communication.start())
        cerr << "hik communication thread failed" << endl;
    else
    {
        if (its.ItsInit(&hiktsc))
            cout << "Traffic signal control simulate machine start!!!" << endl;
        else
            cerr << "its thread failed" <<endl;
    }

#ifdef __WIN32__
	return a.exec();
#else
	while(1)
	{
		sleep(10);
	}
	return 0;
#endif
}
