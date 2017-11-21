# -*- coding: utf-8 -*-

# Created by: PyQt5 UI code generator 5.6
#
# WARNING! All changes made in this file will be lost!

####################################################################
#作者：jaymade
#版本：V2.0 build20171030
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

from PyQt5 import QtCore, QtGui, QtWidgets
from PyQt5.QtWidgets import QFileDialog, QMessageBox 
import paramiko
import threading
import os
import configparser
import time
import encodings.idna              #增加idna编码，否则打包成exe直行时会报错缺少该编码
from ftplib import FTP

#使用pyinstaller打包会提示找不到QT运行环境，需要将qwindows.dll与exe程序放置到同一目录，且加入这句增加QT环境变量
os.environ['QT_QPA_PLATFORM_PLUGIN_PATH'] = '.'


#需要升级的信号机IP地址
tsc_ip = []

#信号机延时升级时间
sleep_time = 0.0

#信号机升级完后是否需要立刻执行
is_reboot_now = True

#信号机升级包全路径
update_file_allpath = ''

#信号机升级包文件名
update_file_name = ''

#配置文件下载目录
savedirectory = ''

class Ui_updateForm(object):
    def setupUi(self, updateForm):
        updateForm.setObjectName("updateForm")
        updateForm.setWindowModality(QtCore.Qt.NonModal)
        updateForm.resize(449, 560)
        #限定窗口大小不让最大化和伸缩
        updateForm.setFixedSize(updateForm.width(), updateForm.height());

        self.updateButton = QtWidgets.QPushButton(updateForm)
        self.updateButton.setGeometry(QtCore.QRect(284, 147, 111, 31))
        self.updateButton.setObjectName("updateButton")
        self.ipEdit = QtWidgets.QTextEdit(updateForm)
        self.ipEdit.setGeometry(QtCore.QRect(30, 60, 201, 161))
        self.ipEdit.setObjectName("ipEdit")
        self.label = QtWidgets.QLabel(updateForm)
        self.label.setGeometry(QtCore.QRect(30, 20, 210, 31))
        self.label.setObjectName("label")
        self.line = QtWidgets.QFrame(updateForm)
        self.line.setGeometry(QtCore.QRect(30, 230, 391, 20))
        self.line.setFrameShape(QtWidgets.QFrame.HLine)
        self.line.setFrameShadow(QtWidgets.QFrame.Sunken)
        self.line.setObjectName("line")
        self.delayHourEdit = QtWidgets.QLineEdit(updateForm)
        self.delayHourEdit.setGeometry(QtCore.QRect(306, 94, 41, 20))
        self.delayHourEdit.setObjectName("delayHourEdit")
        self.label_3 = QtWidgets.QLabel(updateForm)
        self.label_3.setGeometry(QtCore.QRect(352, 89, 81, 31))
        self.label_3.setObjectName("label_3")
        self.radioRebootNowButton = QtWidgets.QRadioButton(updateForm)
        self.radioRebootNowButton.setGeometry(QtCore.QRect(260, 59, 200, 31))
        self.radioRebootNowButton.setAcceptDrops(True)
        self.radioRebootNowButton.setChecked(True)
        self.radioRebootNowButton.setObjectName("radioRebootNowButton")
        self.buttonGroup = QtWidgets.QButtonGroup(updateForm)
        self.buttonGroup.setObjectName("buttonGroup")
        self.buttonGroup.addButton(self.radioRebootNowButton)
        self.radioRebootAfterButton = QtWidgets.QRadioButton(updateForm)
        self.radioRebootAfterButton.setGeometry(QtCore.QRect(260, 89, 51, 31))
        self.radioRebootAfterButton.setObjectName("radioRebootAfterButton")
        self.buttonGroup.addButton(self.radioRebootAfterButton)
        self.line_2 = QtWidgets.QFrame(updateForm)
        self.line_2.setGeometry(QtCore.QRect(240, 30, 20, 201))
        self.line_2.setFrameShape(QtWidgets.QFrame.VLine)
        self.line_2.setFrameShadow(QtWidgets.QFrame.Sunken)
        self.line_2.setObjectName("line_2")
        self.label_2 = QtWidgets.QLabel(updateForm)
        self.label_2.setGeometry(QtCore.QRect(260, 20, 111, 31))
        self.label_2.setObjectName("label_2")
        self.line_3 = QtWidgets.QFrame(updateForm)
        self.line_3.setGeometry(QtCore.QRect(260, 120, 161, 16))
        self.line_3.setFrameShape(QtWidgets.QFrame.HLine)
        self.line_3.setFrameShadow(QtWidgets.QFrame.Sunken)
        self.line_3.setObjectName("line_3")
        self.label_4 = QtWidgets.QLabel(updateForm)
        self.label_4.setGeometry(QtCore.QRect(32, 243, 51, 31))
        self.label_4.setObjectName("label_4")
        self.getLogButton = QtWidgets.QPushButton(updateForm)
        self.getLogButton.setGeometry(QtCore.QRect(284, 187, 111, 31))
        self.getLogButton.setObjectName("getLogButton")
        self.textInfoBrowser = QtWidgets.QTextBrowser(updateForm)
        self.textInfoBrowser.setGeometry(QtCore.QRect(30, 280, 391, 261))
        self.textInfoBrowser.setFrameShape(QtWidgets.QFrame.Box)
        self.textInfoBrowser.setFrameShadow(QtWidgets.QFrame.Sunken)
        self.textInfoBrowser.setObjectName("textInfoBrowser")

        self.retranslateUi(updateForm)
        #changed by liujie 20171020
        self.updateButton.clicked.connect(self.updateFile)
        self.getLogButton.clicked.connect(self.getlogFile)
        QtCore.QMetaObject.connectSlotsByName(updateForm)

    def retranslateUi(self, updateForm):
        _translate = QtCore.QCoreApplication.translate
        updateForm.setWindowTitle(_translate("updateForm", "信号机批量升级工具V2.0"))
        #updateForm.setToolTip(_translate("updateForm", "<html><head/><body><p>123</p></body></html>"))
        #updateForm.setStatusTip(_translate("updateForm", "123"))
        self.updateButton.setText(_translate("updateForm", "升级"))
        #self.ipEdit.setToolTip(_translate("updateForm", "<html><head/><body><p>test</p></body></html>"))
        
        self.ipEdit.setHtml(_translate("updateForm", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:\'SimSun\'; font-size:9pt; font-weight:400; font-style:normal;\">\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">192.168.1.101</p>\n"
"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">192.168.109.103-192.168.109.105</p></body></html>"))
        loadconfig()
        self.ipEdit.setText("\n".join(list(tsc_ip)))
        self.label.setText(_translate("updateForm", "逐行输入需要升级或连接的信号机IP："))
        self.delayHourEdit.setText(_translate("updateForm", "0"))
        self.label_3.setText(_translate("updateForm", "小时升级"))
        self.radioRebootNowButton.setText(_translate("updateForm", "立即升级（无需重启）"))
        self.radioRebootAfterButton.setText(_translate("updateForm", "延迟"))
        self.label_2.setText(_translate("updateForm", "升级方式："))
        self.label_4.setText(_translate("updateForm", "说明："))
        self.getLogButton.setText(_translate("updateForm", "获取配置和日志"))
        self.textInfoBrowser.setHtml(_translate("updateForm", "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
"<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\n"
"p, li { white-space: pre-wrap; }\n"
"</style></head><body style=\" font-family:\'SimSun\'; font-size:9pt; font-weight:400; font-style:normal;\">\n</body></html>"))


    ####################################################################
    #功能：将升级包上传至信号机制定目录并开始升级
    ####################################################################
    def updateFile(self, updateForm):
        global tsc_ip
        self.textInfoBrowser.append("需要升级的信号机IP：")		
        iplist = self.ipEdit.toPlainText().strip()
        self.textInfoBrowser.append(iplist)
        configfile = open("./ipconfig.ini", 'w+')
        configfile.write(iplist)
        configfile.close()
        tsc_ip = iplist.split('\n')
        
        global update_file_allpath   
        update_file_allpath, file_type = QFileDialog.getOpenFileName(self,  
                                    "选择信号机设备端软件升级包",  
                                    '',  
                                    "tar.gz Files (*.tar.gz)")
									#'',
									#QFileDialog.DontUseNativeDialog)   #设置文件扩展名过滤,注意用双分号间隔 
        if(update_file_allpath.strip() == ""):
            return
        if(update_file_allpath.find('hiktsc') == -1):
            self.textInfoBrowser.append("升级包格式错误，请重新确认！")
            return
        button = QMessageBox.information(self,"升级包路径确认","升级包路径:"+ update_file_allpath,
                                      QMessageBox.Yes | QMessageBox.No)
        if button == QMessageBox.Yes:        
          self.textInfoBrowser.append("升级包路径：")
          self.textInfoBrowser.append(update_file_allpath)
          self.textInfoBrowser.append('************************开始升级**************************')
          global is_reboot_now
          is_reboot_now = self.radioRebootNowButton.isChecked()
          global sleep_time
          sleep_time = (float)(self.delayHourEdit.text())
          #多线程升级
          for each_ip in tsc_ip: 
              thread=threading.Thread(target=upload,args=(self,each_ip,))
              thread.start()

    ####################################################################
    #功能：获取信号机home下所有配置和日志并导出到本地
    ####################################################################
    def getlogFile(self, updateForm):
        global tsc_ip
        self.textInfoBrowser.setText("")
        self.textInfoBrowser.append("需要连接的信号机IP：")		
        iplist = self.ipEdit.toPlainText().strip()
        self.textInfoBrowser.append(iplist)
        configfile = open("./ipconfig.ini", 'w+')
        configfile.write(iplist)
        configfile.close()
        tsc_ip = iplist.split('\n')
        global savedirectory
        savedirectory = QFileDialog.getExistingDirectory(self,  
                                    "选取保存目录",  
                                    "")      
        if(savedirectory.strip() == ""):
            self.textInfoBrowser.append("未选择可用路径，请重新选择！")
            return
        self.textInfoBrowser.append("配置和日志保存目录：")
        self.textInfoBrowser.append(savedirectory)
        self.textInfoBrowser.append('************************开始获取**************************')
        for each_ip in tsc_ip: 
            thread=threading.Thread(target=downloadconfig,args=(self,each_ip,))
            thread.start()
        
####################################################################
#功能：sftp上传
####################################################################
def sftpUpload(self,ip='192.168.1.101',uploadfilepath='',remotepath='/opt/'):
    global update_file_name
    try:
        filepath = os.path.dirname(uploadfilepath)
        update_file_name = uploadfilepath[len(filepath)+1:]
        t = paramiko.Transport((ip,22))
        t.connect(username = "root", password = "hiklinux")
        sftp = paramiko.SFTPClient.from_transport(t)
        sftp.put(uploadfilepath,remotepath + update_file_name)
        t.close()
        self.textInfoBrowser.append('[%s] %s 上传升级包成功'%(time.strftime("%Y-%m-%d %H:%M:%S",time.localtime(time.time())),ip)) 
    except Exception as e:
        self.textInfoBrowser.append('[%s] %s 上传升级包失败:%s'%(time.strftime("%Y-%m-%d %H:%M:%S",time.localtime(time.time())),ip,e)) 

####################################################################
#功能：ssh登录后执行升级脚本(支持不重启升级)
#备注：无法直接启动主控程序（无法定位原因，跟paramiko库有关，本次2.0版本已经修复解决:）)
####################################################################
def ssh2(self,ip):
    global update_file_name, is_reboot_now, sleep_time
    try: 
        username = 'root'
        passwd = 'hiklinux'
          
        ssh = paramiko.SSHClient()
        ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        ssh.connect(ip,22,username,passwd,timeout=5)
        
        #判断是300升级包还是500升级包
        if update_file_name.find('300') != -1:
            hiktscname = 'hikTSC300'
        else:
            hiktscname = 'hikTSC500'
        
        #如果是直接升级，则手动直行升级包，支持TSC500和TSC300型号互相升级（配置工具不支持）
        if is_reboot_now == True or sleep_time == 0:
            cmd = 'cd /opt &&tar -zxvf ' + update_file_name + ' && cd ' + update_file_name.replace('.tar.gz','  ') + \
                '&&killall -9 hikTSC500;killall -9 hikTSC300;killall -9 hikTSC;'+' sh update.sh && rm -f /opt/' + update_file_name + ' && nohup /root/' + hiktscname + ' >/dev/null 2>&1 &'         
            stdin, stdout, stderr = ssh.exec_command(cmd) 
            out = stdout.readlines()
            self.textInfoBrowser.append('[%s] %s 升级成功\n'%(time.strftime("%Y-%m-%d %H:%M:%S",time.localtime(time.time())),ip))
        else:
            stdin, stdout, stderr = ssh.exec_command('sleep ' + str((int)(sleep_time*3600)) + '&&' + 'cd /opt &&tar -zxvf ' + update_file_name + ' && cd ' + update_file_name.replace('.tar.gz','  ') + \
                '&&killall -9 hikTSC500;killall -9 hikTSC300;killall -9 hikTSC;'+' sh update.sh && rm -f /opt/' + update_file_name + ' && nohup /root/' + hiktscname + ' >/dev/null 2>&1 &') 
            out = stdout.readlines()
            self.textInfoBrowser.append('[%s] %s 升级成功，将在%0.3f小时运行升级后程序\n'%(time.strftime("%Y-%m-%d %H:%M:%S",time.localtime(time.time())),ip,sleep_time))
        #关闭ssh连接
        ssh.close()
    except Exception as e:
        self.textInfoBrowser.append('[%s] %s 升级失败:%s'%(time.strftime("%Y-%m-%d %H:%M:%S",time.localtime(time.time())),ip,e))

####################################################################
#功能：从信号机内下载/home下所有文件并压缩
####################################################################
def downloadconfig(self,ip):
    global savedirectory
    try: 
        username = 'root'
        passwd = 'hiklinux'
        #解压升级包并执行升级脚本，然后重启
        cmd = 'tar -zcvf /opt/' + ip +'_home.tar.gz /home  /etc/network/interfaces&'
        ssh = paramiko.SSHClient()
        ssh.set_missing_host_key_policy(paramiko.AutoAddPolicy())
        ssh.connect(ip,22,username,passwd,timeout=5)
        stdin, stdout, stderr = ssh.exec_command(cmd) 
        out = stdout.readlines()
        ssh.close()
        #print ('%s /home目录压缩成功\n'%(ip))
        ftp=FTP()
        ftp.set_debuglevel(2)    #打开调试级别2，显示详细信息  
        ftp.connect(ip,21)                
        ftp.login('root','hiklinux')   #you can not see me
        bufsize = 1024
        fp = open(savedirectory + '/'+ ip + '_home.tar.gz','wb')
        ftp.retrbinary('RETR ' + '/opt/'+ ip +'_home.tar.gz',fp.write,bufsize) #接收服务器上文件并写入本地文件  
        ftp.set_debuglevel(0)                                           
        fp.close()                                                      
        ftp.quit() 
        self.textInfoBrowser.append('[%s] %s_home.tar.gz获取成功'%(time.strftime("%Y-%m-%d %H:%M:%S",time.localtime(time.time())),ip))
    except Exception as e:
        self.textInfoBrowser.append('[%s] %s_home.tar.gz获取失败:%s'%(time.strftime("%Y-%m-%d %H:%M:%S",time.localtime(time.time())),ip,e))


####################################################################
#功能：升级线程入口函数，先ftp上传升级包，再通过SSH进行升级重启
####################################################################
def upload(self,ip):
    global update_file_allpath    
    sftpUpload(self,ip,update_file_allpath,remotepath='/opt/')     #sftp上传
    ssh2(self,ip) 


####################################################################
#功能：从ipconfig.ini文件中获取需要升级的信号机IP
####################################################################
def loadconfig():
    if os.path.isfile("./ipconfig.ini") == False:
        configfile = open ("./ipconfig.ini",'a+')     
    else:
        configfile = open ("./ipconfig.ini")
    for line in configfile:
        line = line.replace("\n","")
        if bool(line) == True:           
            tsc_ip.append(line)
        #print (tsc_ip)
    configfile.close()


class mywindow(QtWidgets.QWidget,Ui_updateForm):  
    def __init__(self):  
        super(mywindow,self).__init__()  
        self.setupUi(self)  
  
if __name__=="__main__":  
    import sys  
  
    app=QtWidgets.QApplication(sys.argv)  
    myshow=mywindow()  
    myshow.show()  
    sys.exit(app.exec_())  


#(Now, only God knows)

