<project xmlns="com.autoesl.autopilot.project" name="jpet-online-compress" top="fpga_compress">
    <includePaths/>
    <libraryPaths/>
    <Simulation argv="compress -f ../../../res/all.dij -o  ../../../out/all.ansu -s human">
        <SimFlow name="csim" setup="true" ldflags="" clean="true" csimMode="0" lastCsimMode="0"/>
    </Simulation>
    <files xmlns="">
        <file name="../lib/ansu/src/util.cpp" sc="0" tb="1" cflags=" -I../lib/ansu/include  -I../source  -I../lib/ansu/cli/lib/CLI11/include  -I../lib/ansu/lib/cereal/include  -Wno-unknown-pragmas" csimflags=" -Wno-unknown-pragmas" blackbox="false"/>
        <file name="../testbench/test.cpp" sc="0" tb="1" cflags=" -I../lib/ansu/include  -I../source  -I../lib/ansu/cli/lib/CLI11/include  -I../lib/ansu/lib/cereal/include  -Wno-unknown-pragmas" csimflags=" -Wno-unknown-pragmas" blackbox="false"/>
        <file name="../testbench/driver.hpp" sc="0" tb="1" cflags=" -I../lib/ansu/include  -I../source  -I../lib/ansu/cli/lib/CLI11/include  -I../lib/ansu/lib/cereal/include  -Wno-unknown-pragmas" csimflags=" -Wno-unknown-pragmas" blackbox="false"/>
        <file name="../lib/ansu/src/common/io/archive.cpp" sc="0" tb="1" cflags=" -I../lib/ansu/include  -I../source  -I../lib/ansu/cli/lib/CLI11/include  -I../lib/ansu/lib/cereal/include  -Wno-unknown-pragmas" csimflags=" -Wno-unknown-pragmas" blackbox="false"/>
        <file name="jpet-online-compress/source/ansu_fpga.cpp" sc="0" tb="false" cflags="-Ijpet-online-compress/lib/ansu/include -Ijpet-online-compress/lib/ansu/include -Wl,--no-undefined" csimflags="" blackbox="false"/>
        <file name="jpet-online-compress/source/ansu_fpga.hpp" sc="0" tb="false" cflags="-Ijpet-online-compress/lib/ansu/include -Wl,--no-undefined" csimflags="" blackbox="false"/>
    </files>
    <solutions xmlns="">
        <solution name="solution1" status="active"/>
    </solutions>
</project>

