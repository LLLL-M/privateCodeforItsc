#include "communication.h"
#include "tsc.h"
#include "log.h"
#include "singleton.h"
#ifdef __linux__
#include <cstdlib>
#include <signal.h>
#endif

#if 0
void FeaturesTest(Tscdb &db)
{
	Features features = {1, 0, 1, 0, 1, 0};
	db.Store(features);
	memset(&features, 0, sizeof(Features));
	db.Load(features);
	cout << (int)features.gps << endl;
	cout << (int)features.watchdog << endl;
	cout << (int)features.voltCheck << endl;
	cout << (int)features.curCheck << endl;
	cout << (int)features.urgentPriority << endl;
	cout << endl << endl;
}

void ChannelTest(Tscdb &db)
{
	ChannelItem		table[MAX_CHANNEL_NUM];
	memset(table, 0, sizeof(table));

	table[0].channelId = 1;
	table[0].conflictChannel = 0x11;
	table[8].channelId = 9;
	table[8].conflictChannel = 0x22;
	db.Store(table);

	memset(table, 0, sizeof(table));
	db.Load(table);
	cout << (int)table[0].channelId << endl;
	cout << (int)table[0].conflictChannel << endl;
	cout << (int)table[8].channelId << endl;
	cout << (int)table[8].conflictChannel << endl;
	cout << endl << endl;
}

void PhaseTest(Tscdb &db)
{
	PhaseItem		table[MAX_PHASE_NUM];

#if 1
	memset(table, 0, sizeof(table));
	table[0].phaseId = 1;
	table[0].greenTime = 15;
	table[0].greenBlinkTime = 5;
	table[0].yellowTime = 3;
	table[0].allRedTime = 2;
	table[0].pedPassTime = 10;
	table[0].pedClearTime = 5;
	table[0].minGreenTime = 10;
	table[0].maxGreenTime = 30;
	table[0].autoRequest = 1;
	table[0].unitExtendTime = 3;
	table[0].checkTime = 6;
	table[0].channelBits = 0x101;
	table[10].phaseId = 11;
	table[10].greenTime = 20;
	table[10].greenBlinkTime = 3;
	table[10].yellowTime = 3;
	table[10].allRedTime = 2;
	table[10].pedPassTime = 15;
	table[10].pedClearTime = 6;
	table[10].minGreenTime = 15;
	table[10].maxGreenTime = 35;
	table[10].autoRequest = 1;
	table[10].unitExtendTime = 5;
	table[10].checkTime = 3;
	table[10].channelBits = 0x80000002;
	db.Store(table);
#endif
	memset(table, 0, sizeof(table));
	db.Load(table);
	int i;
	for (i = 0; i < MAX_PHASE_NUM; i++)
	{
		if (table[i].phaseId == 0)
			continue;
		cout << (int)table[i].phaseId << endl;
		cout << (int)table[i].greenTime << endl;
		cout << (int)table[i].greenBlinkTime << endl;
		cout << (int)table[i].yellowTime << endl;
		cout << (int)table[i].allRedTime << endl;
		cout << (int)table[i].pedPassTime << endl;
		cout << (int)table[i].pedClearTime << endl;
		cout << (int)table[i].minGreenTime << endl;
		cout << (int)table[i].maxGreenTime << endl;
		cout << (int)table[i].autoRequest << endl;
		cout << (int)table[i].unitExtendTime << endl;
		cout << (int)table[i].checkTime << endl;
		cout << (UInt32)table[i].channelBits << endl;
		cout << endl;
	}
	cout << endl << endl;
}

void SchemeTest(Tscdb &db)
{
	SchemeItem		table[MAX_SCHEME_NUM];
	int i, j;

	memset(table, 0, sizeof(table));
	table[2].schemeId = 3;
	table[2].cycleTime = 100;
	table[2].phaseOffset = 10;
	table[2].coordinatePhase = 1;
	table[2].phaseturn[0] = 1;
	table[2].phaseturn[1] = 2;
	table[2].phaseturn[2] = 5;
	table[2].phaseturn[3] = 4;
	db.Store(table);

	memset(table, 0, sizeof(table));
	db.Load(table);
	for (i = 0; i < MAX_SCHEME_NUM; i++)
	{
		if (table[i].schemeId == 0)
			continue;
		cout << (int)table[i].schemeId        << endl;
		cout << (int)table[i].cycleTime       << endl;
		cout << (int)table[i].phaseOffset     << endl;
		cout << (int)table[i].coordinatePhase << endl;
		for (j = 0; j < MAX_PHASE_NUM; j++)
		{
			if (table[i].phaseturn[j] == 0)
				break;
			cout << (int)table[i].phaseturn[j] << ',';
		}
		cout << endl;
	}
	cout << endl << endl;
}

void ScheduleTest(Tscdb &db)
{
	ScheduleItem	table[MAX_SCHEDULE_NUM];
	int i;

	memset(table, 0, sizeof(table));
	table[2].scheduleId = 3;
	table[2].week = 7;
	table[2].month = 3;
	table[2].day = 12;
	table[2].startHour = 14;
	table[2].startMin = 58;
	table[2].endHour = 17;
	table[2].endMin = 10;
	table[2].rule.ctrlType = 1;
	table[2].rule.ctrlMode = 2;
	table[2].rule.ctrlId = 3;
	db.Store(table);

	memset(table, 0, sizeof(table));
	db.Load(table);
	for (i = 0; i < MAX_SCHEDULE_NUM; i++)
	{
		if (table[i].scheduleId == 0)
			continue;
		cout << (int)table[i].scheduleId   << endl;
		cout << (int)table[i].week         << endl;
		cout << (int)table[i].month        << endl;
		cout << (int)table[i].day          << endl;
		cout << (int)table[i].startHour    << endl;
		cout << (int)table[i].startMin     << endl;
		cout << (int)table[i].endHour      << endl;
		cout << (int)table[i].endMin       << endl;
		cout << (int)table[i].rule.ctrlType << endl;
		cout << (int)table[i].rule.ctrlMode << endl;
		cout << (int)table[i].rule.ctrlId  << endl;
		cout << endl;
	}
	cout << endl << endl;
}

void DetectorTest(Tscdb &db)
{
	DetectorItem	table[MAX_DETECTOR_NUM];
	int i;

	memset(table, 0, sizeof(table));
	table[2].detectorId = 3;
	table[2].noResponseTime = 1;
	table[2].maxContinuousTime = 5;
	table[2].maxVehcileNum = 100;
	db.Store(table);

	memset(table, 0, sizeof(table));
	db.Load(table);
	for (i = 0; i < MAX_DETECTOR_NUM; i++)
	{
		if (table[i].detectorId == 0)
			continue;
		cout << (int)table[i].detectorId        << endl;
		cout << (int)table[i].noResponseTime    << endl;
		cout << (int)table[i].maxContinuousTime << endl;
		cout << (int)table[i].maxVehcileNum     << endl;
		cout << endl;
	}
	cout << endl << endl;
}

void BasicTest(Tscdb &db)
{
	Basic basic;

	db.Load(basic);
	cout << "areaNumber: " << (int)basic.areaNumber << endl;
	cout << "roadNumber: " << (int)basic.roadNumber << endl;
	cout << "version: " << basic.version << endl;
	cout << "identifyCode: " << basic.identifyCode << endl;
	cout << "bootYellowBlinkTime: " << (int)basic.bootYellowBlinkTime << endl;
	cout << "bootAllRedTime: " << (int)basic.bootAllRedTime << endl;
	cout << "vehFlowUploadCycleTime: " << (int)basic.vehFlowUploadCycleTime << endl;
	cout << "transitionCycle: " << (int)basic.transitionCycle << endl;
	cout <<  "ip: " << basic.ip << endl;
	cout << "port: " << (int)basic.port << endl;
}

void TscdbTest()
{
	Tscdb db;

	if (!db.Open())
		return;
	//FeaturesTest(db);
	//ChannelTest(db);
	//PhaseTest(db);
	//SchemeTest(db);
	//ScheduleTest(db);
	//DetectorTest(db);
	//BasicTest(db);
	db.Close();
}
#endif

#ifdef __linux__
static void SigHandle(int sigNum)
{
	Log &log = Singleton<Log>::GetInstance();
	switch (sigNum)
	{
		case SIGSEGV:	log.Write("This program has sigmentfault"); break;
		case SIGINT:	INFO("This program has been interrupted by ctrl+C"); break;
		case SIGTERM:	INFO("This program has been interrupted by command 'kill' or 'killall'"); break;
	}
	system("echo 1 > /proc/sys/vm/drop_caches");
	exit(1);
}
#endif

int main(void)
{
#ifdef __linux__
	signal(SIGSEGV, SigHandle);
	signal(SIGINT, SigHandle);		//for ctrl + c
	signal(SIGTERM, SigHandle);    //for command 'kill' or 'killall'
#endif
	YellowFault yf;
	Sock &sock = Singleton<Sock>::GetInstance();
	Tsc &tsc = Singleton<Tsc>::GetInstance();
	//Its &its = Singleton<Tsc>::GetInstance();
	sock.SetUpMachineAddr(tsc.basic.ip, tsc.basic.port);
	if (!sock.CreateUdpSocket(20000))
		return -1;
	Communication communication(tsc, sock);
	communication.start();
	yf.start();
	tsc.start();
	while (1) sleep(10000);
	return 0;
}
