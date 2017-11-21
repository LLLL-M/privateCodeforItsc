#!/usr/bin/python
# coding=gbk

####################################################################
#���ߣ�jaymade
#�汾��V1.0 build20160914
#���ܣ�ʵ���źŻ��豸�˵�����������/home��������־����
####################################################################

'''/////////////////////////////////////////////////////////////////  
//                            _ooOoo_                             //  
//                           o8888888o                            //      
//                           88" . "88                            //      
//                           (| -_- |)                            //      
//                           O\  =  /O                            //  
//                        ____/`---'\____                         //                          
//                      .'  \\|     |//  `.                       //  
//                     /  \\|||  :  |||//  \                      //      
//                    /  _||||| -:- |||||-  \                     //  
//                    |   | \\\  -  /// |   |                     //  
//                    | \_|  ''\---/''  |   |                     //          
//                    \  .-\__  `-`  ___/-. /                     //          
//                  ___`. .'  /--.--\  `. . ___                   //      
//                ."" '<  `.___\_<|>_/___.'  >'"".                //  
//              | | :  `- \`.;`\ _ /`;.`/ - ` : | |               //      
//              \  \ `-.   \_ __\ /__ _/   .-` /  /               //  
//        ========`-.____`-.___\_____/___.-`____.-'========       //      
//                             `=---='                            //  
//        ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^       //  
//         ���汣��                             ����BUG           //  
/////////////////////////////////////////////////////////////////''' 

#(When I wrote this, only God and I understood what I was doing)

import paramiko
import threading
import os
import configparser
import time
import encodings.idna              #����idna���룬��������exeֱ��ʱ�ᱨ��ȱ�ٸñ���
from ftplib import FTP

#��Ҫ�������źŻ�IP��ַ
tsc_ip = []

#�źŻ���ʱ����ʱ��
sleep_time = 0

#�������ļ�������Ҫ�����������.gz'ͬһĿ¼�£���DS-TSC500�źŻ���Ӧ��������Ϊhiktsc-update.tar.gz��DS-TSC300�źŻ���Ӧ��������Ϊhiktsc300-update.tar.gz
update_file_name = 'hiktsc300-update.tar.gz'


####################################################################
#���ܣ�ftp�ϴ�
####################################################################
def ftpUpload(ip='192.168.1.102',uploadfilepath='SetIP.sh',remotepath='/opt/'):
    try:
        ftp=FTP()
        #ftp.set_debuglevel(2)    #�򿪵��Լ���2����ʾ��ϸ��Ϣ  
        ftp.connect(ip,21)                
        ftp.login('root','hiklinux')   #you can not see me
        bufsize = 1024
        fp = open(uploadfilepath,'rb')
        ftp.storbinary('STOR '+ remotepath + uploadfilepath,fp,bufsize)  #upload file
        ftp.set_debuglevel(0)                                           
        fp.close()                                                      
        ftp.quit() 
        print ('%s �ϴ��������ɹ�'%(ip))
    except Exception as e:
        print ('%s �ϴ�������ʧ��:%s \n'%(ip,e))


####################################################################
#���ܣ�sftp�ϴ�
####################################################################
def sftpUpload(ip='192.168.1.102',uploadfilepath='SetIP.sh',remotepath='/opt/'):
    try:
        t = paramiko.Transport((ip,22))
        t.connect(username = "root", password = "hiklinux")
        sftp = paramiko.SFTPClient.from_transport(t)
        sftp.put(uploadfilepath,remotepath + uploadfilepath)
        t.close()
        print ('%s �ϴ��������ɹ�'%(ip))      
    except Exception as e:
        print ('%s �ϴ�������ʧ��:%s \n'%(ip,e))


####################################################################
#���ܣ�ssh��¼��ִ�������ű��������źŻ�
#��ע���޷�ֱ���������س����޷���λԭ�򣬸�paramiko���йأ�
####################################################################
def ssh2(ip):
    try: 
        username = 'root'
        passwd = 'hiklinux'
          
        ssh = paramiko.SSHClient()
        ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        ssh.connect(ip,22,username,passwd,timeout=5)
        
        #�����ֱ�����������ֶ�ֱ����������֧��TSC500��TSC300�ͺŻ������������ù��߲�֧�֣�
        if sleep_time == 0:
            cmd = 'cd /opt &&tar -zxvf ' + update_file_name + ' && cd ' + update_file_name.replace('.tar.gz','  ') + \
            '&& sh update.sh && rm -f /opt/' + update_file_name
            #print(cmd)
            stdin, stdout, stderr = ssh.exec_command(cmd) 
            out = stdout.readlines()
            print('%s �����ű�ִ�гɹ�'%(ip))

        stdin, stdout, stderr = ssh.exec_command('reboot -d ' + str(sleep_time) + '&') 
        #out = stdout.readlines()
        #��Ļ���
        #for o in out:            
        #    print (o.replace("\n","")),
        ssh.close()
        print ('%s �����ɹ�\n'%(ip))
    except Exception as e:
        print ('%s ����ʧ��:%s\n'%(ip,e))



####################################################################
#���ܣ��߳���ں�������ftp�ϴ�����������ͨ��SSH������������
####################################################################
def upload(ip):
    global update_file_name    
    #ftpUpload(ip,update_file_name,remotepath='/opt/')     #ftp�ϴ�
    sftpUpload(ip,update_file_name,remotepath='/opt/')     #sftp�ϴ�
    ssh2(ip) 

####################################################################
#���ܣ���config.ini�ļ��л�ȡ��Ҫ�������źŻ�IP���Զ��ж�����������
####################################################################
def loadconfig():
    global update_file_name
    try: 
        if os.path.exists('./hiktsc300-update.tar.gz'):  
            update_file_name = 'hiktsc300-update.tar.gz'            
        else:  
            update_file_name = 'hiktsc-update.tar.gz'
        print ('����������:%s'%(update_file_name))
        
        file = open ("config.ini")
        for line in file:
            line = line.replace("\n","")
            if bool(line) == True :           
                tsc_ip.append(line)
            #print (tsc_ip)
    except Exception as e:
        print ('���������ļ�ʧ��:%s\n'%(e))


####################################################################
#���ܣ����źŻ�������/home�������ļ���ѹ��
####################################################################
def downloadconfig(ip):
    try: 
        username = 'root'
        passwd = 'hiklinux'
        #��ѹ��������ִ�������ű���Ȼ������
        cmd = 'tar -zcvf /opt/' + ip +'_home.tar.gz /home  &'
        ssh = paramiko.SSHClient()
        ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        ssh.connect(ip,22,username,passwd,timeout=5)
        stdin, stdout, stderr = ssh.exec_command(cmd) 
        out = stdout.readlines()
        #��Ļ���
        for o in out:            
            print (o.replace("\n","")),
        ssh.close()
        print ('%s /homeĿ¼ѹ���ɹ�\n'%(ip))
        ftp=FTP()
        ftp.set_debuglevel(2)    #�򿪵��Լ���2����ʾ��ϸ��Ϣ  
        ftp.connect(ip,21)                
        ftp.login('root','hiklinux')   #you can not see me
        bufsize = 1024
        fp = open(ip + '_home.tar.gz','wb')
        ftp.retrbinary('RETR ' + '/opt/'+ ip +'_home.tar.gz',fp.write,bufsize) #���շ��������ļ���д�뱾���ļ�  
        ftp.set_debuglevel(0)                                           
        fp.close()                                                      
        ftp.quit() 
        print ('\n%s_home.tar.gz��ȡ�ɹ��������ڵ�ǰĿ¼\n'%(ip))
        input("������������˳�")
    except Exception as e:
        print ('\n%s_home.tar.gz��ȡʧ��:%s\n'%(ip,e))
        input("������������˳�")


####################################################################
#���ܣ�main�������
####################################################################
if __name__=='__main__':
    print ("HIKTSC_TOOLS V1.0 build20160914\n")
    print ("#########################################################")
    print ("����1����config.ini�ļ��������źŻ�������������")
    print ("����2�������źŻ�/home���ļ�ѹ����������")
    print ("#########################################################\n")
    inputnum = input("��������ţ�")
    if inputnum == '1':
        loadconfig()
        print("\n��ʼ�������źŻ�����������")
        print (tsc_ip)
        print (" ")
        inputsec = input("�������Ƿ���Ҫ��ʱ������\n����������������س�:\n��ʱ��������������ʱ����ʱ�䣨��λСʱ����")
        if inputsec == '':
            print ('\n��������')
        else:
            sleep_time = 3600 * int(inputsec)
            print ('\n��ʱ%sСʱ������\n'%int(inputsec))
        
        for each_ip in tsc_ip: 
            thread=threading.Thread(target=upload,args=(each_ip,))
            thread.start()

        while True:          #��������ʱ���ֳ���ʼ�ղ��˳�
            pass
            time.sleep(10) 
    elif inputnum == '2':
        inputip =input("�������źŻ�IP��")
        print ('\n���������У����Ե�Ƭ��...')
        downloadconfig(inputip)       #�����źŻ�/home�������ļ�������
    else:
        print ("��Ч�����")
     
    
#(Now, only God knows)
