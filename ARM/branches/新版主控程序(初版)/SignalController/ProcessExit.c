/******************************************************************************

                  ��Ȩ���� (C), 2001-2014, HikVision

 ******************************************************************************
  �� �� ��   : ProcessExit.c
  �� �� ��   : ����
  ��    ��   : xiaowh
  ��������   : 2014��7��16��
  ����޸�   :
  ��������   : ��Ҫ�������������̻�ȡ�ӽ��̵�״̬����Ҫע���������ӽ���ͨ
               �����ctrl+c�˳��ӽ��̣������̱���Ҫ����ctrl+c�������źŵ�Ӱ
               �죬���򸸽��̿����Ѿ����˳������޷���⵽�ӽ��̵��˳�
  �����б�   :
              main
  �޸���ʷ   :
  1.��    ��   : 2014��7��16��
    ��    ��   : xiaowh
    �޸�����   : �����ļ�

******************************************************************************/


#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>



int main(void)
{
    pid_t nchildPid;
    int status = 0;

    nchildPid = fork();

    if(nchildPid < 0)
    {
        printf("fork error \n");
    }
    else if (nchildPid == 0)
    {
        printf("child process start ...\n");
    	signal(SIGHUP, SIG_IGN);
    	signal(SIGPIPE, SIG_IGN);
    	signal(SIGQUIT, SIG_IGN);
	    signal(SIGTERM, SIG_IGN);
	    
        sleep(3);
        printf("child process exit ....\n");
        exit(4);
    }
    else
    {
        printf("parent .....\n");
        waitpid(nchildPid ,&status,0);
        printf("catch child process exit , status 0x%x\n",status);
        

    }



    return 0;
}



