all: libits tsc500 tsc300
#cp tsc500/hikTSC500 tsc300/hikTSC300 /nfs

libits:
	make -C $@ clean;make -C $@

tsc500:
	make -C $@ clean;make -C $@
	cp tsc500/hikTSC500 /nfs

tsc300:
	make -C $@ clean;make -C $@
	cp tsc300/hikTSC300 /nfs

.PHONY: libits tsc500 tsc300
