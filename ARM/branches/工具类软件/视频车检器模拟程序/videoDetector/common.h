#ifndef COMMON_H
#define COMMON_H

#include <QMessageBox>

#define C_LANGUAGE_CN	1
#define C_LANGUAGE_EN	2

#define LANGUAGE_CHINESE	"translate_CN.qm"
#define LANGUAGE_ENGLISH	"translate_EN.qm"

#define CONF_FILE_NAME	"VDCONF.dat"

#define ERR_MSG(parent,str) QMessageBox::information(parent, tr("ERROR"), str);

#endif // COMMON_H
