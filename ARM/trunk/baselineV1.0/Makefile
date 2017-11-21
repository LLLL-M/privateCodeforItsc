all: parse_ini hikconfig logsystem tsc500 tsc300 tools done

MAKEFLAGS=s

parse_ini:
	make -C libs/$@
hikconfig:
	make -C libs/$@

logsystem:
	make -C libs/$@

tsc500:
	make -C $@
	echo "$@ is done ."
tsc300:
	make -C $@
	echo "$@ is done ."
tools:
	make -C $@ clean;make -C $@
done:
	echo "all is done ."
.PHONY: parse_ini hikconfig logsystem tsc500 tsc300 tools
