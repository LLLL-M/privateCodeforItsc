#!/bin/sh

TSC300=hiktsc300-update
TSC500=hiktsc-update
DATE=`date +%Y年%m月%d日-%H:%M:%S`

#将tsc300及tsc500目录下面的最新hikTSC拷贝到对应升级包目录中
CopyTheLatestVersion()
{
	if [ -f $1 ]	
	then
		echo "Copy $1 to $2 ."
		cp $1 $2
	else
		echo "Please make sure the $2hikTSC is the latest one "
	fi
}

#将BinaryTextConvert拷贝到各自的升级包中
CopyTools()
{
	echo "copy BinaryTextConvert to packages ."
	cp ../tools/BinaryTextConvert ./hiktsc300-update/
	cp ../tools/BinaryTextConvert ./hiktsc-update/	
}


#制作压缩包
CompressUpdatePackage()
{
	local packageName="$1".tar.gz

	if [ -f $packageName ]
	then
		rm -f $packageName
	fi
#先删除再添加
	rm -f $1/VERSION.txt
	echo $DATE > $1/VERSION.txt
	svn log -l 5 ../ >> $1/VERSION.txt	
	
	tar zcvf $packageName $1/* >> /dev/null

	echo "Compress $packageName ."
}

#制作最新的hikTSC
MakeHikTSCLatest()
{
	echo "we will make the latest version"
	cd ../
	make -s
	cd update/
}

MakeHikTSCLatest

CopyTheLatestVersion "../tsc300/hikTSC" "$TSC300/"
CopyTheLatestVersion "../tsc500/hikTSC" "$TSC500/"
CopyTools

rm -f *.tar.gz

CompressUpdatePackage $TSC300
CompressUpdatePackage $TSC500



echo "Done ."

