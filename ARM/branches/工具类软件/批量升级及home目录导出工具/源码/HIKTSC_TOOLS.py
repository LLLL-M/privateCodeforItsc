#!/usr/bin/python
# coding=gbk

####################################################################
#作者：jaymade
#版本：V1.0 build20160914
#功能：实现信号机设备端的批量升级和/home下配置日志导出
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
//         佛祖保佑                             永无BUG           //  
/////////////////////////////////////////////////////////////////''' 

#(When I wrote this, only God and I understood what I was doing)

import paramiko
import threading
import os
import configparser
import time
import encodings.idna              #增加idna编码，否则打包成exe直行时会报错缺少该编码
from ftplib import FTP

#需要升级的信号机IP地址
tsc_ip = []

#信号机延时重启时间
sleep_time = 0

#升级包文件名（需要跟程序放置在.gz'同一目录下），DS-TSC500信号机对应升级包名为hiktsc-update.tar.gz，DS-TSC300信号机对应升级包名为hiktsc300-update.tar.gz
update_file_name = 'hiktsc300-update.tar.gz'


####################################################################
#功能：ftp上传
####################################################################
def ftpUpload(ip='192.168.1.102',uploadfilepath='SetIP.sh',remotepath='/opt/'):
    try:
        ftp=FTP()
        #ftp.set_debuglevel(2)    #打开调试级别2，显示详细信息  
        ftp.connect(ip,21)                
        ftp.login('root','hiklinux')   #you can not see me
        bufsize = 1024
        fp = open(uploadfilepath,'rb')
        ftp.storbinary('STOR '+ remotepath + uploadfilepath,fp,bufsize)  #upload file
        ftp.set_debuglevel(0)                                           
        fp.close()                                                      
        ftp.quit() 
        print ('%s 上传升级包成功'%(ip))
    except Exception as e:
        print ('%s 上传升级包失败:%s \n'%(ip,e))


####################################################################
#功能：sftp上传
####################################################################
def sftpUpload(ip='192.168.1.102',uploadfilepath='SetIP.sh',remotepath='/opt/'):
    try:
        t = paramiko.Transport((ip,22))
        t.connect(username = "root", password = "hiklinux")
        sftp = paramiko.SFTPClient.from_transport(t)
        sftp.put(uploadfilepath,remotepath + uploadfilepath)
        t.close()
        print ('%s 上传升级包成功'%(ip))      
    except Exception as e:
        print ('%s 上传升级包失败:%s \n'%(ip,e))


####################################################################
#功能：ssh登录后执行升级脚本并重启信号机
#备注：无法直接启动主控程序（无法定位原因，跟paramiko库有关）
####################################################################
def ssh2(ip):
    try: 
        username = 'root'
        passwd = 'hiklinux'
          
        ssh = paramiko.SSHClient()
        ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        ssh.connect(ip,22,username,passwd,timeout=5)
        
        #如果是直接升级，则手动直行升级包，支持TSC500和TSC300型号互相升级（配置工具不支持）
        if sleep_time == 0:
            cmd = 'cd /opt &&tar -zxvf ' + update_file_name + ' && cd ' + update_file_name.replace('.tar.gz','  ') + \
            '&& sh update.sh && rm -f /opt/' + update_file_name
            #print(cmd)
            stdin, stdout, stderr = ssh.exec_command(cmd) 
            out = stdout.readlines()
            print('%s 升级脚本执行成功'%(ip))

        stdin, stdout, stderr = ssh.exec_command('reboot -d ' + str(sleep_time) + '&') 
        #out = stdout.readlines()
        #屏幕输出
        #for o in out:            
        #    print (o.replace("\n","")),
        ssh.close()
        print ('%s 升级成功\n'%(ip))
    except Exception as e:
        print ('%s 升级失败:%s\n'%(ip,e))



####################################################################
#功能：线程入口函数，先ftp上传升级包，再通过SSH进行升级重启
####################################################################
def upload(ip):
    global update_file_name    
    #ftpUpload(ip,update_file_name,remotepath='/opt/')     #ftp上传
    sftpUpload(ip,update_file_name,remotepath='/opt/')     #sftp上传
    ssh2(ip) 

####################################################################
#功能：从config.ini文件中获取需要升级的信号机IP并自动判断升级包名称
####################################################################
def loadconfig():
    global update_file_name
    try: 
        if os.path.exists('./hiktsc300-update.tar.gz'):  
            update_file_name = 'hiktsc300-update.tar.gz'            
        else:  
            update_file_name = 'hiktsc-update.tar.gz'
        print ('升级包名称:%s'%(update_file_name))
        
        file = open ("config.ini")
        for line in file:
            line = line.replace("\n","")
            if bool(line) == True :           
                tsc_ip.append(line)
            #print (tsc_ip)
    except Exception as e:
        print ('加载配置文件失败:%s\n'%(e))


####################################################################
#功能：从信号机内下载/home下所有文件并压缩
####################################################################
def downloadconfig(ip):
    try: 
        username = 'root'
        passwd = 'hiklinux'
        #解压升级包并执行升级脚本，然后重启
        cmd = 'tar -zcvf /opt/' + ip +'_home.tar.gz /home  &'
        ssh = paramiko.SSHClient()
        ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        ssh.connect(ip,22,username,passwd,timeout=5)
        stdin, stdout, stderr = ssh.exec_command(cmd) 
        out = stdout.readlines()
        #屏幕输出
        for o in out:            
            print (o.replace("\n","")),
        ssh.close()
        print ('%s /home目录压缩成功\n'%(ip))
        ftp=FTP()
        ftp.set_debuglevel(2)    #打开调试级别2，显示详细信息  
        ftp.connect(ip,21)                
        ftp.login('root','hiklinux')   #you can not see me
        bufsize = 1024
        fp = open(ip + '_home.tar.gz','wb')
        ftp.retrbinary('RETR ' + '/opt/'+ ip +'_home.tar.gz',fp.write,bufsize) #接收服务器上文件并写入本地文件  
        ftp.set_debuglevel(0)                                           
        fp.close()                                                      
        ftp.quit() 
        print ('\n%s_home.tar.gz获取成功，保存在当前目录\n'%(ip))
        input("按两次任意键退出")
    except Exception as e:
        print ('\n%s_home.tar.gz获取失败:%s\n'%(ip,e))
        input("按两次任意键退出")


####################################################################
#功能：main函数入口
####################################################################
if __name__=='__main__':
    print ("HIKTSC_TOOLS V1.0 build20160914\n")
    print ("#########################################################")
    print ("输入1：将config.ini文件中所有信号机进行批量升级")
    print ("输入2：上载信号机/home下文件压缩包到本地")
    print ("#########################################################\n")
    inputnum = input("请输入序号：")
    if inputnum == '1':
        loadconfig()
        print("\n开始对以下信号机进行升级：")
        print (tsc_ip)
        print (" ")
        inputsec = input("升级后是否需要延时重启？\n立刻重启，请输入回车:\n延时重启，请输入延时重启时间（单位小时）：")
        if inputsec == '':
            print ('\n立刻重启')
        else:
            sleep_time = 3600 * int(inputsec)
            print ('\n延时%s小时后重启\n'%int(inputsec))
        
        for each_ip in tsc_ip: 
            thread=threading.Thread(target=upload,args=(each_ip,))
            thread.start()

        while True:          #批量升级时保持程序始终不退出
            pass
            time.sleep(10) 
    elif inputnum == '2':
        inputip =input("请输入信号机IP：")
        print ('\n正在上载中，请稍等片刻...')
        downloadconfig(inputip)       #下载信号机/home下所有文件到本地
    else:
        print ("无效的序号")
     
    
#(Now, only God knows)
