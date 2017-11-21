#ifndef __LCB_H
#define __LCB_H

/*	�ƿذ������λ�ӹܵ�����ǰ������
	���֧��8����λ��8����ͨ����(ÿ����������ʹ�ò�ͬ�����űȺ�����)
	����Ҳ֧�ָ�Ӧ���Ƶ��޷�ӹ�
	�������õ���λ�����1��ʼ������ֻ����1��λ��3��λ�����������֧��
	��ͨ����������Ҳ�����Ǵ�1��ʼ�����ģ�����ֻ�з���1�ͷ���3�����Ҳ�ǲ���֧�ֵ�
	֧��˫�����޷�ӹܣ�����λ������Ȼ���ܳ���8��
	֧���޷�ӹܵĹ��ܿ����ã������ڽӹܹ����п��Խ��к��̳�ͻ���̳�ͻ�����Ϩ��Ĺ��ϼ�⣬���ҹ��ϼ��Ҳ������
	�޷�ӹܲ�����ĳ��ƿذ���нӹܣ�����������õ�ͨ���������������һ���ƿذ�û������ͨ������ô��ʹ�õڶ����ƿذ���нӹܣ�����ڶ����ƿذ���Ȼû������ͨ����ʹ�õ��������Դ�����
	
	��֧�������к�����λ�Ľӹ�
	���ڸ�����λ�ӹܺ��޷�������������������λһֱ�̵Ƶ�������м����м��
	
	���¼���������ܻ᲻׼��
	1.�������ú����û�м��һ�����ھͽ��нӹ�
	2.Э�����ƵĹ�������
 */
 
 
/* light control board���LCB */
#define MAX_SUPPORT_PHASE_NUM		8	//���֧����λ����
#define MAX_SUPPORT_SCHEME_NUM		8	//���֧�ֵ���ͨ��������
/* ������ͨ����N��Ӧ�����N�����űȱ�N,�ټ���Ĭ�ϰѸ�Ӧ������Ϊ��0����,
   ���������������������űȸ�����Ϊ(��󷽰��ĸ��� + 1) */
#define MAX_SUPPORT_PHASETURN_NUM	(MAX_SUPPORT_SCHEME_NUM + 1)	//���֧�ֵ��������
#define MAX_SUPPORT_SPLIT_NUM		(MAX_SUPPORT_SCHEME_NUM	+ 1)	//���֧�ֵ����űȸ���

/*	ʹ����չcan id,��29λ,ֻ����18bit�����嶨������:
	17-12(no)		11-8(index)			7-4(type)				3-0(id)
	����can���ݰ�	����������ű�	0:������Ϣ,1:��λ��Ϣ		0xf
	�ı��,��0��ʼ	��������[0-7]	2:������Ϣ,3:���ű���Ϣ		������֮ǰ��can id��ͻ	
									4:ÿ�뷢�͵�������Ϣ,�������޷�ӹ�ʹ��
	����˳��Ϊ:������Ϣ-->��λ��Ϣ-->������Ϣ-->���ű���Ϣ	*/
#define LCB_CONFIG_ID	0x0f
#define LCB_CONFIG_MASK	LCB_CONFIG_ID
#define LCB_CAN_EXTID(no,index,type)	((1 << 31) | (no << 12) | (index << 8) | (type << 4) | LCB_CONFIG_ID)	//��չid�ĵ�31bit����Ϊ1
#define LCB_BASEINFO_CAN				LCB_CAN_EXTID(0, 0, BASEINFO)
#define LCB_PHASEINFO_CAN(no,index)		LCB_CAN_EXTID(no, index, PHASEINFO)
#define LCB_PHASETURNINFO_CAN(no,index)	LCB_CAN_EXTID(no, index, PHASETURNINFO)
#define LCB_SPLITINFO_CAN(no,index)		LCB_CAN_EXTID(no, index, SPLITINFO)

typedef struct
{
	uint32_t id:4;
	uint32_t type:4;
	uint32_t index:4;
	uint32_t no:6;
	uint32_t flag:2;
	uint32_t :12;
} CanExtId;

typedef enum
{
	BASEINFO = 0,
	PHASEINFO = 1,
	PHASETURNINFO = 2,
	SPLITINFO = 3,
	RUNINFO = 4,
	REBOOT = 5,
} LCBcanDataType;

typedef struct _LCB_phase_info_
{
	uint64_t greenFlashTime:8;		//����ʱ��
	uint64_t yellowTime:8;			//�Ƶ�ʱ��
	uint64_t allredTime:8;			//ȫ��ʱ��
	uint64_t pedFlashTime:8;		//��������ʱ��
	uint64_t channelbits:16;		//��λ��Ӧ��ͨ��,bit[0-15]�ֱ��Ӧͨ��1-16
} LCBphaseinfo;

struct _LCB_phaseturn_info_
{	//��������ж�Ӧ˳�����λ,ÿ�ֽڵ���λΪ��1����λ,����λΪ��2����λ
	uint64_t first:8;
	uint64_t second:8;
	uint64_t third:8;
	uint64_t fourth:8;
	uint64_t fifth:8;
	uint64_t sixth:8;
	uint64_t seventh:8;
	uint64_t eighth:8;
};
typedef union
{
	struct _LCB_phaseturn_info_ turn;
	uint8_t phases[MAX_SUPPORT_PHASE_NUM];
} LCBphaseturninfo;

struct _LCB_split_info_
{	//���ű���Ϣ,���1-8��λ�����ű�
	uint64_t phase1:8;
	uint64_t phase2:8;
	uint64_t phase3:8;
	uint64_t phase4:8;
	uint64_t phase5:8;
	uint64_t phase6:8;
	uint64_t phase7:8;
	uint64_t phase8:8;
};
typedef union
{
	struct _LCB_split_info_ split;
	uint8_t times[MAX_SUPPORT_PHASE_NUM];
} LCBsplitinfo;

typedef enum _light_value_
{
	LOFF = 0,
	LGREEN = 1,
	LRED = 2,
	LYELLOW = 4,
	LGREEN_FLASH = 5,
	LYELLOW_FLASH = 6,
	LGREEN_FLASH_PED = 7,	//������������ʱ��������λ���̵�
} LightValue;

typedef struct _LCB_run_info_
{
	uint64_t schemeid:8;		//��ǰ���еķ�����
	uint64_t runtime:8;			//��ǰ���е�ʱ��,��0��ʼ
	uint64_t phaseR1:8;			//��1��ʱ���е���λ
	uint64_t phaseR2:8;			//��2��ʱ���е���λ
	uint64_t lightvalueR1:4;	//��1��ʱ������λ�ĵ��ֵ,��ֵ�ο� enum _light_value_
	uint64_t lightvalueR2:4;	//��2��ʱ������λ�ĵ��ֵ,��ֵ�ο� enum _light_value_
} LCBruninfo;

typedef struct _LCB_base_info_
{
	uint64_t isTakeOver:1;			//�Ƿ�ӹܿ���
	uint64_t isVoltCheckEnable:1;	//�Ƿ�����ѹ���,0:�ر�,1:����
	uint64_t isCurCheckEnable:1;	//�Ƿ����������,0:�ر�,1:����
	uint64_t phaseNum:5;			//��λ����
	uint64_t minRedCurVal:6;		//��С��Ƶ���ֵ,�������ֵ����Ϊ�޵���
	uint64_t schemeNum:4;			//��ͨ�����ĸ�����������������ȫ�졢�صơ���Ӧ����
	uint64_t canTotalNum:6;			//��ȥ�����1��������Ϣ���������ܹ����͸��ƿذ��can���ݰ�����
	uint64_t :40;					//����40bit
} LCBbaseinfo;

typedef struct _LCB_config_
{
	LCBbaseinfo			baseinfo;
	LCBphaseinfo		phaseinfo[MAX_SUPPORT_PHASE_NUM];		//��λ�������Ϣ
	LCBphaseturninfo	phaseturninfo[MAX_SUPPORT_PHASETURN_NUM];	//����������Ϣ
	LCBsplitinfo		splitinfo[MAX_SUPPORT_SPLIT_NUM];		//���űȵ������Ϣ
#define CONFIG_IS_READY		(0x1234)
#define CONFIG_UPDATE		(0x2345)
	uint32_t configstate:16;							//���õ�״̬,����ֵΪ����������궨��
	uint32_t controlBoardNo:16;							//�ӹܿ��Ƶĵƿذ��
	uint32_t allchannelbits;							//���õ�����ͨ��
} LCBconfig;

static __INLINE int CheckLCBconfigValidity(LCBconfig *p)
{
	int i, j;
	int totalNum = p->baseinfo.schemeNum * 2 + 1 
					+ p->baseinfo.phaseNum;
	
	for (i = 0; i < MAX_SUPPORT_PHASE_NUM; i++)
	{
		if (p->phaseinfo[i].channelbits == 0)
			break;
	}
	for (j = 0; j < MAX_SUPPORT_PHASETURN_NUM; j++)
	{
		if (p->phaseturninfo[j].turn.first == 0
		|| p->splitinfo[j].split.phase1 == 0)
			break;
	}
	return (i > 0 && j > 0
			&& totalNum == p->baseinfo.canTotalNum
			&& i == p->baseinfo.phaseNum 
			&& j == p->baseinfo.schemeNum + 1);
}

#endif
