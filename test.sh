#!/bin/bash

# Generate golden
# for f in local/lors/raw_filtered/*.txt; do base=`basename $f .txt`; ./dijcli process $f -a 16 > ../jpet-online-compress/res/golden/"$base".dij; done
# Generate inputs
# for f in local/lors/raw/*.txt; do base=`basename $f .txt`; ./dijcli process $f -a 16 > ../jpet-online-compress/res/in/"$base".dij; done

rm local/run.log;
for f in res/in/*.dij; do
	sanitf=`basename $f .dij`;
	echo Compressing $f;
	solution1/csim/build/csim.exe compress -f $f -o local/out/$sanitf.ansu
	lib/ansu/ansu_cli decompress local/out/$sanitf.ansu local/out/$sanitf.vout
	diff res/golden/$sanitf.dij local/out/$sanitf.vout >> local/run.log
done
bat local/run.log
