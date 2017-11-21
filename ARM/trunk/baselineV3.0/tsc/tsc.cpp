#include <ctime>
#include <unistd.h>
#include <iostream>
#include <algorithm>
#include "tsc.h"
#include "parse.h"
#include "network.h"
#include "ctrl.h"
#include "singleton.h"
#include "log.h"

//Head Table
#define TSC_VER 					"ver"
#define TSC_CONFIG_TYPE				"type"
#define TSC_UPLOAD_TYPE				"upload"
#define TSC_HEAD_ID					"id"
#define	TSC_HEAD_ASYNC				"async"
#define TSC_CTRL_SRC				"src"
#define	TSC_SRC_IP					"srcip"

using namespace std;

//Load the tsc.db into TscList<>.use class
template<typename T>
void Tsc::Load(T &t, const char* name)
{
	TscRetval ret;
	ret = t.Parse(db.fetch(name));
	if(ret)
	{
		t.use = t.bak;
	}
	else
	{
		cout << ret.msg<< endl;
	}
}

Tsc::Tsc() : log(Singleton<Log>::GetInstance()), db("tsc.db")
{
	//The UpdateWholeUsr() function isn't include the single table
	//, so the single table should load	with new function
	Load(network,"network");
	Load(serial,"serial");
	Load(wireless,"wireless");
	Load(panel,"panel");
	Load(multilock,"multilock");

	vehDetector.Parse(db.fetch("vehdetector"));
	pedDetector.Parse(db.fetch("peddetector"));
	countdown.Parse(db.fetch("countdown"));
	channel.Parse(db.fetch("channel"));
	phase.Parse(db.fetch("phase"));
	scheme.Parse(db.fetch("scheme"));
	timeinterval.Parse(db.fetch("timeinterval"));
	schedule.Parse(db.fetch("schedule"));
	unit.Parse(db.fetch("unit"));
	prior.Parse(db.fetch("prior"));
	network.Parse(db.fetch("network"));
	wifi.Parse(db.fetch("wifi"));
	serial.Parse(db.fetch("serial"));
	redcheck.Parse(db.fetch("redcheck"));
	TscRetval ret = WholeCheck();
	UpdateWholeUsr(ret);
	if (!ret)
		log.Write(ret.msg);

	string password_store = db.fetch("password");
	if (!password_store.empty())
		password = password_store;
	string timezone_store = db.fetch("timezone");
	if (!timezone_store.empty())
		timezone = stoi(timezone_store);
	#if defined(__linux__) && defined(__arm__)   //Only execute in the arm linux mode
	if (network.Exist(1))
	{
		Network::SetNetwork("eth0", network.use[1].ip.data(), network.use[1].netmask.data(), network.use[1].gateway.data());
		INFO("set eth0 succ");
	}
	if (network.Exist(2))
	{
		Network::SetNetwork("eth1", network.use[2].ip.data(), network.use[2].netmask.data(), network.use[2].gateway.data());
		INFO("set eth1 succ");
	}
	if (network.Exist(3))
	{
		Network::SetNetwork("wlan", network.use[3].ip.data(), network.use[3].netmask.data(), network.use[3].gateway.data());
		INFO("set wlan succ");
	}	
	#endif
}

template<typename T, typename D>
TscRetval Tsc::Deal(T && t, bool upload, D && data, UInt8 id, const char *name, function<TscRetval()> Check)
{
	TscRetval ret;
	if (upload)
	{
		rwl.r_lock();
		ret = t.Json(id);
		rwl.r_unlock();
		return ret;
	}

	ret = t.Parse(data);
	if (!ret)
		return ret;
	if (Check != nullptr)
	{
		ret = Check();
		if (!ret)
		{
			rwl.r_lock();
			t.bak = t.use;
			rwl.r_unlock();
			return ret;
		}
	}
	
	rwl.w_lock();
	t.use = t.bak;
	rwl.w_unlock();
	//重新读取配置存入数据库
	rwl.r_lock();
	ret = t.Json();
	rwl.r_unlock();
	db.store(name, ret.msg);
	//进行配置更新
	Ctrl &ctl = Singleton<Ctrl>::GetInstance();
	ctl.ConfigUpdate();
	return true;
}

TscRetval Tsc::HeadParse(zframe_t *frame, TscHead &head)
{
	Json::Reader reader;
	Json::Value value;
	TscRetval ret;

	const char *data = (char *)zframe_data(frame);
	size_t size = zframe_size(frame);

	if (reader.parse(data, data + size, value))
	{
		JsonParse(value, TSC_CONFIG_TYPE, head.type, TSC_VER, head.ver, TSC_UPLOAD_TYPE, head.upload,
		          TSC_HEAD_ID, head.id, TSC_HEAD_ASYNC, head.async, TSC_CTRL_SRC, head.src, TSC_SRC_IP, head.srcip);

		if (head.type >= TSC_TYPE_MAX || head.type < TSC_TYPE_PUBKEY)
			ret.Err("The head type %s is out of range!", TSC_CONFIG_TYPE);
	}
	else
	{
		ret.succ = false;
		ret.msg  = reader.getFormattedErrorMessages();
	}
	return ret;
}

TscRetval Tsc::ParseSecFrame(const TscHead &head, zframe_t* frame)
{
	TscRetval ret;
	Ctrl &ctl = Singleton<Ctrl>::GetInstance();
	switch (head.type)
	{
		case TSC_TYPE_PUBKEY:
			{
				//To Execute at Communication Module
				break;
			}

		case TSC_TYPE_LOGIN:
			{
				//To Execute at Communication Module
				break;
			}

		case TSC_TYPE_UNIT:
			{
				if (!head.upload)
				{
					ret = unit.Parse(frame);
					if(ret)
					{
						db.store("unit", unit.Json().msg);
						ctl.ConfigUpdate();
					}
				}
				else
				{
					ret = unit.Json();
				}
				break;
			}

		case TSC_TYPE_CHANNEL:
			{
				ret = Deal(channel, head.upload, frame, head.id, "channel", [this]()->TscRetval{
					TscRetval ret = PhaseCheck();
					ret += ChannelCheck();
					ret += PriorCheck();
					ret += MultilockCheck();
					return ret;
				});
				break;
			}

		case TSC_TYPE_PHASE:
			{
				ret = Deal(phase, head.upload, frame, head.id, "phase", [this]()->TscRetval{
					TscRetval ret = PhaseCheck();
					ret += SchemeCheck();
					return ret;
				});
				break;
			}

		case TSC_TYPE_SCHEME:
			{
				ret = Deal(scheme, head.upload, frame, head.id, "scheme", [this]()->TscRetval{
					TscRetval ret = SchemeCheck();
					ret += TimeintervalCheck();
					return ret;
				});
				break;
			}

		case TSC_TYPE_TIMEINTERVAL:
			{
				ret = Deal(timeinterval, head.upload, frame, head.id, "timeinterval", [this]()->TscRetval{
					TscRetval ret = TimeintervalCheck();
					ret += ScheduleCheck();
					return ret;
				});
				break;
			}

		case TSC_TYPE_SCHEDULE:
			{
				ret = Deal(schedule, head.upload, frame, head.id, "schedule", [this]()->TscRetval{return ScheduleCheck();});
				break;
			}

		case TSC_TYPE_WHOLE:
			{
				//To Execute at Communication Module
				break;
			}

		case TSC_TYPE_VEHDETECTOR:
			{
				ret = Deal(vehDetector, head.upload, frame, head.id, "vehdetector", nullptr);
				break;
			}

		case TSC_TYPE_PEDDETECTOR:
			{
				ret = Deal(pedDetector, head.upload, frame, head.id, "peddetector", nullptr);
				break;
			}

		case TSC_TYPE_PRIOR:
			{
				ret = Deal(prior, head.upload, frame, head.id, "prior", [this]()->TscRetval{return PriorCheck();});
				break;
			}

		case TSC_TYPE_COUNTDOWN:
			{
				ret = Deal(countdown, head.upload, frame, head.id, "countdown", nullptr);	
				break;
			}

		case TSC_TYPE_MANUAL_CTRL:
			{
				if (head.upload)
				{
					ret.Err("Manual control don't support upload!");
					return ret;
				}
				TscCtrl manualctrl;
				ret = manualctrl.Parse(frame);

				if (ret)
				{
					manualctrl.ctrlType = CLIENT_CONTROL;
					ControlRule rule(manualctrl.ctrlType, manualctrl.ctrlMode, manualctrl.ctrlId, manualctrl.duration);
					bool ret = ctl.SetRule(rule);
					INFO("manual control, ctrlMode: %d, ctrlId: %d, setRule: %d", rule.ctrlMode, rule.ctrlId, ret);
				}
				break;
			}

		case TSC_TYPE_SYSTIME:
			{
				if (!head.upload)
				{
					ret = TimeSet(frame);
					if (ret.succ)
					{
						db.store("timezone", to_string(timezone));	
						ctl.ConfigUpdate();
					}
				}
				else
				{
					Json::Value root;
					Json::FastWriter writer;
					root["UTC"] = (Json::Int64)(time(nullptr) - timezone * 3600);
					root["timezone"] = timezone;
					ret.msg = writer.write(root);
				}
				break;
			}

		case TSC_TYPE_REMOTE:
			{
				//Temporary control needn't store
				break;
			}

		case TSC_TYPE_NETWORK:
			{
				ret = Deal(network, head.upload, frame, head.id, "network", nullptr);
				if (ret.succ && !head.upload)
				{
				#if defined(__linux__) && defined(__arm__)
					if (network.Exist(1))
					{
						Network::SetNetwork("eth0", network.use[1].ip.data(), network.use[1].netmask.data(), network.use[1].gateway.data());
						INFO("set eth0 succ");
					}
					if (network.Exist(2))
					{
						Network::SetNetwork("eth1", network.use[2].ip.data(), network.use[2].netmask.data(), network.use[2].gateway.data());
						INFO("set eth1 succ");
					}
					if (network.Exist(3))
					{
						Network::SetNetwork("wlan", network.use[3].ip.data(), network.use[3].netmask.data(), network.use[3].gateway.data());
						INFO("set wlan succ");
					}	
				#endif
				}
				break;
			}

		case TSC_TYPE_WIFI:
			{
				if (!head.upload)
				{
					ret = wifi.Parse(frame);
					if(ret)
					{
						db.store("wifi", wifi.Json().msg);
					}
				}
				else
				{
					ret = wifi.Json();
				}
				break;
			}

		case TSC_TYPE_SERIAL:
			{
				ret = Deal(serial, head.upload, frame, head.id, "serial", nullptr);
				break;
			}

		case TSC_TYPE_CHANNEL_CHECK:
			{
				//Temporary control needn't store
				break;
			}

		case TSC_TYPE_REDCHECK:
			{
				if (!head.upload)
				{
					ret = redcheck.Parse(frame);
					if(ret)
					{
						db.store("redcheck", redcheck.Json().msg);
					}
				}
				else
				{
					ret = redcheck.Json();
				}
				break;
			}

		case TSC_TYPE_WIRELESS:
			{
				ret = Deal(wireless, head.upload, frame, head.id, "wireless", nullptr);			
				break;
			}

		case TSC_TYPE_PANEL:
			{
				ret = Deal(panel, head.upload, frame, head.id, "panel", nullptr);		
				break;
			}

		case TSC_TYPE_LOG:
			{
				Json::Reader reader;
				Json::Value value;
				time_t start = 0;
				time_t end = 0;
				if (reader.parse((char *)zframe_data(frame), (char *)zframe_data(frame) + zframe_size(frame), value))
				{
					if (value.isMember(TSC_LOG_START_TIME))
						start = value[TSC_LOG_START_TIME].asUInt64();

					if (value.isMember(TSC_LOG_END_TIME))
						end = value[TSC_LOG_END_TIME].asUInt64();
				}
				else
				{
					ret.succ = false;
					ret.msg = reader.getFormattedErrorMessages();
					return ret;
				}

				if (head.upload)
				{
					ret.msg = log.Read2Json(start, end);
				}
				else
				{
					log.Erase(start, end);
				}
				break;
			}

		case TSC_TYPE_UPDATE:
			{
				//To Execute at Communication Module
				break;
			}

		case TSC_TYPE_MULTILOCK:
			{
				ret = Deal(multilock, head.upload, frame, head.id, "multilock", nullptr);
				break;
			}

		case TSC_TYPE_PASSWORD:
			{
				if (!head.upload)
				{
					ret = PasswordUpdate(frame);
					if (ret.succ)
						db.store("password", password);
				}
				else
				{
					Json::Value root;
					Json::FastWriter writer;
					root["password"] = password;
					ret.msg = writer.write(root);
				}
				break;
			}
	}
	if (!head.upload && ret)
		db.commit();	//只有下载成功时才commit
	return ret;
}

/*解析json*/
template<typename T, int minval, int maxval>
TscRetval TscList<T, minval, maxval>::Parse(const char * data, size_t size)
{
	Json::Reader reader;
	Json::Value root;
	TscRetval ret;

	if (reader.parse(data, data + size, root))
	{
		if(root.isArray())
		{
			for (unsigned int i = 0; i < root.size(); i++)
			{
				if (!root[i].isMember(TSC_ID))
				{
					ret.succ = false;
					ret.msg = "'id' isn't exist in json text!";
					break;
				}

				int id = root[i][TSC_ID].asInt();

				if (id < minval || id > maxval)
				{
					ret.Err("id %d is invalid!", id);
					break;
				}

				if (root[i][TSC_ACTION].asInt() == ACTION_DELETE)
				{
					bak.erase(id);
				}
				else
				{
					ret = bak[id].Parse(root[i]);

					if (!ret.succ)
					{
						break;
					}
				}
			}
		}
	}
	else
	{
		ret.succ = false;
		ret.msg = reader.getFormattedErrorMessages();
	}

	if (!ret.succ)
	{	//如果解析出错则恢复正常使用的配置
		bak = use;
	}

	return ret;
}

void Tsc::UpdateWholeUsr(TscRetval &ret)
{
	if (ret.succ == true)
	{
		rwl.w_lock();
		channel.use = channel.bak;
		phase.use = phase.bak;
		scheme.use = scheme.bak;
		timeinterval.use = timeinterval.bak;
		schedule.use = schedule.bak;
		vehDetector.use = vehDetector.bak;
		pedDetector.use = pedDetector.bak;
		prior.use = prior.bak;
		countdown.use = countdown.bak;
		rwl.w_unlock();
	}
	else
	{
		rwl.r_lock();
		channel.bak = channel.use;
		phase.bak = phase.use;
		scheme.bak = scheme.use;
		timeinterval.bak = timeinterval.use;
		schedule.bak = schedule.use;
		vehDetector.bak = vehDetector.use;
		pedDetector.bak = pedDetector.use;
		prior.bak = prior.use;
		countdown.bak = countdown.use;
		rwl.r_unlock();
	}
}

/*生成json*/
template<typename T, int minval, int maxval>
TscRetval TscList<T, minval, maxval>::Json(int id)
{
	Json::Value root;
	TscRetval ret;

	if (id == 0)
	{
		for (auto &i : use)
			root.append(i.second.Json());
	}
	else
	{
		if (id >= minval && id <= maxval && use.find(id) != use.end())
			root.append(use[id].Json());
	}

	if (root.size() == 0)
	{
		ret.Err("Can't find list exist, when id = %d", id);
	}
	else
	{
		Json::FastWriter writer;
		ret.msg = writer.write(root);
	}
	return ret;
}

template<typename T>
TscRetval TscSingle<T>::Json()
{
	Json::Value root;
	TscRetval ret;
	root = conf.Json();
	if (root.size() == 0)
	{
		ret.Err("Can't find list exist");
	}
	else
	{
		Json::FastWriter writer;
		ret.msg = writer.write(root);
	}
	return ret;
}

TscRetval TscUnit::Parse(const char *data, size_t size)
{
	Json::Reader reader;
	Json::Value value;
	TscRetval ret;

	if (reader.parse(data, data + size, value))
	{
		if(!value.isArray())
		{
			JsonParse(value, TSC_AREA, area, TSC_JUNCTION, junction, TSC_BOOTFLASH, bootFlash,
			          TSC_BOOTALLRED, bootAllRed, TSC_TRAFFIC_COLLECT,
			          collect, TSC_GREENWAVE_TRANS, transition, TSC_GPS, GPS,
			          TSC_WATCHDOG, watchdog, TSC_VOLT_CHECK, voltCheck,
			          TSC_CUR_CHECK, curCheck, TSC_FAULT_FLASH, faultFlash, TSC_TAKEOVER, takeover);
		}
		else
		{
			ret.Err("The unit table shouldn't be transformed as array!");
			return ret;
		}
	}
	else
	{
		ret.succ = false;
		ret.msg = reader.getFormattedErrorMessages();
	}
	return ret;
}

Json::Value TscUnit::Json()
{
	Json::Value root;
	Json::FastWriter writer;

	root[TSC_AREA]            = area;
	root[TSC_JUNCTION]        = junction;
	root[TSC_BOOTFLASH]       = bootFlash;
	root[TSC_BOOTALLRED]      = bootAllRed;
	root[TSC_TRAFFIC_COLLECT] = collect;
	root[TSC_GREENWAVE_TRANS] = transition;
	root[TSC_GPS]             = GPS;
	root[TSC_WATCHDOG]        = watchdog;
	root[TSC_VOLT_CHECK]      = voltCheck;
	root[TSC_CUR_CHECK]       = curCheck;
	root[TSC_FAULT_FLASH]     = faultFlash;
	root[TSC_TAKEOVER]        = takeover;

	return root;
}


TscRetval TscChannel::Parse(const Json::Value &value)
{
	TscRetval ret;
	JsonParse(value, TSC_ID, id, TSC_TYPE, type, TSC_STATUS, status, TSC_CONFILCT_BITS,
	          conflictBits, TSC_COUNTDOWN, countdown, TSC_VEH_BITS, vehBits, TSC_PED_BITS, pedBits, TSC_DESC, desc);
	return ret;
}

Json::Value TscChannel::Json()
{
	Json::Value root;

	root[TSC_ID]            = id;
	root[TSC_TYPE]          = type;
	root[TSC_STATUS]        = status;
	root[TSC_CONFILCT_BITS] = (Json::Value::UInt64)conflictBits.to_ullong();
	root[TSC_COUNTDOWN]     = countdown;
	root[TSC_VEH_BITS]      = (Json::Value::UInt64)vehBits.to_ullong();
	root[TSC_PED_BITS]      = (Json::Value::UInt64)pedBits.to_ullong();
	root[TSC_DESC]          = desc;

	return root;
}

TscRetval TscPhase::Parse(const Json::Value &value)
{
	TscRetval ret;
	JsonParse(value, TSC_ID, id,TSC_DESC, desc, TSC_GREENFLASH, greenFlash, TSC_YELLOW_TIME, yellow, TSC_ALLRED_TIME, allred,
	          TSC_REDYELLOW_TIME, redyellow, TSC_PED_CLEAR_TIME, pedClear, TSC_MIN_GREEN_TIME, minGreen, TSC_MAX_GREEN_TIME,
	          maxGreen, TSC_MAX_GREEN2_TIME, maxGreen2, TSC_UNI_EXTEND_TIME, unitExtend, TSC_CHECK_TIME, checkTime,
	          TSC_CHANNEL_BITS, channelBits, TSC_AUTO_REQ, autoReq);
	return ret;
}

Json::Value TscPhase::Json()
{
	Json::Value root;
	root[TSC_ID]              = id;
	root[TSC_DESC]			  = desc;
	root[TSC_GREENFLASH]      = greenFlash;
	root[TSC_YELLOW_TIME]     = yellow;
	root[TSC_ALLRED_TIME]     = allred;
	root[TSC_REDYELLOW_TIME]  = redyellow;
	root[TSC_PED_CLEAR_TIME]  = pedClear;
	root[TSC_MIN_GREEN_TIME]  = minGreen;
	root[TSC_MAX_GREEN_TIME]  = maxGreen;
	root[TSC_MAX_GREEN2_TIME] = maxGreen2;
	root[TSC_UNI_EXTEND_TIME] = unitExtend;
	root[TSC_CHECK_TIME]      = checkTime;
	root[TSC_CHANNEL_BITS]    = (Json::Value::UInt64)channelBits.to_ullong();
	root[TSC_AUTO_REQ]        = autoReq;
	return root;
}

TscRetval TscScheme::Parse(const Json::Value &value)
{
	TscRetval ret;
	JsonParse(value, TSC_ID, id, TSC_DESC, desc, TSC_CYCLE_TIME, cycle, TSC_OFFSET, offset,
	          TSC_COORDINATE_PHASE, coordPhase, TSC_TIME_MODIFY, timing, TSC_TURN_MODIFY, turn);
	return ret;
}

Json::Value TscScheme::Json()
{
	Json::Value root;
	Json::Value timingJson;
	Json::Value turnJson;

	root[TSC_ID]               = id;
	root[TSC_DESC]             = desc;
	root[TSC_CYCLE_TIME]       = cycle;
	root[TSC_OFFSET]           = offset;
	root[TSC_COORDINATE_PHASE] = coordPhase;

	int j = 0;
	for (auto &i: timing)
	{
		timingJson[j]["phase"]   = i.second.phase;
		timingJson[j]["time"]    = i.second.time;
		timingJson[j]["status"]  = i.second.status;
		timingJson[j]["barrier"] = i.second.barrier;
		j++;
	}

	root[TSC_TIME_MODIFY] = timingJson;

	for (unsigned int j = 0; j < turn.size(); j++)
	{
		for (unsigned int k = 0; k < turn[j].size(); k++)
			turnJson[j][k]  = turn[j][k];
	}
	root[TSC_TURN_MODIFY] = turnJson;

	return root;
}

TscRetval TscTimeinterval::Parse(const Json::Value &value)
{
	TscRetval ret;

	id = value[TSC_ID].asInt();

	if (value.isMember(TSC_TIME_INTERVAL))
	{
		const Json::Value &tiv = value[TSC_TIME_INTERVAL];

		if (tiv.size() < MAX_TIMEINTERVAL_NUM)
			JsonParse(value, TSC_TIME_INTERVAL, section);
		else
		{
			ret.Err("The timeinterval size[%d] is out of range![1, %d]", tiv.size(), MAX_TIMEINTERVAL_NUM);
			return ret;
		}

		if (value.isMember(TSC_INTERVAL_SEQ_DEL))
		{
			unsigned int DelSeq = value[TSC_INTERVAL_SEQ_DEL].asInt();

			if ( DelSeq > section.size())
			{
				ret.Err("The DelSeq[%d] is out of range[%d]", DelSeq, section.size());
				return ret;
			}
			else
				section.erase(section.begin() + DelSeq - 1);
		}

		if (value.isMember(TSC_INTERVAL_SEQ))
		{
			unsigned int seq = value[TSC_INTERVAL_SEQ].asInt();

			if ( seq > section.size())
			{
				ret.Err("The Seq[%d] is out of range[%d]", seq, section.size());
				return ret;
			}
			else
			{
				auto &hour   = section[seq - 1].hour;
				auto &minute = section[seq - 1].minute;
				auto &mode   = section[seq - 1].mode;
				auto &scheme = section[seq - 1].scheme;

				JsonParse(value, TSC_HOUR, hour, TSC_MINUTE, minute, TSC_CONTROL_MODE, mode, TSC_SCHEME_ID, scheme);
			}
		}
	}
	return ret;
}

Json::Value TscTimeinterval::Json()
{
	Json::Value root;
	Json::Value tiv;

	root[TSC_ID]         = id;
	for (unsigned int j = 0; j < section.size(); j++)
	{
		tiv[j][TSC_HOUR]         = section[j].hour;
		tiv[j][TSC_MINUTE]       = section[j].minute;
		tiv[j][TSC_CONTROL_MODE] = section[j].mode;
		tiv[j][TSC_SCHEME_ID]    = section[j].scheme;
	}
	root[TSC_TIME_INTERVAL] = tiv;

	return root;
}

TscRetval TscSchedule::Parse(const Json::Value &value)
{
	TscRetval ret;
	JsonParse(value, TSC_ID, id, TSC_DATE, date, TSC_WEEK, week,
	          TSC_TIME_INTERVAL_ID, timeinterval);
	return ret;
}

Json::Value TscSchedule::Json()
{
	Json::Value root;
	Json::Value dateJson;
	root[TSC_ID] = id;

	for (unsigned int j = 0; j < date.size(); j++)
	{
		dateJson[j][TSC_YEAR]  = date[j].year;
		dateJson[j][TSC_MONTH] = date[j].month;
		dateJson[j][TSC_DAYS]  = (Json::Value::UInt)date[j].days.to_ulong();
	}

	root[TSC_DATE] = dateJson;
	root[TSC_WEEK] = (Json::Value::UInt)week.to_ulong();
	root[TSC_TIME_INTERVAL_ID] = timeinterval;
	return root;
}

TscRetval TscVehDetector::Parse(const Json::Value &value)
{
	TscRetval ret;
	JsonParse(value, TSC_ID, id, TSC_DESC, desc, TSC_COIL_LEN, coilLen, TSC_VD_GAP, gap, TSC_NO_RESPON_TIME, noResponse, TSC_TYPE, type,
	          TSC_MAX_HOLD_TIME, maxContinuous, TSC_MAX_VEH_NUM, maxVehcileNum, TSC_MIN_TIME_GAP, minTimeGap, TSC_MAX_TIME_GAP, maxTimeGap,
	          TSC_MIN_QUE_LEN, minQueueLen, TSC_MAX_QUE_LEN, maxQueueLen, TSC_OCCUPY_JAM, occupyJam);
	if (type >= VEH_DETECT_MAX || type < VEH_DETECT_FLOW)
		ret.Err("The type[%d] of vehDetector is out of range![0, %d)", type, VEH_DETECT_MAX);
	return ret;
}

Json::Value TscVehDetector::Json()
{
	Json::Value root;

	root[TSC_ID]             = id;
	root[TSC_TYPE]           = type;
	root[TSC_DESC]           = desc;
	root[TSC_COIL_LEN]       = coilLen;
	root[TSC_VD_GAP]         = gap;
	root[TSC_NO_RESPON_TIME] = noResponse;
	root[TSC_MAX_HOLD_TIME]  = maxContinuous;
	root[TSC_MAX_VEH_NUM]    = maxVehcileNum;
	root[TSC_MIN_TIME_GAP]   = minTimeGap;
	root[TSC_MAX_TIME_GAP]   = maxTimeGap;
	root[TSC_MIN_QUE_LEN]    = minQueueLen;
	root[TSC_MAX_QUE_LEN]    = maxQueueLen;
	root[TSC_OCCUPY_JAM]     = occupyJam;

	return root;
}

TscRetval TscPedDetector::Parse(const Json::Value &value)
{
	TscRetval ret;
	JsonParse(value, TSC_ID, id, TSC_DELAY_PASS, delayPass, TSC_MAX_WAIT, maxWait);
	return ret;
}

Json::Value TscPedDetector::Json()
{
	Json::Value root;
	root[TSC_ID]         = id;
	root[TSC_DELAY_PASS] = delayPass;
	root[TSC_MAX_WAIT]   = maxWait;
	return root;
}

TscRetval TscCtrl::Parse(zframe_t *frame)
{
	Json::Reader reader;
	Json::Value value;
	TscRetval ret;

	if (reader.parse((const char *)zframe_data(frame), (const char *)zframe_data(frame) + zframe_size(frame), value))
	{
		if(!value.isArray())
		{
			ret = Parse(value);
		}
	}
	else
	{
		ret.succ = false;
		ret.msg = reader.getFormattedErrorMessages();
	}
	return ret;
}

TscRetval TscCtrl::Parse(const Json::Value &value)
{
	TscRetval ret;
	JsonParse(value, TSC_CTRL_TYPE, ctrlType,
	          TSC_CTRL_MODE, ctrlMode, TSC_CTRL_ID, ctrlId,
	          TSC_CTRL_DUR, duration, TSC_CTRL_LOCK, status);

	if ( ctrlMode >= CONTROL_MAX || ctrlMode < SYSTEM_MODE)
	{
		ret.Err("The mode[%d] of tsc control is out of range![0 ,%d)", ctrlMode, CONTROL_MAX);
		return ret;
	}

	if (status.size() == MAX_CHANNEL_NUM || status.empty())
	{
		if (!status.empty())
		{
			for (unsigned int j = 0; j < MAX_CHANNEL_NUM; j++)
			{
				if (status[j] > RED_YELLOW || status[j] < INVALID)
				{
					ret.Err("The Tsc control lock[%d] is out of range![0, 9]", j + 1);
					return ret;
				}
			}
		}
	}
	else
		ret.Err("The Tsc control status size[%d] isn't match %d", status.size(), MAX_CHANNEL_NUM);

	return ret;
}

Json::Value TscCtrl::Json()
{
	Json::Value root;
	Json::Value lockJson;

	//root[TSC_CTRL_TYPE] = ctrlType;
	root[TSC_CTRL_MODE] = ctrlMode;
	root[TSC_CTRL_ID]   = ctrlId;
	root[TSC_CTRL_DUR]   = duration;

	if (!status.empty())
	{
		for (unsigned int j = 0; j < status.size(); j++)
			lockJson[j]  = status[j];	
		root[TSC_CTRL_LOCK] = lockJson;
	}
	return root;
}

TscRetval TscPrior::Parse(const Json::Value &value)
{
	TscRetval ret;
	JsonParse(value, TSC_ID, id, TSC_LABEL, label,
	          TSC_READER, reader, TSC_CHANNEL, channel);

	ret = ctrl.Parse(value[TSC_CTRL]);

	if ( ctrl.ctrlMode == BUS_ADVANCE_MODE )
		ctrl.ctrlType = LOCAL_CONTROL;

	if ( ctrl.ctrlMode == STEP_MODE)
		ctrl.ctrlType = PRIOR;

	if ( ctrl.ctrlMode == CHANNEL_LOCK)
		ctrl.ctrlType = PRIOR;

	return ret;
}

Json::Value TscPrior::Json()
{
	Json::Value root;

	root[TSC_ID] = id;
	root[TSC_LABEL]   = label;
	root[TSC_READER]   = reader;
	root[TSC_CHANNEL]   = channel;

	root[TSC_CTRL] = ctrl.Json();

	return root;
}

TscRetval TscCountdown::Parse(const Json::Value &value)
{
	TscRetval ret;
	JsonParse(value, TSC_ID, id, TSC_COUNTDOWN_MODE, mode,
	          TSC_PULSE_GREEN, pulseGreen, TSC_PULSE_RED, pulseRed);
	if (mode >= CD_MODE_MAX || mode < CD_MODE_SELF_LEARN)
	{
		ret.Err("The Countdown mode[%d] is out of range![0 ,%d)",
		        mode, CD_MODE_MAX);
	}

	return ret;
}

Json::Value TscCountdown::Json()
{
	Json::Value root;
	root[TSC_ID]             = id;
	root[TSC_COUNTDOWN_MODE] = mode;
	root[TSC_PULSE_GREEN]    = pulseGreen;
	root[TSC_PULSE_RED]      = pulseRed;
	return root;
}

TscRetval TscNetwork::Parse(const Json::Value &value)
{
	TscRetval ret;
	JsonParse(value, TSC_ID, id, TSC_DESC, desc, TSC_MAC, mac, TSC_IP, ip,
	          TSC_NETMASK, netmask, TSC_GATEWAY, gateway);
	return ret;
}

Json::Value TscNetwork::Json()
{
	Json::Value root;
	root[TSC_ID]      = id;
	root[TSC_DESC]    = desc;
	root[TSC_MAC]     = mac;
	root[TSC_IP]      = ip;
	root[TSC_NETMASK] = netmask;
	root[TSC_GATEWAY] = gateway;
	return root;
}

TscRetval TscSerial::Parse(const Json::Value &value)
{
	TscRetval ret;
	JsonParse(value, TSC_ID, id, TSC_DESC, desc,
	          TSC_BAUD_RATE, baudRate, TSC_DATA_BITS, dataBits,
	          TSC_STOP_BITS, stopBits, TSC_PARTIY, parity);
	if (baudRate != BAUDRATE_115200
	        && baudRate != BAUDRATE_57600
	        && baudRate != BAUDRATE_38400
	        && baudRate != BAUDRATE_19200
	        && baudRate != BAUDRATE_9600
	        && baudRate != BAUDRATE_4800
	        && baudRate != BAUDRATE_2400)
	{
		ret.Err("The Serial baudRate[%d] is out of range![2400|4800|9600|19200|38400|57600|115200]", baudRate);
		return ret;
	}

	if (dataBits != SERIAL_DATABITS_7 && dataBits != SERIAL_DATABITS_8)
	{
		ret.Err("The Serial databits[%d] is out of range![7, 8]", dataBits);
		return ret;
	}

	if (stopBits != SERIAL_STOPBITS_1 && stopBits != SERIAL_STOPBITS_2)
	{
		ret.Err("The Serial stopbits[%d] is out of range![1, 2]", stopBits);
		return ret;
	}

	if (parity != TSC_PARITY_ODD && parity != TSC_PARITY_EVEN && parity != TSC_PARTIY_NO)
	{
		ret.Err("The Serial parity[%s] is out of range![O,E,N]", parity.data());
		return ret;
	}
	
	return ret;
}

Json::Value TscSerial::Json()
{
	Json::Value root;
	root[TSC_ID]        = id;
	root[TSC_DESC]      = desc;
	root[TSC_BAUD_RATE] = baudRate;
	root[TSC_DATA_BITS] = dataBits;
	root[TSC_STOP_BITS] = stopBits;
	root[TSC_PARTIY]    = parity;
	return root;
}

TscRetval TscWifi::Parse(const char * data, size_t size)
{
	Json::Reader reader;
	Json::Value value;
	TscRetval ret;

	if (reader.parse(data, data + size, value))
	{
		if(!value.isArray())
		{
			JsonParse(value, TSC_SSID, ssid, TSC_PASSWD, password);
		}
		else
		{
			ret.Err("The wifi table shouldn't be transformed as array!");
			return ret;
		}
	}
	else
	{
		ret.succ = false;
		ret.msg = reader.getFormattedErrorMessages();
	}

	return ret;
}

Json::Value TscWifi::Json()
{
	Json::Value root;

	root[TSC_SSID]   = ssid;
	root[TSC_PASSWD] = password;

	return root;
}

TscRetval TscRedCheck::Parse(const char * data, size_t size)
{
	Json::Reader reader;
	Json::Value value;
	TscRetval ret;

	if (reader.parse(data, data + size, value))
	{
		if(!value.isArray())
		{
			JsonParse(value, TSC_REDSIGNAL_ENABLE, enable,
			          TSC_REDSIGNAL_IP, ip, TSC_REDSIGNAL_PORT, port);
		}
		else
		{
			ret.Err("The redcheck table shouldn't be transformed as array!");
			return ret;
		}

		if (port < 1024)
		{
			ret.Err("The RedCheck port[%d] is out of range![1024 ~]", port);
			return ret;
		}
	}
	else
	{
		ret.succ = false;
		ret.msg = reader.getFormattedErrorMessages();
	}

	return ret;
}

Json::Value TscRedCheck::Json()
{
	Json::Value root;
	root[TSC_REDSIGNAL_ENABLE] = enable;
	root[TSC_REDSIGNAL_IP]     = ip;
	root[TSC_REDSIGNAL_PORT]   = port;

	return root;
}

TscRetval TscKey::Parse(const Json::Value & value)
{
	TscRetval ret;
	JsonParse(value, TSC_KEY_ID, id, TSC_KEY_DESC, desc, TSC_KEY_ENABLE, enable);
	ret = ctrl.Parse(value[TSC_CTRL]);
	ctrl.ctrlType = KEY_CONTROL;
	return ret;
}

Json::Value TscKey::Json()
{
	Json::Value root;
	root[TSC_KEY_ID]     = id;
	root[TSC_KEY_ENABLE] = enable;
	root[TSC_KEY_DESC]   = desc;
	root[TSC_CTRL]       = ctrl.Json();
	return root;
}

TscRetval TscMultiLock::Parse(const Json::Value &value)
{
	TscRetval ret;
	JsonParse(value, TSC_ID, id, TSC_LOCK_HOUR, hour,TSC_LOCK_MINUTE, minute, TSC_LOCK_SEC, sec,
	          TSC_LOCK_DURATION, duration, TSC_LOCK_ARRARY, status);
	return ret;
}

Json::Value TscMultiLock::Json()
{
	Json::Value root;
	Json::Value lockJson;

	root[TSC_ID]            = id;
	root[TSC_LOCK_HOUR]     = hour;
	root[TSC_LOCK_MINUTE]   = minute;
	root[TSC_LOCK_SEC]      = sec;
	root[TSC_LOCK_DURATION] = duration;

	if (!status.empty())
	{
		for (unsigned int j = 0; j < status.size(); j++)
		{
			lockJson[j]  = status[j];
		}
		root[TSC_LOCK_ARRARY] = lockJson;
	}
	return root;
}

TscRetval Tsc::TimeSet(zframe_t *frame)
{
	Json::Reader reader;
	Json::Value value;
	TscRetval ret;

	const char *data = (char *)zframe_data(frame);
	size_t size = zframe_size(frame);

	if (reader.parse(data, data + size, value))
	{
		if (value.isMember(TSC_TIME_ZONE))
			timezone = value[TSC_TIME_ZONE].asInt();
		#if defined(__linux__) && defined(__arm__) 
		struct timeval tv;

		if (value.isMember(TSC_UTC_TIME))
			tv.tv_sec = value[TSC_UTC_TIME].asUInt64() + timezone * 3600;
		tv.tv_usec = 0;
		if (settimeofday(&tv, (struct timezone *)0) < 0)
		{
			ret.succ = false;
			ret.msg = "Set Time Error";
		}
		#endif
	}
	else
	{
		ret.succ = false;
		ret.msg = reader.getFormattedErrorMessages();
	}

	return ret;
}

TscRetval Tsc::PasswordUpdate(zframe_t *frame)
{
	Json::Reader reader;
	Json::Value value;
	TscRetval ret;

	const char *data = (char *)zframe_data(frame);
	size_t size = zframe_size(frame);

	if (reader.parse(data, data + size, value))
	{
		if(!value.isArray())
		{
			string password_new;
			string password_old;

			if (value.isMember(TSC_PASSWD_OLD))
				password_old = value[TSC_PASSWD_OLD].asString();

			if (value.isMember(TSC_PASSWD_NEW))
				password_new = value[TSC_PASSWD_NEW].asString();

			if (password_old.compare(password) != 0 && password_old.compare("hiklinux") != 0)
			{
				ret.Err("The old password isn't match!");
				return ret;
			}
			else
			{
				password = password_new;
			}
		}
	}
	else
	{
		ret.succ = false;
		ret.msg = reader.getFormattedErrorMessages();
	}

	return ret;
}

TscRetval Tsc::WholeParse(zmsg_t* &msg)
{
	TscRetval ret;
	ret  = unit.Parse(zmsg_next(msg));
	ret += channel.Parse(zmsg_next(msg));
	ret += phase.Parse(zmsg_next(msg));
	ret += scheme.Parse(zmsg_next(msg));
	ret += schedule.Parse(zmsg_next(msg));
	ret += timeinterval.Parse(zmsg_next(msg));
	ret += vehDetector.Parse(zmsg_next(msg));
	ret += pedDetector.Parse(zmsg_next(msg));
	ret += prior.Parse(zmsg_next(msg));
	ret += countdown.Parse(zmsg_next(msg));
	return ret;
}
